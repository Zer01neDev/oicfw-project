/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	csoread.c: SystemControl csoread Code
	This file are based from a reverse of M33/GEN SystemControl
	Much part of this code are taken from OE/Wildcard Source Code

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <pspsdk.h>
#include <pspkernel.h>
#include "malloc.h"

#include "csoread.h"
#include "umd9660_driver.h"

extern int sceKernelDeflateDecompress(void *dst,int dsize,void *src,int *pparam);

#define DEV_NAME "CSO"

// log output switch
#define OUT_LOG_ISO 0

// thread priority in read
// when READ_PRIORITY >= 48 then OutRun2006 are not work.
//#define READ_PRIORITY 47

// short sleep around read/decompress for higher thread switching
//#define SHORT_SLEEP

// index buffer size
#define CISO_INDEX_SIZE (512/4)

// compresed data buffer cache size
#define CISO_BUF_SIZE 0x2000


#ifdef SHORT_SLEEP
#define SWITCH_THREAD() sceKernelDelayThread(1)
#else
#define SWITCH_THREAD() {}
#endif

/****************************************************************************
****************************************************************************/

//#ifdef CISO_BUF_SIZE
//static unsigned char ciso_data_buf[CISO_BUF_SIZE] __attribute__((aligned(64)));
//#else
//static unsigned char ciso_data_buf[0x800] __attribute__((aligned(64)));
//#endif
static unsigned char *ciso_data_buf = NULL; //0x0000A340

static unsigned int ciso_index_buf[CISO_INDEX_SIZE] __attribute__((aligned(64)));//0x0000A380

static unsigned int ciso_buf_pos;    // file poisiotn of the top of ciso_data_buf //0x0000A580
static unsigned int ciso_cur_index;  // index(LBA) number of the top of ciso_index_buf //0x0000A584

// header buffer
static CISO_H ciso; //0x0000A588
// ciso.block_size = 0x0000A598
// ciso.total_bytes = 0x0000A590
/****************************************************************************
	Mount UMD event callback
****************************************************************************/
static int max_sectors;//0x0000A5A0

// 0x00005748
int CisoOpen(int umdfd)
{
	int ret;
	ciso.magic = 0;
	ciso_buf_pos = 0x7FFFFFFF;
	sceIoLseek(umdfd, 0, PSP_SEEK_SET);
	result = sceIoRead(umdfd, &ciso, sizeof(ciso));
	ret = result;

	if (result >= 0)
	{
		// header check error
		ret = SCE_KERNEL_ERROR_NOFILE;

		// check CISO header
		if (ciso.magic == 0x4F534943)
		{
			ciso_cur_index = 0xFFFFFFFF;
			max_sectors = (ciso.total_bytes / ciso.block_size);
			if (ciso_data_buf != NULL)
			{
				ret = 0;
			}
			else
			{
				ciso_data_buf = (unsigned char *)oe_malloc(8256);//0x2040
				ret = -1;

				if (ciso_data_buf)
				{
					ret = 0;
					if ((ciso_data_buf & 0x3F) == 0)
						ret = 0;

					ciso_data_buf = ((ciso_data_buf & 0xFFFFFFC0) + 64);
				}
			}
		}
	}
	return ret;
}

/****************************************************************************
	get file pointer in sector
****************************************************************************/
//0x0000550C
static int inline ciso_get_index(u32 sector, int *pindex)
{
	int ret;
	int result;
	int index_off;

	// search index
	index_off = sector - ciso_cur_index;
	if (ciso_cur_index == 0xFFFFFFFF)
	{
		label5:
		result = ReadUmdFileRetry(ciso_index_buf, sizeof(ciso_index_buf), sizeof(ciso)+sector*4);
		ret = result;
		if (result >= 0)
		{
			ciso_cur_index = sector;
			index_off = 0;

			label9:
			ret = 0;

			// get file posision and sector size
			*pindex = ciso_index_buf[index_off];
		}
	}
	else
	{
		if (index_off < 0)
			goto label5;
		if (((index_off < 0x00000080)) != 0x00000000)
			goto label9;

		goto label5;
	}
	return ret;
}

/****************************************************************************
	Read one sector
****************************************************************************/
static int ciso_read_one(void *buf, int sector)
{
	int result;
	int index,index2;
	int dpos,dsize;

	// get current index
	result = ciso_get_index(sector, &index);
	if(result < 0) 
	{
		return result;
	}

	// get file posision and sector size
	dpos = (index & 0x7fffffff) << ciso.align;//0x0000A59D

	if(index & 0x80000000)
	{
		// plain sector
#ifdef SHORT_SLEEP
	sceKernelDelayThread(1);
#endif
		//result = dhReadFileRetry(&ciso_fd,dpos,buf,0x800);
	result = ReadUmdFileRetry(buf, 0x800, dpos);

#ifdef SHORT_SLEEP
	sceKernelDelayThread(1);
#endif
		return result;
	}

	// compressed sector

	// get sectoer size from next index
	result = ciso_get_index(sector+1,&index2);
	if(result < 0) return result;
	
	dsize = ((index2 & 0x7fffffff) << ciso.align) - dpos;
	
	// adjust to maximum size for scramble(shared) sector index
	if((dsize <= 0) || (dsize > 0x800)) dsize = 0x800;

#ifdef CISO_BUF_SIZE
	SWITCH_THREAD();
	// read sector buffer
	if( (dpos < ciso_buf_pos) || ( (dpos+dsize) > (ciso_buf_pos+CISO_BUF_SIZE))  )
	{
		// seek & read
		//result = dhReadFileRetry(&ciso_fd,dpos,ciso_data_buf,CISO_BUF_SIZE);
		result = ReadUmdFileRetry(ciso_data_buf, CISO_BUF_SIZE, dpos);
		SWITCH_THREAD();
		if(result<0)
		{
			ciso_buf_pos = 0xfff00000; // set invalid position
			return result;
		}
		ciso_buf_pos = dpos;
	}
	result = sceKernelDeflateDecompress(buf, 0x800, ciso_data_buf + dpos - ciso_buf_pos, NULL);
	SWITCH_THREAD();

#else
	// seek
	// read compressed data
	SWITCH_THREAD();
	//result = dhReadFileRetry(&ciso_fd,dpos,ciso_data_buf,dsize);
	result = ReadUmdFileRetry(ciso_data_buf, dsize, dpos);
	SWITCH_THREAD();
	if(result < 0) return result;

	result = sceKernelDeflateDecompress(buf, 0x800, ciso_data_buf, NULL);
	SWITCH_THREAD();

#endif

	if(result < 0) return result;

	return 0x800;
}

/****************************************************************************
	Read Request
****************************************************************************/
//0x000055AC
int CisofileReadSectors(int lba, int nsectors, void *buf)
{
	int result;
	int i;

	int num_bytes = nsectors * SECTOR_SIZE;

#ifdef READ_PRIORITY
	int cur_prio = sceKernelGetThreadCurrentPriority();
	sceKernelChangeThreadPriority(0, READ_PRIORITY);
#endif
	for(i = 0; i < num_bytes; i+=0x800)
	{
		result = ciso_read_one(buf, lba);
		if(result < 0)
		{
			nsectors = result;
			break;
		}
		buf += 0x800;
		lba++;
	}
#ifdef READ_PRIORITY
	sceKernelChangeThreadPriority(0, cur_prio);
#endif

	return nsectors;
}

/****************************************************************************
****************************************************************************/
//0x000054F0
int CisofileGetDiscSize(int umdfd)
{
	return (int)(ciso.total_bytes) / ciso.block_size;
}


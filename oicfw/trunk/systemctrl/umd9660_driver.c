/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	umd9660_driver.c: SystemControl UMD9660 Driver Code
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
#include <pspthreadman_kernel.h>
#include <psperror.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "malloc.h"
#include "umd9660_driver.h"
#include "isoread.h"
#include "csoread.h"
#include "main.h"


int discout; //0x0000A220
int disctype;// 0x0000A21C
SceUID umdfd; //0x0000A208
int umd_open; //0x0000A304
u8 *sectorbuf; //0x0000A2FC
char umdfilename[128]; //0x0000AC3C
SceOff umd_cur_offset; // 0x0000AC20
SceUID umd_sema; //0x0000A204
SceInt64 umd_file_len; //0x0000A218
int umd_is_cso; //0x0000A300
int umd_cur_sector = -1; //0x0000A224


// OK
void sctrlSESetDiscType(int type)
{
	int k1 = pspSdkSetK1(0);
	disctype = type;
	pspSdkSetK1(k1);
}

// OK
// 0x0000525C
void sctrlSESetDiscOut(int out)
{
	int k1 = pspSdkSetK1(0);
	PatchLowIO();
	ClearCaches();
	discout = out;
	sceKernelCallSubIntrHandler(4, 0x1A, 0, 0);
	sceKernelDelayThread(0x0000C350);
	pspSdkSetK1(k1);
}

// OK
void sctrlSESetUmdFile(char *file)
{
	strncpy(umdfilename, file, 0x80);
	umdfilename[127] = 0;
	sceIoClose(umdfd);
	umd_open = 0;
	umdfd = -1;
}

// OK
// 0x00004B60
void SetUmdFile(char *file)
{
	strncpy(umdfilename, file, 0x00000047);
	sceIoClose(umdfd);
	umd_open = 0;
	umdfd = -1;
}

// OK
//0x000047FC
char *GetUmdFile()
{
	return umdfilename;
}

//0x00004BAC
int GetIsoDiscSize()
{
	if(umd_is_cso == 0)
	{
		return IsofileGetDiscSize(umdfd);
	}
	else
	{
		return CisofileGetDiscSize(umdfd);
	}
}

//0x00004BCC
int OpenIso()
{
	sceIoClose(umdfd);

	umd_open = 0;

	if((umdfd = sceIoOpen(umdfilename, PSP_O_RDONLY | 0xF0000, 0777)) < 0)
	{
		return -1;
	}

	umd_is_cso = 0;
	if(CisoOpen(umdfd) >= 0)
	{
		umd_is_cso = 1;
	}

	umd_file_len = GetIsoDiscSize();
	umd_cur_sector = -1;
	umd_open = 1;

	return 0;
}

//0x00004D24
int ReadUmdFileRetry(void *buf, int size, int fpointer)
{
	int i, read;

	//4b5c
	for(i = 0; i < 16; i++)
	{
		if(sceIoLseek32(umdfd, fpointer, PSP_SEEK_SET) >= 0)
		{
			for(i = 16; i > 0; i--)
			{
				if((read = sceIoRead(umdfd, buf, size)) >= 0)
				{
					return read;
				}
				OpenIso();
			}
			return 0x80010013;
		}
		OpenIso();
	}

	return 0x80010013;
}

// 0x00004AD8
int umd9660_init()
{
	if (sectorbuf != 0)
	{
		label6:
		ret = 0;
		if (!(umd_sema >= 0))
		{
			umd_sema = sceKernelCreateSema("EcsUmd9660DeviceFile", 0, 1, 1, 0);//0x00007A24
			ret = (umd_sema < 0) ? umd_sema : 0;
		}
	}
	else
	{
		sectorbuf = oe_malloc(SECTOR_SIZE);
		ret = 0xFFFFFFFF;
		if (sectorbuf != 0)
			goto label6;
	}
	return ret;
}

//0x00004A58
int umd9660_exit(PspIoDrvArg* arg)
{
	sceKernelWaitSema(umd_sema, 1, 0);

	if(sectorbuf != 0)
	{
		oe_free(sectorbuf);
		sectorbuf = 0;
	}

	if(umd_sema >= 0)
	{
		sceKernelDeleteSema(umd_sema);
		umd_sema = -1;
	}
	
	sceIoClose(umdfd);

	return 0;
}

//0x00004C78
int umd9660_open(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode)
{
	sceKernelWaitSema(umd_sema, 1, 0);

	//00004f50
	int i;
	for(i = 0; i < 16; i++)
	{
		if(sceIoLseek32(umdfd, 0, PSP_SEEK_SET) >= 0)
		{
			//00004dac
			arg->arg = 0;
			umd_cur_offset = 0;
			sceKernelSignalSema(umd_sema, 1);
			return 0;
		}
		OpenIso();
	}

	sceKernelSignalSema(umd_sema, 1);

	return 0x80010013;
}

// OK
int umd9660_close()
{
	sceKernelWaitSema(umd_sema, 1, 0);
	sceKernelSignalSema(umd_sema, 1);
	return 0;
}

typedef struct
{
	int lba;//0x0000ABF8
	int nsectors;//0x0000ABFC
	void *buf;//0x0000AC00
	int *eod;
} UmdReadParams;

UmdReadParams umd_read_params;//8b8c

//0x0000503C
int Umd9660ReadSectors3()
{
	if(umd_open == 0)
	{
		int i;
		for(i = 16; i; i--)
		{
			if(sceIoLseek32(umdfd, 0, PSP_SEEK_CUR) >= 0)
			{
				break;
			}
			OpenIso();
		}

		if(umd_open == 0)
		{
			return 0x80010013;
		}
	}

	if(umd_is_cso == 0)
	{
		return IsofileReadSectors(umd_read_params.lba, umd_read_params.nsectors, umd_read_params.buf);
	}
	else
	{
		return CisofileReadSectors(umd_read_params.lba, umd_read_params.nsectors, umd_read_params.buf);
	}
}

//0x0000490C
int Umd9660ReadSectors(int lba, int nsectors, void *buf)
{
	umd_read_params.lba = lba;
	umd_read_params.nsectors = nsectors;
	umd_read_params.buf = buf;
	return sceKernelExtendKernelStack(0x2000, (void *)Umd9660ReadSectors3, 0);
}

//0x00004934
int umd9660_read(PspIoDrvFileArg *arg, char *data, int len)
{
	sceKernelWaitSema(umd_sema, 1, 0);

	int i = len;
	if(umd_file_len < umd_cur_offset + len) //len >= pos + a2
	{
		i = umd_file_len - umd_cur_offset; //len - pos
	}

	int ret = Umd9660ReadSectors(umd_cur_offset, i, data);
	if (ret > 0)
	{
		umd_cur_offset = (umd_cur_offset + ret);
	}

	sceKernelSignalSema(umd_sema, 1);
	return ret;
}

//0x000047A0
SceOff umd9660_lseek(PspIoDrvFileArg *arg, SceOff offset, int whence)
{
	sceKernelWaitSema(umd_sema, 1, 0);

	if(whence == PSP_SEEK_SET)
	{
		umd_cur_offset = offset;
	}
	else if(whence == PSP_SEEK_CUR)
	{
		umd_cur_offset += offset;
	}
	else if(whence == PSP_SEEK_END)
	{
		umd_cur_offset = umd_file_len - offset;
	}
	else
	{
		sceKernelSignalSema(umd_sema, 1);
		return 0x80010016;
	}

	//00004ef8
	umd_cur_offset = (umd_cur_offset < umd_file_len) ? umd_cur_offset : umd_file_len;
	sceKernelSignalSema(umd_sema, 1);
	return umd_cur_offset;
}

//0x00004720
int umd9660_ioctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	sceKernelWaitSema(umd_sema, 1, 0);

	if(cmd == 0x1D20001)
	{
		*((SceOff *)outdata) = umd_cur_offset;
		sceKernelSignalSema(umd_sema, 1);
		return 0;
		//->5018
	}
	else
	{
		sceKernelSignalSema(umd_sema, 1);
		return -1;
	}
}

//0x00004be4
int sub_00004BE4(int lba)
{
	if(umd_cur_sector == lba)
	{
		return 0;
	}

	int ret = Umd9660ReadSectors(lba, 1, sectorbuf, 0);
	if(ret < 0)
	{
		return ret;
	}  

	umd_cur_sector = lba;
	return 0;
}

/******************************************************************************

	UMD access RAW routine

	lba_param[0] = 0 , unknown
	lba_param[1] = cmd,3 = ctrl-area , 0 = data-read
	lba_param[2] = top of LBA
	lba_param[3] = total LBA size
	lba_param[4] = total byte size
	lba_param[5] = byte size of center LBA
	lba_param[6] = byte size of start  LBA
	lba_param[7] = byte size of last   LBA

******************************************************************************/

typedef struct
{
	int unknown1;
	int cmd;
	int lba_top;
	int lba_size;
	int byte_size_total;
	int byte_size_centre;
	int byte_size_start;
	int byte_size_last;
} LbaParams;

//0x00005040
int sub_00005040(void *indata, void *outdata, int outlen)
{
  int boffs, lba, bsize, ret;
  LbaParams *params = (LbaParams *)indata;

  u8 *buf = (u8 *)outdata;

  int size = params->byte_size_total; //s2 = v1[0x10 / 4];

  if(params->byte_size_start && (params->byte_size_centre || params->byte_size_last))
  {
    boffs = 0x800 - params->byte_size_start;
  }
  else
  {
    boffs = params->byte_size_start;
  }

  lba = params->lba_top;

  if(boffs > 0)
  {
    bsize = 0x800 - boffs;
    if(bsize > size) bsize = size;

    if(bsize > 0)
    {
      ret = sub_00004BE4(lba);
      if(ret < 0)
      {
        return ret;
      }

      memcpy(buf, &sectorbuf[boffs], bsize);

      size -= bsize;
      buf += bsize;
      lba++;
    }
  }

  int burst_size = size & 0xFFFFF800;
  if(burst_size)
  {
    //5394
#if 0
    if(burst_size < 0)
    {
      //53c8
      burst_size += 0x7FF;
    }
#endif

    //539c
    ret = Umd9660ReadSectors(lba, burst_size / 0x800, buf, 0);
    if(ret < 0)
    {
      //->5310
      return ret;
    }

    lba += burst_size / 0x800;
    buf += burst_size;
    size -= burst_size;
    //->52e4
  }

  //52e4
  ret = 0;
  if(size > 0)
  {
    ret = sub_00004BE4(lba);
    if(ret >= 0)
    {
      memcpy(buf, sectorbuf, size) ;
      ret = 0;
    }
  }

  return ret;
}

//0x00004664
void umd9660_devctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	sceKernelWaitSema(umd_sema, 1, 0);
	if (cmd == 0x01F20001)
	{
		*((u32 *)outdata) = -1;
		*((u32 *)(outdata + 4)) = 0x10;
		sceKernelSignalSema(umd_sema, 1);
	}
	else
	{
		if (cmd == 0x01F20002)
		{
			*((u32 *)outdata) = umd_file_len;
			sceKernelSignalSema(umd_sema, 1);
		}
		else
		{
			if (cmd != 0x01F00003)
			{
				sceKernelSignalSema(umd_sema, 0x00000001);
			}
			else
			{
				sceKernelSignalSema(umd_sema, 1);
			}
		}
	}
	return 0;
}

//0x00005408
int umd9660_devctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int ret;

	sceKernelWaitSema(umd_sema, 1, 0);

	if(cmd == 0x1F00003)
	{
		//54ac
		sceKernelSignalSema(umd_sema, 1);
		return 0;
	}
	else if(cmd > 0x1F00003)
	{
		//54e4
		if(cmd == 0x1F20002)
		{
			//556c
			*((u32 *)outdata) = umd_file_len;
			sceKernelSignalSema(umd_sema, 1);
			return 0;
		}

		if(cmd > 0x1F20002)
		{
			//5544
			if(cmd - 0x1F200A1 >= 2)
			{
				//5494
				sceKernelDelayThread(3000000);
				while(0 == 0) {}
			}

			//5558
			if(indata != 0)
			{
				//55dc
				if(outdata == 0)
				{
				  return 0x80010016;
				}

				ret = sub_00005040(indata, outdata, outlen);
				sceKernelSignalSema(umd_sema, 1);
				return ret;
			}

			//5560
			return 0x80010016;
		}

		if(cmd != 0xFE0D0001)
		{
			//5494
			sceKernelDelayThread(3000000);
			while(0 == 0) {}
		}

		*((u32 *)outdata) = -1;
		*((u32 *)(outdata + 4)) = 0x10;

		sceKernelSignalSema(umd_sema, 1);
		return 0;
	}
	else if(cmd == 0x1E38012)
	{
		//558c
		if(outlen < 0)
		{
			outlen += 3;
		}

		memset(outdata, 0, outlen);
		*((u32 *)outdata) = 0xE0000800;
		*((u32 *)(outdata + 0x8)) = 0;
		*((u32 *)(outdata + 0x1C)) = umd_file_len;
		*((u32 *)(outdata + 0x24)) = umd_file_len;

		sceKernelSignalSema(umd_sema, 1);
		return 0;
	}
	else if(cmd == 0x1E380C0)
	{
		//5558
		if(indata != 0)
		{
			//55dc
			if(outdata == 0)
			{
				return 0x80010016;
			}

			ret = sub_00005040(indata, outdata, outlen);
			sceKernelSignalSema(umd_sema, 1);
			return ret;
		}
		else
		{
			return 0x80010016;
		}
	}
	else if(cmd == 0x1E18030)
	{
		//5530
		sceKernelSignalSema(umd_sema, 1);
		return 1;
	}

	//5494
	sceKernelDelayThread(3000000);

	//549C
	while(0 == 0)
	{}
}

PspIoDrvFuncs umd9660_driver_funcs =
{
	umd9660_init, //0x4c30
	umd9660_exit, //0x4c94
	umd9660_open, //0x4d0c
	umd9660_close, //0x4de4
	umd9660_read, //0x4e24
	0,
	umd9660_lseek, //0x4eb4
	umd9660_ioctl, //0x4fbc
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	umd9660_devctl, //0x5408
	0
};

PspIoDrv umd9660_driver = { "umd", 4, 0x800, "UMD9660", &umd9660_driver_funcs };

//0x00005188
PspIoDrv *getumd9660_driver()
{
	return &umd9660_driver;
}

//0x00004DE0
int Umd9660ReadSectors2(int lba, int nsectors, void *buf)
{
	if(umd_open == 0)
	{
		int i;
		for(i = 16; i > 0; i++)
		{
			if(sceIoLseek32(umdfd, 0, PSP_SEEK_CUR) >= 0)
			{
				break;
			}
			OpenIso();
		}

		if(umd_open == 0)
		{
			return 0x80010013;
		}
	}

	if(umd_is_cso == 0)
	{
		return IsofileReadSectors(lba, nsectors, buf);
	}
	else
	{
		return CisofileReadSectors(lba, nsectors, buf);
	}
}

//0x00004ED0
int UmdRead_secbuf(int lba)
{
	if(umd_cur_sector == lba)
	{
		return 0;
	}

	int ret = Umd9660ReadSectors2(lba, 1, sectorbuf);
	if(ret < 0)
	{
		return ret;
	}

	return 0;
}

typedef struct
{
  void *drvState;
  u8 *buf;
  SceInt64 read_size;
  LbaParams *lbaparams;
} UmdUnknownParam1;

//0x00004F10
int umd_read_block2(void *a0)
{
	int boffs, lba, bsize, ret;

	UmdUnknownParam1 *params = (UmdUnknownParam1 *)a0;

	u8 *buf = params->buf;

	int size = params->lbaparams->byte_size_total;

	if(params->lbaparams->byte_size_start && (params->lbaparams->byte_size_centre || params->lbaparams->byte_size_last))
	{
		boffs = 0x800 - params->lbaparams->byte_size_start;
	}
	else
	{
		boffs = params->lbaparams->byte_size_start;
	}

	lba = params->lbaparams->lba_top;

	if(boffs > 0)
	{
		bsize = 0x800 - boffs;
		if(bsize > size) bsize = size;

		if(bsize > 0)
		{
			ret = UmdRead_secbuf(lba);
			if(ret < 0)
			{
				return ret;
			}

			memcpy(buf, &sectorbuf[boffs], bsize);

			size -= bsize;
			buf += bsize;
			lba++;
		}
	}

	int burst_size = size & 0xFFFFF800;
	if(burst_size)
	{
#if 0
		if(burst_size < 0)
		{
			burst_size += 0x7FF;
		}
#endif
		ret = Umd9660ReadSectors2(lba, burst_size/0x800, buf);
		if(ret < 0)
		{
			return ret;
		}

		lba += burst_size / 0x800;
		buf += burst_size;
		size -= burst_size;
	}

	ret = 0;
	if(size > 0)
	{
		ret = UmdRead_secbuf(lba);
		if(ret >= 0)
		{
			memcpy(buf, sectorbuf, size);
			ret = 0;
		}
	}

	return ret;
}

//0x000048D4
int umd_read_block(void *drvState, u8 *buf, SceInt64 read_size, LbaParams *lba_param)
{
	UmdUnknownParam1 params;
	params.drvState = drvState;
	params.buf = buf;
	params.read_size = read_size;
	params.lbaparams = lba_param;
	return sceKernelExtendKernelStack(0x2000, (void *)umd_read_block2, &params);
}

// OK
void *sceUmdManGetUmdDiscInfo_Patched()
{
	void *(*sceUmdManGetUmdDiscInfo)(void) = (void*)sctrlHENFindFunction("sceUmdMan_driver", "sceUmdMan_driver", 0xB5A7A399);
	u32 *info = (u32 *)sceUmdManGetUmdDiscInfo();
	sceKernelWaitSema(umd_sema, 1, 0);
	sceKernelSignalSema(umd_sema, 1);

	if(umd_file_len >= 0)
	{
		//patch the returned disc info
		info[0x64 / 4] = 0xE0000800;
		info[0x68 / 4] = 0;
		info[0x6C / 4] = 0;
		info[0x70 / 4] = umd_file_len;
		info[0x74 / 4] = umd_file_len;
		info[0x80 / 4] = 1;
		info[0x84 / 4] = 1;
	}

	return info;
}

//0x00005238
int sceUmdManGetUmdDiscInfo_Dummy(void *a0)
{
	u32 *info = (u32 *)a0;
	info[0x4 / 4] = 0x10;
	return 0;
}

u8 umd_dummy_id[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

extern int sceKernelSetQTGP3(u8 *);
extern int sceKernelCallSubIntrHandler(int, int, int, int);

// 0x00004594
int sceGpioPortReadPatched()
{
	int GPRValue = *((int *) 0xBE240004);
	if (discout != 0)
	{
		GPRValue = GPRValue & 0xFBFFFFFF;
	}
	return GPRValue;
}

int (*sceGpioPortRead)();
u32 GpioPortRead[4]; // 0 = 0x0000AC18, 1 = 0x0000AC1C, 2 = 0x0000AC34, 3 = 0x0000AC38

// 0x000051D8
void PatchLowIO()
{
	sceGpioPortRead = (void*)sctrlHENFindFunction("sceLowIO_Driver", "sceGpio_driver", 0x4250D44A);
	GpioPortRead[0] = sceGpioPortRead;//0x0000AC08
	GpioPortRead[1] = (sceGpioPortRead+4);//0x0000AC24
	GpioPortRead[2] = sceGpioPortRead[0];
	GpioPortRead[3] = sceGpioPortRead[1];
	REDIRECT_FUNCTION(sceGpioPortRead, sceGpioPortReadPatched);
}

void DoAnyUmd()
{
	SceModule2 *mod;
	umd9660_init();
	SysMemForKernel_A7188E84 (0x00007918);
	mod = sceKernelFindModuleByName("sceUmd9660_driver");
	if(mod)
	{
		*((int *) 0x0000AC08) = _lw(mod->text_addr+0x1F58);
		*((int *) 0x0000AC24) = _lw(mod->text_addr+0x1F58);
		MAKE_CALL(mod->text_addr+0x1F58, umd_read_block);

		*((int *) 0x0000AC0C) = _lw(mod->text_addr+0x6A5C);
		*((int *) 0x0000AC28) = _lw(mod->text_addr+0x6A5C);
		MAKE_JUMP(mod->text_addr+0x6A5C, sceUmdManGetUmdDiscInfo_Patched);

		mod = sceKernelFindModuleByName("sceUmdMan_driver");

		if (mod)
		{
			*((int *) 0x0000AC10) = _lw(mod->text_addr+0xC018);
			*((int *) 0x0000AC2C) = _lw(mod->text_addr+0xC018);
			_sw(0x24040000 | disctype, mod->text_addr+0xC018);

			*((int *) 0x0000AC14) = _lw(mod->text_addr+0xC088);
			*((int *) 0x0000AC30) = _lw(mod->text_addr+0xC088);
			_sw(0x24040000 | disctype, mod->text_addr+0xC088);

			if (GpioPortRead[0] == 0)
			{
				PatchLowIO();
			}
			discout = 0;
			ClearCaches();
			sceKernelCallSubIntrHandler(4, 0x1A, 0, 0);
			sceKernelDelayThread(0x0000C350);
		}
	}
}

void DoAnyUmd()
{
	umd9660_init(0);

	//set dummy UMD id
	//SysMemForKernel_C7E57B9C = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xC7E57B9C);
	sceKernelSetQTGP3(umd_dummy_id);

	u32 *mod = (u32 *)sceKernelFindModuleByName("sceUmd9660_driver");

	if(mod == NULL)
	{
		return;
	}

	u32 text_addr = *(mod+0x6C/4);

	MAKE_JUMP(text_addr+0x1FE8, umd_read_block);
	MAKE_CALL(text_addr+0x5F74, sceUmdManGetUmdDiscInfo_Patched);
	u32 sceUmdManGetDiscInfo = sctrlHENFindFunction("sceUmdMan_driver", "sceUmdMan_driver", 0x8609D1E4);
	REDIRECT_FUNCTION(sceUmdManGetDiscInfo, sceUmdManGetUmdDiscInfo_Dummy);
	ClearCaches();
	sceKernelCallSubIntrHandler(4, 0x1A, 0, 0);
	sceKernelDelayThread(50000);
}


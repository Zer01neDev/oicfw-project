/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	virtualpbpmgr.c: VshControl Virtual PBP Manager Code
	This code are taken from OE/Wildcard Source Code

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

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "virtualpbpmgr.h"
#include "isofs_driver.h"

#define MAX_FILES 128

int g_index;
SceUID vpsema;
VirtualPbp *vpbps;
InternalState *states;

u8 virtualsfo[408] =
{
	0x00, 0x50, 0x53, 0x46, 0x01, 0x01, 0x00, 0x00,
	0x94, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 
	0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x04, 0x02, 
	0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00, 0x12, 0x00, 0x04, 0x02, 
	0x0A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x04, 0x02, 
	0x05, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x18, 0x00, 0x00, 0x00, 0x27, 0x00, 0x04, 0x04, 
	0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x20, 0x00, 0x00, 0x00, 0x36, 0x00, 0x04, 0x02,
	0x05, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x24, 0x00, 0x00, 0x00, 0x45, 0x00, 0x04, 0x04, 
	0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x2C, 0x00, 0x00, 0x00, 0x4C, 0x00, 0x04, 0x02, 
	0x40, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 
	0x30, 0x00, 0x00, 0x00, 0x42, 0x4F, 0x4F, 0x54, 
	0x41, 0x42, 0x4C, 0x45, 0x00, 0x43, 0x41, 0x54, 
	0x45, 0x47, 0x4F, 0x52, 0x59, 0x00, 0x44, 0x49, 
	0x53, 0x43, 0x5F, 0x49, 0x44, 0x00, 0x44, 0x49, 
	0x53, 0x43, 0x5F, 0x56, 0x45, 0x52, 0x53, 0x49,
	0x4F, 0x4E, 0x00, 0x50, 0x41, 0x52, 0x45, 0x4E, 
	0x54, 0x41, 0x4C, 0x5F, 0x4C, 0x45, 0x56, 0x45, 
	0x4C, 0x00, 0x50, 0x53, 0x50, 0x5F, 0x53, 0x59, 
	0x53, 0x54, 0x45, 0x4D, 0x5F, 0x56, 0x45, 0x52, 
	0x00, 0x52, 0x45, 0x47, 0x49, 0x4F, 0x4E, 0x00, 
	0x54, 0x49, 0x54, 0x4C, 0x45, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x4D, 0x47, 0x00, 0x00, 
	0x55, 0x43, 0x4A, 0x53, 0x31, 0x30, 0x30, 0x34, 
	0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x31, 0x2E, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x31, 0x2E, 0x30, 0x30, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 
	0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 
	0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 
	0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 
	0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 
	0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 
	0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

typedef struct  __attribute__((packed))
{
	u32 signature;
	u32 version;
	u32 fields_table_offs;
	u32 values_table_offs;
	int nitems;
} SFOHeader;

typedef struct __attribute__((packed))
{
	u16 field_offs;
	u8  unk;
	u8  type; // 0x2 -> string, 0x4 -> number
	u32 unk2;
	u32 unk3;
	u16 val_offs;
	u16 unk4;
} SFODir;

//0x00001D5C
void GetSFOTitle(char *title, int n, char *sfo)
{
	SFOHeader *header = (SFOHeader *)sfo;
	SFODir *entries = (SFODir *)(sfo+0x14);
	int i;

	for (i = 0; i < header->nitems; i++)
	{
		if (strcmp(sfo+header->fields_table_offs+entries[i].field_offs, "TITLE") == 0)
		{
			memset(title, 0, n);
			strncpy(title, sfo+header->values_table_offs+entries[i].val_offs, n);
		}
	}
}

//0x00001E38
int virtualpbp_init()
{
	vpbps = (VirtualPbp *)oe_malloc(MAX_FILES*sizeof(VirtualPbp));
	memset(vpbps, 0, MAX_FILES*sizeof(VirtualPbp));

	if (!vpbps)
	{
		return -1;
	}

	states = (InternalState *)oe_malloc(MAX_FILES*sizeof(InternalState));
	memset(states, 0, MAX_FILES*sizeof(InternalState));
	if (!states)
	{
		return -1;
	}

	vpsema = sceKernelCreateSema("VirtualPBPMgr", 0, 1, 1, NULL);
	if (vpsema < 0)
	{
		return vpsema;
	}

	memset(vpbps, 0, MAX_FILES*sizeof(VirtualPbp));
	g_index = 0;
	return 0;
}

//0x00001F08
int virtualpbp_exit()
{
	sceKernelWaitSema(vpsema, 1, NULL);
	oe_free(vpbps);
	oe_free(states);
	sceKernelDeleteSema(vpsema);
	return 0;
}

//0x00001F5C
int virtualpbp_reset()
{
	sceKernelWaitSema(vpsema, 1, NULL);

	memset(vpbps, 0, MAX_FILES*sizeof(VirtualPbp));
	memset(states, 0, MAX_FILES*sizeof(InternalState));
	g_index = 0;

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

//0x00001FCC
void getlba_andsize(PspIoDrvFileArg *arg, const char *file, int *lba, int *size)
{
	SceIoStat stat;

	memset(&stat, 0, sizeof(SceIoStat));
	if (isofs_getstat(arg, file, &stat) >= 0)
	{
		*lba = stat.st_private[0];
		*size = stat.st_size;
	}
}

//0x00002044
int virtualpbp_add(char *isofile, ScePspDateTime *mtime, VirtualPbp *res)
{
	int offset;
	PspIoDrvFileArg arg;

	sceKernelWaitSema(vpsema, 1, NULL);

	if (g_index >= MAX_FILES)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	memset(vpbps[g_index].isofile, 0, 128);
	strncpy(vpbps[g_index].isofile, isofile, 128);
	sctrlSESetUmdFile(isofile);

	memset(&arg, 0, sizeof(arg));

	if (isofs_init(NULL) < 0)
	{
		isofs_exit(NULL);
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	if (isofs_open(&arg, "/PSP_GAME/PARAM.SFO", PSP_O_RDONLY, 0) >= 0)
	{
		char *buf = (char *)oe_malloc(1024);

		isofs_read(&arg, buf, 1024);
		isofs_close(&arg);

		GetSFOTitle(vpbps[g_index].sfotitle, 64, buf);
		oe_free(buf);
	}
	else
	{
		isofs_exit(NULL);
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	//getlba_andsize(&arg, "/PSP_GAME/PARAM.SFO", &vpbps[g_index].psfo_lba, &vpbps[g_index].psfo_size);
	getlba_andsize(&arg, "/PSP_GAME/ICON0.PNG", &vpbps[g_index].i0png_lba, &vpbps[g_index].i0png_size);
	getlba_andsize(&arg, "/PSP_GAME/ICON1.PMF", &vpbps[g_index].i1pmf_lba, &vpbps[g_index].i1pmf_size);
	getlba_andsize(&arg, "/PSP_GAME/PIC0.PNG", &vpbps[g_index].p0png_lba, &vpbps[g_index].p0png_size);
	getlba_andsize(&arg, "/PSP_GAME/PIC1.PNG", &vpbps[g_index].p1png_lba, &vpbps[g_index].p1png_size);
	getlba_andsize(&arg, "/PSP_GAME/SND0.AT3", &vpbps[g_index].s0at3_lba, &vpbps[g_index].s0at3_size);

	isofs_exit(NULL);

	vpbps[g_index].header[0] = 0x50425000;
	vpbps[g_index].header[1] = 0x10000;

	offset = 0x28;

	/*vpbps[g_index].header[2] = offset; // SFO 
	offset += vpbps[g_index].psfo_size;*/

	vpbps[g_index].header[2] = offset; // Virtual SFO
	offset += sizeof(virtualsfo);

	vpbps[g_index].header[3] = offset; // ICON0.PNG
	offset += vpbps[g_index].i0png_size;

	vpbps[g_index].header[4] = offset; // ICON1.PMF
	offset += vpbps[g_index].i1pmf_size;

	vpbps[g_index].header[5] = offset; // PIC0.PNG
	offset += vpbps[g_index].p0png_size;

	vpbps[g_index].header[6] = offset; // PIC1.PNG
	offset += vpbps[g_index].p1png_size;

	vpbps[g_index].header[7] = offset; // SND0.AT3
	offset += vpbps[g_index].s0at3_size;

	vpbps[g_index].header[8] = offset; // DATA.PSP
	vpbps[g_index].header[9] = offset; // DATA.PSAR

	vpbps[g_index].filesize = offset;

	memcpy(&vpbps[g_index].mtime, mtime, sizeof(ScePspDateTime));

	if (res)
	{
		memcpy(res, &vpbps[g_index], sizeof(VirtualPbp));
	}

	g_index++;

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

//0x00002400
int virtualpbp_fastadd(VirtualPbp *pbp)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (g_index >= MAX_FILES)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	memcpy(&vpbps[g_index], pbp, sizeof(VirtualPbp));

	g_index++;

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

//0x000024D8
int virtualpbp_open(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index || states[i].deleted)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	vpbps[i].filepointer = 0;

	sceKernelSignalSema(vpsema, 1);
	return 0x7000+i;
}

//0x0000258C
int virtualpbp_close(SceUID fd)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	fd = fd-0x7000;

	if (fd < 0 || fd >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

//0x0x00002604
int virtualpbp_read(SceUID fd, void *data, SceSize size)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	fd = fd-0x7000;

	if (fd < 0 || fd >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sctrlSESetUmdFile(vpbps[fd].isofile);
	isofs_fastinit();

	PspIoDrvFileArg arg;
	int remaining;
	int n, read, base;
	void *p;
	char filename[32];

	memset(&arg, 0, sizeof(PspIoDrvFileArg));
	remaining = size;
	read = 0;
	p = data;

	while (remaining > 0)
	{
		if (vpbps[fd].filepointer >= 0 && vpbps[fd].filepointer < 0x28)
		{
			u8 *header = (u8 *)&vpbps[fd].header;

			n = 0x28-vpbps[fd].filepointer; 
			if (remaining < n)
			{
				n = remaining;
			}			

			memcpy(p, header+vpbps[fd].filepointer, n);
			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		/*if ((vpbps[fd].filepointer >= vpbps[fd].header[2]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[3]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[2];

			n = vpbps[fd].psfo_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].psfo_lba, vpbps[fd].psfo_size);

			isofs_open(&arg, filename, PSP_O_RDONLY, 0);
			isofs_lseek(&arg, base, PSP_SEEK_SET);
			isofs_read(&arg, p, n);
			isofs_close(&arg);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}*/

		if ((vpbps[fd].filepointer >= vpbps[fd].header[2]) && 
			(vpbps[fd].filepointer < vpbps[fd].header[3]))
		{
			memcpy(virtualsfo+0x118, vpbps[fd].sfotitle, 64);

			base = vpbps[fd].filepointer - vpbps[fd].header[2];

			n = sizeof(virtualsfo)-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			memcpy(p, virtualsfo+base, n);
			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[3]) && 
			(vpbps[fd].filepointer < vpbps[fd].header[4]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[3];

			n = vpbps[fd].i0png_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].i0png_lba, vpbps[fd].i0png_size);

			isofs_open(&arg, filename, PSP_O_RDONLY, 0);
			isofs_lseek(&arg, base, PSP_SEEK_SET);
			isofs_read(&arg, p, n);
			isofs_close(&arg);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[4]) && 
			(vpbps[fd].filepointer < vpbps[fd].header[5]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[4];
			
			n = vpbps[fd].i1pmf_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].i1pmf_lba, vpbps[fd].i1pmf_size);
			
			isofs_open(&arg, filename, PSP_O_RDONLY, 0);
			isofs_lseek(&arg, base, PSP_SEEK_SET);
			isofs_read(&arg, p, n);
			isofs_close(&arg);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[5]) && 
			(vpbps[fd].filepointer < vpbps[fd].header[6]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[5];
			
			n = vpbps[fd].p0png_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].p0png_lba, vpbps[fd].p0png_size);
			
			isofs_open(&arg, filename, PSP_O_RDONLY, 0);
			isofs_lseek(&arg, base, PSP_SEEK_SET);
			isofs_read(&arg, p, n);
			isofs_close(&arg);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[6]) && 
			(vpbps[fd].filepointer < vpbps[fd].header[7]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[6];
			
			n = vpbps[fd].p1png_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].p1png_lba, vpbps[fd].p1png_size);
			
			isofs_open(&arg, filename, PSP_O_RDONLY, 0);
			isofs_lseek(&arg, base, PSP_SEEK_SET);
			isofs_read(&arg, p, n);
			isofs_close(&arg);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[7]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[8]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[7];
			
			n = vpbps[fd].s0at3_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].s0at3_lba, vpbps[fd].s0at3_size);

			isofs_open(&arg, filename, PSP_O_RDONLY, 0);
			isofs_lseek(&arg, base, PSP_SEEK_SET);
			isofs_read(&arg, p, n);
			isofs_close(&arg);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if (vpbps[fd].filepointer >= vpbps[fd].filesize)
		{
			break;
		}
	}

	isofs_exit(NULL);

	sceKernelSignalSema(vpsema, 1);
	return read;
}

//0x00002C90
int virtualpbp_lseek(SceUID fd, SceOff offset, int whence)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	fd = fd - 0x7000;

	if (fd < 0 || fd >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	if (whence == PSP_SEEK_SET)
	{
		vpbps[fd].filepointer = (int)offset;
	}
	else if (whence == PSP_SEEK_CUR)
	{
		vpbps[fd].filepointer += (int)offset;
	}
	else if (vpbps[fd].filepointer == PSP_SEEK_END)
	{
		vpbps[fd].filepointer = vpbps[fd].filesize - (int)offset;
	}
	else
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sceKernelSignalSema(vpsema, 1);
	return vpbps[fd].filepointer;
}

//0x00002DE8
int virtualpbp_getstat(int i, SceIoStat *stat)
{
	int res;

	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index || states[i].deleted)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	res =  sceIoGetstat(vpbps[i].isofile, stat);
	stat->st_size = vpbps[i].filesize;

	memcpy(&stat->st_mtime, &stat->st_ctime, sizeof(ScePspDateTime));

	sceKernelSignalSema(vpsema, 1);
	return res;
}

//0x00002F04
int virtualpbp_chstat(int i, SceIoStat *stat, int bits)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sceIoChstat(vpbps[i].isofile, stat, bits);

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

//0x00002FC0
int virtualpbp_remove(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);
	
	if (i < 0 || i >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sctrlSESetUmdFile("");
	int res = sceIoRemove(vpbps[i].isofile);
	if (res >= 0)
	{
		states[i].deleted = 1;
	}

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

//0x0000309C
int virtualpbp_rmdir(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index || states[i].psdirdeleted)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	states[i].psdirdeleted = 1;

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

//0x00003140
int virtualpbp_dread(SceUID fd, SceIoDirent *dir)
{
	SceFatMsDirentPrivate *private;

	sceKernelWaitSema(vpsema, 1, NULL);

	fd = fd - 0x7000;

	if (fd < 0 || fd >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	if (states[fd].dread == 2)
	{
		states[fd].dread = 0;
		sceKernelSignalSema(vpsema, 1);
		return 0;
	}

	sceIoGetstat(vpbps[fd].isofile, &dir->d_stat);
	memcpy(&dir->d_stat.st_mtime, &dir->d_stat.st_ctime, sizeof(ScePspDateTime));
	private = (SceFatMsDirentPrivate *)dir->d_private;

	if (states[fd].dread == 1)
	{
		dir->d_stat.st_size = vpbps[fd].filesize;
		strcpy(dir->d_name, "EBOOT.PBP");
		if (private)
		{
			strcpy(private->FileName, "EBOOT.PBP");
			strcpy(private->LongName, "EBOOT.PBP");
		}
	}
	else
	{
		dir->d_stat.st_size -= vpbps[fd].filesize;
		strcpy(dir->d_name, "IMAGE.ISO");
		if (private)
		{
			strcpy(private->FileName, "IMAGE.ISO");
			strcpy(private->LongName, "IMAGE.ISO");
		}
	}

	states[fd].dread++;
	sceKernelSignalSema(vpsema, 1);
	return 1;
}

int virtualpbp_getSema(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

char *virtualpbp_getfilename(int i)
{
	virtualpbp_getSema(i);
	return vpbps[i].isofile;
}


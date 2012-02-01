/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	virtualpbpmgr.h: Virtual PBP Manager Header Code
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

#ifndef __VIRTUALPBPMGR_H__
#define __VIRTUALPBPMGR_H__

#include "isofs_driver.h"

typedef struct
{
	u32 header[10];
	char isofile[128];
	char sfotitle[64];
	int psfo_lba;
	int psfo_size;
	int i0png_lba;
	int i0png_size;
	int i1pmf_lba;
	int i1pmf_size;
	int p0png_lba;
	int p0png_size;
	int p1png_lba;
	int p1png_size;
	int s0at3_lba;
	int s0at3_size;
	int filesize;
	int filepointer;
	ScePspDateTime mtime;
} VirtualPbp;

typedef struct
{
	int dread;
	int deleted;
	int psdirdeleted;
} InternalState;

typedef struct
{
	char FileName[13];
	char LongName[256];
} SceFatMsDirentPrivate;

int virtualpbp_init();
int virtualpbp_exit();
int virtualpbp_reset();
int virtualpbp_add(char *isofile, ScePspDateTime *mtime, VirtualPbp *res);
int virtualpbp_fastadd(VirtualPbp *pbp);
int virtualpbp_open(int i);
int virtualpbp_close(SceUID fd);
int virtualpbp_read(SceUID fd, void *data, SceSize size);
int virtualpbp_lseek(SceUID fd, SceOff offset, int whence);
int virtualpbp_getstat(int i, SceIoStat *stat);
int virtualpbp_chstat(int i, SceIoStat *stat, int bits);
int virtualpbp_remove(int i);
int virtualpbp_rmdir(int i);
int virtualpbp_dread(SceUID fd, SceIoDirent *dir);
char *virtualpbp_getfilename(int i);

#endif


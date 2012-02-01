/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	isofs_driver.h: SystemControl IsoFS Driver Header Code
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

#ifndef __ISOFS_DRIVER_H__
#define __ISOFS_DRIVER_H__

#include <systemctrl_se.h>
#include "umd9660_driver.h"

#define ISO9660_FILEFLAGS_FILE	1
#define ISO9660_FILEFLAGS_DIR	2

typedef struct __attribute__((packed))
{
	/* Directory record length. */
	u8	len_dr;
	/* Extended attribute record length. */
	u8	XARlength;
	/* First logical block where file starts. */
	u32	lsbStart;
	u32	msbStart;
	/* Number of bytes in file. */
	u32	lsbDataLength;
	u32	msbDataLength;
	/* Since 1900. */
	u8	year;
	u8	month;
	u8	day;
	u8	hour;
	u8	minute;
	u8	second;
	/* 15-minute offset from Universal Time. */
	u8	gmtOffse;
	/* Attributes of a file or directory. */
	u8	fileFlags;
	/* Used for interleaved files. */
	u8	interleaveSize;
	/* Used for interleaved files. */
	u8	interleaveSkip;
	/* Which volume in volume set contains this file. */
	u16	lsbVolSetSeqNum;
	u16	msbVolSetSeqNum;
	/* Length of file identifier that follows. */
	u8	len_fi;
	/* File identifier: actual is len_fi. */
	/* Contains extra blank byte if len_fi odd. */
	char    fi;
} Iso9660DirectoryRecord;

typedef struct
{
	int opened;
	int lba;
	int filesize;
	int filepointer;
	int attributes;
	int olddirlen;
	int eof;
	int curlba;
} FileHandle;

int sprintf(char *buf, const char *fmt, ...)
    __attribute__((format(printf,2,3)));


void *(*SystemCtrlForKernel_F9584CAD)(int size);//oe_malloc
int (*SystemCtrlForKernel_A65E8BC4)(void *ptr);//oe_free

int (*SystemCtrlForKernel_260CA420)(PspIoDrvArg* arg);//isofs_init
int (*SystemCtrlForKernel_7E6F2BBA)();//isofs_fastinit
int (*SystemCtrlForKernel_B078D9A0)(PspIoDrvArg* arg);//isofs_exit
int isofs_reset();
int (*SystemCtrlForKernel_E82E932B)(PspIoDrvFileArg *arg, char *fullpath, int flags, SceMode mode);//isofs_open
int (*SystemCtrlForKernel_CAE6A8E1)(PspIoDrvFileArg *arg);//isofs_close
int (*SystemCtrlForKernel_324FB7B1)(PspIoDrvFileArg *arg, char *data, int len);//isofs_read
SceOff (*isofs_lseek)(PspIoDrvFileArg *arg, SceOff ofs, int whence);
int (*SystemCtrlForKernel_DC974FF8)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);//isofs_getstat
int isofs_dopen(PspIoDrvFileArg *arg, const char *dirname); 
int isofs_dclose(PspIoDrvFileArg *arg);
int isofs_dread(PspIoDrvFileArg *arg, SceIoDirent *dir);
int isofs_ioctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
int isofs_devctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
PspIoDrv *getisofs_driver();

#endif


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


void *(*oe_malloc)(SceSize size);
int (*oe_free)(void *ptr);

int (*isofs_init)(PspIoDrvArg* arg);
int (*isofs_fastinit)();
int (*isofs_exit)(PspIoDrvArg* arg);
int isofs_reset();
int (*isofs_open)(PspIoDrvFileArg *arg, char *fullpath, int flags, SceMode mode);
int (*isofs_close)(PspIoDrvFileArg *arg);
int (*isofs_read)(PspIoDrvFileArg *arg, char *data, int len);
SceOff (*isofs_lseek)(PspIoDrvFileArg *arg, SceOff ofs, int whence);
int (*isofs_getstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);
int isofs_dopen(PspIoDrvFileArg *arg, const char *dirname); 
int isofs_dclose(PspIoDrvFileArg *arg);
int isofs_dread(PspIoDrvFileArg *arg, SceIoDirent *dir);
int isofs_ioctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
int isofs_devctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
PspIoDrv *getisofs_driver();


#endif


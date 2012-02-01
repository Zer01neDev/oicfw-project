/*
	Game Categories v 12.0
	Copyright (C) 2010, Bubbletune

	umd9660_driver.h: SystemControl UMD9660 Driver Header Code
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

#ifndef __UMD9660_DRIVER_H__
#define __UMD9660_DRIVER_H__

#define SECTOR_SIZE	0x0800

char *GetUmdFile();
void SetUmdFile(char *file);
int OpenIso();
int ReadUmdFileRetry(void *buf, int size, int fpointer);
int Umd9660ReadSectors(int lba, int nsectors, void *buf, int *eod);
PspIoDrv *getumd9660_driver();
void DoAnyUmd();

#endif


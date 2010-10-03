/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	isoread.c: SystemControl isoread Code
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

#include "umd9660_driver.h"

//0x000054B8
int IsofileReadSectors(int lba, int nsectors, void *buf)
{
	int read = ReadUmdFileRetry(buf, SECTOR_SIZE*nsectors, lba*SECTOR_SIZE);

	if (read < 0)
	{
		return read;
	}

	read = read / SECTOR_SIZE;

	return read;
}

//0x0000542C
int IsofileGetDiscSize(int umdfd)
{
	int ret = sceIoLseek(umdfd, 0, PSP_SEEK_CUR);
	int size = sceIoLseek(umdfd, 0, PSP_SEEK_END);

	sceIoLseek(umdfd, ret, PSP_SEEK_SET);

	if (size < 0)
		return size;

	return size / SECTOR_SIZE;
}


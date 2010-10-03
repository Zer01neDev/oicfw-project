/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	csoread.h: SystemControl csoread Header Code
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

#ifndef __CISOREAD_H__
#define __CISOREAD_H__

/*
	complessed ISO(9660) header format
*/
typedef struct ciso_header
{
	unsigned char magic[4];			/* +00 : 'C','I','S','O'                           */
	unsigned long header_size;		/* +04 : header size (==0x18)                      */
	unsigned long long total_bytes;	/* +08 : number of original data size              */
	unsigned long block_size;		/* +10 : number of compressed block size           */
	unsigned char ver;				/* +14 : version 01                                */
	unsigned char align;			/* +15 : align of index (offset = index[n]<<align) */
	unsigned char rsv_06[2];		/* +16 : reserved                                  */
#if 0
// INDEX BLOCK
	unsigned int index[0];			/* +18 : block[0] index (data offset = index<<align) */
	unsigned int index[1];			/* +1C : block[1] index (data offset = index<<align) */
             :
             :
	unsigned int index[last];		/* +?? : block[last]                                 */
	unsigned int index[last+1];		/* +?? : end of last data point                      */
// DATA BLOCK
	unsigned char data[];			/* +?? : compressed data                            */
#endif
} CISO_H;

int CisoOpen(int umdfd);
int CisofileGetDiscSize(int umdfd);
int CisofileReadSectors(int lba, int nsectors, void *buf, int *eod);

#endif


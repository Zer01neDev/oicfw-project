/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	pspmediaman.h: SystemControl Media Manager Error Header Code
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

#ifndef __PSPMEDIAMAN_H__
#define __PSPMEDIAMAN_H__

#define SCE_UMD_INIT				(0x00)
#define SCE_UMD_MEDIA_OUT			(0x01)
#define SCE_UMD_MEDIA_IN			(0x02)
#define SCE_UMD_MEDIA_CHG			(0x04)
#define SCE_UMD_NOT_READY			(0x08)
#define SCE_UMD_READY				(0x10)
#define SCE_UMD_READABLE			(0x20)

#define SCE_UMD_MODE_POWERON		(0x01)
#define SCE_UMD_MODE_POWERCUR		(0x02)

#define SCE_UMD_FMT_UNKNOWN			0x00000	/* UNKNOWN */
#define SCE_UMD_FMT_GAME			0x00010	/* GAME */
#define SCE_UMD_FMT_VIDEO			0x00020	/* VIDEO */
#define SCE_UMD_FMT_AUDIO			0x00040	/* AUDIO */
#define SCE_UMD_FMT_CLEAN			0x00080	/* CLEANNING */


typedef struct SceUmdDiscInfo 
{
	unsigned int uiSize;
	unsigned int uiMediaType;
} SceUmdDiscInfo;

#endif


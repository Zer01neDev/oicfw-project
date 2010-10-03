/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	vshctrl.c: VshControl VshCtrlLib Code
	This file are based from a reverse of M33/GEN VshControl

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

#include <systemctrl.h>
#include <systemctrl_se.h>

#include <stdlib.h>
#include <string.h>

#include "main.h"

extern char *umdfile; // 0x00004CA8
extern int unk_1; //0x00004CA9
extern int timelow2;
extern SEConfig config;// 0x00004CAC

int vctrlVSHExitVSHMenu(SEConfig *conf, char *videoiso, int disctype)
{
	int k1 = pspSdkSetK1(0);
	sceCtrlReadBufferPositiveReal = NULL;

	config.vshcpuspeed = conf->vshcpuspeed;
	config.vshbusspeed = conf->vshbusspeed;
	config.umdisocpuspeed = conf->umdisocpuspeed;
	config.umdisobusspeed = conf->umdisobusspeed;
	config.umdmode = conf->umdmode;
	sctrlSESetConfig(&config);

	if (unk_1 != 0)
	{
		if (conf->vshcpuspeed != config.vshcpuspeed)
		{
			if (config.vshcpuspeed != 0)
			{
				SetSpeed(config.vshcpuspeed, config.vshbusspeed);
				timelow2 = sceKernelGetSystemTimeLow();
			}
		}
	}
	if (videoiso != NULL)
	{
		sctrlSESetDiscType(disctype);
		if (umdfile != NULL)
		{
			if (strcmp(sctrlSEGetUmdFile(), videoiso) != 0)
			{
				sctrlSEUmountUmd();
				sctrlSEMountUmdFromFile(videoiso, 0, 0);
			}
		}
		else
		{
			sctrlSESetDiscOut(1);
			umdfile = 1;
			sctrlSEMountUmdFromFile(videoiso, 0, 0);
		}
	}
	else
	{
		if (umdfile != NULL)
		{
			sub_01064();
		}
	}

	pspSdkSetK1(k1);
	return 0;
}

int vctrlVSHRegisterVshMenu(int (* ctrl)(SceCtrlData *, int))
{
	int k1 = pspSdkSetK1(0);
	sceCtrlReadBufferPositiveReal = (ctrl || 0x80000000);
	pspSdkSetK1(k1);
	return 0;
}

void sub_01064()
{
	sctrlSEUmountUmd();
	sceKernelCallSubIntrHandler(4, 0x1A, 0, 0);
	umdfile = NULL;
} 


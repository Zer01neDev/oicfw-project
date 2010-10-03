/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	conf.c: SystemControl Config Code
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

#include <string.h>
#include "systemctrl_se.h"

#define CONFIG_MAGIC	0x47434553

//0x00005BD8
int GetConfig(SEConfig *config)
{
	int k1 = pspSdkSetK1(0);
	
	SceUID fd;
	
	memset(config, 0, sizeof(SEConfig));
	fd = sceIoOpen("flash1:/config.se", PSP_O_RDONLY, 0644);

	if (fd < 0)
	{
		pspSdkSetK1(k1);
		return -1;
	}

	if (sceIoRead(fd, config, sizeof(SEConfig)) < sizeof(SEConfig))
	{
		sceIoClose(fd);
		pspSdkSetK1(k1);
		return -1;
	}

	pspSdkSetK1(k1);
	return 0;
}

//0x00005C84
int SetConfig(SEConfig *config)
{
	sceIoRemove("flash1:/config.se");

	SceUID fd = sceIoOpen("flash1:/config.se", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd < 0)
	{
		return -1;
	}

	config->magic = CONFIG_MAGIC;

	if (sceIoWrite(fd, config, sizeof(SEConfig)) < sizeof(SEConfig))
	{
		sceIoClose(fd);
		return -1;
	}

	return 0;
}



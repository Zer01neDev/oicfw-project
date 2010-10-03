/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	systemctrl.c: SystemControl System Control Lib Code
	This file are based from a reverse of M33/GEN SystemControl
	Much part of this code are taken from OE/Wildcard Source Code

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
#include <pspthreadman_kernel.h>
#include "main.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "sysmodpatches.h"
#include "umd9660_driver.h"
#include "isofs_driver.h"


SceUID heapid = -1;// 0x00009EBC
int initapitype;// 0x0000AC90[0]
char initfilename;// 0x0000AC94[0]
int keyconfig;// 0x0000AC98[0]
int fileindex;// 0x0000A6FC
char *reboot_module_after;
char *reboot_buf;
int reboot_size;
int reboot_flags;

// OK
void sctrlSESetBootConfFileIndex(int index)
{
	fileindex = index;
}

// OK
void sctrlHENLoadModuleOnReboot(char *module_after, char *buf, int size, int flags)
{
	reboot_module_after = module_after;
	reboot_buf = buf;
	reboot_size = size;
	reboot_flags = flags;
}

//745286D1
void SystemCtrlForKernel_AC0E84D1(int arg1)
{
	var1 = *((int *) 0x0000A2C4);
	*((int *) 0x0000A2C4) = arg1;
	return;
}

int sctrlKernelSetUserLevel(int level)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetUserLevel();
	SceModule2 *mod = sceKernelFindModuleByName("sceThreadManager");
	u32 *thstruct = (u32 *)_lw(mod->text_addr+0x18D40);
	thstruct[0x14/4] = (level ^ 8) << 28;
	pspSdkSetK1(k1);
	return res;
}

// OK
int	sctrlHENIsSE()
{
	return 1;
}

// OK
int	sctrlHENIsDevhook()
{
	return 0;
}

int sctrlHENGetVersion()
{
	return 0x00000500;
}

// OK
int sctrlSEGetVersion()
{
	return 0x00020005;
}

// OK
int sctrlKernelLoadExecVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadExecVSHDisc(file, param);
	pspSdkSetK1(k1);
	return res;
}

// OK
int sctrlKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadExecVSHDiscUpdater(file, param);
	pspSdkSetK1(k1);
	return res;
}

// OK
int sctrlKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadExecVSHMs1(file, param);
	pspSdkSetK1(k1);
	return res;
}

// OK
int sctrlKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadExecVSHMs2(file, param);
	pspSdkSetK1(k1);
	return res;
}

// OK
int sctrlKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);	
	int res = sceKernelLoadExecVSHMs3(file, param);
	pspSdkSetK1(k1);
	return res;
}

// OK
int sctrlKernelLoadExecVSHMs4(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int ret = sceKernelLoadExecVSHMs4(file, param);
	pspSdkSetK1(k1);
	return ret;
}

// OK
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int (* LoadExecVSH)(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param, int unk2);
	int k1 = pspSdkSetK1(0);
	SceModule2 *mod = sceKernelFindModuleByName("sceLoadExec");
	LoadExecVSH = (void*)(mod->text_addr+0x1E58);
	int res = LoadExecVSH(apitype, file, param, 0x10000);
	pspSdkSetK1(k1);
	return res;
}

// OK 0x00001770
int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelExitVSHVSH(param);
	pspSdkSetK1(k1);
	return res;
}

// OK
PspIoDrv *sctrlHENFindDriver(char *drvname)
{
	int ret;
	int k1 = pspSdkSetK1(0);
	SceModule2 *mod = sceKernelFindModuleByName("sceIOFileManager");

	u32 *(* GetDevice)(char*) = (void*)(mod->text_addr+0x2838);
	u32 *u = GetDevice(drvname);
	if(u)
	{
		pspSdkSetK1(k1);
		ret = (PspIoDrv*)u[1];
	}
	else
	{
		pspSdkSetK1(k1);
		ret = NULL;
	}

	return ret;
}

// OK
int sctrlKernelSetInitApitype(int apitype)
{
	int k1 = pspSdkSetK1(0);
	int prev = sceKernelInitApitype();
	initapitype = apitype
	pspSdkSetK1(k1);
	return prev;
}

// OK
int sctrlKernelSetInitFileName(char *filename)
{
	int k1 = pspSdkSetK1(0);
	initfilename = filename;
	pspSdkSetK1(k1);
	return 0;
}

// OK
int sctrlKernelSetInitKeyConfig(int key)
{
	int k1 = pspSdkSetK1(0);
	int prev = sceKernelApplicationType();
	keyconfig = key;
	pspSdkSetK1(k1);
	return prev;
}

// 0x00006120
int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs)
{
	int k1 = pspSdkSetK1(0);
	sctrlSESetUmdFile(file);
	if (noumd != 0 && isofs != 0)
	{
		if (sceIoDelDrv("umd") > 0)
		{
			if (sceIoAddDrv(getumd9660_driver()) > 0)
			{
				if (noumd)
				{
					DoNoUmdPatches();
				}
				if (isofs)
				{
					sceIoDelDrv("isofs");
					sceIoAddDrv(getisofs_driver());
					SceModule2 *mod = sceKernelFindModuleByName("sceIsofs_driver");
					if (mod)
					{
						_sw(0x03E00008, mod->text_addr+4390);
						_sw(0x34020000, mod->text_addr+4394);
						ClearCaches();
					}
					sceIoAssign("disc0:", "umd0:", "isofs0:", IOASSIGN_RDONLY, NULL, 0);
				}
				pspSdkSetK1(k1);
			}
		}
	}
	else
	{
		DoAnyUmd();
		pspSdkSetK1(k1);
	}
	return 0;
}

// OK
int sctrlKernelSetDevkitVersion(int version)
{
	int k1 = pspSdkSetK1(0);
	int prev = sceKernelDevkitVersion();

	int high = version >> 16;
	int low = version & 0xFFFF;

	_sh(high, 0x880112FC);
	_sh(low, 0x880112F4);

	ClearCaches();

	pspSdkSetK1(k1);
	return prev;
}

// OK
int sctrlSESetConfigEx(SEConfig *config, int size)
{
	int k1 = pspSdkSetK1(0);
	SceUID fd = sceIoOpen("flashfat1:/config.se", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0)
	{
		pspSdkSetK1(k1);
		return -1;
	}
	else
	{
		config->magic = CONFIG_MAGIC;
		int read = sceIoRead(fd, config, size);
		if (((read < size)) == 0)
		{
			sceIoClose(fd);
			pspSdkSetK1(k1);
			return -1;
		}
		else
		{
			sceIoClose(fd);
			pspSdkSetK1(k1);
		}
	}
	return 0;
}

// OK
int sctrlSEGetConfigEx(SEConfig *config, int size)
{
	int read = -1;
	int k1 = pspSdkSetK1(0);
	memset(config, 0, size);
	SceUID fd = sceIoOpen("flashfat1:/config.se", PSP_O_RDONLY, 0666);
	if (fd > 0)
	{
		read = sceIoRead(fd, config, size);
		sceIoClose(fd);
		return 0;
	}
	pspSdkSetK1(k1);
	return -1;
}

// OK
int sctrlSESetConfig(SEConfig *config)
{
	return sctrlSESetConfigEx(config, sizeof(SEConfig));
}

// OK
int sctrlSEGetConfig(SEConfig *config)
{
	return sctrlSEGetConfigEx(config, sizeof(SEConfig));
}

//0x000062CC
void SystemCtrlForKernel_72F29A6E()
{
	return 0x1AC88;
}

SceUID unkfpl;
SceUID unkevf;//0x0000ACAC
SceUID unkcb;//((int *) sp)[2]

// 0x000063D0
void sub_063D0 (int arg1, int arg2)
{
	*((int *) 0x0000AC84) = arg2;
	sceKernelSetEventFlag(unkevf, 1);
	return 0;
}

// OK 0x00006400
void trim(char *str)
{
	int len = strlen(str);
	int i;

	for (i = len-1; i >= 0; i--)
	{
		if (str[i] == 0x20 || str[i] == '\t')
		{
			str[i] = 0;
		}
		else
		{
			break;
		}
	}
}

// OK 0x00006458
int GetPlugin(char *buf, int size, char *str, int *activated)
{
	char ch = 0;
	int n = 0;
	int i = 0;
	char *s = str;

	while (1)
	{   
		if (i >= size)
			break;

		ch = buf[i];

		if (ch < 0x20 && ch != '\t')
		{
			if (n != 0)
			{
				i++;
				break;
			}
		}
		else
		{
			*str++ = ch;
			n++;
		}
		i++;
	}

	trim(s);

	*activated = 0;

	if (i > 0)
	{
		char *p = strpbrk(s, " \t");
		if (p)
		{
			char *q = p+1;
         
			while (*q < 0) q++;

			if (strcmp(q, "1") == 0)
			{
				*activated = 1;
			}

			*p = 0;
		}   
	}
	return i;
}


/*
    Open Idea Custom Firmware
    Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

    main.c: OpenIdea Updater Nuclear_Driver Main Code
    This file are based from a reverse of M33/GEN Plutonium_Driver/uranium235_module(u235)

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
#include <pspsysmem_kernel.h>
#include <pspiofilemgr_kernel.h>
#include <pspmodulemgr_kernel.h>

#include <systemctrl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Principal Modules Buffer
#include "galaxy.h"
#include "idcanager.h"
#include "march33.h"
#include "moon.h"
#include "peagasus.h"
#include "popcorn.h"
#include "recovery.h"
#include "satelite.h"
#include "usbdevice.h"
#include "velf.h"
#include "vshctrl.h"
#include "name_plate.h"

// 01g Modules Buffer
#include "custom_ipl_GEN.h"
#include "pspbtjnf_01g.h"
#include "pspbtknf_01g.h"
#include "pspbtlnf_01g.h"
#include "systemctrl_01g.h"

// 02g Modules Buffer
/*#include "pspbtjnf_02g.h"
#include "pspbtknf_02g.h"
#include "pspbtlnf_02g.h"
#include "systemctrl_02g.h"*/
#include "custom_ipl_02g.h"

#define REDIRECT_FUNCTION(a, f) _sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), a);  _sw(0x00000000, a+4);

PSP_MODULE_INFO("Nuclear_Driver", 0x1006, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

int i = 0;
int k1 = 0;
int (*sctrlIoAddDrv)(PspIoDrv *drv);
int ipl_size = 131072; // 0x20000
int IplUpdate_patched = 0;
int (*sceIplUpdateSetIpl)(void *buf, int size);
int (*scePowerRequestColdReset)(int);
int (*sceIplUpdateVerifyIpl)(void *buf, int size);

int sceIplUpdateSetIplPatched(void *buf, int size);
int sceIplUpdateVerifyIplPatched(void *buf, int size);

typedef struct Module
{
	char *dst;
	void* buffer;
	u32 size;
} Module;

// Principal Modules Counts
#define MODULES_COUNT 12
// 0Xg Modules Counts
#define MODULES_COUNT_0Xg 4

// Principal Modules
Module modules[MODULES_COUNT] =
{
	{ "flash0:/kd/galaxy.prx", galaxy, sizeof(galaxy) },
	{ "flash0:/kd/idcanager.prx", idcanager, sizeof(idcanager) },
	{ "flash0:/kd/march33.prx", march33, sizeof(march33) },
	{ "flash0:/kd/moon.prx", moon, sizeof(moon) },
	{ "flash0:/kd/peagasus.prx", peagasus, sizeof(peagasus) },
	{ "flash0:/kd/popcorn.prx", popcorn, sizeof(popcorn) },
	{ "flash0:/vsh/module/recovery.prx", recovery, sizeof(recovery) },
	{ "flash0:/kd/satelite.prx", satelite, sizeof(satelite) },
	{ "flash0:/kd/usbdevice.prx", usbdevice, sizeof(usbdevice) },
	{ "flash0:/kd/velf.prx", velf, sizeof(velf) },
	{ "flash0:/kd/vshctrl.prx", vshctrl, sizeof(vshctrl) },
	{ "flash0:/vsh/resource/name_plate.png", name_plate, sizeof(name_plate) },
};

// For flashing 0xg Modules (PSP Fat(1000)/Slim(2000))
Module *modules_0xg;

// 01g Modules (PSP Fat(1000))
Module modules_01g[MODULES_COUNT_0Xg] =
{
	{ "flash0:/kd/pspbtjnf.bin", pspbtjnf_01g, sizeof(pspbtjnf_01g) },
	{ "flash0:/kd/pspbtknf.bin", pspbtknf_01g, sizeof(pspbtknf_01g) },
	{ "flash0:/kd/pspbtlnf.bin", pspbtlnf_01g, sizeof(pspbtlnf_01g) },
	{ "flash0:/kd/systemctrl.prx", systemctrl_01g, sizeof(systemctrl_01g) },
};

// 02g Modules (PSP Slim(2000))
/*Module modules_02g[MODULES_COUNT_0Xg] =
{
	{ "flash0:/kd/pspbtjnf_02g.bin", pspbtjnf_02g, sizeof(pspbtjnf_02g) },
	{ "flash0:/kd/pspbtknf_02g.bin", pspbtknf_02g, sizeof(pspbtknf_02g) },
	{ "flash0:/kd/pspbtlnf_02g.bin", pspbtlnf_02g, sizeof(pspbtlnf_02g) },
	{ "flash0:/kd/systemctrl_02g.prx", systemctrl_02g, sizeof(systemctrl_02g) },
};*/

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

int sceIoAddDrvPatched(PspIoDrv* drv)
{
	return sctrlIoAddDrv(drv);
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat)
{
	k1 = pspSdkSetK1(0);
	if (strcmp(file, "ms0:/PSP/GAME/UPDATE/EBOOT.PBP") == 0)
	{
		strcpy((char*)file, "ms0:/PSP/GAME/UPDATE/550.PBP");
	}
	pspSdkSetK1(k1);
	return sceIoGetstat(file, stat);
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode)
{
	k1 = pspSdkSetK1(0);
	/*if (!IplUpdate_patched)
	{
		sceIplUpdateSetIpl = (void*)sctrlHENFindFunction("TexSeqCounter", "sceIplUpdate", 0xEE7EB563);
		if (sceIplUpdateSetIpl)
		{
			sctrlHENPatchSyscall((u32)sceIplUpdateSetIpl, sceIplUpdateSetIplPatched);
			sceIplUpdateVerifyIpl = (void*)sctrlHENFindFunction("TexSeqCounter", "sceIplUpdate", 0x0565E6DD);
			sctrlHENPatchSyscall((u32)sceIplUpdateVerifyIpl, sceIplUpdateVerifyIplPatched);
			IplUpdate_patched = 1;
		}
	}*/
	if (strcmp(file, "ms0:/PSP/GAME/UPDATE/EBOOT.PBP") == 0)
	{
		strcpy((char*)file, "ms0:/PSP/GAME/UPDATE/550.PBP");
	}
	pspSdkSetK1(k1);
	return sceIoOpen(file, flags, mode);
}

int NuclearInstallModule(char *filename, char *buffer, u32 size)
{
	SceUID fd = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0) return -1;
	int written = sceIoWrite(fd, buffer, size);
	if (sceIoClose(fd) < 0) return -1;
    return written;
}

int scePowerRequestColdResetPatched(int delay)
{
	k1 = pspSdkSetK1(0);
	sceIoUnassign("flash0:");
	sceIoUnassign("flash1:");

	int ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);
	if(ret < 0) return -1;

	ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);
	if(ret < 0) return -1;

	if(sceKernelGetModel() == PSP_MODEL_STANDARD)
		modules_0xg = modules_01g;
	//else if(sceKernelGetModel() == PSP_MODEL_SLIM_AND_LITE)
	//	modules_0xg = modules_02g;

	for(i = 0; i < MODULES_COUNT; i++)
		NuclearInstallModule(modules[i].dst, modules[i].buffer, modules[i].size);

	for(i = 0; i < MODULES_COUNT_0Xg; i++)
		NuclearInstallModule(modules_0xg[i].dst, modules_0xg[i].buffer, modules_0xg[i].size);

	pspSdkSetK1(k1);
	scePowerRequestColdReset(delay);
	return 0;
}

int sceKernelStartModulePatched(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option)
{
	k1 = pspSdkSetK1(0);
	SceModule2 *mod = sceKernelFindModuleByName("sceNAND_Updater_Driver");
	if (mod)
	{
		if (strcmp(mod->modname, "sceNAND_Updater_Driver") == 0)
		{
			_sh(0xAC60, mod->text_addr+0x0D7E);
			ClearCaches();
		}
	}
	pspSdkSetK1(k1);
	return sceKernelStartModule(modid, argsize, argp, status, option);
}

int NuclearThread(SceSize args, void *argp)
{
	scePowerRequestColdReset = (void*)sctrlHENFindFunction("scePower_Service", "scePower", 0x0442D852);
	sctrlHENPatchSyscall((u32)scePowerRequestColdReset, scePowerRequestColdResetPatched);
	if (((0x0306000F < sceKernelDevkitVersion())) == 0)
	{
		sctrlIoAddDrv = (void*)sctrlHENFindFunction("SystemControl", "IoFileMgrForKernel", 0x8E982A74);
		REDIRECT_FUNCTION(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x8E982A74), sceIoAddDrvPatched);
	}
	if (sceKernelGetModel() == PSP_MODEL_STANDARD)
	{
		//sctrlHENPatchSyscall(sctrlHENFindFunction("sceModuleManager", "ModuleMgrForUser", 0x50F0C1EC), sceKernelStartModulePatched);
	}

	sctrlHENPatchSyscall((u32)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x109F50BC), sceIoOpenPatched);
	sctrlHENPatchSyscall((u32)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xACE946E8), sceIoGetstatPatched);
	ClearCaches();

	sceKernelDelayThread(70000);
	while (1)
	{
		SceModule2 *mod = sceKernelFindModuleByName("OpenIdeaUpdater");
		if (mod == NULL) break;
		sceKernelStopModule(mod->modid, 0, NULL, NULL, NULL);
		sceKernelUnloadModule(mod->modid);
		sceKernelDelayThread(50000);
	}
	SceUID sceupdater = sceKernelLoadModuleForLoadExecVSHMs1("ms0:/PSP/GAME/UPDATE/550.PBP", 0, NULL);
	if (sceupdater >= 0)
	{
		sceKernelStartModule(sceupdater, 0x1F, "ms0:/PSP/GAME/UPDATE/EBOOT.PBP", 0, NULL);
	}
	sceKernelExitDeleteThread(0);
	return 0;
}

int NuclearStartUpdater()
{
	k1 = pspSdkSetK1(0);
	SceUID NuclearThid = sceKernelCreateThread("NuclearThread", NuclearThread, 0xF, 0x1000, 0, NULL);
	if (NuclearThid < 0)
	{
		pspSdkSetK1(k1);
	}
	else
	{
		sceKernelStartThread(NuclearThid, 0, NULL);
		pspSdkSetK1(k1);
	}
	return 0;
}

int sceIplUpdateSetIplPatched(void *buf, int size)
{
	k1 = pspSdkSetK1(0);
	if(sceKernelGetModel() == PSP_MODEL_STANDARD)
	{
		memcpy(custom_ipl+16384, buf, size);
	}
	else
	{
		if(sceKernelGetModel() == PSP_MODEL_SLIM_AND_LITE)
		{
			memcpy(custom_ipl+16384, buf, size);

			for(i = 0; i < 16384; i+=4)
			{
				custom_ipl[i+0] = custom_ipl_02g[i+0];
				custom_ipl[i+1] = custom_ipl_02g[i+1];
				custom_ipl[i+2] = custom_ipl_02g[i+2];
				custom_ipl[i+3] = custom_ipl_02g[i+3];
			}
		}
	}

	ClearCaches();
	int ret = sceIplUpdateSetIpl(custom_ipl, ipl_size);
	pspSdkSetK1(k1);
	return ret;
}

int sceIplUpdateVerifyIplPatched(void *buf, int size)
{
	k1 = pspSdkSetK1(0);
	int ret = sceIplUpdateVerifyIpl(custom_ipl+16384, size - 16384);
	pspSdkSetK1(k1);
	return ret;
}

int module_start(SceSize args, void *argp)
{
	if (argp == NULL)
	{
		return 0;
	}
	else
	{
		if (args == 0) return 0;
		u32 var3 = sctrlHENFindFunction("scePower_Service", "scePower", 0x2085D15D);
		if (var3 == 0) return 0;

		_sw(0x03E00008, var3);
		_sw(0x24020062, var3+4);

		ClearCaches();
		return 0;
	}
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}


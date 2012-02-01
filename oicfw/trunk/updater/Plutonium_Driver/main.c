/*
    Open Idea Custom Firmware
    Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

    main.c: OpenIdea Updater Plutonium_Driver Main Code
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

#include <systemctrl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include "custom_ipl_M33.h"
#include "custom_ipl_GEN.h"
#include "custom_ipl_02g.h"

#include "galaxy.h"
#include "idcanager.h"
#include "march33.h"
#include "moon.h"
#include "peagasus.h"
#include "popcorn.h"
#include "pspbtjnf_01g.h"
#include "pspbtknf_01g.h"
#include "pspbtlnf_01g.h"
#include "recovery.h"
#include "satelite.h"
#include "usbdevice.h"
#include "velf.h"
#include "systemctrl_01g.h"
#include "vshctrl.h"

#define NOP			0x00000000
#define JR_RA		0x03E00008
#define J_OPCODE	0x08000000
#define REDIRECT_FUNCTION(a, f) _sw(J_OPCODE | (((u32)(f) >> 2) & 0x03FFFFFF), a);  _sw(NOP, a+4);


typedef struct Module
{
	char *dst;
	void* buffer;
	u32 size;
} Module;

#define MODULES_COUNT 15
Module modules[MODULES_COUNT] =
{
	{ "flash0:/kd/galaxy.prx", galaxy, sizeof(galaxy) },
	{ "flash0:/kd/idcanager.prx", idcanager, sizeof(idcanager) },
	{ "flash0:/kd/march33.prx", march33, sizeof(march33) },
	{ "flash0:/kd/moon.prx", moon, sizeof(moon) },
	{ "flash0:/kd/peagasus.prx", peagasus, sizeof(peagasus) },
	{ "flash0:/kd/popcorn.prx", popcorn, sizeof(popcorn) },
	{ "flash0:/kd/pspbtjnf.bin", pspbtjnf_01g, sizeof(pspbtjnf_01g) },
	{ "flash0:/kd/pspbtknf.bin", pspbtknf_01g, sizeof(pspbtknf_01g) },
	{ "flash0:/kd/pspbtlnf.bin", pspbtlnf_01g, sizeof(pspbtlnf_01g) },
	{ "flash0:/vsh/module/recovery.prx", recovery, sizeof(recovery) },
	{ "flash0:/vsh/module/satelite.prx", satelite, sizeof(satelite) },
	{ "flash0:/kd/usbdevice.prx", usbdevice, sizeof(usbdevice) },
	{ "flash0:/kd/velf.prx", velf, sizeof(velf) },
	{ "flash0:/kd/systemctrl.prx", systemctrl_01g, sizeof(systemctrl_01g) },
	{ "flash0:/kd/vshctrl.prx", vshctrl, sizeof(vshctrl) },
};

int (*sctrlIoAddDrv)(PspIoDrv *drv);
int ipl_size = 131072; // 0x20000
int IplUpdate_patched = 0;
int (*sceIplUpdateSetIpl)(void *buf, int size);
int (*scePowerRequestColdReset)(int);
int (*sceIplUpdateVerifyIpl)(void *buf, int size);

int sceIplUpdateSetIplPatched(void *buf, int size);
int sceIplUpdateVerifyIplPatched(void *buf, int size);


PSP_MODULE_INFO("Plutonium_Driver", 0x1000, 1, 0);

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

int sceIoAddDrvPatched(PspIoDrv* drv)
{
	return sctrlIoAddDrv(drv);
}

int PlutoniumColdReset()//Plutonium_E30ED0F0
{
	int k1 = pspSdkSetK1(0);
	scePowerRequestColdReset = (void*)sctrlHENFindFunction("scePower_Service", "scePower", 0x0442D852);
	scePowerRequestColdReset(0);
	pspSdkSetK1(k1);
	return 0;
}

int PlutoniumGetModel()//Plutonium_340AC1E4
{
	int k1 = pspSdkSetK1(0);
	int model = sceKernelGetModel();
	pspSdkSetK1(k1);
	return model;
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat)
{
	int k1 = pspSdkSetK1(0);
	if (strcmp(file, "ms0:/PSP/GAME/UPDATE/EBOOT.PBP") == 0)
	{
		strcpy((char*)file, "ms0:/PSP/GAME/UPDATE/550.PBP");
	}
	pspSdkSetK1(k1);
	return sceIoGetstat(file, stat);
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode)
{
	int k1 = pspSdkSetK1(0);
	if (!IplUpdate_patched)
	{
		sceIplUpdateSetIpl = (void*)sctrlHENFindFunction("TexSeqCounter", "sceIplUpdate", 0xEE7EB563);
		if (sceIplUpdateSetIpl != 0)
		{
			sctrlHENPatchSyscall((u32)sceIplUpdateSetIpl, sceIplUpdateSetIplPatched);
			sceIplUpdateVerifyIpl = (void*)sctrlHENFindFunction("TexSeqCounter", "sceIplUpdate", 0x0565E6DD);
			sctrlHENPatchSyscall((u32)sceIplUpdateVerifyIpl, sceIplUpdateVerifyIplPatched);
			IplUpdate_patched = 1;
		}
	}
	if (strcmp(file, "ms0:/PSP/GAME/UPDATE/EBOOT.PBP") == 0)
	{
		strcpy((char*)file, "ms0:/PSP/GAME/UPDATE/550.PBP");
	}
	pspSdkSetK1(k1);
	return sceIoOpen(file, flags, mode);
}

int uranium235_thread(SceSize args, void *argp)
{
	sceKernelSetDdrMemoryProtection((void*)0x08400000, 0x00400000, 0xF);
	SceKernelLMOption option;
	option.size = sizeof(option);
	option.mpidtext = 5;
	option.mpiddata = 5;
	option.position = 0; // *((char *) 0x00025115) = 0x00000001;
	option.access = 1;
	SceUID u235 = sceKernelLoadModule("ms0:/PSP/GAME/UPDATE/u235.prx", 0, &option);
	if (u235 >= 0)
	{
		sceKernelStartModule(u235, 0, NULL, NULL, NULL);
	}
	sceKernelExitDeleteThread(0);
	return 0;
}

int InstallModule(char *filename, char *buffer, u32 size)
{
	SceUID fd = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0) return -1;
	int written = sceIoWrite(fd, buffer, size);
	if (sceIoClose(fd) < 0) return -1;
    return written;
}

int i = 0;
int scePowerRequestColdResetPatched()
{
	int k1 = pspSdkSetK1(0);
	/*SceUID uranium235_thid = sceKernelCreateThread("uranium235", uranium235_thread, 0xF, 0x1000, 0, NULL);
	if (uranium235_thid < 0)
	{
		pspSdkSetK1(k1);
	}
	else
	{
		sceKernelStartThread(uranium235_thid, 0, NULL);
		pspSdkSetK1(k1);
	}*/

	sceIoUnassign("flash0:");
	sceIoUnassign("flash1:");

	int ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);
	if(ret < 0) return -1;

	ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);
	if(ret < 0) return -1;

	for(i = 0; i < MODULES_COUNT; i++)
	{
		InstallModule(modules[i].dst, modules[i].buffer, modules[i].size);
	}
	pspSdkSetK1(k1);
	PlutoniumColdReset();
	return 0;
}

int sceKernelStartModulePatched(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option)
{
	int k1 = pspSdkSetK1(0);
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

int plutonium_thread(SceSize args, void *argp)
{
	sctrlHENPatchSyscall(sctrlHENFindFunction("scePower_Service", "scePower", 0x0442D852), scePowerRequestColdResetPatched);
	if (((0x0306000F < sceKernelDevkitVersion())) == 0)
	{
		sctrlIoAddDrv = (void*)sctrlHENFindFunction("SystemControl", "IoFileMgrForKernel", 0x8E982A74);
		REDIRECT_FUNCTION(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x8E982A74), sceIoAddDrvPatched);
		if (sceKernelGetModel() == PSP_MODEL_STANDARD)
		{
			sctrlHENPatchSyscall(sctrlHENFindFunction("sceModuleManager", "ModuleMgrForUser", 0x50F0C1EC), sceKernelStartModulePatched);
		}
	}
	else
	{
		if (sceKernelGetModel() == PSP_MODEL_STANDARD)
		{
			sctrlHENPatchSyscall(sctrlHENFindFunction("sceModuleManager", "ModuleMgrForUser", 0x50F0C1EC), sceKernelStartModulePatched);
		}
	}

	sctrlHENPatchSyscall((u32)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x109F50BC), sceIoOpenPatched);
	sctrlHENPatchSyscall((u32)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xACE946E8), sceIoGetstatPatched);
	ClearCaches();

	sceKernelDelayThread(70000);//0x000AAE60
	while (1)
	{
		SceModule2 *mod = sceKernelFindModuleByName("plutonium_updater");
		if (mod == NULL) break;
		sceKernelStopModule(mod->modid, 0, NULL, NULL, NULL);
		sceKernelUnloadModule(mod->modid);
		sceKernelDelayThread(50000);//0x0000C350
	}
	SceUID updater = ModuleMgrForKernel_6723BBFF("ms0:/PSP/GAME/UPDATE/550.PBP", 0, NULL);
	if (updater >= 0)
	{
		sceKernelStartModule(updater, 0x1F, "ms0:/PSP/GAME/UPDATE/EBOOT.PBP", 0, NULL);
	}
	sceKernelExitDeleteThread(0);
	return 0;
}

int PlutoniumStartUpdater()//Plutonium_F8547F11
{
	int k1 = pspSdkSetK1(0);
	SceUID plutonium_thid = sceKernelCreateThread("plutonium", plutonium_thread, 0xF, 0x1000, 0, NULL);
	if (plutonium_thid < 0)
	{
		pspSdkSetK1(k1);
	}
	else
	{
		sceKernelStartThread(plutonium_thid, 0, NULL);
		pspSdkSetK1(k1);
	}
	return 0;
}

int sceIplUpdateSetIplPatched(void *buf, int size)
{
	int k1 = pspSdkSetK1(0);
	if(sceKernelGetModel() == PSP_MODEL_STANDARD)
	{
		memcpy(custom_ipl+16384, buf, size);
	}
	else
	{
		if(sceKernelGetModel() == PSP_MODEL_SLIM_AND_LITE)
		{
			memcpy(custom_ipl+16384, buf, size);
			int i = 0;
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
	int k1 = pspSdkSetK1(0);
	sceIplUpdateVerifyIpl = (void*)sctrlHENFindFunction("TexSeqCounter", "sceIplUpdate", 0x0565E6DD);
	int ret = sceIplUpdateVerifyIpl(custom_ipl+16384, ipl_size-16384);
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

		_sw(JR_RA, var3);
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


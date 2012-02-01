/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	sysmodpatches.c: SystemControl System Module Patches Code
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
#include <pspinit.h>
#include <pspcrypt.h>
#include <psploadexec_kernel.h>
#include <psputilsforkernel.h>
#include <psppower.h>
#include <pspreg.h>
#include <pspmediaman.h>

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "sysmodpatches.h"
#include "../vshctrl/main.h"
#include "rebootex.h"
#include <umd9660_driver.h>
#include <isofs_driver.h>
//#include "virtualpbpmgr.h"
#include "systemctrl_se.h"
#include "malloc.h"

#define EBOOT_BIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define BOOT_BIN  "disc0:/PSP_GAME/SYSDIR/BOOT.BIN"

int (* UtilsForKernel_7dd07271)(u8 *dest, u32 destSize, const u8 *src, void *unk, void *unk2);
int (* scePowerSetCpuClockFrequency_k)(int cpufreq);
int (* scePowerSetBusClockFrequency_k)(int busfreq);
int (* scePowerSetClockFrequency_k)(int cpufreq, int ramfreq, int busfreq);//0x0000A6D4
int (* scePowerSetClockFrequency_)(int cpufreq, int ramfreq, int busfreq);
int (* MemlmdDecrypt)(u8 *buf, int size, int *ret, int u);

SEConfig config;//0x0000A720
u32 vshmain_args[0x0400/4];
extern int fileindex;
extern char *reboot_module_after;
extern char *reboot_buf;
extern int reboot_size;
extern int reboot_flags;
int unk;//0x0000A6F8

// OK
void SetConfig(SEConfig *newconfig)
{
	memcpy(&config, newconfig, sizeof(SEConfig));
}

// OK 0x00006538
void LoadStartModule(char *module)
{
	SceUID mod = sceKernelLoadModule(module, 0, NULL);
	if (mod >= 0)
		sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
}

// OK
int ReadLine(SceUID fd, char *str)
{
	char ch = 0;
	int n = 0;

	while (1)
	{
		if (sceIoRead(fd, &ch, 1) != 1)
			return n;

		if (ch < 0x20)
		{
			if (n != 0)
				return n;
		}
		else
		{
			*str++ = ch;
			n++;
		}
	}
}

// OK
u32 FindPowerFunction(u32 nid)
{
	return sctrlHENFindFunction("scePower_Service", "scePower", nid);
}

// OK
u32 FindPowerDriverFunction(u32 nid)
{
	return sctrlHENFindFunction("scePower_Service", "scePower_driver", nid);
}

int plugins_started = 0; //0x0000A5C0
u32 speedup_ms = 0; //0x0000AC84
SceUID unkevf = -1; //0x0000ACAC

int PatchMs_Callback(SceSize args, void *argp)
{
	speedup_ms = (u32)argp;
	sceKernelSetEventFlag(unkevf, 1);
	return 0;
}

int sceKernelStartModulePatched(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option)
{
	if(!sceKernelFindModuleByUID(modid))
	{
		return sceKernelStartModule(modid, argsize, argp, status, option)
	}

	SceUID unkfpl = -1;
	SceUID unkcb = -1;

	char* plugins_file = "";
	int use_plugins = 0;

	char* buffer = NULL;
	char plugin[64];
	int activated = 1;
	int ret = 0;

	u32 devctl = 0x0;

	if(strcmp(mod->modname, "vsh_module"))
	{
		if(plugins_started != 0) // Plugins already started ?
		{
			return sceKernelStartModule(modid, argsize, argp, status, option);
		}

		int type = sceKernelApplicationType();
		if(sceKernelFindModuleByName("sceNp9660_driver"))
		{
			speedup_ms = 1;
		}

		char* media_module = "sceMediaSync";
		if(type == PSP_INIT_KEYCONFIG_POPS)
		{
			media_module = "sceIoFilemgrDNAS";
		}
		if(!sceKernelFindModuleByName(media_module))
		{
			sceKernelStartModule(modid, argsize, argp, status, option);
		}

		plugins_started = 1;
		sctrlSEGetConfig(&config);

		if(type == PSP_INIT_KEYCONFIG_VSH)
		{
			if (!sceKernelFindModuleByName("scePspNpDrm_Driver"))
			{
				return sceKernelStartModule(modid, argsize, argp, status, option);
			}
			else
			{
				use_plugins = config.xmbplugins;
				plugins_file = "ms0:/seplugins/vsh.txt";
			}
		}
		else if(tpe == PSP_INIT_KEYCONFIG_GAME)
		{
			use_plugins = config.gameplugins;
			plugins_file = "ms0:/seplugins/game.txt";
		}
		else if(type == PSP_INIT_KEYCONFIG_POPS)
		{
			use_plugins = config.popsplugins;
			plugin_files = "ms0:/seplugins/pops.txt";
		}
		else
		{
			return sceKernelStartModule(modid, argsize, argp, status, option);
		}
		if(use_plugins < 1)
		{
			return sceKernelStartModule(modid, argsize, argp, status, option);
		}

		if(speedup_ms == 0)
		{
			if(sceIoDevctl("mscmhc0:", 0x02025806, 0, 0, &devctl, 4) >= 0)
			{
				if(devctl == 1)
				{
					unkevf = sceKernelCreateEventFlag("", 0, 0, NULL);
					unkcb = sceKernelCreateCallback("", PatchMs_Callback, 0);
					devctl = sceIoDevctl("fatms0:", 0x02415821, &unkcb, 4, 0, 0);
					if(devctl >= 0)
					{
						sceKernelWaitEventFlagCB(unkevf, 1, 17, 0, 0);
						sceIoDevctl("fatms0:", 0x02415822, &unkcb, 4, 0, 0);
					}
					sceKernelDeleteCallback(unkcb);
					sceKernelDeleteEventFlag(unkevf);
				}
			}
			if(speedup_ms == 0)
			{
				//	I think we don't need to start without plugins if the MS is not sped up
				//	return sceKernelStartModule(modid, argsize, argp, status, option);
			}
		}

		int var_sp_p4 = 0;
		SceUID fd = -1;
		while(1)
		{
			fd = sceIoOpen(plugins_file, PSP_O_RDONLY, 0);
			if(fd >= 0)
			{
				break;
			}
			sceKernelDelayThread(20000);
			var_sp_p4 = var_sp_p4 + 1;
			if(var_sp_p4 >= 16)
			{
				res = sceKernelStartModule(modid, argsize, argp, status, option);
			}
		}

		unkfpl = sceKernelCreateFpl("", PSP_MEMORY_PARTITION_KERNEL, 0, 1024, 1, NULL);
		if(unkfpl < 0)
		{
			res = sceKernelStartModule(modid, argsize, argp, status, option);
		}
		sceKernelAllocateFpl(unkfpl, &buffer, NULL);
		int size = sceIoRead(fd, buffer, 1024);

		for(ret = 1; ret > 0;)
		{
			activated = 0;
			memset(plugin, 0, 64);
			ret = GetPlugin(buffer, size, plugin, &activated);
			buffer = buffer + ret;
			size = size - ret;
			if(ret <= 0)
			{
				sceIoClose(fd);
				break;
			}
			if(activated == 0)
			{
				continue;
			}
			else
			{
				LoadStartModule(plugin);
				continue;
			}
		}
	}
	else
	{
		if(sceKernelApplicationType() != PSP_INIT_KEYCONFIG_VSH || argsize != 0 || config.hidecorrupt == 0)
		{
			res = sceKernelStartModule(modid, argsize, argp, status, option);
		}
		if(config.skiplogo)
		{
			u32* vshmain_args = oe_malloc(1024);
			memset(vshmain_args, 0, 1024);
			vshmain_args[0] = 1024;
			vshmain_args[1] = 0x20;
			vshmain_args[16] = 1;
			vshmain_args[160] = 1;
			vshmain_args[161] = 0x40003;
			res = sceKernelStartModule(modid, 1024, vshmain_args, status, option);
			oe_free(vshmain_args);
		}
	}

	sceKernelFreeFpl(unkfpl);
	sceKernelDeleteFpl(unkfpl);
	return res;
}

void *block;
int drivestate = SCE_UMD_READY | SCE_UMD_MEDIA_IN; // 0x0000A690
SceUID umdcallback; // 0x0000A6C8

// OK 0x00001C54
void UmdCallback(int drivestat)
{
	if (umdcallback >= 0)
	{
		sceKernelNotifyCallback(umdcallback, drivestat);
	}
}

// OK
int sceUmdActivatePatched(const int mode, const char *aliasname)
{
	int k1 = pspSdkSetK1(0);
	sceIoAssign(aliasname, "umd0:", "isofs0:", IOASSIGN_RDONLY, NULL, 0);
	pspSdkSetK1(k1);
	drivestate = 0x00000032;
	if (!((drivestate & 0x00000020) != 0x00000000))
	{
		UmdCallback(SCE_UMD_READABLE);
	}
	return 0;
}

// OK
int sceUmdDeactivatePatched(const int mode, const char *aliasname)
{
	int k1 = pspSdkSetK1(0);
	sceIoUnassign(aliasname);
	drivestate = SCE_UMD_MEDIA_IN | SCE_UMD_READY;
	UmdCallback(drivestate);
	pspSdkSetK1(k1);
	return 0;
}

// OK
int sceUmdGetDiscInfoPatched(SceUmdDiscInfo *disc_info)
{
	int k1 = pspSdkSetK1(0);
	disc_info->uiSize = 8;
	disc_info->uiMediaType = SCE_UMD_FMT_GAME;
	pspSdkSetK1(k1);
	return 0;
}

// OK
int sceUmdRegisterUMDCallBackPatched(SceUID cbid)
{
	int k1 = pspSdkSetK1(0);
	umdcallback = cbid;
	UmdCallback(drivestate);
	pspSdkSetK1(k1);
	return 0;
}

// OK
int sceUmdUnRegisterUMDCallBackPatched(SceUID cbid)
{
	int k1 = pspSdkSetK1(0);
	umdcallback = -1;
	pspSdkSetK1(k1);
	return 0;
}

// OK
int sceUmdGetDriveStatPatched()
{
	int k1 = pspSdkSetK1(0);
	int ret = drivestate;
	pspSdkSetK1(k1);
	return ret;
}

// OK
u32 FindUmdUserFunction(u32 nid)
{
	return sctrlHENFindFunction("sceUmd_driver", "sceUmdUser", nid);
}

// OK
void PatchUmdMan(u32 text_addr)
{
	if (sceKernelBootFrom() == PSP_BOOT_MS)
	{
		// Replace call to sceKernelBootFrom with return PSP_BOOT_DISC
		_sw(0x24020020, text_addr+0x431C);
	}
	ClearCaches();
}

int LoadRebootex(u8 *dest, u32 destSize, const u8 *src, void *unk, void *unk2)
{
	int v1 = 0x88FB0000;
	*((char *) 0x88FB0000) = 0;

	for(v1 = v1; v1 < (void*)0x88FB00D0; v1++)
	{
		((char *) v1)[0] = 0;
	}

	char* theiso = sctrlSEGetUmdFile();
	if (theiso)
	{
		strcpy((void *)0x88FB0000, theiso);
	}

	SEConfig rebootcfg = config;

	for(rebootcfg = rebootcfg; rebootcfg < (void*)0x0000A790; rebootcfg+=0x10)
	{
		(void*)0x88FB0050 += 0x10;
	}

	memcpy((void*)0x88FB0050, ((int *) rebootcfg)[0], 0x03);
	memcpy((void*)0x88FB0050, ((int *) rebootcfg)[0], 0x00);

	((void*)0x88FB00C0) = fileindex;
	((void*)0x88FB00CC) = unk;
	((void*)0x88FB00D0) = reboot_module_after;
	((void*)0x88FB00D4) = reboot_buf;
	((void*)0x88FB00D8) = reboot_size;
	((void*)0x88FB00DC) = reboot_flags;
	sceKernelGzipDecompress(0x88FC0000, 0x4000, rebootex, NULL); // rebootex = 0x00009B94

	return;
}

// OK
void PatchLoadExec(SceModule2 *mod)
{
	_sw(0x3C0188FC, mod->text_addr+0x2820);// OK

	MAKE_CALL(mod->text_addr+0x27DC, LoadRebootex);// OK
	UtilsForKernel_7dd07271 = (void *)(mod->text_addr+0x1BCC);

	// Allow LoadExecVSH in whatever user level
	_sw(0x1000000B, mod->text_addr+0x1EA8);// OK
	_sw(0, mod->text_addr+0x1EE8);// OK

	// Allow ExitVSHVSH in whatever user level
	_sw(0x10000008, mod->text_addr+0x1494);// OK
	_sw(0, mod->text_addr+0x14C8);// OK
}

// Soon OK
void PatchInitLoadExecAndMediaSync(u32 text_addr)
{
	char *filename = sceKernelInitFileName();
	if (filename == NULL)
	{
		sctrlSESetUmdFile("");
		fileindex = 0;
	}
	else
	{
		if (strstr(filename, ".PBP") == 0)
		{
			if (strstr(filename, "disc") != filename){
				sctrlSESetUmdFile("");
				fileindex = 0;
			}
	
			char *theiso = sctrlSEGetUmdFile();
			if (theiso)
			{
				if (strncmp(theiso, "ms0:/", 5) != 0){
					sctrlSESetUmdFile("");
					fileindex = 0;
				}

				if (config.umdmode != 0)
				{
					if (config.umdmode != 1)
					{
						_sw(0x00008021, text_addr+0x94);
					}
					else
					{
						sceIoDelDrv(0x000078A8);
						sceIoAddDrv(sub_04588());
						_sw(0x34020000, text_addr+0x8C);

						if (!config.umdmode)
						{
							_sw(0x00008021, text_addr+0x94);
						}
					}
				}
				else
				{
					DoAnyUmd();
					_sw(0x00008021, text_addr+0x94);
				}
			}
		}
		else
		{
			_sw(0x00008021, text_addr+0x628);
			_sw(0x00008021, text_addr+0x528);
			ClearCaches();
			sctrlSESetUmdFile("");
			fileindex = 0;
		}
	}

	PatchLoadExec(sceKernelFindModuleByName("sceLoadExec"));
	PatchMesgLed();
	ClearCaches();

	if (unk)
	{
		sctrlHENFindFunction("sceClockgen_Driver", "sceClockgen_driver", 0x777FF2D9);
	}
}

// OK
void PatchIsofsDriver(u32 text_addr)
{
	char *iso = sctrlSEGetUmdFile();

	if (iso)
	{
		if (strstr(iso, "ms0:/") == iso)
		{
			if (config.umdmode)
			{
				// make module exit inmediately
				_sw(0x03E00008, text_addr+0x4328);
				_sw(0x24020001, text_addr+0x432C);
				ClearCaches();
			}
		}
	}
}

// Probably OK
void PatchPower(u32 text_addr)
{
	_sw(NOP, text_addr+0xCA8);
	ClearCaches();
}

// OK
void PatchWlan(u32 text_addr)
{
	_sw(NOP, text_addr+0x25F4);
	ClearCaches();
}

// OK
void DoNoUmdPatches()
{
	REDIRECT_FUNCTION(FindUmdUserFunction(0xC6183D47), sceUmdActivatePatched);
	REDIRECT_FUNCTION(FindUmdUserFunction(0xE83742BA), sceUmdDeactivatePatched);
	REDIRECT_FUNCTION(FindUmdUserFunction(0x340B7686), sceUmdGetDiscInfoPatched);
	REDIRECT_FUNCTION(FindUmdUserFunction(0xAEE7404D), sceUmdRegisterUMDCallBackPatched);
	REDIRECT_FUNCTION(FindUmdUserFunction(0xBD2BDE07), sceUmdUnRegisterUMDCallBackPatched);
	MAKE_DUMMY_FUNCTION1(FindUmdUserFunction(0x46EBB729)); // sceUmdCheckMedium
	MAKE_DUMMY_FUNCTION0(FindUmdUserFunction(0x8EF08FCE)); // sceUmdWaitDriveStat
	MAKE_DUMMY_FUNCTION0(FindUmdUserFunction(0x56202973)); // sceUmdWaitDriveStatWithTimer
	MAKE_DUMMY_FUNCTION0(FindUmdUserFunction(0x4A9E5E29)); // sceUmdWaitDriveStatCB
	REDIRECT_FUNCTION(FindUmdUserFunction(0x6B4A146C), sceUmdGetDriveStatPatched);
	MAKE_DUMMY_FUNCTION0(FindUmdUserFunction(0x20628E6F)); // sceUmdGetErrorStat
	ClearCaches();
}

// OK
int sceChkregGetPsCodePatched(u8 *pscode)
{
	pscode[0] = 0x01;
	pscode[1] = 0x00;
	pscode[2] = config.fakeregion + 2;

	u8 *var3 = (config.fakeregion + 0xFFFFFFF5) & 0x000000FF;
	if (!(((config.fakeregion < 0x0000000C)) == 0x00000000))
	{
		var3 = (config.fakeregion + 0x00000002) & 0x000000FF;
	}
	pscode[2] = var3;
	if (pscode[2] == 2)
		pscode[2] = 3;

	pscode[6] = 0x01;
	pscode[7] = 0x00;
	pscode[3] = 0x00;
	pscode[4] = 0x01;
	pscode[5] = 0x00;

	return 0;
}

// OK
void PatchChkreg()
{
	u32 pscode = sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x59F8491D);
	int (* sceChkregGetPsCode)(u8 *);
	u8 code[8];

	if (pscode)
	{
		if (config.fakeregion) // 0x0000A754
		{
			REDIRECT_FUNCTION(pscode, sceChkregGetPsCodePatched);
		}
	}
	ClearCaches();
}

// 0x0000189C
void PatchLowIo(int arg1)
{
	((short *) (arg1 + 0x00008F22))[0] = 0xFFFFAC60;
	//_sw(0xFFFFAC60, ((short *) (arg1 + 0x00008F22))[0]);
	ClearCaches();
}

void sctrlHENSetSpeed(int cpu, int bus)
{
	scePowerSetClockFrequency_k = (void*)FindPowerFunction(0x545A7F3C);
	SetSpeed(cpu, bus);
}

void SetSpeed(int cpu, int bus)
{
	if (cpu == 0x00000014 || cpu == 0x0000004B || cpu == 0x00000064 || cpu == 0x00000085
		|| cpu == 0x0000014D || cpu == 0x0000012C || cpu == 0x0000010A)
	{
		scePowerSetClockFrequency_ = (void*)FindPowerFunction(0x737486F2);
		scePowerSetClockFrequency_k = scePowerSetClockFrequency_;
		scePowerSetClockFrequency_(cpu, cpu, bus);
		MAKE_DUMMY_FUNCTION0((u32)scePowerSetClockFrequency_k);
		MAKE_DUMMY_FUNCTION0((u32)FindPowerFunction(0x545A7F3C));
		MAKE_DUMMY_FUNCTION0((u32)FindPowerFunction(0xB8D7B3FB));
		MAKE_DUMMY_FUNCTION0((u32)FindPowerFunction(0x843FBF43));
		MAKE_DUMMY_FUNCTION0((u32)FindPowerFunction(0xEBD177D6));
		ClearCaches();
	}
}

// OK
void OnImposeLoad()
{
	sctrlSEGetConfig(&config);
	PatchChkreg();

	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_GAME)
	{
		char *theiso = sctrlSEGetUmdFile();

		if (theiso)
		{
			if (strncmp(theiso, "ms0:/", 5) == 0)
			{
				if (config.umdmode)
				{
					sceIoDelDrv("isofs");
					sceIoAddDrv(getisofs_driver());

					if (config.umdmode)
					{
						DoNoUmdPatches();
					}

					sceIoAssign("disc0:", "umd0:", "isofs0:", IOASSIGN_RDONLY, NULL, 0);
				}
			}
		}
	}
	ClearCaches();
}


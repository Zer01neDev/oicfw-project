/*
    Open Idea Custom Firmware
    Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

    main.c: OpenIdea plutonium_updater Main Code
    This file are based from a reverse of M33/GEN plutonium_updater

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

#include <pspctrl.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psputils.h>
#include <pspkernel.h>
#include <pspvshbridge.h>
#include <pspiofilemgr.h>

#include <systemctrl.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define printf pspDebugScreenPrintf

PSP_MODULE_INFO("plutonium_updater", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);


u8 digest[0x10];
SceKernelUtilsMd5Context ctx;

extern int size_Plutonium_Driver;
extern unsigned char Plutonium_Driver[];

extern int size_pspDegeneration_Driver;
extern unsigned char pspDegeneration_Driver[];

int pspDegeneration_843F274A();
int pspDegeneration_9FA0B53E(u32 arg1);

void Plutonium_340AC1E4();
void Plutonium_F8547F11();
void Plutonium_E30ED0F0();

SceUID vshKernelLoadModuleBufferVSH(SceSize bufsize, void *buf, int flags, SceKernelLMOption *option); 

int StopUnloadModule(SceUID modid)
{
	int ret = sceKernelStopModule(modid, 0, NULL, NULL, NULL);
	if (ret < 0) return -1;
	else ret = sceKernelUnloadModule(modid);
	return ret;
}

int LoadStartModule(char* file)
{
	SceUID mod = vshKernelLoadModuleVSH(file, 0, NULL);
	if (mod < 0) return mod;
	else mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
	return mod;
}

int LoadStartModuleBuffer(void* buffer, SceSize bufsize, SceSize args, void *argp)
{
	SceUID mod = vshKernelLoadModuleBufferVSH(bufsize, buffer, 0, NULL);
	if (mod < 0) return mod;
	else mod = sceKernelStartModule(mod, args, argp, NULL, NULL);//(!(argp) ? 0x0 : 0x4)
	return mod;
}

int AssignFlash()
{
	int ret;
	int assign = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);
	if (assign < 0)
	{
		ret = -1;
	}
	else
	{
		assign = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);
		if (assign < 0) ret = -1;
		else ret = 0;
	}
	return ret;
}

int UnassignFlash()
{
	int unassign = sceIoUnassign("flash0:");
	if (unassign >= 0)
	{
		unassign = sceIoUnassign("flash1:");
	}
	return unassign;
}

u8 buf[0x10];

int ReadFile(char *file, u32 seek, void *buf, int size)
{
	int read;
	int seekr;
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
	if (fd < 0)
	{
		fd = -1;
		sceIoClose(fd);
	}
	else
	{
		if (seek <= 0)
		{
			read = sceIoRead(fd, buf, size);
			sceIoClose(fd);
		}
		else
		{
			int var6 = seek >> 0x1F;
			seekr = sceIoLseek(fd, var6, PSP_SEEK_SET);
			if (seekr == seek)
			{
				if (!(seekr != var6))
				{
					read = sceIoRead(fd, buf, size);
					sceIoClose(fd);
				}
				fd = -1;
				sceIoClose(fd);
			}
			else
			{
				fd = -1;
				sceIoClose(fd);
			}
		}
	}
	return read;
}

void ErrorExit(int delay, char *fmt, ...)
{
	va_list list;
	char msg[256];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);
	printf(msg);

	sceKernelDelayThread(delay*1000);
	sceKernelExitGame();
}

int CheckMD5(u8 *md5, u8 *digest, u32 size)
{
	int i;
	int ret;
	if (!size)
	{
		ret = 0;
	}
	else
	{
		for(i = 0; i < 0x10; i++)
		{
			if(md5[i] == digest[i])//memcmp(md5[i], digest[i], 1) != 0)
				ret = 1;
			else
				ret = 0;
		}
	}
	return ret;
}

int CheckBuffer(char *name, u8 *buf, u8 *md5, u32 size)
{
	printf("Verifying %s buffer... ", name);

	sceKernelUtilsMd5BlockInit(&ctx);
	sceKernelUtilsMd5BlockUpdate(&ctx, buf, size);
	sceKernelUtilsMd5BlockResult(&ctx, digest);

	if (CheckMD5(md5, digest, 0x10) != 1)
		ErrorExit(5000, "%s buffer as been corrupted... ", name);
}

void CheckFile(char *file, u8 *md5)
{
	int read;

	printf("Verifying %s... ", file);

	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
	if (fd < 0)
		ErrorExit(5000, "Cannot open file (0x%08X).\n", fd);

	sceKernelUtilsMd5BlockInit(&ctx);

	while ((read = sceIoRead(fd, buf, 4096)) > 0)
	{
		sceKernelUtilsMd5BlockUpdate(&ctx, buf, read);
	}

	sceIoClose(fd);

	sceKernelUtilsMd5BlockResult(&ctx, digest);

	if (CheckMD5(md5, digest, 0x10) != 1)
		ErrorExit(5000, "Incorrect file.\n");
}

u8 u235_md5[0x10] =
{

};

u8 update_md5[0x10] =
{

};

u8 degeneration_md5[0x10] =
{

};

u8 plutonium_md5[0x10] =
{

};

int main_thread(SceSize args, void *argp)
{
	int v0;
	int ids;
	int assign;
	SceCtrlData pad;
	int bypass_bat_check = 0;

	sceCtrlReadBufferPositive(&pad, 1);

	if (pad.Buttons & PSP_CTRL_LTRIGGER && PSP_CTRL_TRIANGLE)
	{
		bypass_bat_check = 1;
	}

	printf("OpenIdea Updater By OpenIdea Project Team (Zer01ne/dridri85)\n\n");
	sceIoChdir("ms0:/PSP/GAME/UPDATE");
	if (sceKernelDevkitVersion() < 0x0306000F)
	{
		if (ReadFile("flash0:/kd/systemctrl.prx", 0x150, buf, 0x10) < 0)
		{
			ErrorExit(5000, "This program requires 3.52 M33-3 or higher.\n");
		}

		if ((buf[0] == 0x1F && buf[1] == 0x8B) || (sceKernelDevkitVersion() < 0x0305020F))
		{
			ErrorExit(5000, "This program requires 3.52 M33-3 or higher.\n");
		}
	}

	if (!bypass_bat_check)
	{
		if (scePowerGetBatteryLifePercent() < 78)
		{
			ErrorExit(5000, "Battery has to be at least at 78%%.\n");
		}
	}
	assign = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, NULL, 0);
	if (assign == 0x80010005)
	{
		label52:
		printf("Flash2 is unformatted.\n");
		printf("Formatting flash2.... ");
		if (LoadStartModule("flash0:/kd/lflash_fatfmt.prx") < 0)
		{
			ErrorExit(0x000001F4, "Error loading lflash_fatfmt.");
		}
		if (UnassignFlash() < 0)
		{
			ErrorExit(5000, "Error unassigning flashes.");
		}

		char *argv[2];
		argv[0] = "fatfmt";
		argv[1] = "lflash0:0,2";

		if (vshLflashFatfmtStartFatfmt(2, argv) < 0)
		{
			ErrorExit(5000, "Error in lflash_fatfmt.");
		}

		if (AssignFlash() < 0)
		{
			ErrorExit(5000, "Error reassigning flashes.");
		}

		sceKernelDelayThread(200000);
		struct SceKernelLoadExecVSHParam param;
		printf("Restarting app...\n");
		memset(&param, 0, sizeof(param));
		param.size = sizeof(param);
		param.args = strlen("ms0:/PSP/GAME/UPDATE/PUPD.PBP")+1;
		param.argp = "ms0:/PSP/GAME/UPDATE/PUPD.PBP";
		param.key = "updater";
		v0 = sctrlKernelLoadExecVSHMs1("ms0:/PSP/GAME/UPDATE/PUPD.PBP", &param);
	}
	else
	{
		if (assign == SCE_KERNEL_ERROR_NODEV)
			goto label52;

		if (assign >= 0)
		{
			v0 = sceIoUnassign("flash2:");
		}
	}
	if ((v0 & 0x9) == 0x9)
	{
		CheckFile("550.PBP", update_md5);
		CheckFile("u235.prx", u235_md5);
		CheckBuffer("pspDegeneration_Driver", pspDegeneration_Driver, degeneration_md5, size_pspDegeneration_Driver);
		CheckBuffer("Plutonium_Driver", Plutonium_Driver, plutonium_md5, u32 size_Plutonium_Driver);
	}
	int mod = LoadStartModuleBuffer(pspDegeneration_Driver, size_pspDegeneration_Driver, 0, NULL);
	if (mod >= 0)
	{
		ids = pspDegeneration_843F274A();
		if (ids >= 0)
		{
			if (ids != 0x00000000)
			{
				printf("Some of your idstorage keys have not the correct values, probably due to a downgrader. In order to continue, press X to restore the correct values, press R to exit.\n\n");
				while (1)
				{
					sceCtrlReadBufferPositive(&pad, 1);

					if (pad.Buttons & PSP_CTRL_CROSS)
						break;
					if (pad.Buttons & PSP_CTRL_RTRIGGER)
						ErrorExit(5000, "Update canceled by user.\n");
					sceKernelDelayThread(10000);
				}
				if (pspDegeneration_9FA0B53E(ids) < 0)
				{
					ErrorExit(5000, "Fail in idstorage correction!\n");
				}
				sceKernelDelayThread(800000);
				printf("Idstorage corrected succesfully.\n\n");
			}
		}
		else
		{
			ErrorExit(5000, "Error 0x%08X checking idstorage degeneration.\n");
		}
	}
	else
	{
		ErrorExit(5000, "Error 0x%08X loading pspDegeneration_Driver.\n", mod);
	}
	printf("Press X to start the update, R to exit.\n\n");
	while (1)
	{
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
			break;
		if (pad.Buttons & PSP_CTRL_RTRIGGER)
			ErrorExit(5000, "Update canceled by user.\n");
		sceKernelDelayThread(10000);
	}
	argp = (int*)bypass_bat_check;
	if (StopUnloadModule(mod) < 0)
	{
		ErrorExit(5000, "Error unloading degeneration.\n");
		if (LoadStartModuleBuffer(Plutonium_Driver, size_Plutonium_Driver, 0, NULL) < 0)
			ErrorExit(5000, "Error 0x%08X loading Plutonium_Driver.\n");
	}
	else
	{
		if (LoadStartModuleBuffer(Plutonium_Driver, size_Plutonium_Driver, 0, NULL) < 0)
			ErrorExit(5000, "Error 0x%08X loading Plutonium_Driver.\n");

		if (LoadStartModule("Plutonium_Touch.prx") < 0)
			ErrorExit(5000, "Error 0x%08X loading Plutonium_Touch.\n");
	}
	printf("Starting sce updater. Wait...\n");
	sceKernelDelayThread(500000);
	Plutonium_F8547F11(0, 0);
	sceKernelExitDeleteThread(0);
	return 0;
}

int module_start(SceSize args, void *argp)
{
	pspDebugScreenInit();
	SceUID thid = sceKernelCreateThread("user_main", main_thread, 0x10, (0x4000 << 0xA), PSP_THREAD_ATTR_VSH, NULL);//(0x4000 << 0xA)
	if(thid >= 0)
		sceKernelStartThread(thid, args, argp);
	else
		main_thread(args, argp);
	return 0;
}


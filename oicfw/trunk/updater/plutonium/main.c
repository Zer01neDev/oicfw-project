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

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspvshbridge.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

PSP_MODULE_INFO("plutonium_updater", 0x800, 0, 1);

extern int size_plutonium;
extern unsigned char plutonium[];

int PlutoniumGetModel();
int PlutoniumStartUpdater();
SceUID vshKernelLoadModuleBufferVSH(SceSize bufsize, void *buf, int flags, SceKernelLMOption *option); 

int Plutonium_340AC1E4();
int Plutonium_E30ED0F0();
int Plutonium_F8547F11();

int LoadStartModuleBuffer(void *buffer, int size, int a2);
SceUID LoadStartModule(const char* file, SceSize args, void* argp, int kernel);

void ErrorExit(int milisecs, char *fmt, ...)
{
	va_list list;
	char msg[256];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	pspDebugScreenPrintf(msg);

	sceKernelDelayThread(milisecs*1000);
	sceKernelExitGame();
}

int main(int argc, char** argv)
{
	u32 mod;
	SceCtrlData pad;
	pspDebugScreenInit();
	pspDebugScreenPrintf("OpenIdea Updater By OpenIdea Project Team (Zer01ne/dridri85)\n\n");
	pspDebugScreenPrintf("Press X to start the update, R to exit.\n\n");

	while(1)
	{
		sceCtrlReadBufferPositive(&pad, 1);
		if(pad.Buttons & PSP_CTRL_RTRIGGER)
		{
			ErrorExit(6000, "Update canceled by user.");
		}
		if(pad.Buttons & PSP_CTRL_CROSS)
		{
			pspDebugScreenPrintf("Loading/Starting Plutonium_Driver...");
			//LoadStartModule("ms0:/PSP/GAME/UPDATE/plutonium_da.prx", 0, NULL, 1);
			mod = LoadStartModuleBuffer(plutonium, size_plutonium, 0);
			if(mod < 0) {
				ErrorExit(6000, "\nError 0x%08X Loading/Starting Plutonium_Driver.\n", mod);
			}
			else {
				pspDebugScreenPrintf("OK\n");
				pspDebugScreenPrintf("Loading/Starting Sony Updater Wait...");
				sceKernelDelayThread(1000*1000*1.5);
				PlutoniumGetModel();
				PlutoniumStartUpdater();
				//Plutonium_340AC1E4();
				//Plutonium_F8547F11();
			}
			break;
		}
	}
	return 0;
}

int LoadStartModuleBuffer(void *buffer, int size, int a2)
{
    SceUID v0 = vshKernelLoadModuleBufferVSH(size, buffer, 0, NULL);
    if(v0 < 0) return -1;
    if(a2 == 0) return sceKernelStartModule(v0, 0, 0, NULL, NULL);
    return 0;
} 

SceUID LoadStartModule(const char* file, SceSize args, void* argp, int kernel)
{
	SceKernelLMOption option;
	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);
	option.mpidtext = kernel;
	option.mpiddata = kernel;
	option.position = 0;
	option.access = 1;

	SceUID modid, dev;
	modid = sceKernelLoadModule(file, 0, kernel>0?&option:NULL);
	if (modid >= 0){
		dev = sceKernelStartModule(modid, args, argp, NULL, NULL);
		if(dev < 0){
			
			return dev;
		}
	}else{
		
		return modid;
	}
	return modid;
}


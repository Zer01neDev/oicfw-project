/*
    Open Idea Custom Firmware
    Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

    main.c: OpenIdea Updater Main Code
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

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspvshbridge.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

PSP_MODULE_INFO("OpenIdeaUpdater", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

extern int size_nuclear;
extern unsigned char nuclear[];

int vshKernelGetModel();
int NuclearStartUpdater();
SceUID vshKernelLoadModuleBufferVSH(SceSize bufsize, void *buf, int flags, SceKernelLMOption *option); 

int LoadStartModuleBuffer(void *buffer, int size);

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

int LoadStartModuleBuffer(void *buffer, int size)
{
    SceUID mod = vshKernelLoadModuleBufferVSH(size, buffer, 0, NULL);
    if(mod < 0) return -1;
    return sceKernelStartModule(mod, 0, 0, NULL, NULL);
}

int ReadFile(char *file, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
	if (!fd) return -1;
	int rd = sceIoRead(fd, buf, size);
	sceIoClose(fd);
	return rd;
}

u8 digest[16];

u8 md5_nuclear[0x10] =
{
	
};

int main(int argc, char** argv)
{
	u32 mod;
	char buffer[5];
	SceCtrlData pad;

	pspDebugScreenInit();

	if(sceKernelDevkitVersion() < 0x0306000F)
	{
		ReadFile("flash0:/kd/systemctrl.prx", buffer, 4);

		if (memcmp(buffer, "~PSP", 4) != 0)
			ErrorExit(5000, "This program requires 3.52 M33-3 or higher.\n");
	}

	int model = vshKernelGetModel();
	if(model != 0 && model != 1)
	{
		ErrorExit(5000, "This program only For PSP Fat/Slim.\n");
	}

	/*pspDebugScreenPrintf("Checking Nuclear_Driver...");

	if (sceKernelUtilsMd5Digest((u8 *)nuclear, size_nuclear, digest) < 0)
	{
		ErrorExit(4000, "Error for check Nuclear_Driver.\n");
	}
	if (memcmp(digest, md5_nuclear, 16) != 0)
		ErrorExit(4000, "Error Nuclear_Driver is corrupt.\n");
	else
		ErrorExit(4000, "OK\n");

	sceKernelDelayThread(1000*1000*1.5);
	pspDebugScreenClear();*/

	pspDebugScreenPrintf("OpenIdea Updater By OpenIdea Project Team (Zer01ne/dridri85)\n\n");
	pspDebugScreenPrintf("Press X to start the update, R to exit.\n\n");

	while(1)
	{
		sceCtrlReadBufferPositive(&pad, 1);
		if(pad.Buttons & PSP_CTRL_RTRIGGER)
		{
			ErrorExit(5000, "Update canceled by user.");
		}
		if(pad.Buttons & PSP_CTRL_CROSS)
		{
			pspDebugScreenPrintf("Loading/Starting Nuclear_Driver...");
			mod = LoadStartModuleBuffer(nuclear, size_nuclear);
			if(mod < 0)
			{
				ErrorExit(5000, "\nError 0x%08X Loading/Starting Nuclear_Driver.\n", mod);
			}
			else
			{
				pspDebugScreenPrintf("OK\n");
				pspDebugScreenPrintf("Loading/Starting Sony Updater Wait...");
				sceKernelDelayThread(1000*1000*1.5);
				NuclearStartUpdater();
			}
			break;
		}
	}
	return 0;
}


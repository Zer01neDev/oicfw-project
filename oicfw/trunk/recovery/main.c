/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	main.c: Recovery Main Code
	This file are based from a reverse of M33/GEN Recovery
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
#include <pspreg.h>
#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <pspusbdevice.h>
#include <pspvshbridge.h>
#include <pspsysmem_kernel.h>
#include <psputility_sysparam.h>

#include <systemctrl.h>

#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "mydebug.h"
#include "en_recovery.h"
#include "../vshctrl/systemctrl_se.h"

PSP_MODULE_INFO("Recovery mode", 0x0800, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);


int sprintf(char *buf, const char *fmt, ...)
    __attribute__((format(printf,2,3)));

#define printf myDebugScreenPrintf
#define RGB(r, g, b) (0xFF000000 | ((b)<<16) | ((g)<<8) | (r))
#define setTextColor myDebugScreenSetTextColor
#define setBackColor myDebugScreenSetBackColor
#define setXY myDebugScreenSetXY
#define delay sceKernelDelayThread
#define clearScreen myDebugScreenClear

SEConfig config;
int configchanged = 0;

int usbStarted = 0;
int usbModuleStatus = 0;

int LoadStartModule(char *module)
{
	SceUID mod = vshKernelLoadModuleVSH(module, 0, NULL);

	if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
		return mod;

	if (mod >= 0)
	{
		mod = sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
		if (mod < 0)
			return mod;
	}

	return 0;
}

void GetSpeed(int *cpuSpeed, int *busSpeed, int *changed)
{
	if (*cpuSpeed != 333 && *cpuSpeed != 266 && *cpuSpeed != 222 && *cpuSpeed != 300 && *cpuSpeed != 133
		&& *cpuSpeed != 100 && *cpuSpeed != 75 && *cpuSpeed != 20 && *cpuSpeed != 0)
	{
		*cpuSpeed = 0;
		*changed = 1;
	}

	if (*busSpeed != 166 && *busSpeed != 133 && *busSpeed != 111 && *busSpeed != 66 && *busSpeed != 50 
		&& *busSpeed != 37 && *busSpeed != 10 && *busSpeed != 150 && *busSpeed != 150)
	{
		*busSpeed = 0;
		*changed = 1;
	}
}

void SetSpeed(int *cpuSpeed, int *busSpeed)
{
	if (*cpuSpeed == 0)
	{
		*cpuSpeed = 20;
		*busSpeed = 10;
	}
	else if (*cpuSpeed  == 20)
	{
		*cpuSpeed = 75;
		*busSpeed = 37;
	}
	else if (*cpuSpeed  == 75)
	{
		*cpuSpeed = 100;
		*busSpeed = 50;
	}
	else if (*cpuSpeed  == 100)
	{
		*cpuSpeed = 133;
		*busSpeed = 66;
	}
	else if (*cpuSpeed == 133)
	{
		*cpuSpeed = 222;
		*busSpeed = 111;
	}
	else if (*cpuSpeed == 222)
	{
		*cpuSpeed = 266;
		*busSpeed = 133;
	}
	else if (*cpuSpeed == 266)
	{
		*cpuSpeed = 300;
		*busSpeed = 150;
	}
	else if (*cpuSpeed  == 300)
	{
		*cpuSpeed = 333;
		*busSpeed = 166;
	}
	else if (*cpuSpeed  == 333)
	{
		*cpuSpeed = 0;
		*busSpeed = 0;
	}
}

// Init Random Function 
void InitRandom(SceKernelUtilsMt19937Context *ctx)
{
	u32 tick = sceKernelGetSystemTimeLow();
	sceKernelUtilsMt19937Init(ctx, tick);
}

// Get Random Function
int GetRandom(int min, int max, SceKernelUtilsMt19937Context *ctx)
{
	return (sceKernelUtilsMt19937UInt(ctx) % (max - min + 1)) + min;
}

void trim(char *str)
{
	int i;
	int len = strlen(str);

	for (i = len-1; i >= 0; i--)
	{
		if (str[i] == 0x20 || str[i] == '\t')
			str[i] = 0;
		else
			break;
	}
}

int SetPlugin(SceUID fd, char *str, int *activated)
{
	int n = 0;
	char ch = 0;
	char *s = str;

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

	trim(s);

	*activated = 0;

	char *p = strpbrk(s, " \t");
	if (p)
	{
		char *q = p+1;

		while (*q < 0) q++;

		if (strcmp(q, "1") == 0)
			strcpy(q, "0");
		if (strcmp(q, "0") == 0)
			strcpy(q, "1");

		*p = 0;   
	}
}

typedef struct
{
	char plugins[10][64];
	int *activated;
} Plugins;

int GetPlugin(SceUID fd, char *str, int *activated)
{
	int n = 0;
	char ch = 0;
	char *s = str;

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

	//trim(s);

	*activated = 0;

	char *p = strrchr(s, ' ');
	if (p)
	{
		if (strcmp(p+1, "1") == 0)
			*activated = 1;
		if (strcmp(p+1, "0") == 0)
			*activated = 0;
		*p = 0;
	}
}

int get_registry_value(const char *dir, const char *name, unsigned int *val)
{
    int ret = 0;
    struct RegParam reg;
    REGHANDLE h;

    memset(&reg, 0, sizeof(reg));
    reg.regtype = 1;
    reg.namelen = strlen("/system");
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy(reg.name, "/system");
    if(sceRegOpenRegistry(&reg, 2, &h) == 0)
    {
        REGHANDLE hd;
        if(!sceRegOpenCategory(h, dir, 2, &hd))
        {
            REGHANDLE hk;
            unsigned int type, size;

            if(!sceRegGetKeyInfo(hd, name, &hk, &type, &size))
            {
                if(!sceRegGetKeyValue(hd, hk, val, 4))
                {
                    ret = 1;
                    sceRegFlushCategory(hd);
                }
            }
            sceRegCloseCategory(hd);
        }
        sceRegFlushRegistry(h);
        sceRegCloseRegistry(h);
    }
    return ret;
}

int set_registry_value(const char *dir, const char *name, unsigned int val)
{
    int ret = 0;
    struct RegParam reg;
    REGHANDLE h;

    memset(&reg, 0, sizeof(reg));
    reg.regtype = 1;
    reg.namelen = strlen("/system");
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy(reg.name, "/system");
    if(sceRegOpenRegistry(&reg, 2, &h) == 0)
    {
        REGHANDLE hd;
        if(!sceRegOpenCategory(h, dir, 2, &hd))
        {
            if(!sceRegSetKeyValue(hd, name, &val, 4))
            {
                ret = 1;
                sceRegFlushCategory(hd);
            }
			else
			{
				sceRegCreateKey(hd, name, REG_TYPE_INT, 4);
				sceRegSetKeyValue(hd, name, &val, 4);
				ret = 1;
                sceRegFlushCategory(hd);
			}
            sceRegCloseCategory(hd);
        }
        sceRegFlushRegistry(h);
        sceRegCloseRegistry(h);
    }
	return ret;
}

int ReadLine(SceUID fd, char *str)
{
	int n = 0;
	char ch = 0;

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

char string[71][71];

void GetLanguage()
{
	//de  nl
	int i;
	SceUID fd;
	u32 language;
	char language_txt[40];
	get_registry_value("/CONFIG/SYSTEM/XMB", "language", &language);

	if(language == PSP_SYSTEMPARAM_LANGUAGE_JAPANESE)
		strcpy(language_txt, "ms0:/seplugins/ja_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_ENGLISH)
		strcpy(language_txt, "ms0:/seplugins/en_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_FRENCH)
		strcpy(language_txt, "ms0:/seplugins/fr_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_SPANISH)
		strcpy(language_txt, "ms0:/seplugins/es_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_GERMAN)
		strcpy(language_txt, "ms0:/seplugins/.._recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_ITALIAN)
		strcpy(language_txt, "ms0:/seplugins/it_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_DUTCH)
		strcpy(language_txt, "ms0:/seplugins/.._recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_PORTUGUESE)
		strcpy(language_txt, "ms0:/seplugins/pt_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_RUSSIAN)
		strcpy(language_txt, "ms0:/seplugins/ru_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_KOREAN)
		strcpy(language_txt, "ms0:/seplugins/ko_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL)
		strcpy(language_txt, "ms0:/seplugins/ch1_recovery.txt");
	if(language == PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED)
		strcpy(language_txt, "ms0:/seplugins/ch2_recovery.txt");

	fd = sceIoOpen(language_txt, PSP_O_RDONLY, 0777);
	if(fd > 0) 
	{
		for (i = 0; i < 71; i++)
			ReadLine(fd, string[i]);
		sceIoClose(fd);
	}
	else 
	{
		for (i = 0; i < 71; i++)
			strcpy(string[i], en_recovery[i]);
	}
}

int AssignDevice(int assigning)
{
	int ret;
	if (sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0) < 0)
	{
		ret = -1;
	}
	else
	{
		if (assigning == 0)
		{
			if (sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0) >= 0)
				ret = 0;
			else
				ret = -1;
		}
	}
	return ret;
}

int UnassignDevice(int unassigning)
{
	int unassign = sceIoUnassign("flash0:");
	if (unassign >= 0)
	{
		label6:
		if (unassigning == 0)
		{
			unassign = sceIoUnassign("flash1:");
			if (unassign >= 0)
			{
				label12:
				unassign = sceIoUnassign("flash2:");

				if (unassign >= 0)
				{
					if (vshKernelGetModel() == PSP_MODEL_SLIM_AND_LITE)
						sceIoUnassign("flash3:");
				}
				else
				{
					if (unassign == SCE_KERNEL_ERROR_NODEV)
						if (vshKernelGetModel() == PSP_MODEL_SLIM_AND_LITE)
							sceIoUnassign("flash3:");
				}
			}
			else
			{
				if (unassign == SCE_KERNEL_ERROR_NODEV)
					goto label12;
			}
		}
	}
	else
	{
		if (unassign == SCE_KERNEL_ERROR_NODEV)
			goto label6;
	}
	return unassign;
}

int FormatFlash1()
{
	char *argv[2];
	int res = UnassignDevice(0);
	if (res > 0)
	{
		argv[0] = "fatfmt";
		argv[1] = "lflash0:0,1";
		res = vshLflashFatfmtStartFatfmt(2, argv);
		if (res > 0)
		{
			res = AssignDevice(0);
			if (res > 0)
			{
				sceIoMkdir("flash1:/dic", 0777);
				sceIoMkdir("flash1:/gps", 0777);
				sceIoMkdir("flash1:/net", 0777);
				sceIoMkdir("flash1:/net/http", 0777);
				sceIoMkdir("flash1:/registry", 0777);
				sceIoMkdir("flash1:/vsh", 0777);
				sceIoMkdir("flash1:/vsh/theme", 0777);
				res = 0x00000000;
			}
		} else res = 0xFFFFFFFE;
	}
	return res;
}

void EnableUsbStore()
{
	if(!usbModuleStatus) 
	{
		LoadStartModule("flash0:/kd/usbdevice.prx");
		LoadStartModule("flash0:/kd/semawm.prx");
		LoadStartModule("flash0:/kd/usbstor.prx"); 
		LoadStartModule("flash0:/kd/usbstormgr.prx");
		LoadStartModule("flash0:/kd/usbstorms.prx");
		LoadStartModule("flash0:/kd/usbstorboot.prx");
		usbModuleStatus = 1;
	}

	sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
	sceUsbstorBootSetCapacity(0x800000);
	sceUsbActivate(0x1c8);
	usbStarted = 1;
}

void DisableUsbStore()
{
	if (usbStarted)
	{
	    sceUsbDeactivate(0x1c8);
	    sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
	    sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
		usbStarted = 0;
		pspUsbDeviceFinishDevice();
	}
}

#define PROGRAM "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP"

char hide[64], game[64], bootprog[64], noumd[64], region[64], vshmenu[64], usbdev[64], update[64], hidepic[64],
	versiontxt[64], useslimcolor[64], hidemac[64];
char patch[128], bootbin[128], xmbplugins[128], gameplugins[128], popsplugins[128];
char vshspeed[64], umdspeed[64];
char *plugins_p[16];
char plugins[11][64];
char button[50];
int nvsh = 0, ngame = 0, npops = 0, nupd = 0;

char *usbdevices[6] =
{
	"Memory Stick",
	"Flash 0",
	"Flash 1",
	"Flash 2",
	"Flash 3",
	string[47]
};

char *umdmodes[4] =
{
	string[16],
	string[17],
	string[18],
	string[19]
};

char *vshmenuoptions[3] =
{
	string[0],
	"VshMenu",
	"Recovery",
	"XmbConfig"
};

char *regions[14] =
{
	string[0],
	string[3],
	string[4],
	string[5],
	string[6],
	string[7],
	string[8],
	string[9],
	string[10],
	string[11],
	string[12],
	string[13],
	string[14],
	string[15]
};

int module_start(SceSize args, void *argp)
{
	// Get PSP Language and read **_recovery.txt
	GetLanguage();

	// Dynamique Pointeur Alloc
	int i;
	char **tmp;

	tmp = malloc(13*sizeof(char*));
	tmp[0] = malloc(13*71*sizeof(char));

	for(i = 1; i != 13; i++)
		tmp[i] = tmp[0] + 71*i;

	// Main Menu
	sprintf(tmp[1], "%s -> ", string[21]);
	sprintf(tmp[2], "%s /PSP/GAME/RECOVERY/EBOOT.PBP", string[24]);
	sprintf(tmp[3], "%s -> ", string[22]);
	sprintf(tmp[4], "%s -> ", string[25]);
	sprintf(tmp[5], "%s -> ", string[26]);
	sprintf(tmp[6], "%s -> ", string[27]);
	sprintf(tmp[7], "%s -> ", string[23]);

	// Toggle USB
	sprintf(tmp[8], "%s (flash0)", string[20]);
	sprintf(tmp[9], "%s (flash1)", string[20]);
	sprintf(tmp[10], "%s (flash2)", string[20]);
	sprintf(tmp[11], "%s (flash3)", string[20]);
	sprintf(tmp[12], "%s (UMD)", string[20]);

	myDebugScreenInit();
	char *items[] = { string[20], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], string[28], 0 };
	char *mainmsg = string[61];
	char *advitems[] = { string[29], tmp[7], tmp[8], tmp[9], tmp[10], tmp[11], tmp[12], string[34], 0 };
	char *advmsg = string[22];
	char *conitems[] = { string[29], "", "", "", "", "", "", "", "", "", "", "", "", "", 0 };
	char *conmsg = string[21];
	char *adconitems[] = { string[29], "", "", "", "", "", 0 };
	char *speeditems[] = { string[29], "", "", 0 };
	char *regitems[] = { string[29], "", string[30], string[31], 0 };
	int p = 1, u, o, res;
	int oldselection;

	SceKernelUtilsMt19937Context ctx;

	InitRandom(&ctx);
	int random = GetRandom(0, 255, &ctx);
	int timelow = sceKernelGetSystemTimeLow();
	int r = (random ^ 85 ^~ ((timelow ^~ 8)|(timelow ^ 16)|(timelow ^ 32)));
	int g = (random ^ 170 ^~ ((timelow ^ 8)|(timelow ^~ 16)|(timelow ^ 32)));
	int b = (random ^ 255 ^~ ((timelow ^ 8)|(timelow ^ 16)|(timelow ^~ 32)));

	myDebugScreenSetBackColor(RGB(0, 0, 0));
	while(p)
	{
		clearScreen();
		int result;
		result = doMenu(items, 8, 0, mainmsg, 0, 5, r, g, b, string[70]);
		if(result == 0)
		{
			if(!usbStarted){
				printf(" > %s", string[32]); EnableUsbStore();
			}
			else {
				printf(" > %s", string[33]); DisableUsbStore();
			}
			delay(1000000);
		}
		if(result == 3) 
		{
			u = 1;

			while(u)
			{
				clearScreen();
				result = doMenu(advitems, 8, 0, advmsg, 2, 5, r, g, b, string[70]);

				if(result == 0)
				{
					printf(" > %s...", string[29]); 
					DisableUsbStore();
					delay(1000000); 
					u = 0; 
				}
				else if (result == 7)
				{
					printf(" > %s... ", string[35]);
					int res = FormatFlash1();
					if(res == SCE_KERNEL_ERROR_NODEV)
						printf("Cannot unassign flash");
					else if(res == 0xFFFFFFFE)
						printf("WARNING: Error formating flash1");
					else if(res == 0xFFFFFFFF)
						printf("PANIC: error re-assigning flash");
					else
						printf("OK Rebooting");

					delay(1100000);
					res = vshKernelExitVSHVSH(NULL);
					break;
				}
				else if (result == 6)
				{
					if(!usbStarted){
						printf(" > %s", string[32]); pspUsbDeviceSetDevice(PSP_USBDEVICE_UMD9660, 0, 0); EnableUsbStore();
					}
					else {
						printf(" > %s", string[33]); DisableUsbStore();
					}
					delay(1000000);
				}
				else if (result == 5)
				{
					if(!usbStarted){
						printf(" > %s", string[32]); pspUsbDeviceSetDevice(PSP_USBDEVICE_FLASH3, 0, 0); EnableUsbStore();
					}
					else {
						printf(" > %s", string[33]); DisableUsbStore();
					}
					delay(1000000);
				}
				else if (result == 4)
				{
					if(!usbStarted){
						printf(" > %s", string[32]); pspUsbDeviceSetDevice(PSP_USBDEVICE_FLASH2, 0, 0); EnableUsbStore();
					}
					else {
						printf(" > %s", string[33]); DisableUsbStore();
					}
					delay(1000000);
				}
				else if (result == 3)
				{
					if(!usbStarted){
						printf(" > %s", string[32]); pspUsbDeviceSetDevice(PSP_USBDEVICE_FLASH1, 0, 0); EnableUsbStore();
					}
					else {
						printf(" > %s", string[33]); DisableUsbStore();
					}
					delay(1000000);
				}
				else if (result == 2)
				{
					if(!usbStarted){
						printf(" > %s", string[32]); pspUsbDeviceSetDevice(PSP_USBDEVICE_FLASH0, 0, 0); EnableUsbStore();
					}
					else {
						printf(" > %s", string[33]); DisableUsbStore();
					}
					delay(1000000);
				}
				else if (result == 1)
				{
					while (1)
					{
						clearScreen();

						if (!configchanged)
							sctrlSEGetConfig(&config);

						sprintf(patch, "%s (%s: %s)", string[37], string[36], config.umdactivatedplaincheck ? string[1] : string[0]);
						sprintf(bootbin, "%s (%s: %s)", string[38], string[36], config.executebootbin ? string[1] : string[0]);
						sprintf(xmbplugins, "%s (%s: %s)", string[64], string[36], config.xmbplugins ? string[0] : string[1]);
						sprintf(gameplugins, "%s (%s: %s)", string[65], string[36], config.gameplugins ? string[0] : string[1]);
						sprintf(popsplugins, "%s (%s: %s)", string[66], string[36], config.popsplugins ? string[0] : string[1]);

						adconitems[1] = patch;
						adconitems[2] = bootbin;
						adconitems[3] = xmbplugins;
						adconitems[4] = gameplugins;
						adconitems[5] = popsplugins;

						result = doMenu(adconitems, 6, 0, string[23], 2, 5, r, g, b, string[70]);

						if (result != 0 && result != -1)
						{
							if (!configchanged)
								configchanged = 1;				
						}

						if(result == 0) { printf(" > %s...", string[29]); delay(1000000); break; }

						if (result == 1)
						{
							config.umdactivatedplaincheck = !config.umdactivatedplaincheck;
							printf(" > %s: %s", string[37], config.umdactivatedplaincheck ? string[1] : string[0]);
							delay(1100000);
						}
						else if (result == 2)
						{
							config.executebootbin = !config.executebootbin;
							printf(" > %s: %s", string[38], config.executebootbin ? string[1] : string[0]);
							delay(1100000);
						}
						else if (result == 3)
						{
							config.xmbplugins = !config.xmbplugins;
							printf(" > %s: %s", string[64], config.xmbplugins ? string[0] : string[1]);
							delay(1100000);
						}
						else if (result == 4)
						{
							config.gameplugins = !config.gameplugins;
							printf(" > %s: %s", string[65], config.gameplugins ? string[0] : string[1]);
							delay(1100000);
						}
						else if (result == 5)
						{
							config.popsplugins = !config.popsplugins;
							printf(" > %s: %s", string[66], config.popsplugins ? string[0] : string[1]);
							delay(1100000);
						}
					}
				}
			}
		}
		if(result == 1) 
		{
			o = 1;
			oldselection = 0;

			while(o) 
			{
				char skip[96];				

				clearScreen();
				if (!configchanged)
					sctrlSEGetConfig(&config);

				sprintf(skip, "%s (%s: %s)", string[39], string[36], config.skiplogo ? string[1] : string[0]);
				sprintf(hide, "%s (%s: %s)", string[40], string[36], config.hidecorrupt ? string[1] : string[0]);
				sprintf(game, "%s (%s: %s)", string[41], string[36], config.gamekernel150 ? "1.50 Kernel" : "5.XX Kernel");
				sprintf(bootprog, "%s /PSP/GAME/BOOT/EBOOT.PBP (%s: %s)", string[42], string[36], config.startupprog ? string[1] : string[0]);
				sprintf(noumd, "%s (%s: %s)", string[43], string[36], umdmodes[config.umdmode]);
				sprintf(region, "%s (%s: %s)", string[44], string[36], regions[config.fakeregion]);
				//sprintf(vshmenu, "%s (%s: %s)", string[45], string[36], config.vshmenu ? string[0] : string[1]);
				sprintf(vshmenu, "%s (%s: %s)", string[45], string[36], vshmenuoptions[config.vshmenu]);
				sprintf(usbdev, "%s (%s: %s)", string[46], string[36], usbdevices[config.usbdevice]);
				sprintf(update, "%s (%s: %s)", string[48], string[36], config.notusedaxupd ? string[0] : string[1]);
				sprintf(hidepic, "%s (%s: %s)", string[49], string[36], config.hidepics ? string[1] : string[0]);
				sprintf(versiontxt, "%s (%s: %s)", string[67], string[36], config.useversiontxt ? string[1] : string[0]);
				//sprintf(speedms, "%s (%s: %s)", string[68], string[36], speedupms[config.speedupmsaccess]);
				sprintf(useslimcolor, "%s (%s: %s)", string[68], string[36], config.slimcolors ? string[1] : string[0]);
				sprintf(hidemac, "%s (%s: %s)", string[69], string[36], config.hidemac ? string[1] : string[0]);

				conitems[1] = skip;
				conitems[2] = hide;
				conitems[3] = game;
				conitems[4] = bootprog;
				conitems[5] = noumd;
				conitems[6] = region;
				conitems[7] = vshmenu;
				conitems[8] = usbdev;
				conitems[9] = update;
				conitems[10] = hidepic;
				conitems[11] = versiontxt;
				//conitems[12] = speedms;
				conitems[12] = useslimcolor;
				conitems[13] = hidemac;

				result = doMenu(conitems, 14, oldselection, conmsg, 2, 5, r, g, b, string[70]);

				if (result != 0 && result != -1)
				{
					if (!configchanged)
						configchanged = 1;
				}

				if(result == 0) { printf(" > %s...", string[29]); delay(1000000); o = 0; }
				else if(result == 1) 
				{
					config.skiplogo = !config.skiplogo;
					printf(" > %s: %s", string[39], (config.skiplogo) ? string[1] : string[0]);
					delay(1100000); 
				}
				else if(result == 2) 
				{
					config.hidecorrupt = !config.hidecorrupt;
					printf(" > %s: %s", string[40], (config.hidecorrupt) ? string[1] : string[0]); 
					delay(1100000); 
				}
				else if (result == 3)
				{
					config.gamekernel150 = !config.gamekernel150;
					printf(" > %s: %s", string[41], (config.gamekernel150) ? string[62] : string[63]); 
					delay(1100000); 
				}
				else if (result == 4)
				{
					config.startupprog = !config.startupprog;
					printf(" > %s /PSP/GAME/BOOT/EBOOT.PBP: %s", string[42], (config.startupprog) ? string[1] : string[0]); 
					delay(1100000); 
				}
				else if (result == 5)
				{
					if(config.umdmode == MODE_UMD)
						config.umdmode = MODE_MARCH33;
					else if(config.umdmode == MODE_MARCH33)
						config.umdmode = MODE_NP9660;
					else if(config.umdmode == MODE_NP9660)
						config.umdmode = MODE_OE_LEGACY;
					else if(config.umdmode == MODE_OE_LEGACY)
						config.umdmode = MODE_UMD;

					//printf(" > %s: %s", string[43], umdmodes[config.umdmode]);
					oldselection = 5;
					delay(300000);
				}
				else if (result == 6)
				{
					if(config.fakeregion == FAKE_REGION_DISABLED)
						config.fakeregion = FAKE_REGION_JAPAN;
					else if(config.fakeregion == FAKE_REGION_JAPAN)
						config.fakeregion = FAKE_REGION_AMERICA;
					else if(config.fakeregion == FAKE_REGION_AMERICA)
						config.fakeregion = FAKE_REGION_EUROPE;
					else if(config.fakeregion == FAKE_REGION_EUROPE)
						config.fakeregion = FAKE_REGION_KOREA;
					else if(config.fakeregion == FAKE_REGION_KOREA)
						config.fakeregion = FAKE_REGION_UNK;
					else if(config.fakeregion == FAKE_REGION_UNK)
						config.fakeregion = FAKE_REGION_UNK2;
					else if(config.fakeregion == FAKE_REGION_UNK2)
						config.fakeregion = FAKE_REGION_AUSTRALIA;
					else if(config.fakeregion == FAKE_REGION_AUSTRALIA)
						config.fakeregion = FAKE_REGION_HONGKONG;
					else if(config.fakeregion == FAKE_REGION_HONGKONG)
						config.fakeregion = FAKE_REGION_TAIWAN;
					else if(config.fakeregion == FAKE_REGION_TAIWAN)
						config.fakeregion = FAKE_REGION_RUSSIA;
					else if(config.fakeregion == FAKE_REGION_RUSSIA)
						config.fakeregion = FAKE_REGION_CHINA;
					else if(config.fakeregion == FAKE_REGION_CHINA)
						config.fakeregion = FAKE_REGION_DEBUG_TYPE_I;
					else if(config.fakeregion == FAKE_REGION_DEBUG_TYPE_I)
						config.fakeregion = FAKE_REGION_DEBUG_TYPE_II;
					else if(config.fakeregion == FAKE_REGION_DEBUG_TYPE_II)
						config.fakeregion = FAKE_REGION_DISABLED;
					oldselection = 6;
					delay(300000);
				}
				else if (result == 7)
				{
					//config.vshmenu = !config.vshmenu;
					//printf(" > %s: %s", string[45], (config.vshmenu) ? string[0] : string[1]);
					if(config.vshmenu == VSHMENU_DISABLED)
						config.vshmenu =  VSHMENU_VSHMENU;
					else if(config.vshmenu == VSHMENU_VSHMENU)
						config.vshmenu = VSHMENU_RECOVERY;
					else if(config.vshmenu == VSHMENU_RECOVERY)
						config.vshmenu = VSHMENU_DISABLED;
					//printf(" > %s: %s", string[45], vshmenuoptions[config.vshmenu]);
					oldselection = 7;
					delay(300000);
				}
				else if (result == 8)
				{
					if(config.usbdevice == USBDEVICE_MEMORYSTICK)
						config.usbdevice = USBDEVICE_FLASH0;
					else if(config.usbdevice == USBDEVICE_FLASH0)
						config.usbdevice = USBDEVICE_FLASH1;
					else if(config.usbdevice == USBDEVICE_FLASH1)
						config.usbdevice = USBDEVICE_FLASH2;
					else if(config.usbdevice == USBDEVICE_FLASH2)
						config.usbdevice = USBDEVICE_FLASH3;
					else if(config.usbdevice == USBDEVICE_FLASH3)
						config.usbdevice = USBDEVICE_UMD9660;
					else if(config.usbdevice == USBDEVICE_UMD9660)
						config.usbdevice = USBDEVICE_MEMORYSTICK;
					//printf(" > %s: %s", string[46], usbdevices[(config.usbdevice)]);
					oldselection = 8;
					delay(300000);
				}
				else if (result == 9)
				{
					config.notusedaxupd = !config.notusedaxupd;
					printf(" > %s: %s", string[48], (config.notusedaxupd) ? string[0] : string[1]);
					delay(1100000);
				}
				else if (result == 10)
				{
					config.hidepics = !config.hidepics;
					printf(" > %s: %s", string[49], (config.hidepics) ? string[1] : string[0]);
					delay(1100000);
				}
				else if (result == 11)
				{
					config.useversiontxt = !config.useversiontxt;
					printf(" > %s: %s", string[67], (config.useversiontxt) ? string[1] : string[0]);
					delay(1100000);
				}
				else if (result == 12)
				{
					//config.speedupmsaccess = !config.speedupmsaccess;
					//printf(" > %s: %s", string[68], speedupms[(config.speedupmsaccess)]);
					config.slimcolors = !config.slimcolors;
					printf(" > %s: %s", string[68], (config.slimcolors) ? string[1] : string[0]);
					delay(1100000);
				}
				else if (result == 13)
				{
					config.hidemac = !config.hidemac;
					printf(" > %s: %s", string[69], (config.hidemac) ? string[1] : string[0]);
					delay(1100000);
				}
				else if (result != -1)
				{
					oldselection = 0;
				}
			}
		}
		if (result == 2)
		{
			//LoadStartModule("flash0:/kd/systemctrl.prx");
			struct SceKernelLoadExecVSHParam param;
			memset(&param, 0, sizeof(param));
			param.size = sizeof(param);
			param.args = strlen(PROGRAM)+1;
			param.argp = PROGRAM;
			param.key = "updater";
			res = vshKernelLoadExecVSHMs1(PROGRAM, &param);
			break;
		}
		if (result == 4)
		{
			oldselection = 0;

			while (1)
			{
				clearScreen();

				if (!configchanged)
					sctrlSEGetConfig(&config);

				// Get VSH CPU/BUS Speed
				GetSpeed(&config.vshcpuspeed, &config.vshbusspeed, &configchanged);
				// Get UMD/ISO CPU/BUS Speed
				GetSpeed(&config.umdisocpuspeed, &config.umdisobusspeed, &configchanged);

				sprintf(vshspeed, "%s (%s: %d)", string[51], string[36], config.vshcpuspeed);
				sprintf(umdspeed, "%s (%s: %d)", string[52], string[36], config.umdisocpuspeed);

				if (config.vshcpuspeed == 0)
					sprintf(vshspeed+strlen(vshspeed)-2, "%s)", string[2]);
				if (config.umdisocpuspeed == 0)
					sprintf(umdspeed+strlen(umdspeed)-2, "%s)", string[2]);

				speeditems[1] = vshspeed;
				speeditems[2] = umdspeed;

				result = doMenu(speeditems, 3, oldselection, string[25], 2, 5, r, g, b, string[70]);

				if (result != 0)
				{
					if (!configchanged)
						configchanged = 1;
				}

				if(result == 0)
				{
					printf(" > %s...", string[29]);
					delay(1000000);
					break;
				}

				else if (result == 1)
				{
					// Set VSH CPU/BUS Speed
					SetSpeed(&config.vshcpuspeed, &config.vshbusspeed);

					sprintf(vshspeed, "%s: %d", string[51], config.vshcpuspeed);

					if (config.vshcpuspeed == 0)
						strcpy(vshspeed+strlen(vshspeed)-1, string[2]);

					printf("%s", vshspeed);
					oldselection = 1;
					delay(1100000);
				}

				else if (result == 2)
				{
					// Set UMD/ISO CPU/BUS Speed
					SetSpeed(&config.umdisocpuspeed, &config.umdisobusspeed);

					sprintf(umdspeed, "%s: %d", string[52], config.umdisocpuspeed);

					if (config.umdisocpuspeed == 0)
						strcpy(umdspeed+strlen(umdspeed)-1, string[2]);

					printf("%s", umdspeed);
					oldselection = 2;
					delay(1100000);
				}
			}
		}

		if (result == 5)
		{
			while (1)
			{
				clearScreen();

				int i;
				char *p;
				SceUID fd;

				ngame = 0;
				nvsh = 0;
				npops = 0;
				nupd = 0;

				int res;
				Plugins vsh;
				memset(plugins_p, 0, sizeof(plugins_p));

				fd = sceIoOpen("ms0:/seplugins/vsh.txt", PSP_O_RDONLY, 0777);
				memset(plugins, 0, sizeof(plugins));

				if (fd >= 0)
				{
					memset(vsh.plugins, 0, sizeof(vsh.plugins));
					for (i = 0; i < 7; i++)
					{
						if (GetPlugin(fd, vsh.plugins[i+1], &vsh.activated) > 0)
						{
							p = strrchr(vsh.plugins[i+1], '/');
							if (p) strcpy(vsh.plugins[i+1], p+1);
							strcat(vsh.plugins[i+1], " [VSH]");
							sprintf(vsh.plugins[i+1], "%s %d", vsh.plugins[i+1], vsh.activated);
							strcat(vsh.plugins[i+1], (vsh.activated) ? " (Enabled) " : " (Disabled)");
							strcpy(plugins[i+1], vsh.plugins[i+1]);
							nvsh++;
						}
						else
						{
							break;
						}
					}
					sceIoClose(fd);
				}

				/*fd = sceIoOpen("ms0:/seplugins/game.txt", PSP_O_RDONLY, 0777);

				if (fd >= 0)
				{
					for (i = 0; i < 5; i++)
					{
						if (ReadLine(fd, plugins[i+nvsh+1]) > 0)
						{
							p = strrchr(plugins[i+nvsh+1], '/');
							if (p)
							{
								strcpy(plugins[i+nvsh+1], p+1);
							}
						
							strcat(plugins[i+nvsh+1], " [GAME]");
							ngame++;	
						}
						else
						{
							break;
						}
					}
					sceIoClose(fd);
				}
				
				fd = sceIoOpen("ms0:/seplugins/pops.txt", PSP_O_RDONLY, 0777);

				if (fd >= 0)
				{
					for (i = 0; i < 5; i++)
					{
						if (ReadLine(fd, plugins[i+nvsh+ngame+1]) > 0)
						{
							p = strrchr(plugins[i+nvsh+ngame+1], '/');
							if (p)
							{
								strcpy(plugins[i+nvsh+ngame+1], p+1);
							}

							strcat(plugins[i+nvsh+ngame+1], " [POPS]");
							npops++;	
						}
						else
						{
							break;
						}
					}
					sceIoClose(fd);
				}

				fd = sceIoOpen("ms0:/seplugins/updater.txt", PSP_O_RDONLY, 0777);

				if (fd >= 0)
				{
					for (i = 0; i < 5; i++)
					{
						if (ReadLine(fd, plugins[i+nvsh+ngame+npops+1]) > 0)
						{
							p = strrchr(plugins[i+nvsh+ngame+npops+1], '/');
							if (p)
							{
								strcpy(plugins[i+nvsh+ngame+npops+1], p+1);
							}

							strcat(plugins[i+nvsh+ngame+npops+1], " [UPDATER]");
							nupd++;	
						}
						else
						{
							break;
						}
					}
					sceIoClose(fd);
				}*/

				strcpy(plugins[0], string[29]);

				for (i = 0; i < nvsh+1; i++)
				{
					plugins_p[i] = plugins[i];
				}

				result = doMenu(plugins_p, nvsh+1, 0, string[26], 2, 5, r, g, b, string[70]);

				if(result == 0) { printf(" > %s...", string[29]); delay(1000000); break; }
			}
		}

		if (result == 6)
		{
			while (1)
			{
				unsigned int value = 0;

				clearScreen();

				sprintf(button, "%s (%s ", string[53], string[36]);

				get_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", &value);

				if (value == 0)
					sprintf(button, "%s%s)", button, string[54]);
				else
					sprintf(button, "%s%s)", button, string[55]);

				regitems[1] = button;

				result = doMenu(regitems, 4, 0, string[27], 2, 5, r, g, b, string[70]);

				if(result == 0) 
				{
					printf(" > %s...", string[29]);
					delay(800000);
					break;
				}
				else if (result == 1)
				{
					value = !value;
					set_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", value); 
					sprintf(button, " > %s: ", string[53]);
					if (value == 0)
						sprintf(button, "%s%s)", button, string[54]);
					else
						sprintf(button, "%s%s)", button, string[55]);

					printf("%s", button);
					delay(1000000);
				}
				else if (result == 2)
				{
					get_registry_value("/CONFIG/MUSIC", "wma_play", &value);

					if (value == 1)
					{
						printf(" > %s", string[56]);
					}
					else
					{
						printf(" > %s", string[57]);
						set_registry_value("/CONFIG/MUSIC", "wma_play", 1);
					}
					delay(1000000);
				}
				else if (result == 3)
				{
					get_registry_value("/CONFIG/BROWSER", "flash_activated", &value);

					if (value == 1)
					{
						printf(" > %s", string[58]);
					}
					else
					{
						printf(" > %s", string[59]);
						set_registry_value("/CONFIG/BROWSER", "flash_activated", 1);
						set_registry_value("/CONFIG/BROWSER", "flash_play", 1);
					}
					delay(1000000);
				}
			}
		}

		if(result == 7) 
		{
			printf(" > %s", string[60]);
			delay(700000);

			if (configchanged)
				sctrlSESetConfig(&config);
			res = vshKernelExitVSHVSH(NULL);
			free(tmp[0]);
			free(tmp);
			break;
		}
	}
	return res;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}


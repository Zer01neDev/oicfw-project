/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	main.c: VshControl Main Code
	This file are based from a reverse of M33/GEN VshControl
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
#include <pspusb.h>
#include <pspreg.h>
#include <psprtc.h>
#include <pspctrl.h>
#include <psppower.h>

#include "pspusbdevice.h"
#include "pspsysmem_kernel.h"
#include "systemctrl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "systemctrl_se.h"

#include "umd9660_driver.h"
#include "isofs_driver.h"

#include "main.h"
#include "virtualpbpmgr.h"
#include "isofs_driver.h"

PSP_MODULE_INFO("VshControl", 0x1007, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define EBOOT_BIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define BOOT_BIN  "disc0:/PSP_GAME/SYSDIR/BOOT.BIN"

int SEVvv;
SEConfig config;
SceModule2 *fmod;
int disc_is_no_out; //0x00004C74
int umdfile_is_iso; // 0x00004CA8
int unk_1 = 0; //0x00004CA9
SceKernelLMOption sateliteLMopt; // 0x00004B28
SceUID vshmenu_mod = -1; //0x00004974
SceUID usbdevice_mod = -1;
int (* sceUsbStopReal)(const char* driverName, int size, void *args);
int (* sceUsbStartReal)(const char* driverName, int size, void *args);
int (* sceRegSetKeyValueReal)(REGHANDLE hd, const char *name, void *buf, SceSize size);
int (* sceResmgr_driver_9DC14891Real)(char *arg1, int arg2, int arg3);
void (*SetSpeed)(int cpu, int bus);
void (*SetConfig)(SEConfig *newconfig);

SceUID gamedfd = -1, game150dfd = -1, game5xxdfd = -1, isodfd = -1, over5xx = 0, overiso = 0;
SceUID paramsfo = -1;
int vpbpinited = 0, isoindex = 0, cachechanged = 0;
VirtualPbp vpbp;
VirtualPbp *cache = NULL;
int referenced[32];
STMOD_HANDLER previous = NULL;
int timelow1/*0x00004CA0*/, timelow2/*0x00004B40*/, timelow3;


#if _PSP_FW_VERSION == 500
#define MAJOR_VERSION "5"
#define MINOR_VERSION "00"
#endif
#if _PSP_FW_VERSION == 550
#define MAJOR_VERSION "5"
#define MINOR_VERSION "50"
#endif

wchar_t verinfo[] = MAJOR_VERSION L"." MINOR_VERSION L" OI R ";
wchar_t macinfo[] = L"[BlackDev'sTeam]";

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

int LoadStartModule(char *module)
{
	SceUID mod = sceKernelLoadModule(module, 0, NULL);
	if (mod < 0) return mod;
	return sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
}

void KXploitString(char *str)
{
	if (str)
	{
		char *perc = strchr(str, '%');

		if (perc)
		{
			strcpy(perc, perc+1);
		}
	}
}

void FixPath(const char *file, char *patch1, char *patch2)
{
	char str[256];

	if (strstr(file, "ms0:/PSP/GAME/") == file)
	{
		strcpy(str, (char *)file);

		char *p = strstr(str, patch1);

		if (p)
		{
			strcpy((char *)file+13, patch2);
			strncpy((char *)file+17, str+14, p-(str+14));
			strcpy((char *)file+17+(p-(str+14)), p+5);
		}
	}
}

int CorruptIconPatch(char *name, int g150)
{
	char path[256];
	SceIoStat stat;

	if (g150)
	{
		sprintf(path, "ms0:/PSP/GAME150/%s%%/EBOOT.PBP", name);
	}

	else
	{
		sprintf(path, "ms0:/PSP/GAME/%s%%/EBOOT.PBP", name);
	}

	memset(&stat, 0, sizeof(SceIoStat));

	if (sceIoGetstat(path, &stat) >= 0)
	{
		strcpy(name, "__SCE"); // hide icon
		return 1;
	}

	return 0;
}

int GetIsoIndex(const char *file)
{
	char number[5];

	if (strstr(file, "ms0:/PSP/GAME/MMMMMISO") != file)
		return -1;

	char *p = strchr(file+17, '/');

	if (!p)
		return strtol(file+22, NULL, 10);

	memset(number, 0, 5);
	strncpy(number, file+22, p-(file+22));

	return strtol(number, NULL, 10);
}

u32 FindSystemCtrlKernelFunction(u32 nid)
{
	return sctrlHENFindFunction("SystemControl", "SystemCtrlForKernel", nid);
}

void RebootVSHWithError(u32 error)
{
	struct SceKernelLoadExecVSHParam param;
	u32 vshmain_args[0x20/4];

	memset(&param, 0, sizeof(param));
	memset(vshmain_args, 0, sizeof(vshmain_args));

	vshmain_args[0/4] = 0x0400;
	vshmain_args[4/4] = 0x20;
	vshmain_args[0x14/4] = error;

	param.size = sizeof(param);
	param.args = 0x400;
	param.argp = vshmain_args;
	param.vshmain_args_size = 0x400;
	param.vshmain_args = vshmain_args;
	param.configfile = "/kd/pspbtcnf.txt";

	sctrlKernelExitVSH(&param);
}

void LoadReboot150()
{
	SceUID reboot150 = sceKernelLoadModule("flash0:/kd/reboot150.prx", 0, NULL);
	if (reboot150 < 0)
		reboot150 = sceKernelLoadModule("ms0:/seplugins/reboot150.prx", 0, NULL);
	if (reboot150 >= 0)
		sceKernelStartModule(reboot150, 0, NULL, NULL, NULL);
}

int LoadExecVSHCommonPatched(int apitype, char *file, struct SceKernelLoadExecVSHParam *param)
{
	int index;	
	int k1 = pspSdkSetK1(0);

	SetUmdFile("");
	index = GetIsoIndex(file);

	if(index < 0)
	{
		FixPath(file, "__150", "150/");
		FixPath(param->argp, "__150", "150/");
		FixPath(file, "__5XX", "5XX/");
		FixPath(param->argp, "__5XX", "5XX/");

		if(strstr(file, "ms0:/PSP/GAME150/") == file)
		{
			label39:
			KXploitString(file);
			KXploitString(param->argp);
			param->args = strlen(param->argp)+1;
			LoadReboot150();
		}
		else
		{
			if(strstr(file, "ms0:/PSP/GAME/") != file)
			{
				label26:
				if(strstr(file, "EBOOT.BIN"))
				{
					if(config.executebootbin)
					{
						strcpy(file, "disc0:/PSP_GAME/SYSDIR/BOOT.BIN");
						param->argp = file;
					}
					if(config.umdactivatedplaincheck)
						param->args = strlen(param->argp)+1;
				}
			}
			else
			{
				if(strstr(file, "ms0:/PSP/GAME/UPDATE/") != 0)
					goto label26;

				if (config.gamekernel150)
				{
					goto label39;
				}
			}
		}
		pspSdkSetK1(k1);
	}
	else
	{
		if(config.executebootbin)
			strcpy(file, "disc0:/PSP_GAME/SYSDIR/BOOT.BIN");
		else
			strcpy(file, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN");

		apitype = 0x120;
		param->args = strlen(file)+1;
		param->argp = file;
		SetUmdFile(virtualpbp_getfilename(index));

		if (config.umdmode != MODE_MARCH33)
		{
			if (config.umdmode == MODE_NP9660)
			{
				sctrlSESetBootConfFileIndex(1);
			}
		}
		else
		{
			sctrlSESetBootConfFileIndex(1);
		}

		/*if(config.umdmode == MODE_UMD)
			sctrlSESetBootConfFileIndex(0);
		else if(config.umdmode == MODE_OE_LEGACY)
			sctrlSESetBootConfFileIndex(0);
		else if(config.umdmode == MODE_MARCH33)
			sctrlSESetBootConfFileIndex(1);
		else if(config.umdmode == MODE_NP9660)
			sctrlSESetBootConfFileIndex(2);*/

		pspSdkSetK1(k1);
	}
	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

void ApplyNamePatch(SceIoDirent *dir, char *patch)
{
	if (dir->d_name[0] != '.')
	{
		int patchname = 1;

		if (config.hidecorrupt)
		{
			if (CorruptIconPatch(dir->d_name, 1))
				patchname = 0;
		}

		if (patchname)
		{
			strcat(dir->d_name, patch);
		}
	}
}

void ApplyIsoNamePatch(SceIoDirent *dir)
{
	if (dir->d_name[0] != '.')
	{
		memset(dir->d_name, 0, 256);
		sprintf(dir->d_name, "MMMMMISO%d", isoindex++);
	}
}

int ReadCache()
{
	int i;
	SceUID fd;

	if (!cache)
		cache = (VirtualPbp *)oe_malloc(32*sizeof(VirtualPbp));

	memset(cache, 0, sizeof(VirtualPbp)*32);
	memset(referenced, 0, sizeof(referenced));

	for (i = 0; i < 0x10; i++)
	{
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache.bin", PSP_O_RDONLY, 0777);
		if (fd >= 0)
			break;
	}

	if (i == 0x10)
		return -1;

	sceIoRead(fd, cache, sizeof(VirtualPbp)*32);
	sceIoClose(fd);
	return 0;
}

int SaveCache()
{
	SceUID fd;
	int i = 0;
	if (!cache)
		return -1;

	for(i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] != 0 && !referenced[i])
		{
			cachechanged = 1;
			memset(&cache[i], 0, sizeof(VirtualPbp));
		}
	}

	if (!cachechanged)
		return 0;

	cachechanged = 0;

	for (i = 0; i < 0x10; i++)
	{
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache.bin", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd >= 0)
			break;
	}
	if (i == 0x10)
		return -1;

	sceIoWrite(fd, cache, sizeof(VirtualPbp)*32);
	sceIoClose(fd);
	return 0;
}

int IsCached(char *isofile, ScePspDateTime *mtime, VirtualPbp *res)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] != 0)
		{
			if (strcmp(cache[i].isofile, isofile) == 0)
			{
				if (memcmp(mtime, &cache[i].mtime, sizeof(ScePspDateTime)) == 0)
				{
					memcpy(res, &cache[i], sizeof(VirtualPbp));
					referenced[i] = 1;
					return 1;
				}
			}
		}
	}

	return 0;
}

int Cache(VirtualPbp *pbp)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] == 0)
		{
			referenced[i] = 1;
			memcpy(&cache[i], pbp, sizeof(VirtualPbp));
			cachechanged = 1;
			return 1;
		}
	}
	return 0;
}

// OK
SceUID sceIoDopenPatched(const char *dirname)
{
	int res, index;
	int k1 = pspSdkSetK1(0);

	FixPath(dirname, "__150", "150/");
	FixPath(dirname, "__5XX", "5XX/");

	index = GetIsoIndex(dirname);
	if(index < 0)
	{
		if (strcmp(dirname, "ms0:/PSP/GAME") == 0)
		{
			pspSdkSetK1(k1);
			res = sceIoDopen(dirname);
			gamedfd = res;
			pspSdkSetK1(0);
			game150dfd = sceIoDopen("ms0:/PSP/GAME150");
			over5xx = 0;
			overiso = 0;
		}
		else
		{
			pspSdkSetK1(k1);
			res = sceIoDopen(dirname);
			pspSdkSetK1(0);
		}
	}
	else
	{
		res = virtualpbp_open(index);
	}

	printf("Directory: %s UID: 0x%08X\n", dirname, res);

	pspSdkSetK1(k1);
	return res;
}

// OK
int sceIoDreadPatched(SceUID fd, SceIoDirent *dir)
{
	int res;
	int k1 = pspSdkSetK1(0);

	if (vpbpinited == 0)
	{
		label8:
		if (fd < 0)
		{
			label69:
			res = sceIoDread(fd, dir);
			if (res > 0)
			{
				if (config.hidecorrupt)
				{
					CorruptIconPatch(dir->d_name, 0);
				}
			}
		}
		else
		{
			if (fd != gamedfd)
				goto label69;

			if (game150dfd < 0)
			{
				label17:
				if (game5xxdfd >= 0)
				{
					label27:
					res = sceIoDread(game5xxdfd, dir);
					if (res > 0)
					{
						ApplyNamePatch(dir, "__5XX");
					}
					else
					{
						sceIoDclose(game5xxdfd);
						game5xxdfd = -1;
						over5xx = 1;

						label37:
						if (game150dfd >= 0)
						{
							label59:
							if (isodfd < 0)
								goto label69;

							label62:
							res = sceIoDread(isodfd, dir);
							if (res <= 0)
							{
								sceIoDclose(isodfd);
								isodfd = -1;
								overiso = 1;
								goto label69;
							}
							else
							{
								char fullpath[128];
								int res2 = -1;
								int docache;

								if (FIO_S_ISDIR(dir->d_stat.st_mode))
									goto label62;

								strcpy(fullpath, "ms0:/ISO/");
								strcat(fullpath, dir->d_name);
								if (IsCached(fullpath, &dir->d_stat.st_mtime, &vpbp) == 0)
								{
									res2 = virtualpbp_add(fullpath, &dir->d_stat.st_mtime, &vpbp);
									docache = 1;
								}
								else
								{
									res2 = virtualpbp_fastadd(&vpbp);
									docache = 0;
								}
								if (!(res2 < 0))
								{
									ApplyIsoNamePatch(dir);
									dir->d_stat.st_mode = 0x11FF;
									dir->d_stat.st_attr = 0x0010;
									dir->d_stat.st_size = 0;
									//dir->d_stat.st_ctime = 0;
									dir->d_stat.st_mtime.hour = dir->d_stat.st_ctime.hour;
									dir->d_stat.st_mtime.year = dir->d_stat.st_ctime.year;
									dir->d_stat.st_mtime.month = dir->d_stat.st_ctime.month;
									dir->d_stat.st_mtime.day = dir->d_stat.st_ctime.day;
									if (!(docache == 0))
									{
										Cache(&vpbp);
									}
									if (umdfile_is_iso != 0)
									{
										sub_00001064();
									}
								}
							}
						}
						else
						{
							if (game5xxdfd >= 0)
							{
								goto label59;
							}
							else
							{
								if (isodfd >= 0)
								{
									goto label62;
								}
								else
								{
									if (overiso != 0)
									{
										goto label69;
									}
									else
									{
										isodfd = sceIoDopen("ms0:/ISO");
										if (isodfd < 0)
										{
											overiso = 1;
											goto label69;
										}
										else
										{
											if (vpbpinited != 0)
											{
												virtualpbp_reset();
											}
											else
											{
												virtualpbp_init();
												vpbpinited = 1;
											}
											ReadCache();
											isoindex = 0;
											goto label59;
										}
									}
								}
							}
						}
					}
				}
				else
				{
					if (isodfd >= 0)
					{
						goto label37;
					}
					else
					{
						if (over5xx != 0)
						{
							goto label37;
						}
						else
						{
							game5xxdfd = sceIoDopen("ms0:/PSP/GAME5XX");
							if (game5xxdfd >= 0)
							{
								goto label27;
							}
							else
							{	
								over5xx = 1;
								goto label37;
							}
						}
					}
				}
			}
			else
			{
				res = sceIoDread(game150dfd, dir);
				if (res <= 0)
				{
					sceIoDclose(game150dfd);
					game150dfd = -1;
					goto label17;
				}
				else
				{
					ApplyNamePatch(dir, "__150");
				}
			}
		}
	}
	else
	{
		res = virtualpbp_dread(fd, dir);
		if (res < 0) goto label8;
	}

	pspSdkSetK1(k1);
	return res;
}

// OK
int sceIoDclosePatched(SceUID fd)
{
	int res;
	int k1 = pspSdkSetK1(0);

	if (vpbpinited == 0)
	{
		label11:
		if (fd == gamedfd)
		{
			gamedfd = -1;
			over5xx = 0;
			overiso = 0;
			SaveCache();
		}
		pspSdkSetK1(k1);
		res = sceIoDclose(fd);
	}
	else
	{
		res = virtualpbp_close(fd);
		if (res < 0) goto label11;
		pspSdkSetK1(k1);
	}
	return res;
}

// OK
SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode)
{
	int res = -1, index;
	int k1 = pspSdkSetK1(0);

	FixPath(file, "__150", "150/");
	FixPath(file, "__5XX", "5XX/");

	printf("File: %s UID: 0x%08X\n", file, res);

	index = GetIsoIndex(file);
	if (index < 0)
	{
		if(strstr(file, "disc0:/PSP_GAME/PARAM.SFO") == 0)
		{
			pspSdkSetK1(k1);
			res = sceIoOpen(file, flags, mode);
		}
		else
		{
			pspSdkSetK1(k1);
			paramsfo = sceIoOpen(file, flags, mode);
			res = paramsfo;
		}
	}
	else
	{
		if (umdfile_is_iso != 0)
		{
			sub_00001064();
		}
		res = virtualpbp_open(index);
		pspSdkSetK1(k1);
	}

	return res;
}

// OK
int sceIoReadPatched(SceUID fd, void *data, SceSize size)
{
	int res = -1;
	int k1 = pspSdkSetK1(0);

	if (vpbpinited)
	{
		res = virtualpbp_read(fd, data, size);		
	}

	if (fd == paramsfo)
	{
		int i;

		pspSdkSetK1(k1);		
		res = sceIoRead(fd, data, size);
		pspSdkSetK1(0);

		if (res > 4)
		{
			for (i = 0; i < res-4; i++)
			{
				if (memcmp(data+i, MAJOR_VERSION".", 2) == 0)
				{
					if (strlen(data+i) == 4)
					{
						memcpy(data+i, MAJOR_VERSION"."MINOR_VERSION, 4);
						break;
					}
				}
			}
		}
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	if (res < 0)
		res = sceIoRead(fd, data, size);

	return res;
}

// OK
int sceIoClosePatched(SceUID fd)
{
	int res = -1;
	int k1 = pspSdkSetK1(0);

	if (vpbpinited)
	{
		res = virtualpbp_close(fd);
	}

	if (fd == paramsfo)
	{
		paramsfo = -1;
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoClose(fd);

	return res;
}

// OK
SceOff sceIoLseekPatched(SceUID fd, SceOff offset, int whence)
{
	int res = -1;
	int k1 = pspSdkSetK1(0);

	if (vpbpinited)
	{
		res = virtualpbp_lseek(fd, offset, whence);
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoLseek(fd, offset, whence);

	return res;
}

// OK
int sceIoLseek32Patched(SceUID fd, int offset, int whence)
{
	int res = -1;
	int k1 = pspSdkSetK1(0);

	if (vpbpinited)
	{
		res = virtualpbp_lseek(fd, offset, whence);
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoLseek32(fd, offset, whence);

	return res;
}

// OK
int sceIoGetstatPatched(const char *file, SceIoStat *stat)
{
	int index;
	int k1 = pspSdkSetK1(0);

	FixPath(file, "__150", "150/");
	FixPath(file, "__5XX", "5XX/");

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_getstat(index, stat);
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	return sceIoGetstat(file, stat);
}

// OK
int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits)
{
	int index;
	int k1 = pspSdkSetK1(0);

	FixPath(file, "__150", "150/");
	FixPath(file, "__5XX", "5XX/");

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_chstat(index, stat, bits);
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	return sceIoChstat(file, stat, bits);
}

// OK
int sceIoRemovePatched(const char *file)
{
	int index;
	int k1 = pspSdkSetK1(0);

	FixPath(file, "__150", "150/");
	FixPath(file, "__5XX", "5XX/");

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_remove(index);
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	return sceIoRemove(file);
}

// OK
int sceIoRmdirPatched(const char *path)
{
	int index;
	int k1 = pspSdkSetK1(0);

	FixPath(path, "__150", "150/");
	FixPath(path, "__5XX", "5XX/");

	index = GetIsoIndex(path);
	if (index >= 0)
	{
		int res = virtualpbp_rmdir(index);
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	return sceIoRmdir(path);
}

// OK
int sceIoMkdirPatched(const char *dir, SceMode mode)
{
	int k1 = pspSdkSetK1(0);

	if (strcmp(dir, "ms0:/PSP/GAME") == 0)
	{
		sceIoMkdir("ms0:/PSP/GAME150", mode);
		sceIoMkdir("ms0:/PSP/GAME5XX", mode);
		sceIoMkdir("ms0:/ISO", mode);
		sceIoMkdir("ms0:/ISO", mode);
		sceIoMkdir("ms0:/ISO/VIDEO", mode);
		sceIoMkdir("ms0:/seplugins", mode);
	}

	pspSdkSetK1(k1);
	return sceIoMkdir(dir, mode);
}

// Patch Targets Updated
void PatchIoFileMgr()
{
	fmod = sceKernelFindModuleByName("sceIOFileManager");

#define SCEIODOPEN_ADDR   0x144C
#define SCEIODREAD_ADDR   0x15CC
#define SCEIODCLOSE_ADDR  0x167C
#define SCEIOOPEN_ADDR    0x3CD0
#define SCEIOCLOSE_ADDR   0x3C90
#define SCEIOREAD_ADDR    0x3DE8
#define SCEIOLSEEK_ADDR   0x3E58
#define SCEIOLSEEK32_ADDR 0x3E90
#define SCEIOGETSTAT_ADDR 0x3F84
#define SCEIOCHSTAT_ADDR  0x3FA4
#define SCEIOREMOVE_ADDR  0x171C
#define SCEIORMDIR_ADDR   0x3F44
#define SCEIOMKDIR_ADDR   0x3F28

	sctrlHENPatchSyscall(fmod->text_addr+SCEIODOPEN_ADDR, sceIoDopenPatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIODREAD_ADDR, sceIoDreadPatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIODCLOSE_ADDR, sceIoDclosePatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOOPEN_ADDR, sceIoOpenPatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOREAD_ADDR, sceIoReadPatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOCLOSE_ADDR, sceIoClosePatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOLSEEK_ADDR, sceIoLseekPatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOLSEEK32_ADDR, sceIoLseek32Patched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOGETSTAT_ADDR, sceIoGetstatPatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOCHSTAT_ADDR, sceIoChstatPatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOREMOVE_ADDR, sceIoRemovePatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIORMDIR_ADDR, sceIoRmdirPatched);// Updated
	sctrlHENPatchSyscall(fmod->text_addr+SCEIOMKDIR_ADDR, sceIoMkdirPatched);// Updated
}

// Patch Updated
void PatchVshMain(u32 text_addr)
{
	// Allow old sfo's.
#if _PSP_FW_VERSION == 500
	#define VSHMAIN_OLDSFO_ADDR1 0xEE30
	#define VSHMAIN_OLDSFO_ADDR2 0xEE38
	#define VSHMAIN_OLDSFO_ADDR3 0xF0D8
#endif
#if _PSP_FW_VERSION == 550
	#define VSHMAIN_OLDSFO_ADDR1 0xF690
	#define VSHMAIN_OLDSFO_ADDR2 0xF698
	#define VSHMAIN_OLDSFO_ADDR3 0xF938
#endif

	_sw(NOP, text_addr+VSHMAIN_OLDSFO_ADDR2);// Updated
	_sw(NOP, text_addr+VSHMAIN_OLDSFO_ADDR1);// Updated
	_sw(0x10000023, text_addr+VSHMAIN_OLDSFO_ADDR3);// New

	PatchIoFileMgr();
}

// Patch Updated
void PatchSysconfPlugin(u32 text_addr)
{
#if _PSP_FW_VERSION == 500
	#define SP_MODULE_NAME 0x23DE0
	#define SP_VERINFO_PATCH_ADDR 0x15EE0
	#define SP_MACINFO_PATCH_ADDR 0x27C48
#endif
#if _PSP_FW_VERSION == 550
	#define SP_MODULE_NAME 0x235D4
	#define SP_VERINFO_PATCH_ADDR 0x15D74
	#define SP_MACINFO_PATCH_ADDR 0x272FC
#endif

	// Patching Version Info
	SEVvv = sctrlSEGetVersion();
	//verinfo[9] = (SEVvv&0xF)+0xXX; (0xXX = Hex code of Caractere)
	verinfo[9] = (SEVvv&0xF)+0x31;

	memcpy((void *)(text_addr+SP_MODULE_NAME), verinfo, sizeof(verinfo));
	_sw(0x3C020000 | ((text_addr+SP_MODULE_NAME) >> 16), text_addr+SP_VERINFO_PATCH_ADDR);
	_sw(0x34420000 | ((text_addr+SP_MODULE_NAME) & 0xFFFF), text_addr+SP_VERINFO_PATCH_ADDR+4);

	// Patching Mac Info
	#if _PSP_FW_VERSION == 500
		memcpy((void *)(text_addr+SP_MACINFO_PATCH_ADDR), macinfo, sizeof(macinfo));
	#endif
	#if _PSP_FW_VERSION == 550
	if(config.hidemac)
		memcpy((void *)(text_addr+SP_MACINFO_PATCH_ADDR), macinfo, sizeof(macinfo));
	#endif
}

// Patch Updated
void PatchGamePlugin(u32 text_addr)
{
#if _PSP_FW_VERSION == 500
	#define GP_PATCH_ADDR1 0x0F7A8
	#define GP_PATCH_ADDR2 0x0F7B4
	#define GP_PATCH_ADDR3 0x11764
	#define GP_PATCH_ADDR4 0x11768
#endif
#if _PSP_FW_VERSION == 550
	#define GP_PATCH_ADDR1 0x10A14
	#define GP_PATCH_ADDR2 0x10A20
	#define GP_PATCH_ADDR3 0x129D0
	#define GP_PATCH_ADDR4 0x129D4
#endif

	_sw(JR_RA, text_addr+GP_PATCH_ADDR3);// Updated
	_sw(MOVE_2, text_addr+GP_PATCH_ADDR4);// Updated

	if(config.hidepics){
		_sw(MOVE_1, text_addr+GP_PATCH_ADDR1);// PIC0.PNG
		_sw(MOVE_1, text_addr+GP_PATCH_ADDR2);// PIC1.PNG
	}
}

/*void PatchHtmlViewerPlugin(u32 text_addr)
{
	_sw(0x3C020206, text_addr+0x04E5C);
	_sw(0x3C0A0206, text_addr+0x1059C);
	_sw(0x30312E36, text_addr+0x1AF10);
	_sw(0x10000206, text_addr+0x1D520);
}*/

// Patch Targets Updated
void PatchMsVideoMainPlugin(u32 text_addr)
{
	// Patch resolution limit to 130560 pixels (480x272)
    // Allow play avc <= 480*272
#if _PSP_FW_VERSION == 500
	#define MSVM_RES_PATCH1_ADDR 0x2F3E4
	#define MSVM_RES_PATCH2_ADDR 0x2F46C
	#define MSVM_RES_PATCH3_ADDR 0x31B88
	#define MSVM_RES_PATCH4_ADDR 0x31BFC
	#define MSVM_RES_PATCH5_ADDR 0x31D08
	#define MSVM_RES_PATCH6_ADDR 0x377A4
	#define MSVM_RES_PATCH7_ADDR 0x5EB20
	#define MSVM_RES_PATCH8_ADDR 0x7088C
	#define MSVM_BR_PATCH1_ADDR  0x31B24
	#define MSVM_BR_PATCH2_ADDR  0x31BB0
#endif
#if _PSP_FW_VERSION == 550
	#define MSVM_RES_PATCH1_ADDR 0x2F77C
	#define MSVM_RES_PATCH2_ADDR 0x2F804
	#define MSVM_RES_PATCH3_ADDR 0x32118
	#define MSVM_RES_PATCH4_ADDR 0x3218C
	#define MSVM_RES_PATCH5_ADDR 0x32278
	#define MSVM_RES_PATCH6_ADDR 0x37EA8
	#define MSVM_RES_PATCH7_ADDR 0x5F20C
	#define MSVM_RES_PATCH8_ADDR 0x71E64
	#define MSVM_BR_PATCH1_ADDR  0x320B4
	#define MSVM_BR_PATCH2_ADDR  0x32140
#endif

	_sh(0xFE00, text_addr+MSVM_RES_PATCH1_ADDR);// Updated
	_sh(0xFE00, text_addr+MSVM_RES_PATCH2_ADDR);// Updated
	_sh(0xFE00, text_addr+MSVM_RES_PATCH3_ADDR);// Updated
	_sh(0xFE00, text_addr+MSVM_RES_PATCH4_ADDR);// Updated
	_sh(0xFE00, text_addr+MSVM_RES_PATCH5_ADDR);// Updated
	_sh(0xFE00, text_addr+MSVM_RES_PATCH6_ADDR);// Updated
	_sh(0xFE00, text_addr+MSVM_RES_PATCH7_ADDR);// Updated
	_sh(0xFE00, text_addr+MSVM_RES_PATCH8_ADDR);// Updated
#ifdef MSVM_RES_PATCH9_ADDR
	_sh(0xFE00, text_addr+MSVM_RES_PATCH9_ADDR);
#endif

	// Patch bitrate limit	(increase to 16387)
	_sh(0x4003, text_addr+MSVM_BR_PATCH1_ADDR);// Updated
	_sh(0x4003, text_addr+MSVM_BR_PATCH2_ADDR);// Updated
}

int vshCtrlReadBufferPositivePatched(SceCtrlData *pad_data, int count)
{
	int ret = sceCtrlReadBufferPositive(pad_data, count);
	int k1 = pspSdkSetK1(0);

	if(unk_1 != 0)
	{
		if(config.vshcpuspeed != 0)
		{
			if(config.vshcpuspeed != 222)
			{
				if(scePowerGetCpuClockFrequency() == 222)
				{
					timelow3 = sceKernelGetSystemTimeLow();
					if(timelow3-timelow2 >= 1000000)
					{
						SetSpeed(config.vshcpuspeed, config.vshbusspeed);
						timelow2 = timelow3;
					}
				}
			}
		}
	}
	else
	{
		if(config.vshcpuspeed != 0)
		{
			timelow3 = sceKernelGetSystemTimeLow();
			if(timelow3-timelow1 >= 10000000)// 10 seconds (O_O)
			{
				unk_1 = 1;
				SetSpeed(config.vshcpuspeed, config.vshbusspeed);
				timelow2 = timelow3;
			}
		}
	}

	if(sceKernelFindModuleByName("VshCtrlSatelite"))
	{
		if(sceCtrlReadBufferPositiveReal == NULL)
		{
			if(vshmenu_mod >= 0)
			{
				if(sceKernelStopModule(vshmenu_mod, 0, NULL, NULL, NULL) >= 0)
					sceKernelUnloadModule(vshmenu_mod);
			}
		}
		else
		{
			sceCtrlReadBufferPositiveReal(pad_data, count);
		}
	}
	else
	{
		if(!sceKernelFindModuleByName("htmlviewer_plugin_module"))
		{
			if(!sceKernelFindModuleByName("sceVshOSK_Module"))
			{
				if(!sceKernelFindModuleByName("camera_plugin_module"))
				{
					if(pad_data->Buttons & PSP_CTRL_SELECT)
					{
						sceKernelSetDdrMemoryProtection((void*)0x08400000, 0x00400000, 0xF);

						sateliteLMopt.size = sizeof(sateliteLMopt);
						sateliteLMopt.mpiddata = 5;
						sateliteLMopt.creserved[2] = 1;
						sateliteLMopt.mpidtext = 5;

						vshmenu_mod = sceKernelLoadModule("flash0:/vsh/module/satelite.prx", 0, &sateliteLMopt);
						if(vshmenu_mod >= 0)
						{
							char* umd_file = NULL;
							if(umdfile_is_iso)
							{
								umd_file = sctrlSEGetUmdFile();
								//strlen(umd_file)+1
							}
							sceKernelStartModule(vshmenu_mod, 0, umd_file, NULL, NULL);
							pad_data->Buttons = (pad_data->Buttons&0xFFFFFFFE);
						}
					}
				}
			}
		}
	}
	pspSdkSetK1(k1);
	return ret;
}

void sceResmgr_driver_9DC14891Patched(char *arg1, SceSize size, int arg3)
{
	sceResmgr_driver_9DC14891Real(arg1, size, arg3);
	int k1 = pspSdkSetK1(0);

	if (strstr(arg1, "release:"MAJOR_VERSION"."MINOR_VERSION) != NULL)
	{
		SceUID versiontxt = sceIoOpen("flash0:/vsh/etc/version.txt", PSP_O_RDONLY, 0777);
		if (versiontxt >= 0)
		{
			int bytes = sceIoRead(versiontxt, arg1, size);
			((int *) arg3)[0] = bytes;
			sceIoClose(versiontxt);
			pspSdkSetK1(k1);
		}
	}
	else
	{
		pspSdkSetK1(k1);
	}
}

SceUID vshKernelLoadModuleVSHPatched(const char *file, int flags, SceKernelLMOption *option)
{
	SceUID modid = sceKernelLoadModuleVSH(file, flags, option);
	if(modid >= 0)
	{
		int k1 = pspSdkSetK1(0);
		if(!config.notusedaxupd)
		{
			fmod = sceKernelFindModuleByName("SceUpdateDL_Library");
			if(fmod)
			{
				if(sceKernelFindModuleByName("sceVshNpSignin_Module") == NULL)
				{
					if(sceKernelFindModuleByName("npsignup_plugin_module") == NULL)
					{
#if _PSP_FW_VERSION == 500
						// DA Patch
						strcpy(((char*)fmod->text_addr+0x2B6C), "http://www.blackdevsteam.net/zer01ne/psp-updatelist.txt");
						strcpy(((char*)fmod->text_addr+0x2BB0), "http://www.blackdevsteam.net/zer01ne/psp-updatelist.txt");
						strcpy(((char*)fmod->text_addr+0x2B28), "http://www.blackdevsteam.net/zer01ne/psp-updatelist.txt");
						strcpy(((char*)fmod->text_addr+0x2BF4), "http://www.blackdevsteam.net/zer01ne/psp-updatelist.txt");
#endif
#if _PSP_FW_VERSION == 550
						// Total_Noob Patch
						_sw(NOP, fmod->text_addr+0x2044);
						_sw(NOP, fmod->text_addr+0x2080);
						_sw(NOP, fmod->text_addr+0x208C);
						strcpy(((char*)fmod->text_addr+0x329C), "www.blackdevsteam.net/zer01ne/");
						strcpy(((char*)fmod->text_addr+0x32D4), "psp-updatelist.txt");
						// BubbleTune Patch
						//_sw(fmod->text_addr+0x3294, fmod->text_addr+0x34A0);
						//strcpy(((char*)fmod->text_addr+0x3294), "http://www.blackdevsteam.net/zer01ne/psp-updatelist.txt");
#endif
					}
				}
			}
		}
		fmod = sceKernelFindModuleByName("npsignup_plugin_module");
		if(fmod)
		{
			_sw(0x10000008, fmod->text_addr+0x44B8);
		}
		fmod = sceKernelFindModuleByName("sceVshNpSignin_Module");
		if(fmod)
		{
#if _PSP_FW_VERSION == 500
			_sw(0x10000008, fmod->text_addr+0x44B8);
#endif
#if _PSP_FW_VERSION == 550
			_sw(0x10000008, fmod->text_addr+0x6760);
			_sw(0x10000018, fmod->text_addr+0x6810);
#endif
		}
		fmod = sceKernelFindModuleByName("sceNp");
		if(fmod)
		{
			// 0x24060005: li $a2, 5 -> li $a2, 9
			//_sw(0x24060009, mod->text_addr+0x4604);
			_sw(0x24060006, fmod->text_addr+0x4604);
			_sw(0x24070014, fmod->text_addr+0x460C);
		}
#if _PSP_FW_VERSION == 550
		fmod = sceKernelFindModuleByName("sceVshNpInstaller_Module");
		if(fmod)
			_sw(NOP, fmod->text_addr+0x4FEC);
#endif
		pspSdkSetK1(k1);
	}
	return modid;
}

void PatchUpdatePlugin(u32 text_addr)
{
#if _PSP_FW_VERSION == 500
	#define VVV_ADDR_LOW 0x1F44
	#define VVV_ADDR_HIGH 0x210C
#endif
#if _PSP_FW_VERSION == 550
	#define VVV_ADDR_LOW 0x1F58
	#define VVV_ADDR_HIGH 0x2120
#endif
	SEVvv = sctrlSEGetVersion();
	_sw(0x3C050000 | (SEVvv >> 16), text_addr+VVV_ADDR_LOW);
	_sw(0x34A40000 | (SEVvv & 0xFFFF), text_addr+VVV_ADDR_HIGH);
}

int sceUsbStartPatched(const char* driverName, int size, void *args)
{
	int k1 = pspSdkSetK1(0);

	if(strcmp(driverName, "USBStor_Driver") == 0)
	{
		if(config.usbdevice > USBDEVICE_MEMORYSTICK)
		{
			if(config.usbdevice <= USBDEVICE_UMD9660)
			{
				if (umdfile_is_iso != 0)
				{
					if (config.usbdevice == USBDEVICE_UMD9660)
					{
						disc_is_no_out = 1;
						sub_00001064();
					}
				}
				if(usbdevice_mod < 0)
					usbdevice_mod = sceKernelLoadModule("flash0:/kd/usbdevice.prx", 0, NULL);
				if(usbdevice_mod >= 0)
				{
					if(sceKernelStartModule(usbdevice_mod, 0, NULL, NULL, NULL) < 0)
					{
						sceKernelUnloadModule(usbdevice_mod);
						usbdevice_mod = -1;
					}
					else
					{
						if(pspUsbDeviceSetDevice((config.usbdevice + 0xFFFFFFFF), 0, 0) < 0)
						{
							pspUsbDeviceSetDevice((USBDEVICE_MEMORYSTICK + 0xFFFFFFFF), 0, 0);
						}
					}
				}
			}
		}
	}

	pspSdkSetK1(k1);
	return sceUsbStartReal(driverName, size, args);
}

int sceUsbStopPatched(const char* driverName, int size, void *args)
{
	int res = sceUsbStopReal(driverName, size, args);
	int k1 = pspSdkSetK1(0);

	if(strcmp(driverName, "USBStor_Driver") == 0)
	{
		if(usbdevice_mod >= 0)
		{
			if(config.usbdevice > USBDEVICE_MEMORYSTICK)
			{
				if(config.usbdevice <= USBDEVICE_UMD9660)
				{
					pspUsbDeviceFinishDevice();
					if(sceKernelStopModule(usbdevice_mod, 0, NULL, NULL, NULL) >= 0)
					{
						sceKernelUnloadModule(usbdevice_mod);
						usbdevice_mod = -1;
					}
					if(disc_is_no_out != 0)
					{
						if(config.usbdevice == USBDEVICE_UMD9660)
						{
							sctrlSESetDiscOut(1);
							sctrlSEMountUmdFromFile(sctrlSEGetUmdFile(), 0, 0);
							umdfile_is_iso = 1;
							disc_is_no_out = 0;
						}
					}
				}
			}
		}
	}
	pspSdkSetK1(k1);
	return res;
}

int vshIoDevctlPatched(const char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int ret = sceIoDevctl(dev, cmd, indata, inlen, outdata, outlen);

	if (cmd == 0x01E18030)
	{
		ret = (umdfile_is_iso) ? 1 : ret;
	}

	return ret;
}

void PatchDriver()
{
	fmod = sceKernelFindModuleByName("sceVshBridge_Driver");
	if (!config.vshmenu)
	{
		MAKE_CALL(fmod->text_addr+0x264, vshCtrlReadBufferPositivePatched);
		u32 var11 = sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
		sceCtrlReadBufferPositiveReal = (void*)var11;
		sctrlHENPatchSyscall(var11, vshCtrlReadBufferPositivePatched);
	}

#if _PSP_FW_VERSION == 500
	#define DEVCTL_ADDR 0x724
#endif
#if _PSP_FW_VERSION == 550
	#define DEVCTL_ADDR 0x3EEC
#endif
	MAKE_CALL(fmod->text_addr+DEVCTL_ADDR, vshIoDevctlPatched);

	MAKE_CALL(fmod->text_addr+0x10D4, vshKernelLoadModuleVSHPatched);
	/*u32 orgaddr = (void*)sctrlHENFindFunction(fmod->modname, "sceVshBridge", 0xA5628F0D);
	sctrlHENPatchSyscall(orgaddr, vshKernelLoadModuleVSHPatched);*/

	if(config.useversiontxt)
	{
		u32 var20 = sctrlHENFindFunction("sceMesgLed", "sceResmgr", 0x9DC14891);
		sceResmgr_driver_9DC14891Real = (void*)var20;
		sctrlHENPatchSyscall(var20, sceResmgr_driver_9DC14891Patched);
	}

	u32 var25 = sctrlHENFindFunction("sceUSB_Driver", "sceUsb", 0xAE5DE6AF);
	sceUsbStartReal = (void*)var25;
	sctrlHENPatchSyscall(var25, sceUsbStartPatched);
	u32 var30 = sctrlHENFindFunction("sceUSB_Driver", "sceUsb", 0xC2464FA0);
	sceUsbStopReal = (void*)var30;
	sctrlHENPatchSyscall(var30, sceUsbStopPatched);
}

int OnModuleRelocated(SceModule2* mod)
{
/* 
	// Patch found on moon.prx
	// Give Error 80550424 An error has occurred. You have been disconnected from PlayStationNetwork.
	mod = sceKernelFindModuleByName("sceNpCore");
	if(mod){
		_sw(NOP, mod->text_addr+0x109C);
		_sw(0x06020010, mod->text_addr+0x1144);
	}
	// Not Realy Useful
	mod = sceKernelFindModuleByName("sceNpAuth");
	if(mod)
		_sw(0x06020010, mod->text_addr+0x2B74);
	mod = sceKernelFindModuleByName("SceParseHTTPheader_Library");
	if(mod)
		_sw(0x06020010, mod->text_addr+0x083C);
	mod = sceKernelFindModuleByName("SceHttp_Library");
	if(mod)
		_sw(0x06020010, mod->text_addr+0x18A9C);
	mod = sceKernelFindModuleByName("sceNpService");
	if(mod)
		_sw(0x06020010, mod->text_addr+0x115D0);
	if (strcmp(mod->modname, "sceVshNpSignin_Module") == 0)
		_sw(0x06020010, mod->text_addr+0xBB9C);
	// Patch found on moon.prx End
*/
	if(strcmp(mod->modname, "vsh_module") == 0){
		PatchVshMain(mod->text_addr);
	}
	else if(strcmp(mod->modname, "sysconf_plugin_module") == 0){
		PatchSysconfPlugin(mod->text_addr);
	}
	/*else if (strcmp(mod->modname, "htmlviewer_plugin_module") == 0){
		PatchHtmlViewerPlugin(mod->text_addr);
	}*/
	else if(strcmp(mod->modname, "msvideo_main_plugin_module") == 0)
	{
		PatchMsVideoMainPlugin(mod->text_addr);
	}
	else if(strcmp(mod->modname, "game_plugin_module") == 0){
		PatchGamePlugin(mod->text_addr);
	}
	else if(strcmp(mod->modname, "update_plugin_module") == 0)
	{
		if (config.notusedaxupd == 0)
			PatchUpdatePlugin(mod->text_addr);
	}

	PatchDriver();

	ClearCaches();

	if (!previous)
		return 0;

	return previous(mod);
}

int module_start(SceSize args, void *argp)
{
	fmod = sceKernelFindModuleByName("sceLoadExec");
	if(fmod)
	{
#if _PSP_FW_VERSION == 500
	#define LE_VSH_COMMON_CALL1_ADDR 0x17FC
	#define LE_VSH_COMMON_CALL2_ADDR 0x1824
	#define LE_VSH_COMMON_CALL3_ADDR 0x1974
	#define LE_VSH_COMMON_CALL4_ADDR 0x199C
#endif
#if _PSP_FW_VERSION == 550
	#define LE_VSH_COMMON_CALL1_ADDR 0x18B8
	#define LE_VSH_COMMON_CALL2_ADDR 0x18E0
	#define LE_VSH_COMMON_CALL3_ADDR 0x1A30
	#define LE_VSH_COMMON_CALL4_ADDR 0x1A58
#endif
		MAKE_CALL(fmod->text_addr+LE_VSH_COMMON_CALL1_ADDR, LoadExecVSHCommonPatched); // Updated sceKernelLoadExecVSHDisc
		MAKE_CALL(fmod->text_addr+LE_VSH_COMMON_CALL2_ADDR, LoadExecVSHCommonPatched); // Updated sceKernelLoadExecVSHDiscUpdater
		MAKE_CALL(fmod->text_addr+LE_VSH_COMMON_CALL3_ADDR, LoadExecVSHCommonPatched); // Updated sceKernelLoadExecVSHMs1
		MAKE_CALL(fmod->text_addr+LE_VSH_COMMON_CALL4_ADDR, LoadExecVSHCommonPatched); // Updated sceKernelLoadExecVSHMs2
	}

	fmod = sceKernelFindModuleByName("SystemControl");
	if(fmod)
	{
		oe_malloc = (void*)FindSystemCtrlKernelFunction(0xF9584CAD); 
		oe_free = (void*)FindSystemCtrlKernelFunction(0xA65E8BC4);
		isofs_init = (void*)FindSystemCtrlKernelFunction(0x260CA420);
		isofs_fastinit = (void*)FindSystemCtrlKernelFunction(0x7E6F2BBA);
		isofs_exit = (void*)FindSystemCtrlKernelFunction(0xB078D9A0);
		isofs_open = (void*)FindSystemCtrlKernelFunction(0xE82E932B);
		isofs_close = (void*)FindSystemCtrlKernelFunction(0xCAE6A8E1);
		isofs_read = (void*)FindSystemCtrlKernelFunction(0x324FB7B1);
		isofs_lseek = (void*)FindSystemCtrlKernelFunction(0x3BC8E648);
		isofs_getstat = (void*)FindSystemCtrlKernelFunction(0xDC974FF8);
		SetSpeed = (void*)FindSystemCtrlKernelFunction(0x98012538);
		SetConfig = (void*)FindSystemCtrlKernelFunction(0x2F157BAF);
		SetUmdFile = (void*)FindSystemCtrlKernelFunction(0xB64186D0);
	}

	sctrlSEGetConfig(&config);

	if(config.vshcpuspeed != 0)
		timelow1 = sceKernelGetSystemTimeLow();

	ClearCaches();
	previous = sctrlHENSetStartModuleHandler(OnModuleRelocated);
	return 0;
}


/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	kubridge.c: SystemControl KuBridge Lib Code
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
#include <pspmodulemgr_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspinit.h>
#include <kubridge.h>

#include <string.h>

// OK
SceUID kuKernelLoadModule(const char *path, int flags, SceKernelLMOption *option)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadModule(path, flags, option);
	pspSdkSetK1(k1);
	return res;
}

// OK
SceUID kuKernelLoadModuleWithApitype2(int apitype, const char *path, int flags, SceKernelLMOption *option)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadModuleForLoadExecForUser(apitype, path, flags, option);
	pspSdkSetK1(k1);
	return res;
}

// OK
int kuKernelInitApitype()
{
	return sceKernelInitApitype();
}

// OK
int kuKernelInitFileName(char *filename)
{
	int k1 = pspSdkSetK1(0);
	strcpy(filename, sceKernelInitFileName());
	pspSdkSetK1(k1);
	return 0;
}

// OK
int kuKernelBootFrom()
{
	return sceKernelBootFrom();
}

// OK
int kuKernelInitKeyConfig()
{
	return kuKernelApplicationType();
	//return sceKernelInitKeyConfig();
}

// OK
int kuKernelGetUserLevel(void)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetUserLevel();
	pspSdkSetK1(k1);
	return res;
}

// OK
int kuKernelSetDdrMemoryProtection(void *addr, int size, int prot)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelSetDdrMemoryProtection(addr, size, prot);
	pspSdkSetK1(k1);
	return res;
}

// OK
int kuKernelGetModel(void)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetModel();
	pspSdkSetK1(k1);
	return res;
}


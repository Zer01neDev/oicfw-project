/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	malloc.c: SystemControl Malloc Code
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
#include <pspsysmem_kernel.h>

#include "malloc.h"

SceUID heapid = -1;

int oe_malloc_init()
{
	int ret;
	int heap;
	int apptype = sceKernelApplicationType();
	if (apptype == PSP_INIT_KEYCONFIG_VSH)
	{
		heap = sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, 0x0000B000, 1, "SctrlHeap"); // "SctrlHeap" = 0x00007A24
		ret = (heap < 0) ? heap : 0;
	}
	else
	{
		ret = 0;
		if (apptype == PSP_INIT_KEYCONFIG_GAME)
		{
			if (sceKernelInitApitype() != 0x123)
				ret = (heap < 0) ? heap : 0;
			else
				ret = 0;
		}
	}
	return ret;
}

int mallocterminate()
{
	return sceKernelDeleteHeap(heapid);
}

// OK
int oe_malloc(SceSize size)
{
	Kprintf("free: %d\n", sceKernelHeapTotalFreeSize(heapid));
	return sceKernelAllocHeapMemory(heapid, size);
}

// OK
void oe_free(void *ptr)
{
	Kprintf("freex: %d\n", sceKernelHeapTotalFreeSize(heapid));
	sceKernelFreeHeapMemory(heapid, ptr);
}


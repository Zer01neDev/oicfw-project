/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	ctr0.c: Recovery Main Code
	This file are based from a reverse of M33/GEN Recovery

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

extern int main_thread(SceSize args, void *argp);

int module_start(SceSize args, void *argp)
{
	if(args == 0){
		SceUID recovery_thid = sceKernelCreateThread("user_main", main_thread, 0x10, 0x4000, 0, NULL);
		sceKernelStartThread(recovery_thid, args, argp);
	} else main_thread(0, NULL);
	return 0;
}


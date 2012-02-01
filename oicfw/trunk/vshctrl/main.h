/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	main.h: VshControl Main Header Code
	This code are taken from OE/Wildcard Source Code

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

#ifndef ___MAIN_H___
#define ___MAIN_H___

#include <pspctrl.h>

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000
#define SC_OPCODE	0x0000000C
#define JR_RA		0x03E00008
#define MOVE_1		0x00601021
#define MOVE_2		0x00001021
#define MOVE_3		0x00002021
#define MOVE_4		0x02E02021
#define LUI			0x3C080000
#define NOP			0x00000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0FFFFFFC) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2) & 0x03FFFFFF), a); 
#define MAKE_SYSCALL(a, n) _sw(SC_OPCODE | (n << 6), a);
#define JUMP_TARGET(x) (0x80000000 | ((x & 0x03FFFFFF) << 2))
#define MAKE_PATCH(a, f) a = ((((int)f >> 0x00000002) & 0x03FFFFFF) | JAL_OPCODE);

#define REDIRECT_FUNCTION(a, f) _sw(J_OPCODE | (((u32)(f) >> 2) & 0x03FFFFFF), a);  _sw(NOP, a+4);
#define MAKE_DUMMY_FUNCTION0(a) _sw(0x03E00008, a); _sw(0x00001021, a+4);
#define MAKE_DUMMY_FUNCTION1(a) _sw(0x03E00008, a); _sw(0x24020001, a+4);


void ClearCaches();
u32  sctrlHENFindFunction(const char* szMod, const char* szLib, u32 nid);
void PatchSyscall(u32 funcaddr, void *newfunc);
void (*SetSpeed)(int cpu, int bus);
void (*SetConfig)(SEConfig *newconfig);
int (*sceCtrlReadBufferPositiveReal)(SceCtrlData *pad_data, int count);
void sub_00001064();

int sceKernelCallSubIntrHandler(int unk1, u32 unk2, int unk3, int unk4);

#define FindProc sctrlHENFindFunction

#endif


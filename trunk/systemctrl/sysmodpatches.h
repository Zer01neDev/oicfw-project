/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	sysmodpatches.h: SystemControl System Module Patches Header Code
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

#ifndef __SYSMODPATCHES_H__
#define __SYSMODPATCHES_H__

#include "systemctrl_se.h"

void SetConfig(SEConfig *newconfig);
SEConfig *GetConfig();
void PatchUmdMan(u32 text_addr);
void PatchNandDriver(u32 text_addr);
void PatchInitLoadExecAndMediaSync(u32 text_addr);
void PatchVshMain(u32 text_addr);
void PatchSysconfPlugin(u32 text_addr);
void OnImposeLoad();
void DoNoUmdPatches();
void PatchIsofsDriver(u32 text_addr);
void PatchWlan(u32 text_addr);
void PatchPower(u32 text_addr);

#endif


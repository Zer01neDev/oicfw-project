/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	mydebug.c: Recovery Debug Screen Code
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

/*****************************************
 * Recovery - mydebug			 *
 * 			by harleyg :)	 *
 *****************************************/

#ifndef __MYDEBUG_H__
#define __MYDEBUG_H__

#include <psptypes.h>
#include <pspmoduleinfo.h>

void myDebugScreenInit(void);
void myDebugScreenPrintf(const char *fmt, ...) __attribute__((format(printf,1,2)));
void myDebugScreenSetBackColor(u32 color);
void myDebugScreenSetTextColor(u32 color);
void myDebugScreenPutChar(int x, int y, u32 color, u8 ch);
void myDebugScreenSetXY(int x, int y);
void myDebugScreenSetOffset(int offset);
int myDebugScreenGetX(void);
int myDebugScreenGetY(void);
void myDebugScreenClear(void);
int myDebugScreenPrintData(const char *buff, int size);

#endif


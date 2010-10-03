/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	menu.c: Recovery Menu Maker Code
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
 * Recovery - menu			 *
 * 			by harleyg :)	 *
 * 	based on menu example by danzel	 *
 *****************************************/

#include <pspctrl.h>
#include <psputils.h>
#include <pspthreadman.h>
#include "mydebug.h"

#define SELECTBUTTON PSP_CTRL_CROSS
#define CANCELBUTTON PSP_CTRL_TRIANGLE
#define RGB(r, g, b) (0xFF000000 | ((b)<<16) | ((g)<<8) | (r))
#define printf myDebugScreenPrintf
#define setTextColor myDebugScreenSetTextColor
#define setXY myDebugScreenSetXY
#define clearScreen myDebugScreenClear

int doMenu(char* picks[], int count, int selected, char* message, int x, int y, int r, int g, int b, char *bar)
{
	int j;
	int done = 0;

	while (!done)
	{
		int i;
		SceCtrlData pad;
		int onepressed = 0;
		setXY(0, 1);
		setTextColor(RGB(r, g, b));
		printf("Open Idea Recovery mode\n");
		printf("%s", message);

		for(i = 0; i < count; i++)
		{
			setXY(x, y+i);
			if(picks[i] == 0) break;
			if(selected == i)
			{
				setTextColor(RGB(r, g, b));
				printf(" %s\n", picks[i]);
			} else {
				setTextColor(RGB(190, 190, 190));
				printf(" %s\n", picks[i]);
			}
		}
		setTextColor(RGB(r, g, b));
		setXY(0, y+23);
		//printf(" ******************************************************************* \n");
		//printf(" ******************************************************************* \n");
		printf(" ");

		for(j = 0; j < 67; j++)
			printf("%s", bar);
		printf(" \n");

		while (!onepressed)
		{
			sceCtrlReadBufferPositive(&pad, 1);
			onepressed =
			((pad.Buttons & SELECTBUTTON) ||
			(pad.Buttons & CANCELBUTTON) ||
			(pad.Buttons & PSP_CTRL_UP) ||
			(pad.Buttons & PSP_CTRL_DOWN));
		}
		if (pad.Buttons & SELECTBUTTON)	{ done = 1; }
		if (pad.Buttons & PSP_CTRL_UP) { selected = (selected + count - 1) % count; }
		if (pad.Buttons & PSP_CTRL_DOWN){ selected = (selected+1) % count; }
		if (pad.Buttons & CANCELBUTTON) { done = 1; selected = -1; }
		while (onepressed)
		{
			sceCtrlReadBufferPositive(&pad, 1); 
			onepressed =
			((pad.Buttons & SELECTBUTTON) ||
			(pad.Buttons & CANCELBUTTON) ||
			(pad.Buttons & PSP_CTRL_UP) ||
			(pad.Buttons & PSP_CTRL_DOWN));
		}
	}
	return selected;
}


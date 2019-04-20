/*
*   This file is part of ModMoon
*   Copyright (C) 2018-2019 Swiftloke
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

#include "main.hpp"
#include "titleselects.hpp"
#include "utils.hpp"
#include "initialsetup.hpp"
#include "error.hpp"
#include "toolsmenu.hpp"
/*Ah. It's great to finally see this in action. This button hasn't had a true implementation
since the beginning of the project, WAY back in September 2017.
There are five options to work with, they should just call various parts of the project.
-Smash Controls Modifier
-Active Title Selection
-Tutorial
-Mods migrator
-Dark Mode toggle (this may later be expanded into a full settings menu)
*/


//Unlike the main menu "shift in" logic, which was developed at the very beginning of the project,
//this logic uses sDraw's significant developments since then. Instead of being weird and playing
//with pixel positions, we'll just use the vertex interpolation functionality.
//Final x position should be 18.
//Y values- 6, 49, 93, 138, 184 (a 43-45 pixel change)
//Coming in from the left, -290 (the width of the button + 1)
//Coming in from the right, 321 (screen width + 1)

bool toolsmenushift(float& interpfactor, bool plus)
{
	if (plus)
	{
		interpfactor += 0.05;
		if (interpfactor > 1)
		{
			interpfactor = 1;
			return true;
		}
	}
	else
	{
		interpfactor -= 0.05;
		if (interpfactor < 0)
		{
			interpfactor = 0;
			return true;
		}
	}
	return false;
}

void toolsmenudraw(float interpfactor, int position, int highlighteralpha, bool shouldblink)
{
	const int initialxvals[] = { -290, 401, -290, 401, -290 };
	const int toolsyvals[] = { 6, 56, 102, 146, 190 };
	//Where the highlighter goes to actually be over the button... Some buttons have
	//parts that stick out.
	const int highlighteradds[] = { 3, 0, 2, 2, 4 };
	sdraw::framestart();
	sdraw::MM::shader_basic->bind();
	if (shouldblink)
	{
		sdraw::setfs("constColor", 0, RGBA8(255, 255, 255, 255));
		sdraw::drawrectangle(0, 0, 400, 240);
		sdraw::drawon(GFX_BOTTOM);
		sdraw::drawrectangle(0, 0, 320, 240);
	}
	else
	{
		drawtopscreen();
		sdraw::drawon(GFX_BOTTOM);
		sdraw::MM::shader_twocoords->bind();
		sdraw::setfs("texture");
		sdraw::drawtexture(backgroundbot, 0, 0);
		sdraw::setfs("highlighter", 0, HIGHLIGHTERCOLORANDALPHA(toolsmenuhighlighter.highlightercolor, highlighteralpha));
		sdraw::drawtexture(toolsmenuhighlighter, initialxvals[position] - 13, \
			toolsyvals[position] - 9 + highlighteradds[position], \
			18 - 15, toolsyvals[position] - 9 + highlighteradds[position], interpfactor);
		sdraw::setfs("texture");
		sdraw::drawtexture(activetitlesbutton,  initialxvals[0], toolsyvals[0], 18, toolsyvals[0], interpfactor);
		sdraw::drawtexture(smashcontrolsbutton, initialxvals[1], toolsyvals[1], 18, toolsyvals[1], interpfactor);
		sdraw::drawtexture(tutorialbutton,      initialxvals[2], toolsyvals[2], 18, toolsyvals[2], interpfactor);
		sdraw::drawtexture(modinstallbutton,     initialxvals[3], toolsyvals[3], 18, toolsyvals[3], interpfactor);
		sdraw::drawtexture(sdraw::darkmodeshouldactivate ? lightmodebutton : darkmodebutton, \
											    initialxvals[4], toolsyvals[4], 18, toolsyvals[4], interpfactor);
	}
	sdraw::frameend();
}

void toolsmenushiftout()
{
	float shift = 1;
	while (shift > 0)
	{
		toolsmenushift(shift, false);
		toolsmenudraw(shift, 0, 0, false);
	}
}

void toolsmenushiftin()
{
	float shift = 0;
	while (shift < 1)
	{
		toolsmenushift(shift, true);
		toolsmenudraw(shift, 0, 0, false);
	}
}

void toolsmenu()
{
	float shift = 0;
	bool shiftin = true;
	static int position = 0;
	int highlighteralpha = 0;
	bool highlighterplus = true;
	touchPosition tpos, opos;
	bool shouldblink = false; //Toggling dark mode needs this
	while (aptMainLoop())
	{
		toolsmenushift(shift, shiftin);
		highlighterhandle(highlighteralpha, highlighterplus);
		toolsmenudraw(shift, position, highlighteralpha, shouldblink);
		hidScanInput();
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		hidTouchRead(&tpos);
		if(kDown)
			highlighteralpha = 255;
		if (kDown & KEY_A)
		{
			switch (position)
			{
				case 0:
				{
					toolsmenushiftout();
					activetitleselect();
					toolsmenushiftin();
					break;
				}
				case 1: 
				{
					toolsmenushiftout();
					controlsmodifier();
					toolsmenushiftin();
					break;
				}
				case 2:
				{
					toolsmenushiftout();
					tutorial(false);
					toolsmenushiftin();
					break;
				}
				case 3:
				{
					modpackDownload();
					break;
				}
				case 4: 
				{
					shouldblink = true;
					while (kHeld & KEY_A)
					{
						hidScanInput();
						kHeld = hidKeysHeld();
						toolsmenudraw(shift, position, highlighteralpha, shouldblink);
					}
					shouldblink = false;
					toggledarkmode();
					break;
				}
			}
		}
		if(kDown & KEY_UP)
		{
			position--;
			if(position < 0)
				position = 4;
		}
		if (kDown & KEY_DOWN)
		{
			position++;
			if(position > 4)
				position = 0;
		}
		if (kDown & KEY_B)
		{
			shiftin = false;
			while (shift > 0)
			{
				toolsmenushift(shift, shiftin);
				highlighterhandle(highlighteralpha, highlighterplus);
				toolsmenudraw(shift, position, highlighteralpha, false);
			}
			break;
		}
		if (kDown & KEY_X)
		{
			string helptext;
			string helptext2 = "None";
			switch (position)
			{
			case 0: 
			{
				helptext = "Active Title Selection:\nAdd or remove titles from\nModMoon's list of titles to use."; 
				helptext2 = "See the in-menu help option\nfor more information.";
				break;
			}
			case 1: 
			{
				helptext = "Smash Controls Modifier:\nFeature for Super Smash Bros.\nfor Nintendo 3DS only.";
				helptext2 = "Allows you to modify the in-\ngame controls much further\nthan the game itself allows."; 
				break; 
			}
			case 2: helptext = "Tutorial:\nReplay the tutorial shown at the\nfirst start of ModMoon."; break;
			case 3: 	
			{
				helptext = "Modpack Installer:\nInstall mods from the internet!\nSimply enter a link to a zip file\nand let ModMoon do its thing.";
				break; 
			}
			case 4: helptext = " Mode:\nChange the theme of ModMoon!"; helptext.insert(0, sdraw::darkmodeshouldactivate ? "Light" : "Dark");
			}
			error(helptext);
			if(helptext2 != "None")
				error(helptext2);
		}
		if (kDown & KEY_Y) titleselect();
		//Get the highlighter in position
		if(touched(activetitlesbutton, 18, 6, tpos))
			position = 0;
		//Actually activate it
		else if (buttonpressed(activetitlesbutton, 18, 6, opos, kHeld))
		{
			toolsmenushiftout();
			activetitleselect();
			toolsmenushiftin();
		}

		if (touched(smashcontrolsbutton, 18, 49, tpos))
			position = 1;
		else if (buttonpressed(smashcontrolsbutton, 18, 49, opos, kHeld))
		{
			toolsmenushiftout();
			controlsmodifier();
			toolsmenushiftin();
		}

		if (touched(tutorialbutton, 18, 93, tpos))
			position = 2;
		else if (buttonpressed(tutorialbutton, 18, 93, opos, kHeld))
		{
			toolsmenushiftout();
			tutorial(false);
			toolsmenushiftin();
		}

		if (touched(migrationbutton, 18, 138, tpos))
			position = 3;
		else if (buttonpressed(migrationbutton, 18, 138, opos, kHeld))
		{
			modpackDownload();
		}
		
		//Dark mode / light mode buttons have the same coordinate info so it doesn't matter
		if (touched(darkmodebutton, 18, 184, tpos))
			shouldblink = true;
		else if (buttonpressed(darkmodebutton, 18, 184, opos, kHeld))
			toggledarkmode();
		else
			shouldblink = false;

		opos = tpos;
	}
}

void toggledarkmode()
{
	bool currentdarkmodestatus = sdraw::darkmodeshouldactivate;
	sdraw::darkmodeshouldactivate = !currentdarkmodestatus;
	config.write("DarkModeEnabled", !currentdarkmodestatus);
}
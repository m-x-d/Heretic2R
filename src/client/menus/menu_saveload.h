//
// menu_saveload.h -- logic shared between save and load menus.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern menuframework_s s_savegame_menu;
extern menuframework_s s_loadgame_menu;

extern char m_savestrings[MAX_SAVEGAMES][64];

void Create_Savestrings(void);
//
// menu_loadgame.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern cvar_t* m_banner_load;

extern void M_Menu_Loadgame_f(void);
extern void InitSaveLoadActions(menu_saveload_action_t* items, int num_items);
//
// menu_playerconfig.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern cvar_t* m_banner_pconfig;

extern cvar_t* m_item_name;
extern cvar_t* m_item_skin;
extern cvar_t* m_item_shownames;
extern cvar_t* skin_temp;

// Cvars shared between client and menu_playerconfig logic.
extern cvar_t* player_name;
extern cvar_t* playerdir;

void M_Menu_PlayerConfig_f(void);
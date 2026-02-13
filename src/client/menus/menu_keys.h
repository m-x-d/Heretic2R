//
// menu_keys.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

#define MENUKEY_NONE	(-1) //mxd

extern qboolean bind_grab;

extern menuframework_t s_keys_menu;

extern cvar_t* m_banner_action_keys;
extern cvar_t* m_banner_move_keys;
extern cvar_t* m_banner_short_keys;
extern cvar_t* m_banner_dt_keys;
extern cvar_t* m_banner_system_keys;

extern void M_FindKeysForCommand(int command_index, int* twokeys);

// Menu sections.
extern void M_Menu_ActionKeys_f(void);
extern void M_Menu_MoveKeys_f(void);
extern void M_Menu_ShortKeys_f(void);
extern void M_Menu_DoubletapKeys_f(void);
extern void M_Menu_SystemKeys_f(void);
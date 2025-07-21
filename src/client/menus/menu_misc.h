//
// menu_misc.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern cvar_t* m_banner_misc;

extern cvar_t* m_item_alwaysrun;
extern cvar_t* m_item_crosshair;
extern cvar_t* m_item_autotarget;
extern cvar_t* m_item_caption;
extern cvar_t* m_item_violence;
extern cvar_t* m_item_yawspeed;
extern cvar_t* m_item_console;
extern cvar_t* m_item_autoweapon;

extern void M_Menu_Misc_f(void);

// Functions shared between vid_dll and menu_misc logic.
extern void Key_ClearTyping(void);
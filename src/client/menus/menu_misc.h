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
extern cvar_t* m_item_noalttab;
extern cvar_t* m_item_joystick;
extern cvar_t* m_item_autotarget;
extern cvar_t* m_item_caption;
extern cvar_t* m_item_violence;
extern cvar_t* m_item_yawspeed;
extern cvar_t* m_item_console;
extern cvar_t* m_item_autoweapon;

// Cvars shared between vid_dll and menu_misc logic.
extern cvar_t* win_noalttab;

// Cvars shared between input and menu_misc logic.
extern cvar_t* in_joystick;

void M_Menu_Misc_f(void);

// Functions shared between vid_dll and menu_misc logic.
void Key_ClearTyping(void);
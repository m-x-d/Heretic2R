//
// menu_sound.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "client/menu.h"

extern cvar_t* m_banner_sound;

extern cvar_t* m_item_sndbackend; //mxd
extern cvar_t* m_item_effectsvol;
extern cvar_t* m_item_musicvol; //mxd
extern cvar_t* m_item_soundquality;
extern cvar_t* m_item_menutrack; //mxd
extern cvar_t* m_item_menutrack_none; //mxd

extern void M_Menu_Sound_f(void);
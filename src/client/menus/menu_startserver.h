//
// menu_startserver.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern cvar_t* m_banner_startserver;

extern cvar_t* m_item_begin;
extern cvar_t* m_item_startmap;
extern cvar_t* m_item_rules;
extern cvar_t* m_item_timelimit;
extern cvar_t* m_item_fraglimit;
extern cvar_t* m_item_maxplayers;
extern cvar_t* m_item_hostname;
extern cvar_t* m_item_deathmatch;
extern cvar_t* m_item_coop;

extern void M_Menu_StartServer_f(void);
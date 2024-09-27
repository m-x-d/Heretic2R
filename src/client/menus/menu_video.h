//
// menu_video.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

// Variables shared between vid_menu and menu_video logic.
extern int s_current_menu_index;
extern char* gl_drivername_labels[NUM_DRIVERNAMES];

extern cvar_t* m_banner_video;

// Cvars shared between client and menu_video logic.
extern cvar_t* r_detail;

void M_Menu_Video_f(void);

// Functions shared between vid_dll and menu_video logic.
void VID_MenuInit(void);
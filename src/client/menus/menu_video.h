//
// menu_video.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern cvar_t* m_banner_video;

// Cvars shared between client and menu_video logic.
extern cvar_t* r_detail;

void M_Menu_Video_f(void);

// Functions shared between vid_dll and menu_video logic.
void VID_MenuInit(void);
void VID_MenuSetDetail(int detail);
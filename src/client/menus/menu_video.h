//
// menu_video.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern cvar_t* m_banner_video;

// Cvars shared between client and menu_video logic.
extern cvar_t* m_r_detail;
extern cvar_t* m_gl_minlight;

extern void M_Menu_Video_f(void);
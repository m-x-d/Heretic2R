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
extern cvar_t* m_item_caption;
extern cvar_t* m_item_show_splash_movies; //mxd
extern cvar_t* m_item_violence;
extern cvar_t* m_item_screenshot_format; //mxd
extern cvar_t* m_item_yawspeed;
extern cvar_t* m_item_console;
extern cvar_t* m_item_autoweapon;

typedef enum //mxd
{
	SSF_JPG,
	SSF_PNG,
	SSF_NUM_FORMATS
} ScreenshotSaveFormat_t;

extern const char* screenshot_formats[SSF_NUM_FORMATS]; //mxd

extern void M_Menu_Misc_f(void);
extern ScreenshotSaveFormat_t M_GetCurrentScreenshotSaveFormat(void); //mxd
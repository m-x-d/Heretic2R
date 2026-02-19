//
// menu_downloadoptions.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern cvar_t* m_banner_download;

extern cvar_t* m_item_allowdownload;
extern cvar_t* m_item_allowdownloadmap;
extern cvar_t* m_item_allowdownloadsound;
extern cvar_t* m_item_allowdownloadplayer;
extern cvar_t* m_item_allowdownloadmodel;

extern void M_Menu_DownloadOptions_f(void);
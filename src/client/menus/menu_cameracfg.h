//
// menu_cameracfg.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern cvar_t* m_banner_cameracfg;
extern cvar_t* m_item_lookspring;
extern cvar_t* m_item_cameradamp;
extern cvar_t* m_item_camera_position_lerp; //mxd
extern cvar_t* m_item_autoaim; //mxd

void M_Menu_CameraCfg_f(void);
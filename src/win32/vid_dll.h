//
// vid_dll.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include <windows.h>
#include "Heretic2.h"

GAME_DECLSPEC extern HWND cl_hwnd;

extern cvar_t* vid_ref;
extern cvar_t* vid_fullscreen;

extern qboolean vid_restart_required; // New in H2

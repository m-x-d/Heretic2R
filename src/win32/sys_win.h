//
// sys_win.h -- Win32-specific Heretic II header file
//
// Copyright 1998 Raven Software
//

#pragma once

#include <windows.h>
#include "Heretic2.h"

Q2DLL_DECLSPEC extern HWND cl_hwnd;
extern qboolean ActiveApp;
extern qboolean Minimized;

extern uint sys_msg_time;

void IN_MouseEvent(int mstate);
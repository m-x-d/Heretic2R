//
// input.h -- Keyboard/mouse/gamepad input handling.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

extern void IN_Init(void);
extern void IN_Update(void);
extern void IN_Shutdown(void);

extern void In_FlushQueue(void);
extern void IN_Move(usercmd_t* cmd); // Add additional movement on top of the keyboard move cmd.

extern void IN_GetClipboardText(char* out, size_t n);

extern inline void Sys_CpuPause(void); //mxd
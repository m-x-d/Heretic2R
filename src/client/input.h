//
// input.h -- external (non-keyboard) input devices
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

extern void IN_Init(void);
extern void IN_Commands(void); // Opportunity for devices to stick commands on the script buffer.
extern void IN_Frame(void);
extern void IN_Move(usercmd_t* cmd); // Add additional movement on top of the keyboard move cmd.
extern void IN_Activate(qboolean active);
extern void IN_DeactivateMouse(void); // Local to in_win.c in Q2.
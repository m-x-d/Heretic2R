//
// input.h -- external (non-keyboard) input devices
//
// Copyright 1998 Raven Software
//

#pragma once

void IN_Init(void);
void IN_Shutdown(void);
void IN_Commands(void); // Opportunity for devices to stick commands on the script buffer
void IN_Frame(void);
void IN_Move(usercmd_t* cmd); // Add additional movement on top of the keyboard move cmd
void IN_Activate(qboolean active);
void IN_DeactivateMouse(void); // Local to in_win.c in Q2
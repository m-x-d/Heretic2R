//
// gl1_SDL.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void RI_EndFrame(void);
extern void* R_GetProcAddress(const char* proc);

extern qboolean RI_InitContext(void* win);
extern void RI_ShutdownContext(void);

extern int RI_PrepareForWindow(void);
extern void R_SetVsync(void);
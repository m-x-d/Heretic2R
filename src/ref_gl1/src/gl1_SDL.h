//
// gl1_SDL.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void R_EndFrame(void);
extern void* R_GetProcAddress(const char* proc);

extern qboolean R_InitContext(void* win);
extern void R_ShutdownContext(void);

extern int R_PrepareForWindow(void);
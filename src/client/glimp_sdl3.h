//
// glimp_sdl.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern qboolean GLimp_Init(void);
extern void GLimp_Shutdown(void);

extern qboolean GLimp_InitGraphics(int* pwidth, int* pheight, qboolean fullscreen);
extern void GLimp_ShutdownGraphics(void);
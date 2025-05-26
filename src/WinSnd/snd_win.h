//
// snd_win.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "snd_local.h"

SNDLIB_DECLSPEC extern void S_Activate(qboolean active);

extern qboolean SNDDMA_Init(void);
extern void SNDDMA_Shutdown(void);
extern int SNDDMA_GetDMAPos(void);
extern void SNDDMA_BeginPainting(void);
extern void SNDDMA_Submit(void);

// Local forward declarations for snd_win.c.
static void FreeSound(void);
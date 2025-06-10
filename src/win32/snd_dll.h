//
// snd_dll.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

#define DEFAULT_SOUND_LIBRARY_NAME	"WinSnd" //mxd //TODO: WinSnd -> snd_win?

extern cvar_t* snd_dll;

// Sound module logic.
extern void SndDll_Init(void);
extern void SndDll_FreeLibrary(void);
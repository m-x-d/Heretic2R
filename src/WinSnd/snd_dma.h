//
// snd_dma.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "snd_local.h"

extern dma_t dma;

extern int paintedtime;

extern cvar_t* s_volume;
extern cvar_t* s_sounddir; // H2
extern cvar_t* s_testsound;
extern cvar_t* s_loadas8bit;
extern cvar_t* s_khz;
extern cvar_t* s_primary;

extern void S_Shutdown(void);
extern void S_IssuePlaysound(playsound_t* ps);

// Local forward declarations for snd_dma.c:
static void S_StartSound(const vec3_t origin, int entnum, int entchannel, sfx_t* sfx, float fvol, int attenuation, float timeofs);
static void S_StopAllSounds(void);
static sfx_t* S_RegisterSound(const char* name);
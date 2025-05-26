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

// Exported sound library functions.
SNDLIB_DECLSPEC extern void S_Init(void);
SNDLIB_DECLSPEC extern void S_Shutdown(void);
SNDLIB_DECLSPEC extern void S_Update(const vec3_t origin, const vec3_t forward, const vec3_t right, const vec3_t up);
SNDLIB_DECLSPEC extern sfx_t* S_FindName(const char* name, qboolean create);

SNDLIB_DECLSPEC extern void S_StartSound(const vec3_t origin, int entnum, int entchannel, sfx_t* sfx, float fvol, int attenuation, float timeofs);
SNDLIB_DECLSPEC extern void S_StartLocalSound(const char* sound);

SNDLIB_DECLSPEC extern void S_StopAllSounds(void);
SNDLIB_DECLSPEC extern void S_StopAllSounds_Sounding(void);

SNDLIB_DECLSPEC extern void S_BeginRegistration(void);
SNDLIB_DECLSPEC extern sfx_t* S_RegisterSound(const char* name);
SNDLIB_DECLSPEC extern void S_EndRegistration(void);

extern void S_IssuePlaysound(playsound_t* ps);
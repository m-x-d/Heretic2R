//
// snd_dma.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "snd_local.h"

// Exported sound library functions.
SNDLIB_DECLSPEC extern void S_Init(void);
SNDLIB_DECLSPEC extern void S_Shutdown(void);
SNDLIB_DECLSPEC extern void S_Update(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up);
SNDLIB_DECLSPEC extern sfx_t* S_FindName(char* name, qboolean create);

SNDLIB_DECLSPEC extern void S_StartSound(const vec3_t origin, int entnum, int entchannel, sfx_t* sfx, float fvol, int attenuation, float timeofs);
SNDLIB_DECLSPEC extern void S_StartLocalSound(const char* sound);

SNDLIB_DECLSPEC extern void S_StopAllSounds(void);
SNDLIB_DECLSPEC extern void S_StopAllSounds_Sounding(void);

SNDLIB_DECLSPEC extern void S_BeginRegistration(void);
SNDLIB_DECLSPEC extern sfx_t* S_RegisterSound(const char* name);
SNDLIB_DECLSPEC extern void S_EndRegistration(void);
//
// snd_main.h
//
// Copyright 2025 mxd
//

#pragma once

#include "snd_local.h"

extern sound_t sound; // Q2: dma_t dma.

extern int paintedtime;
extern int s_rawend;

extern channel_t channels[MAX_CHANNELS];
extern playsound_t s_pendingplays;

extern cvar_t* s_volume;
extern cvar_t* s_sounddir; // H2
extern cvar_t* s_testsound;
extern cvar_t* s_loadas8bit;
extern cvar_t* s_khz;

extern cvar_t* s_underwater_gain_hf; // YQ2

extern sfxcache_t* S_LoadSound(sfx_t* s);
extern void S_ClearBuffer(void);

// Local forward declarations for snd_main.c:
static void S_StopAllSounds(void);
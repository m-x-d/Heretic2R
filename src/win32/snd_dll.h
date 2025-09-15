//
// snd_dll.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

#define DEFAULT_SOUND_LIBRARY_NAME	"snd_sdl3" //mxd

#define MAX_SNDLIBS	16 //mxd

typedef struct //mxd
{
	char title[32];	// Reflib title ("SDL3", "OpenAL" etc.).
	char id[16];	// Value to store in snd_dll cvar ("snd_sdl3", "snd_openal" etc.).
} sndlib_info_t;

extern sndlib_info_t sndlib_infos[MAX_SNDLIBS]; //mxd
extern int num_sndlib_infos; //mxd

extern cvar_t* snd_dll;

// Sound module logic.
extern void SND_Init(void); //mxd
extern void SND_Shutdown(void); //mxd
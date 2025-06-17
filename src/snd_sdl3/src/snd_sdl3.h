//
// snd_ogg.h
//
// Copyright 2025 mxd
//

#pragma once

#include "snd_local.h"

extern qboolean SDL_BackendInit(void);

extern void SDL_Update(void);
extern void SDL_Spatialize(channel_t* ch);
extern qboolean SDL_Cache(sfx_t* sfx, const wavinfo_t* info, byte* data, short volume, int begin_length, int  end_length, int attack_length, int fade_length);
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
extern qboolean SDL_Cache(sfx_t* sfx, const wavinfo_t* info, byte* data);
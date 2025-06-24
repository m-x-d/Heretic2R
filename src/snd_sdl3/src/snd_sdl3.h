//
// snd_ogg.h
//
// Copyright 2025 mxd
//

#pragma once

#include "snd_local.h"

extern qboolean SNDSDL3_BackendInit(void);
extern void SNDSDL3_BackendShutdown(void);

extern void SNDSDL3_Update(void);
extern void SNDSDL3_Spatialize(channel_t* ch);
extern qboolean SNDSDL3_Cache(sfx_t* sfx, const wavinfo_t* info, byte* data);
extern void SNDSDL3_RawSamples(int num_samples, uint rate, int width, int num_channels, const byte* data, float volume);
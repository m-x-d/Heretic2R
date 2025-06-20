//
// snd_ogg.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void OGG_Init(void);
extern void OGG_Shutdown(void);

extern void OGG_Stream(void);
extern void OGG_PlayTrack(int track, qboolean looping);
extern void OGG_Stop(void);
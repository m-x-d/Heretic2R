//
// fx_debris.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern client_entity_t* FXDebris_Throw(vec3_t origin, int material, vec3_t dir, float ke, float scale, int flags, qboolean altskin);
extern void FXDebris_SpawnChunks(int type, int flags, vec3_t origin, int num, int material, vec3_t dir, float ke, vec3_t mins, float scale, qboolean altskin);
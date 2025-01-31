//
// fx_debris.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern client_entity_t* FXDebris_Throw(const vec3_t origin, int material, const vec3_t dir, float ke, float scale, int flags, qboolean altskin);
extern qboolean FXDebris_Vanish(struct client_entity_s* self, centity_t* owner);
extern void FXDebris_SpawnChunks(int type, int flags, const vec3_t origin, int num, int material, const vec3_t dir, float ke, const vec3_t mins, float scale, qboolean altskin);
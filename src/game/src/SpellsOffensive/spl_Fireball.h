//
// spl_Fireball.h -- Named 'spl_flyingfist.h' in original logic --mxd.
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Shared.h"

extern void SpellCastFlyingFist(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);
extern edict_t* FlyingFistReflect(edict_t* self, edict_t* other, const vec3_t vel);

//mxd. Required by save system...
extern void FlyingFistThink(edict_t* self);
extern void FlyingFistTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
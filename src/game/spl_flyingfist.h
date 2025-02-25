//
// spl_flyingfist.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastFlyingFist(edict_t* caster, vec3_t start_pos, vec3_t aim_angles);
extern edict_t* FlyingFistReflect(edict_t* self, edict_t* other, vec3_t vel);
//
// spl_MeteorBarrier.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastMeteorBarrier(edict_t* caster, const vec3_t start_pos);
extern edict_t* MeteorBarrierReflect(edict_t* self, edict_t* other, vec3_t vel);
//
// spl_BlueRing.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastBlueRing(edict_t* caster);
extern edict_t* FindRingRadius(edict_t* from, const vec3_t org, float rad, const edict_t* ring_ent);

//mxd. Required by save system...
extern void RingThink(edict_t* self);
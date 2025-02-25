//
// spl_BlueRing.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastBlueRing(edict_t* Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float value);
extern edict_t* FindRingRadius(edict_t* from, const vec3_t org, float rad, const edict_t* ring_ent);
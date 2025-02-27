//
// spl_RedRain.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastRedRain(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, vec3_t AimDir, float Value);
extern edict_t* RedRainMissileReflect(edict_t* self, edict_t* other, vec3_t vel);
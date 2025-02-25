//
// spl_RedRain.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastRedRain(edict_t* Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float Value);
extern edict_t* RedRainMissileReflect(edict_t* self, edict_t* other, vec3_t vel);
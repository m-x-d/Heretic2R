//
// spl_Phoenix.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastPhoenix(edict_t* Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float Value);
extern edict_t* PhoenixMissileReflect(edict_t* self, edict_t* other, vec3_t vel);
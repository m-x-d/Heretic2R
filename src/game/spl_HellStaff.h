//
// spl_HellStaff.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastHellstaff(edict_t* Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir);
extern edict_t* HellboltReflect(edict_t* self, edict_t* other, vec3_t vel);
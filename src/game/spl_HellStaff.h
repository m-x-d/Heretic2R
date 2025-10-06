//
// spl_HellStaff.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastHellstaff(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);
extern edict_t* HellboltReflect(edict_t* self, edict_t* other, const vec3_t vel);
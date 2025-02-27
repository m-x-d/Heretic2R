//
// spl_Phoenix.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastPhoenix(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);
extern edict_t* PhoenixMissileReflect(edict_t* self, edict_t* other, vec3_t vel);
//
// spl_magicmissile.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastMagicMissile(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir);
extern edict_t* MagicMissileReflect(edict_t* self, edict_t* other, vec3_t vel);
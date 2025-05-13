//
// m_tcheckrik_spells.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastInsectStaff(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir, qboolean power);
extern void SpellCastGlobeOfOuchiness(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir);
extern void SpellCastInsectSpear(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, int offset);

extern edict_t* SpearProjReflect(edict_t* self, edict_t* other, vec3_t vel);
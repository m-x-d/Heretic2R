//
// m_tcheckrik_spells.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

extern void SpellCastInsectStaff(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir, qboolean power);
extern void SpellCastGlobeOfOuchiness(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir);
extern void SpellCastInsectSpear(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, int offset);

extern edict_t* SpearProjReflect(edict_t* self, edict_t* other, vec3_t vel);

//mxd. Required by save system...
extern void InsectGlobeOfOuchinessGrowThink(edict_t* self);
extern void InsectSpearProjectileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void InsectStaffBoltThink(edict_t* self);
extern void InsectStaffBoltTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void InsectTrackingSpearProjectileThink(edict_t* self);

// Local forward declarations for m_tcheckrik_spells.c.
static void InsectStaffBoltInit(edict_t* bolt);
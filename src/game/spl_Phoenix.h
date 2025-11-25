//
// spl_Phoenix.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

extern void SpellCastPhoenix(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);
extern edict_t* PhoenixMissileReflect(edict_t* self, edict_t* other, const vec3_t vel);

//mxd. Required by save system...
extern void PhoenixMissileThink(edict_t* self);
extern void PhoenixMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
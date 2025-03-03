//
// spl_wall.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

extern void SpellCastFireWall(edict_t* caster, vec3_t start_pos, vec3_t aim_angles);

// Local forward declarations for spl_wall.c:
static void FireBlastBlocked(edict_t* self, trace_t* trace);
static void FireBlastStartThink(edict_t* self);
static void FireWallMissileBlocked(edict_t* self, trace_t* trace);
static void FireWallMissileStartThink(edict_t* self);
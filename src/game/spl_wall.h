//
// spl_wall.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

extern void SpellCastFireWall(edict_t* caster, vec3_t start_pos, vec3_t aim_angles);

//mxd. Required by save system...
extern void FireBlastBlocked(edict_t* self, trace_t* trace);
extern void FireBlastStartThink(edict_t* self);
extern void FireBlastThink(edict_t* self);
extern void FireWallMissileBlocked(edict_t* self, trace_t* trace);
extern void FireWallMissileStartThink(edict_t* self);
extern void FireWallMissileThink(edict_t* self);
extern void FireWallMissileWormThink(edict_t* self);
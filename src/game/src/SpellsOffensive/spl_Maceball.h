//
// spl_Maceball.h -- Named 'spl_maceballs.h' in original logic --mxd.
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Shared.h"

extern void SpellCastMaceball(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);

//mxd. Required by save system...
extern void MaceballBounce(edict_t* self, trace_t* trace);
extern void MaceballThink(edict_t* self);
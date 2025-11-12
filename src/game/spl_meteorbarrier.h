//
// spl_MeteorBarrier.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

extern void SpellCastMeteorBarrier(edict_t* caster, const vec3_t start_pos);
extern edict_t* MeteorBarrierReflect(edict_t* self, edict_t* other, vec3_t vel);

//mxd. Required by save system...
extern void Kill_Meteor(edict_t* self);
extern void MeteorBarrierBounceThink(edict_t* self);
extern void MeteorBarrierHuntThink(edict_t* self);
extern void MeteorBarrierOnBlocked(edict_t* self, trace_t* trace);
extern void MeteorBarrierSearchInitThink(edict_t* self);
extern void MeteorBarrierSearchThink(edict_t* self);
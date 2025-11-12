//
// m_spreadermist.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

extern void spreader_mist(edict_t* self, float x, float y, float z);
extern void spreader_mist_fast(edict_t* self, float x, float y, float z);
extern void spreader_toss_grenade(edict_t* self);

extern edict_t* CreateRadiusDamageEnt(edict_t* position_owner, edict_t* damage_owner, int damage, int delta_damage, float radius, float delta_radius, int dflags, float lifetime, float think_increment, const vec3_t origin, const vec3_t offset, qboolean attach);

//mxd. Required by save system...
extern void RadiusDamageEntThink(edict_t* self);
extern void SpreaderGrenadeBounced(edict_t* self, trace_t* trace);
extern void SpreaderGrenadeThink(edict_t* self);
extern void SpreaderGrenadeDieThink(edict_t* self);
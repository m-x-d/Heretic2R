//
// m_priestess.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_high_priestess(edict_t* self);
extern void HighPriestessStaticsInit(void);

//mxd. Required by save system...
extern void PriestessPostThink(edict_t* self);
extern void PriestessProjectile1Blocked(edict_t* self, trace_t* trace);
extern void PriestessProjectile1DrunkenThink(edict_t* self);
extern void PriestessProjectile1Think(edict_t* self);
extern void PriestessProjectile2Die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
extern void PriestessProjectile2Think(edict_t* self);
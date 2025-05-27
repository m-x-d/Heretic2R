//
// m_priestess.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_high_priestess(edict_t* self);
extern void HighPriestessStaticsInit(void);

//mxd. Local forward declarations for m_priestess.c.
static void PriestessProjectile1Blocked(edict_t* self, trace_t* trace);
static void PriestessProjectile1DrunkenThink(edict_t* self);
static void PriestessProjectile1Think(edict_t* self);
static void PriestessProjectile2Die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
static void PriestessProjectile2Think(edict_t* self);
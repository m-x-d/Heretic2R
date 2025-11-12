//
// m_morcalavin.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_morcalavin(edict_t* self);
extern void MorcalavinStaticsInit(void);

//mxd. Required by save system...
extern void MorcalavinBeamIsBlocked(edict_t* self, trace_t* trace);
extern void MorcalavinBeamThink(edict_t* self);
extern void MorcalavinDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point);
extern void MorcalavinLaughPostThink(edict_t* self);
extern void MorcalavinLightning2Think(edict_t* self);
extern void MorcalavinLightningThink(edict_t* self);
extern void MorcalavinMissileThink(edict_t* self);
extern void MorcalavinPhaseInPreThink(edict_t* self);
extern void MorcalavinPhaseOutPreThink(edict_t* self);
extern void MorcalavinPostThink(edict_t* self);
extern void MorcalavinProjectile1Blocked(edict_t* self, trace_t* trace);
extern void MorcalavinProjectile2Blocked(edict_t* self, trace_t* trace);
extern void MorcalavinProjectile3Blocked(edict_t* self, trace_t* trace);
extern void MorcalavinTrackingProjectileThink(edict_t* self);

// Local forward declarations for m_morcalavin.c.
static void MorcalavinProjectileInit(edict_t* self, edict_t* proj);
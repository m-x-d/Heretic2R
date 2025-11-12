//
// m_seraph_guard.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_seraph_guard(edict_t* self);
extern void SeraphGuardStaticsInit(void);

//mxd. Required by save system...
extern void SeraphGuardDismember(edict_t* self, int damage, HitLocation_t hl);
extern void SeraphGuardProjectileThink(edict_t* self);
extern void SeraphGuardProjectileBlocked(edict_t* self, trace_t* trace);
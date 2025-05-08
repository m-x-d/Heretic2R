//
// m_seraph_guard.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

void SP_monster_seraph_guard(edict_t* self);
void SeraphGuardStaticsInit(void);

// Local forward declarations for m_seraph_guard.c.
static void SeraphGuardProjectileThink(edict_t* self);
static void SeraphGuardProjectileBlocked(edict_t* self, trace_t* trace);
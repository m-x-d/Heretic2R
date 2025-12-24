//
// m_fish.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_fish(edict_t* self);
extern void FishStaticsInit(void);

//mxd. Required by save system...
extern void FishIsBlocked(edict_t* self, trace_t* trace);
extern void FishThink(edict_t* self);
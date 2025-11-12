//
// m_elflord.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_elflord(edict_t* self);
extern void ElflordStaticsInit(void);

//mxd. Required by save system...
extern void ElfLordPreThink(edict_t* self);
extern void ElfLordProjectileBlocked(edict_t* self, trace_t* trace);
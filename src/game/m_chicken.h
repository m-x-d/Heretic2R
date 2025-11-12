//
// m_chicken.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_chicken(edict_t* self);
extern void ChickenStaticsInit(void);

//mxd. Required by save system...
extern void MorphChickenOut(edict_t* self);
extern void MorphOriginalIn(edict_t* self);
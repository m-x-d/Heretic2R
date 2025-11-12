//
// m_rat.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_rat(edict_t* self);
extern void SP_monster_rat_giant(edict_t* self);
extern void RatStaticsInit(void);

//mxd. Required by save system...
extern void RatTouch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void RatUse(edict_t* self, edict_t* other, edict_t* activator);
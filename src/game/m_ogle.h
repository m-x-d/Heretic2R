//
// m_ogle.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_ogle(edict_t* self);
extern void SP_obj_corpse_ogle(edict_t* self);
extern void OgleStaticsInit(void);
extern qboolean OgleFindTarget(edict_t* self);

//mxd. Required by save system...
extern void OgleDismember(edict_t* self, int damage, HitLocation_t hl);
extern void OgleInitOverlordThink(edict_t* self);
extern void OgleMoodThink(edict_t* self);
extern void OgleStartPushUse(edict_t* self, edict_t* other, edict_t* activator);
extern void OgleUse(edict_t* self, edict_t* other, edict_t* activator);
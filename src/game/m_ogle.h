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
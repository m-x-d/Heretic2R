//
// m_beast.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_trial_beast(edict_t* self);
extern void TBeastStaticsInit(void);

extern qboolean TB_CheckBottom(edict_t* self);
extern qboolean TB_CheckJump(edict_t* self);
extern edict_t* TB_CheckHit(const vec3_t start, vec3_t end);
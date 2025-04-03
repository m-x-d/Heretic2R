//
// m_beast.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

void SP_monster_trial_beast(edict_t* self);
qboolean TB_CheckBottom(edict_t* self);
qboolean TB_CheckJump(edict_t* self);
edict_t* TB_CheckHit(const vec3_t start, vec3_t end); //mxd. check_hit_beast() in original logic.
void TBeastStaticsInit(void);
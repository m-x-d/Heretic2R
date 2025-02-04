//
// m_beast.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

void SP_monster_trial_beast(edict_t* self);
qboolean TB_CheckBottom(edict_t* self);
qboolean TB_CheckJump(edict_t* self);
edict_t* TB_CheckHit(vec3_t start, vec3_t end); //mxd. check_hit_beast() in original logic.
void TBeastStaticsInit(void);
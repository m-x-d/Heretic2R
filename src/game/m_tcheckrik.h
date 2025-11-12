//
// m_tcheckrik.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_tcheckrik_female(edict_t* self);
extern void SP_monster_tcheckrik_male(edict_t* self);
extern void TcheckrikStaticsInit(void);

//mxd. Required by save system...
extern void TcheckrikDismember(edict_t* self, int damage, HitLocation_t hl);
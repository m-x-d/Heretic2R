//
// m_mssithra.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_mssithra(edict_t* self);
extern edict_t* MssithraArrowReflect(edict_t* self, edict_t* other, vec3_t vel);
extern void MssithraStaticsInit(void);
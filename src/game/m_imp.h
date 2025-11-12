//
// m_imp.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_imp(edict_t* self);
extern edict_t* ImpFireballReflect(edict_t* self, edict_t* other, const vec3_t vel);
extern void ImpStaticsInit(void);

//mxd. Required by save system...
extern void ImpFireballBlocked(edict_t* self, trace_t* trace);
extern void ImpIsBlocked(edict_t* self, trace_t* trace);
//
// m_gkrokon.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_Monster_Gkrokon(edict_t* self);
extern edict_t* GkrokonSpooReflect(edict_t* self, edict_t* other, vec3_t vel);
extern void GkrokonStaticsInit(void);
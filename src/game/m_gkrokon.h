//
// m_gkrokon.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

void SP_Monster_Gkrokon(edict_t* self);
edict_t* GkrokonSpooReflect(edict_t* self, edict_t* other, vec3_t vel);
void GkrokonStaticsInit(void);
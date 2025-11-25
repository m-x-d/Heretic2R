//
// m_gkrokon.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_Monster_Gkrokon(edict_t* self);
extern edict_t* GkrokonSpooReflect(edict_t* self, edict_t* other, const vec3_t vel);
extern void GkrokonStaticsInit(void);

//mxd. Required by save system...
extern void GkrokonDismember(edict_t* self, int damage, HitLocation_t hl);
extern void GkrokonSpooIsBlocked(edict_t* self, trace_t* trace);
extern void GkrokonSpooTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
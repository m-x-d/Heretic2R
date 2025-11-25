//
// m_mssithra.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_mssithra(edict_t* self);
extern edict_t* MssithraArrowReflect(edict_t* self, edict_t* other, const vec3_t vel);
extern void MssithraStaticsInit(void);

//mxd. Required by save system...
extern void MssithraArrowExplodeThink(edict_t* self);
extern void MssithraArrowTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void MssithraPostThink(edict_t* self);
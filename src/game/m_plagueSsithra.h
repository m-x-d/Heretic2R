//
// m_plagueSsithra.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_plague_ssithra(edict_t* self);
extern void SP_obj_corpse_ssithra(edict_t* self);
extern void SsithraStaticsInit(void);
extern void SsithraCheckJump(edict_t* self);

// Local forward declarations for m_plagueSsithra.c.
static void SsithraSlideOffThink(edict_t* self);
static void SsithraDismember(edict_t* self, int damage, HitLocation_t hl);
static void SsithraArrowTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
static void SsithraDuckArrowTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
static void SsithraArrowExplodeThink(edict_t* self);
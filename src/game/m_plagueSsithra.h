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

//mxd. Required by save system...
extern qboolean SsithraAlert(edict_t* self, alertent_t* alerter, edict_t* enemy);
extern void SsithraSlideOffThink(edict_t* self);
extern void SsithraArrowTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void SsithraBlocked(edict_t* self, trace_t* trace);
extern void SsithraDismember(edict_t* self, int damage, HitLocation_t hl);
extern void SsithraDuckArrowTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void SsithraSlideFallThink(edict_t* self);
extern void SsithraArrowExplodeThink(edict_t* self);
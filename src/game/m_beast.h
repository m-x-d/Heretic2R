//
// m_beast.h -- mxd. Added public header.
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_monster_trial_beast(edict_t* self);
extern void TBeastStaticsInit(void);

extern qboolean TBeastCheckBottom(edict_t* self);
extern qboolean TBeastCheckJump(edict_t* self);
extern edict_t* TBeastCheckHit(const vec3_t start, vec3_t end);

//mxd. Required by save system...
extern void TBeastBlocked(edict_t* self, trace_t* trace);
extern void TBeastDieUse(edict_t* self, edict_t* other, edict_t* activator);
extern void TBeastPostThink(edict_t* self);
extern void TBeastTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void TBeastUse(edict_t* self, edict_t* other, edict_t* activator);
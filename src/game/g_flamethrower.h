//
// g_flamethrower.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_Edict.h"

extern void SP_flamethrower(edict_t* self);
extern void FlamethrowerStaticsInit(void);

//mxd. Required by save system...
extern void FlamethrowerThink(edict_t* self);
extern void FlamethrowerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void FlamethrowerUse(edict_t* self, edict_t* other, edict_t* activator);
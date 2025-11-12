//
// g_morcalavin_barrier.h
//
// Copyright 2025 mxd.
//

#pragma once

#include "g_Edict.h"

extern void SP_obj_morcalavin_barrier(edict_t* self);

//mxd. Required by save system...
extern void MorcalavinBarrierThink(edict_t* self);
extern void MorcalavinBarrierTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void MorcalavinBarrierUse(edict_t* self, edict_t* other, edict_t* activator);
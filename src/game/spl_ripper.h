//
// spl_ripper.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

extern void SpellCastRipper(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);

//mxd. Required by save system...
extern void RipperExplodeBallThink(edict_t* self);
extern void RipperExplodeBallTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
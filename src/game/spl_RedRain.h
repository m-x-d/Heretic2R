//
// spl_RedRain.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

extern void SpellCastRedRain(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);
extern edict_t* RedRainMissileReflect(edict_t* self, edict_t* other, vec3_t vel);

//mxd. Required by save system...
extern void RedRainMissileThink(edict_t* self);
extern void RedRainMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
extern void RedRainRemove(edict_t* self);
extern void RedRainThink(edict_t* self);
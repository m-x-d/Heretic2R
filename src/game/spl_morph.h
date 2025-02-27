//
// spl_morph.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastMorph(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);
extern edict_t* MorphReflect(edict_t* self, edict_t* other, vec3_t vel);

//TODO: move to p_morph.c?
extern void MorphPlayerToChicken(edict_t* self, edict_t* caster);
extern void PerformPlayerMorph(edict_t* self);
extern void CleanUpPlayerMorph(edict_t* self);
extern void ResetPlayerMorph(edict_t* self);
//
// spl_morph.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

extern void SpellCastMorph(edict_t* Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float Value);
extern edict_t* MorphReflect(edict_t* self, edict_t* other, vec3_t vel);
extern void MorphPlayerToChicken(edict_t* self, edict_t* caster);

//TODO: move to p_morph.c?
extern void PerformPlayerMorph(edict_t* self);
extern void CleanUpPlayerMorph(edict_t* self);
extern void ResetPlayerMorph(edict_t* self);
//
// spl_morph.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

#define MORPH_TELE_TIME 5						// Number of server frames to do the fade.
#define MORPH_TELE_FADE (255 / MORPH_TELE_TIME)	// Amount to fade the player by each fade.

extern void SpellCastMorph(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles);
extern edict_t* MorphReflect(edict_t* self, edict_t* other, vec3_t vel);

//mxd. Required by save system...
extern void MonsterMorphFadeIn(edict_t* self);
extern void MonsterMorphFadeOut(edict_t* self);
extern void MorphMissileThink(edict_t* self);
extern void MorphMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface);
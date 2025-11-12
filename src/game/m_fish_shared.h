//
// m_fish_shared.h -- Data and function declarations shared between m_fish.c and m_fish_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

typedef enum AnimID_e
{
	ANIM_BITE,
	ANIM_MELEE,
	ANIM_RUN1,
	ANIM_RUN2,
	ANIM_RUN3,
	ANIM_WALK1,
	ANIM_WALK2,
	ANIM_WALK3,
	ANIM_STAND1,
	ANIM_PAIN1,
	ANIM_DEATH1,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,
	SND_GIB,
	SND_BITEHIT1,
	SND_BITEHIT2,
	SND_BITEMISS1,
	SND_BITEMISS2,
	SND_GROWL1, //mxd. Unused.
	SND_GROWL2, //mxd. Unused.
	SND_GROWL3, //mxd. Unused.
	SND_SPLASH,
	SND_SLOW_SWIM1,
	SND_SLOW_SWIM2,
	SND_FAST_SWIM1,
	SND_FAST_SWIM2,

	NUM_SOUNDS
} SoundID_t;

extern void fish_dead(edict_t* self);
extern void fish_bite(edict_t* self);
extern void fish_idle(edict_t* self);
extern void fish_walkswim_finished(edict_t* self);
extern void fish_runswim_finished(edict_t* self);
extern void fish_walk(edict_t* self);
extern void fish_run(edict_t* self);
extern void fish_pain_finished(edict_t* self);
extern void fish_update_yaw(edict_t* self);
extern void fish_pause(edict_t* self);
extern void fish_chase(edict_t* self);
extern void fish_update_target_movedir(edict_t* self);
extern void fish_swim_sound(edict_t* self, float fast);
extern void fish_under_water_wake(edict_t* self);
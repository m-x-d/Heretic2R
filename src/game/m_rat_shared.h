//
// m_rat_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

typedef enum AnimID_e
{
	ANIM_EATING1,
	ANIM_EATING2,
	ANIM_EATING3,
	ANIM_STAND1,
	ANIM_STAND2,
	ANIM_STAND3,
	ANIM_STAND4,
	ANIM_STAND5,
	ANIM_STAND6,
	ANIM_STAND7,
	ANIM_STAND8,
	ANIM_WATCH1,
	ANIM_WATCH2,
	ANIM_WALK1,
	ANIM_RUN1,
	ANIM_RUN2,
	ANIM_RUN3,
	ANIM_MELEE1,
	ANIM_MELEE2,
	ANIM_MELEE3,
	ANIM_PAIN1,
	ANIM_DIE1,
	ANIM_DIE2,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_BITEHIT1,
	SND_BITEMISS1,
	SND_BITEMISS2,

	SND_SCRATCH,

	SND_HISS,

	SND_PAIN1,
	SND_PAIN2,

	SND_CHATTER1,
	SND_CHATTER2,
	SND_CHATTER3,

	SND_CHEW1,
	SND_CHEW2,
	SND_CHEW3,

	SND_SWALLOW,

	SND_DIE,
	SND_GIB,

	NUM_SOUNDS
} SoundID_t;

extern void rat_bite(edict_t* self);
extern void rat_pain_init(edict_t* self);
extern void rat_pause(edict_t* self);
extern void rat_jump(edict_t* self);

extern void rat_run_order(edict_t* self);
extern void rat_stand_order(edict_t* self);
extern void rat_eat_order(edict_t* self);

// Sounds.
extern void rat_death_squeal(edict_t* self);
extern void rat_squeal(edict_t* self);
extern void rat_hiss(edict_t* self);
extern void rat_scratch(edict_t* self);
extern void rat_chatter(edict_t* self);
extern void rat_chew(edict_t* self);
extern void rat_swallow(edict_t* self);

extern void rat_ai_run(edict_t* self, float distance);
extern void rat_ai_eat(edict_t* self, float distance);
extern void rat_ai_stand(edict_t* self, float distance);
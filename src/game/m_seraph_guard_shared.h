//
// m_seraph_guard_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

typedef enum AnimID_e
{
	ANIM_STAND,
	ANIM_RUN,
	ANIM_FJUMP,
	ANIM_RUN_MELEE,
	ANIM_WALK,
	ANIM_PAIN,
	ANIM_MELEE1,
	ANIM_MELEE2,
	ANIM_MELEE3,
	ANIM_DEATH1,
	ANIM_DEATH2_GO,
	ANIM_DEATH2_LOOP,
	ANIM_DEATH2_END,
	ANIM_BACKUP,
	ANIM_MISSILE,
	ANIM_DELAY,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_ATTACK,
	SND_ATTACK_MISS,
	SND_PAIN1,
	SND_PAIN2,
	SND_PAIN3,
	SND_PAIN4,
	SND_DEATH1,
	SND_DEATH2,
	SND_DEATH3,
	SND_DEATH4,
	SND_HIT_WALL,
	SND_MISSILE,
	SND_MISSHIT,
	SND_FIST_HIT_WALL,
	SND_SIGHT1,
	SND_SIGHT2,
	SND_SIGHT3,
	SND_SIGHT4,

	NUM_SOUNDS
} SoundID_t;

extern void seraph_guard_death_loop(edict_t* self);
extern void seraph_guard_check_land(edict_t* self);
extern void seraph_guard_dead(edict_t* self);
extern void seraph_guard_check_poke(edict_t* self);

extern void seraph_guard_pause(edict_t* self);
extern void seraph_guard_strike(edict_t* self, float damage, float forward, float up);
extern void seraph_guard_jump(edict_t* self);
extern void seraph_guard_back(edict_t* self, float distance);
extern void seraph_guard_fire(edict_t* self);

#define BIT_BASEBIN		0
#define BIT_PITHEAD		1
#define BIT_SHOULDPAD	2
#define BIT_GUARDHEAD	4
#define BIT_LHANDGRD	8
#define BIT_LHANDBOSS	16
#define BIT_RHAND		32
#define BIT_FRTORSO		64
#define BIT_ARMSPIKES	128
#define BIT_LFTUPARM	256
#define BIT_RTLEG		512
#define BIT_RTARM		1024
#define BIT_LFTLEG		2048
#define BIT_BKTORSO		4096
#define BIT_AXE			8192
#define BIT_WHIP		16384
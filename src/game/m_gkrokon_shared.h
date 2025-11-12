//
// m_gkrokon_shared.h -- Data and function declarations shared between m_gkrokon.c and m_gkrokon_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h" //mxd

// The Gkrokon's animations.
typedef enum AnimID_e
{
	ANIM_STAND1,
	ANIM_STAND2,
	ANIM_STAND3,
	ANIM_STAND4,
	ANIM_CROUCH1,
	ANIM_CROUCH2,
	ANIM_CROUCH3,
	ANIM_WALK1, //TODO: unused.
	ANIM_RUN1,
	ANIM_RUN2,
	ANIM_JUMP1, //TODO: unused.
	ANIM_FJUMP,
	ANIM_MELEE1,
	ANIM_MELEE2,
	ANIM_MISSILE1,
	ANIM_EAT1,
	ANIM_EAT2,
	ANIM_EAT3,
	ANIM_PAIN1,
	ANIM_DIE1,
	ANIM_HOP, //TODO: unused.
	ANIM_RUNAWAY,
	ANIM_SNEEZE,
	ANIM_DELAY,

	NUM_ANIMS
} AnimID_t;

// The Gkrokon's sounds.
typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,
	SND_GIB,
	SND_SPOO,
	SND_IDLE1,
	SND_IDLE2,
	SND_SIGHT, //TODO: unused.
	SND_WALK1,
	SND_WALK2,
	SND_FLEE, //TODO: unused.
	SND_ANGRY, //TODO: unused.
	SND_EATING, //TODO: unused.
	SND_BITEHIT1,
	SND_BITEHIT2,
	SND_BITEMISS1,
	SND_BITEMISS2,

	NUM_SOUNDS
} SoundID_t;

extern void gkrokon_pause(edict_t* self);
extern void gkrokon_sound(edict_t* self, float channel, float sound_index, float attenuation);
extern void gkrokon_walk_sound(edict_t* self);
extern void gkrokon_idle_sound(edict_t* self);
extern void gkrokon_bite(edict_t* self, float right_side);
extern void gkrokon_spoo_attack(edict_t* self);
extern void gkrokon_dead(edict_t* self);
extern void gkrokon_ai_stand(edict_t* self, float dist);
extern void gkrokon_set_stand_anim(edict_t* self);
extern void gkrokon_set_crouch_anim(edict_t* self);

#define BIT_WAIT1			0
#define BIT_SHELLA_P1		1
#define BIT_SPIKE_P1		2
#define BIT_HEAD_P1			4
#define BIT_RPINCHERA_P1	8
#define BIT_RPINCHERB_P1	16
#define BIT_LPINCHERA_P1	32
#define BIT_LPINCHERB_P1	64
#define BIT_LARM_P1			128
#define BIT_RARM_P1			256
#define BIT_ABDOMEN_P1		512
#define BIT_LTHIGH_P1		1024
#define BIT_RTHIGH_P1		2048
#define BIT_SHELLB_P1		4096
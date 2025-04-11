//
// m_gkrokon_shared.h -- Data and function declarations shared between m_gkrokon.c and m_gkrokon_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h" //mxd

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

extern const animmove_t gkrokon_move_stand1;
extern const animmove_t gkrokon_move_stand2;
extern const animmove_t gkrokon_move_stand3;
extern const animmove_t gkrokon_move_stand4;
extern const animmove_t gkrokon_move_crouch1;
extern const animmove_t gkrokon_move_crouch2;
extern const animmove_t gkrokon_move_crouch3;
extern const animmove_t gkrokon_move_walk1;
extern const animmove_t gkrokon_move_run1;
extern const animmove_t gkrokon_move_run2;
extern const animmove_t gkrokon_move_run_away;
extern const animmove_t gkrokon_move_jump1;
extern const animmove_t gkrokon_move_forced_jump;
extern const animmove_t gkrokon_move_melee_attack1;
extern const animmove_t gkrokon_move_melee_attack2;
extern const animmove_t gkrokon_move_missile_attack1;
extern const animmove_t gkrokon_move_missile_attack2;
extern const animmove_t gkrokon_move_eat1;
extern const animmove_t gkrokon_move_eat2;
extern const animmove_t gkrokon_move_eat3;
extern const animmove_t gkrokon_move_pain1;
extern const animmove_t gkrokon_move_death1;
extern const animmove_t gkrokon_move_hop1;
extern const animmove_t gkrokon_move_delay;

void GkrokonPause(edict_t* self);
void gkrokonSound(edict_t* self, float channel, float sound_index, float attenuation);
void gkrokonRandomWalkSound(edict_t* self);
void GkrokonSpoo(edict_t* self);
void GkrokonDead(edict_t* self);

void beetle_ai_stand(edict_t* self, float dist);
void beetle_idle_sound(edict_t* self);
void beetle_to_stand(edict_t* self);
void beetle_to_crouch(edict_t* self);
void gkrokon_bite(edict_t* self, float right_side);

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
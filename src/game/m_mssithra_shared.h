//
// m_mssithra_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

typedef enum AnimID_e
{
	ANIM_CLAW1,
	ANIM_DEATH1,
	ANIM_IDLE1,
	ANIM_JUMP1,
	ANIM_FJUMP,
	// ANIM_PAIN1,
	ANIM_ROAR1,
	ANIM_SHOOTA1,
	ANIM_SHOOTB1,
	ANIM_WALK1,
	ANIM_BACKPEDAL,
	ANIM_RUN,
	ANIM_DELAY,

	ANIM_SHOOT_TRANS,
	ANIM_SHOOT_LOOP,
	ANIM_SHOOT_DETRANS,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,
	SND_SWIPE,
	SND_SWIPEHIT,
	SND_ARROW,
	SND_AEXPLODE,
	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,
	SND_ROAR,
	SND_INWALL,

	NUM_SOUNDS
} SoundID_t;


extern const animmove_t mssithra_move_idle1;
extern const animmove_t mssithra_move_walk1;
extern const animmove_t mssithra_move_backpedal1;
extern const animmove_t mssithra_move_death1;
extern const animmove_t mssithra_move_claw1;
extern const animmove_t mssithra_move_jump1;
extern const animmove_t mssithra_move_fjump;
extern const animmove_t mssithra_move_shoota1;
extern const animmove_t mssithra_move_shootb1;
extern const animmove_t mssithra_move_roar;
extern const animmove_t mssithra_move_run1;
extern const animmove_t mssithra_move_delay;

extern const animmove_t mssithra_move_shoot1_trans;
extern const animmove_t mssithra_move_shoot1_loop;
extern const animmove_t mssithra_move_shoot1_detrans;

extern void mssithraSwipe(edict_t* self);
extern void mssithraArrow(edict_t* self);
extern qboolean mssithraCheckMood(edict_t* self);
extern void mmssithraRandomGrowlSound(edict_t* self);
extern void mssithra_dead(edict_t* self);

extern void mssithraCheckShotLoop(edict_t* self);
extern void mssithra_ShotLoop(edict_t* self);
//
// m_spreader_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

typedef enum AnimID_e
{
	ANIM_ATTACK1,
	ANIM_ATTACK2,
	ANIM_BACKUP,
	ANIM_BACKATTACK,
	ANIM_DEATH1_GO,
	ANIM_DEATH1_LOOP,
	ANIM_DEATH1_END,
	ANIM_DUCK, //TODO: unused.
	ANIM_DUCKATTACK,
	ANIM_DUCKDOWN,
	ANIM_DUCKSTILL,
	ANIM_DUCKUP,
	ANIM_IDLE1,
	ANIM_PAIN1,
	ANIM_PIVOT_LEFT, //TODO: unused.
	ANIM_PIVOT_RIGHT, //TODO: unused.
	ANIM_RUNATTACK,
	ANIM_RUN1,
	ANIM_LAND,
	ANIM_INAIR,
	ANIM_FJUMP,
	ANIM_WALK1,
	ANIM_WALK2, //TODO: unused.
	ANIM_DEATH2,
	ANIM_FLY,
	ANIM_FLYLOOP,
	ANIM_FDIE,
	ANIM_DEAD,
	ANIM_DELAY,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_SPRAYSTART,
	SND_SPRAYLOOP,
	SND_PAIN,
	SND_VOICE1,
	SND_VOICE2,
	SND_THROW,
	SND_DEATH,
	SND_BOMB,

	NUM_SOUNDS
} SoundID_t;

extern void spreader_fly(edict_t* self);
extern void spreader_fly_loop(edict_t* self);
extern void spreader_deadloop_go(edict_t* self);
extern void spreader_become_solid(edict_t* self);

extern void spreader_pause(edict_t* self);
extern void spreader_duck_pause(edict_t* self);
extern void spreader_dead(edict_t* self);

extern void spreader_flyback_move(edict_t* self);
extern void spreader_flyback_loop(edict_t* self);

extern void spreader_show_grenade(edict_t* self);
extern void spreader_pain_sound(edict_t* self);
extern void spreader_mist_start_sound(edict_t* self);
extern void spreader_mist_stop_sound(edict_t* self);
extern void spreader_idle_sound(edict_t* self);
extern void spreader_hide_grenade(edict_t* self);
extern void spreader_jump(edict_t* self);
extern void spreader_land(edict_t* self);
extern void spreader_inair_go(edict_t* self);

#define BIT_PARENT		0
#define BIT_CHILD		1
#define BIT_BODY		2
#define BIT_BOMB		4
#define BIT_RITLEG		8
#define BIT_LFTARM		16
#define BIT_LFTLEG		32
#define BIT_HEAD		64
#define BIT_RITARM		128
#define BIT_TANK3		256
#define BIT_TANK2		512
#define BIT_TANK1		1024
#define BIT_HOSE		2048
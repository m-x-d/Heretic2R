//
// m_spreader_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

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

extern const animmove_t spreader_move_attack1;
extern const animmove_t spreader_move_attack2;
extern const animmove_t spreader_move_backup1;
extern const animmove_t spreader_move_backattack1;
extern const animmove_t spreader_move_death1_go;
extern const animmove_t spreader_move_death1_loop;
extern const animmove_t spreader_move_death1_end;
extern const animmove_t spreader_move_duck1;
extern const animmove_t spreader_move_dkatck1;
extern const animmove_t spreader_move_duckdown;
extern const animmove_t spreader_move_duckstill;
extern const animmove_t spreader_move_duckup;
extern const animmove_t spreader_move_idle1;
extern const animmove_t spreader_move_pain1;
extern const animmove_t spreader_move_pvtlt1;
extern const animmove_t spreader_move_pvtrt1;
extern const animmove_t spreader_move_rnatck1;
extern const animmove_t spreader_move_run1;
extern const animmove_t spreader_move_land;
extern const animmove_t spreader_move_inair;
extern const animmove_t spreader_move_fjump;
extern const animmove_t spreader_move_walk1;
extern const animmove_t spreader_move_walk2;
extern const animmove_t spreader_move_death2;
extern const animmove_t spreader_move_flyloop;
extern const animmove_t spreader_move_fly;
extern const animmove_t spreader_move_fdie;
extern const animmove_t spreader_move_dead;
extern const animmove_t spreader_move_delay;

extern void spreaderFly(edict_t* self);
extern void spreaderFlyLoop(edict_t* self);
extern void spreader_go_deadloop(edict_t* self);
extern void spreaderSolidAgain(edict_t* self);

extern void spreader_pause(edict_t* self);
extern void spreader_duck_pause(edict_t* self);
extern void spreader_dead(edict_t* self);

extern void spreader_flyback_move(edict_t* self);
extern void spreader_flyback_loop(edict_t* self);

extern void spreader_show_grenade(edict_t* self);
extern void spreader_pain_sound(edict_t* self);
extern void spreader_mist_start_sound(edict_t* self);
extern void spreader_miststopsound(edict_t* self);
extern void spreader_idle_sound(edict_t* self);
extern void spreader_hide_grenade(edict_t* self);
extern void spreaderApplyJump(edict_t* self);
extern void spreader_land(edict_t* self);
extern void spreader_go_inair(edict_t* self);

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
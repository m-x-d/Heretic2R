//
// m_plagueSsithra_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

typedef enum AnimID_e
{
	ANIM_STAND1,
	ANIM_WALK1,
	ANIM_BACKPEDAL,
	ANIM_BOUND,
	ANIM_DEATH_A,
	ANIM_DEATH_B,
	ANIM_DIVE,
	ANIM_DUCKSHOOT,
	ANIM_DUCK, //TODO: unused.
	ANIM_GALLOP,
	ANIM_FJUMP,
	ANIM_IDLEBASIC,
	ANIM_IDLERIGHT,
	ANIM_MELEE,
	ANIM_MELEE_STAND,
	ANIM_NAMOR,
	ANIM_PAIN_A,
	ANIM_SHOOT,
	ANIM_STARTLE,
	ANIM_SWIMFORWARD,
	ANIM_SWIMWANDER,
	ANIM_WATER_DEATH,
	ANIM_WATER_IDLE,
	ANIM_WATER_PAIN_A,
	ANIM_WATER_PAIN_B,
	ANIM_WATER_SHOOT,
	ANIM_RUN1,
	ANIM_SPINRIGHT,
	ANIM_SPINRIGHT_GO,
	ANIM_SPINLEFT,
	ANIM_SPINLEFT_GO,
	ANIM_FACEANDNAMOR,
	ANIM_DEAD_A, //TODO: unused.
	ANIM_LOOKRIGHT,
	ANIM_LOOKLEFT,
	ANIM_TRANSUP,
	ANIM_TRANSDOWN,
	ANIM_HEADLESS,
	ANIM_HEADLESSLOOP,
	ANIM_DEATH_C,
	ANIM_DEAD_B,
	ANIM_DEAD_WATER, //TODO: unused.
	ANIM_SLICED,
	ANIM_DELAY,
	ANIM_DUCKLOOP,
	ANIM_UNDUCK,
	ANIM_LUNGE,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,
	SND_GIB,
	SND_SWIPE,
	SND_SWIPEHIT,
	SND_ARROW1,
	SND_ARROW2,
	SND_GROWL1, // Hiss.
	SND_GROWL2, // Shriek.
	SND_GROWL3, // Grunt.
	SND_INWATER,
	SND_NAMOR,
	SND_LAND,
	SND_SWIM,
	SND_SWIM2,

	// Voices.
	SND_SIGHT1,
	SND_SIGHT2,
	SND_SIGHT3,
	SND_SIGHT4,
	SND_SIGHT5,
	SND_SIGHT6,

	SND_ARROW_CHARGE,
	SND_ARROW_FIRE,

	NUM_SOUNDS
} SoundID_t;

extern const animmove_t ssithra_move_idle1;
extern const animmove_t ssithra_move_walk1;
extern const animmove_t ssithra_move_backpedal1;
extern const animmove_t ssithra_move_bound1;
extern const animmove_t ssithra_move_death_a1;
extern const animmove_t ssithra_move_death_b1;
extern const animmove_t ssithra_move_dive1;
extern const animmove_t ssithra_move_duckshoot1;
extern const animmove_t ssithra_move_duck1;
extern const animmove_t ssithra_move_gallop1;
extern const animmove_t ssithra_move_fjump;
extern const animmove_t ssithra_move_idlebasic1;
extern const animmove_t ssithra_move_idleright1;
extern const animmove_t ssithra_move_melee1;
extern const animmove_t ssithra_move_meleest;
extern const animmove_t ssithra_move_namor1;
extern const animmove_t ssithra_move_pain_a1;
extern const animmove_t ssithra_move_shoot1;
extern const animmove_t ssithra_move_startle1;
extern const animmove_t ssithra_move_swimforward1;
extern const animmove_t ssithra_move_swimwander;
extern const animmove_t ssithra_move_water_death1;
extern const animmove_t ssithra_move_water_idle1;
extern const animmove_t ssithra_move_water_pain_a1;
extern const animmove_t ssithra_move_water_pain_b1;
extern const animmove_t ssithra_move_water_shoot1;
extern const animmove_t ssithra_move_run1;
extern const animmove_t ssithra_move_spinright;
extern const animmove_t ssithra_move_spinright_go;
extern const animmove_t ssithra_move_spinleft;
extern const animmove_t ssithra_move_spinleft_go;
extern const animmove_t ssithra_move_faceandnamor;
extern const animmove_t ssithra_move_lookright;
extern const animmove_t ssithra_move_lookleft;
extern const animmove_t ssithra_move_transup;
extern const animmove_t ssithra_move_transdown;
extern const animmove_t ssithra_move_headless;
extern const animmove_t ssithra_move_headlessloop;
extern const animmove_t ssithra_move_death_c;
extern const animmove_t ssithra_move_dead_a;
extern const animmove_t ssithra_move_dead_b;
extern const animmove_t ssithra_move_dead_water;
extern const animmove_t ssithra_move_sliced;
extern const animmove_t ssithra_move_delay;
extern const animmove_t ssithra_move_duckloop;
extern const animmove_t ssithra_move_unduck;
extern const animmove_t ssithra_move_lunge;

extern void ssithra_decide_run(edict_t* self);
extern void ssithra_decide_swimforward(edict_t* self);
extern void ssithra_decide_backpedal(edict_t* self);

extern void ssithra_ai_run(edict_t* self, float distance); //mxd
extern void ssithra_dead(edict_t* self);
extern void ssithraSwipe(edict_t* self);
extern void ssithraArrow(edict_t* self);
extern void ssithraJump(edict_t* self, float up_speed, float forward_speed, float right_speed);
extern void ssithraBoundCheck(edict_t* self);
extern void ssithraDiveCheck(edict_t* self);
extern void ssithraWaterDead(edict_t* self);
extern void ssithraForward(edict_t* self, float forward_dist);
extern void ssithraCheckLeaveWaterSplash(edict_t* self);
extern void ssithraCheckHitWaterSplash(edict_t* self);
extern void ssithraNamorJump(edict_t* self);
extern void ssithraCheckRipple(edict_t* self);
extern void ssithraCheckFacedNamor(edict_t* self);
extern void ssithra_check_namor(edict_t* self);
extern void ssithraVOfs(edict_t* self, float pitch_offset, float yaw_offset, float roll_offset);
extern void ssithraPanicArrow(edict_t* self);
extern void ssithra_pain_react(edict_t* self);
extern void ssithra_water_shoot(edict_t* self);
extern void ssithraCollapse(edict_t* self);
extern void ssithraKillSelf(edict_t* self);
extern void ssithraSound(edict_t* self, float sound_num, float channel, float attenuation);
extern qboolean SsithraCheckMood(edict_t* self);
extern void ssithraApplyJump(edict_t* self);
extern void ssithraCheckDuckArrow(edict_t* self);
extern void ssithraCheckUnDuck(edict_t* self);
extern void ssithraCrouch(edict_t* self);
extern void ssithraUnCrouch(edict_t* self);
extern void ssithraCheckLoop(edict_t* self);
extern void ssithraGrowlSound(edict_t* self);
extern void ssithraStartDuckArrow(edict_t* self);

#define BIT_POLY				0
#define BIT_LOWERTORSO			1
#define BIT_CAPLOWERTORSO		2
#define BIT_LEFTLEG				4
#define BIT_RIGHTLEG			8
#define BIT_UPPERTORSO			16
#define BIT_CAPTOPUPPERTORSO	32
#define BIT_CAPBOTTOMUPPERTORSO	64
#define BIT_LEFTARM				128
#define BIT_RIGHTARM			256
#define BIT_HEAD				512
#define BIT_CENTERSPIKE			1024
#define BIT_LEFT1SPIKE			2048
#define BIT_RIGHT1SPIKE			4096
#define BIT_RIGHT2SPIKE			8192
#define BIT_CAPHEAD				16384
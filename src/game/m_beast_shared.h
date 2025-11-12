//
// m_beast_shared.h -- Data and function declarations shared between m_beast.c and m_beast_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h" //mxd

typedef enum AnimID_e
{
	ANIM_BITEUP,
	ANIM_BITELOW,
	ANIM_BITEUP2,
	ANIM_EATING_TWITCH,
	ANIM_EATING,
	ANIM_EATDOWN,
	ANIM_WALK,
	ANIM_WALKLEFT,
	ANIM_WALKRT, //TODO: rename to ANIM_WALKRIGHT?
	ANIM_JUMP,
	ANIM_FJUMP,
	ANIM_INAIR,
	ANIM_LAND,
	ANIM_GINAIR,
	ANIM_GLAND,
	ANIM_STAND,
	ANIM_DELAY,
	ANIM_DIE,
	ANIM_DIE_NORM,
	ANIM_CHARGE,
	ANIM_ROAR,
	ANIM_WALKATK,
	ANIM_STUN,
	ANIM_SNATCH,
	ANIM_READY_CATCH,
	ANIM_CATCH,
	ANIM_BITEUP_SFIN,
	ANIM_BITELOW_SFIN,
	ANIM_BITEUP2_SFIN,
	ANIM_QUICK_CHARGE,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_ROAR,
	SND_ROAR2,
	SND_SNORT1,
	SND_SNORT2,

	SND_STEP1,
	SND_STEP2,
	SND_LAND,

	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,

	SND_SWIPE, //TODO: unused, no sound file.
	SND_SLAM,
	SND_SNATCH,
	SND_CHOMP,
	SND_TEAR1,
	SND_TEAR2,
	SND_THROW, //TODO: unused.
	SND_CATCH,
	SND_SWALLOW, //TODO: unused, no sound file.

	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,

	SND_CORVUS_SCREAM1,
	SND_CORVUS_SCREAM2,
	SND_CORVUS_SCREAM3,
	SND_CORVUS_DIE,

	NUM_SOUNDS
} SoundID_t;

void tbeast_snort(edict_t* self);
void tbeast_growl(edict_t* self);
void tbeast_check_mood(edict_t* self); //mxd
void tbeast_pause(edict_t* self);
void tbeast_bite(edict_t* self, float forward_offset, float right_offset, float up_offset);
void tbeast_land(edict_t* self);
void tbeast_roar(edict_t* self);
void tbeast_apply_jump(edict_t* self);
void tbeast_ready_catch(edict_t* self);
void tbeast_throw_toy(edict_t* self);
void tbeast_shake_toy(edict_t* self, float forward_offset, float right_offset, float up_offset);
void tbeast_check_snatch(edict_t* self, float forward_offset, float right_offset, float up_offset);
void tbeast_gore_toy(edict_t* self, float jump_height);
void tbeast_anger_sound(edict_t* self);
void tbeast_leap(edict_t* self, float forward_offset, float right_offset, float up_offset);
void tbeast_eat_order(edict_t* self);
void tbeast_footstep(edict_t* self);
void tbeast_walk_order(edict_t* self);
void tbeast_stand_order(edict_t* self);
void tbeast_dead(edict_t* self);
void tbeast_charge(edict_t* self, float force);
void tbeast_done_gore(edict_t* self);
void tbeast_run(edict_t* self, float dist);
void tbeast_check_landed(edict_t* self);
void tbeast_inair(edict_t* self);
void tbeast_gcheck_landed(edict_t* self);
void tbeast_ginair(edict_t* self);
void tbeast_snatch_go(edict_t* self);
void tbeast_roar_knockdown(edict_t* self);
void tbeast_roar_short(edict_t* self);
void tbeast_gibs(edict_t* self);

#define TB_HIBITE_F		150
#define TB_HIBITE_R		0
#define TB_HIBITE_U		108

#define TB_LOBITE_F		150
#define TB_LOBITE_R		0
#define TB_LOBITE_U		36

#define TB_WLKBITE_F	224
#define TB_WLKBITE_U	72

#define TBEAST_STD_MELEE_RNG	128

#define TB_FWD_OFFSET	(-64)
#define TB_UP_OFFSET	(-32)
#define TB_RT_OFFSET	(-24)
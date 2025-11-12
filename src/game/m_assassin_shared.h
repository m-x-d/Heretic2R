//
// m_assassin_shared.h -- Data and function declarations shared between m_assassin.c and m_assassin_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

typedef enum AnimID_e
{
	ANIM_DAGGERL,
	ANIM_DAGGERR,
	ANIM_DAGGERB,
	ANIM_DAGGERC,
	ANIM_NEWDAGGER,
	ANIM_NEWDAGGERB,
	ANIM_BACKFLIP,
	ANIM_FRONTFLIP,
	ANIM_DODGE_RIGHT,
	ANIM_DODGE_LEFT,
	ANIM_DEATHA,
	ANIM_DEATHB,
	ANIM_JUMP,
	ANIM_RUN,
	ANIM_PAIN1,
	ANIM_PAIN2,
	ANIM_DELAY,
	ANIM_STAND,
	ANIM_CROUCH,
	ANIM_UNCROUCH,
	ANIM_EVJUMP,
	ANIM_EVBACKFLIP,
	ANIM_EVFRONTFLIP,
	ANIM_INAIR,
	ANIM_LAND,
	ANIM_FORCED_JUMP,
	ANIM_FJUMP,
	ANIM_BFINAIR,
	ANIM_BFLAND,
	ANIM_FFINAIR,
	ANIM_FFLAND,
	ANIM_EVINAIR,
	ANIM_TELEPORT,
	ANIM_CLOAK,
	ANIM_WALK,
	ANIM_WALK_LOOP,
	ANIM_BACKSPRING,

	ANIM_CROUCH_TRANS,
	ANIM_CROUCH_IDLE,
	ANIM_CROUCH_LOOK_RIGHT,
	ANIM_CROUCH_LOOK_RIGHT_IDLE,
	ANIM_CROUCH_LOOK_L2R,
	ANIM_CROUCH_LOOK_LEFT,
	ANIM_CROUCH_LOOK_LEFT_IDLE,
	ANIM_CROUCH_LOOK_R2L,
	ANIM_CROUCH_LOOK_R2C,
	ANIM_CROUCH_LOOK_L2C,
	ANIM_CROUCH_POKE,
	ANIM_CROUCH_END,

	ANIM_C_IDLE1,
	ANIM_C_RUN1,
	ANIM_C_ATTACK1,
	ANIM_C_ATTACK2,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE1,
	SND_GIB,
	SND_THROW1,
	SND_THROW2,
	SND_DAGHITF,
	SND_DAGHITW,
	SND_JUMP,
	SND_FLIP,
	SND_LAND,
	SND_LANDF,
	SND_SLIDE,
	SND_SLASH1,
	SND_SLASH2,
	SND_GROWL1,
	SND_GROWL2,
	SND_GROWL3,
	SND_CLOAK,
	SND_DECLOAK,

	NUM_SOUNDS
} SoundID_t;

//mxd. Forward declarations for m_assassin_anim.c:
extern void assassin_pause(edict_t* self);
extern void assassin_sound(edict_t* self, float channel, float sound_num, float attenuation);
extern void assassin_inair_go(edict_t* self);
extern void assassin_evade_inair_go(edict_t* self);
extern void assassin_backflip_inair_go(edict_t* self);
extern void assassin_fwdflip_inair_go(edict_t* self);
extern void assassin_run(edict_t* self, float dist);
extern void assassin_walk_loop_go(edict_t* self);
extern void assassin_ai_walk(edict_t* self, float dist);
extern void assassin_post_pain(edict_t* self);
extern void assassin_dead(edict_t* self);
extern void assassin_crouch_idle_decision(edict_t* self);

extern void assassin_growl(edict_t* self);
extern void assassin_attack(edict_t* self, float flags);
extern void assassin_check_loop(edict_t* self, float frame);
extern void assassin_set_crouched(edict_t* self);
extern void assassin_crouched_check_attack(edict_t* self, float attack);
extern void assassin_uncrouch(edict_t* self);
extern void assassin_unset_crouched(edict_t* self);
extern void assassin_stop(edict_t* self);
extern void assassin_jump_go(edict_t* self, float forward_speed, float up_speed, float right_speed);
extern void assassin_skip_frame_skill_check(edict_t* self);
extern void assassin_gone(edict_t* self);
extern void assassin_enable_fmnode(edict_t* self, float node);
extern void assassin_ready_teleport(edict_t* self);
extern void assassin_init_cloak(edict_t* self);

#define BIT_DADDYNULL	0
#define BIT_TORSOFT		1
#define BIT_TORSOBK		2
#define BIT_HEAD		4
#define BIT_LKNIFE		8
#define BIT_RKNIFE		16
#define BIT_R4ARM		32
#define BIT_L4ARM		64
#define BIT_HIPS		128
#define BIT_LCALF		256
#define BIT_RCALF		512
#define BIT_RTHIGH		1024
#define BIT_LTHIGH		2048
#define BIT_KNIFES		4096
#define BIT_LUPARM		8192
#define BIT_RUPARM		16384
//
// m_assassin_shared.h -- Data and function declarations shared between m_assassin.c and m_assassin_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h" //mxd

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

extern const animmove_t assassin_move_daggerl;
extern const animmove_t assassin_move_daggerr;
extern const animmove_t assassin_move_daggerb;
extern const animmove_t assassin_move_daggerc;
extern const animmove_t assassin_move_newdagger;
extern const animmove_t assassin_move_newdaggerb;
extern const animmove_t assassin_move_backflip;
extern const animmove_t assassin_move_frontflip;
extern const animmove_t assassin_move_dodge_right;
extern const animmove_t assassin_move_dodge_left;
extern const animmove_t assassin_move_deatha;
extern const animmove_t assassin_move_deathb;
extern const animmove_t assassin_move_jump;
extern const animmove_t assassin_move_run;
extern const animmove_t assassin_move_pain1;
extern const animmove_t assassin_move_pain2;
extern const animmove_t assassin_move_delay;
extern const animmove_t assassin_move_stand;
extern const animmove_t assassin_move_crouch;
extern const animmove_t assassin_move_uncrouch;

extern const animmove_t assassin_move_evade_jump;
extern const animmove_t assassin_move_evade_backflip;
extern const animmove_t assassin_move_evade_frontflip;
extern const animmove_t assassin_move_inair;
extern const animmove_t assassin_move_evinair;
extern const animmove_t assassin_move_land;
extern const animmove_t assassin_move_forcedjump;
extern const animmove_t assassin_move_fjump;
extern const animmove_t assassin_move_ffinair;
extern const animmove_t assassin_move_ffland;
extern const animmove_t assassin_move_bfinair;
extern const animmove_t assassin_move_bfland;
extern const animmove_t assassin_move_teleport;
extern const animmove_t assassin_move_cloak;
extern const animmove_t assassin_move_walk;
extern const animmove_t assassin_move_walk_loop;
extern const animmove_t assassin_move_backspring;

extern const animmove_t assassin_move_crouch_trans;
extern const animmove_t assassin_move_crouch_idle;
extern const animmove_t assassin_move_crouch_look_right;
extern const animmove_t assassin_move_crouch_look_right_idle;
extern const animmove_t assassin_move_crouch_look_l2r;
extern const animmove_t assassin_move_crouch_look_left;
extern const animmove_t assassin_move_crouch_look_left_idle;
extern const animmove_t assassin_move_crouch_look_r2l;
extern const animmove_t assassin_move_crouch_look_r2c;
extern const animmove_t assassin_move_crouch_look_l2c;
extern const animmove_t assassin_move_crouch_poke;
extern const animmove_t assassin_move_crouch_end;

extern const animmove_t assassin_move_c_idle1;
extern const animmove_t assassin_move_c_run1;
extern const animmove_t assassin_move_c_attack1;
extern const animmove_t assassin_move_c_attack2;

//mxd. Forward declarations for m_assassin_anim.c:
extern void assassin_pause(edict_t* self);
extern void assassin_sound(edict_t* self, float channel, float sound_num, float attenuation);
extern void assassin_go_inair(edict_t* self);
extern void assassin_go_evinair(edict_t* self);
extern void assassin_go_bfinair(edict_t* self);
extern void assassin_go_ffinair(edict_t* self);
extern void assassin_run(edict_t* self, float dist);
extern void assassin_walk_loop_go(edict_t* self);
extern void assassin_ai_walk(edict_t* self, float dist);
extern void assassin_post_pain(edict_t* self);
extern void assassin_dead(edict_t* self);
extern void assassin_crouch_idle_decision(edict_t* self);

extern void assassin_growl(edict_t* self);
extern void assassin_attack(edict_t* self, float flags);
extern void assassinCheckLoop(edict_t* self, float frame);
extern void assassinSetCrouched(edict_t* self);
extern void assassinCrouchedCheckAttack(edict_t* self, float attack);
extern void assassinUnCrouch(edict_t* self);
extern void assassinUndoCrouched(edict_t* self);
extern void assassinStop(edict_t* self);
extern void assassinGoJump(edict_t* self, float forward_speed, float up_speed, float right_speed);
extern void assassin_skip_frame_skill_check(edict_t* self);
extern void assassinGone(edict_t* self);
extern void assassinNodeOn(edict_t* self, float node);
extern void assassinReadyTeleport(edict_t* self);
extern void assassinInitCloak(edict_t* self);

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
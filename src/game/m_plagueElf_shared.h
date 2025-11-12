//
// m_plagueElf_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Edict.h"

typedef enum AnimID_e
{
	ANIM_STAND1,
	ANIM_WALK1,
	ANIM_WALK2,
	ANIM_RUN1,
	ANIM_RUNATK1,
	ANIM_FJUMP,
	ANIM_INAIR,
	ANIM_LAND,
	ANIM_MELEE1,
	ANIM_MELEE2,
	ANIM_DIE1,
	ANIM_DIE2,
	ANIM_DIE3,
	ANIM_DIE4,
	ANIM_FIST1,
	ANIM_LEAN1,
	ANIM_SHAKE1,
	ANIM_PAIN1,
	ANIM_DELAY,
	ANIM_MISSILE,

	ANIM_KDEATH_GO,
	ANIM_KDEATH_LOOP,
	ANIM_KDEATH_END,

	// New animations.
	ANIM_CRAZY_A,
	ANIM_CRAZY_B,
	ANIM_CURSING,
	ANIM_POINT,
	ANIM_SCARED,

	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_IDLE3,
	ANIM_C_WALK1,
	ANIM_C_WALK2,
	ANIM_C_RUN1,
	ANIM_C_ATTACK1,
	ANIM_C_ATTACK2,
	ANIM_C_ATTACK3,
	ANIM_C_ATTACK4,
	ANIM_C_PAIN1,
	ANIM_C_DEATH1,
	ANIM_C_DEATH2,
	ANIM_C_DEATH3,
	ANIM_C_DEATH4,
	ANIM_C_THINKAGAIN,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_PAIN3,
	SND_DIE1,
	SND_DIE2,
	SND_DIE3,
	SND_GIB,
	SND_ATTACKHIT1,
	SND_ATTACKHIT2,
	SND_ATTACKMISS1,
	SND_MOAN1,
	SND_MOAN2,
	SND_SHIVER, //TODO: unused.
	SND_PANT,
	SND_GASP, //TODO: unused.
	SND_SIGH, //TODO: unused.
	SND_ATTACK1,
	SND_ATTACK2,
	SND_ATTACK3,
	SND_DISMEMBER1,
	SND_DISMEMBER2,

	// Sight.
	VOICE_FIRST_GROUP,
	VOICE_SIGHT_EAT_FLESH1,
	VOICE_SIGHT_GET_HIM1,
	VOICE_SIGHT_GET_HIM2,
	VOICE_SIGHT_GET_HIM3,
	VOICE_SIGHT_THERES_ONE,

	// Support.
	VOICE_SUPPORT_LIVER,

	// Valid single sight.
	VOICE_FIRST_ALONE,
	VOICE_MISC_DIE,
	VOICE_MISC_FLESH,
	VOICE_SUPPORT_GONNA_DIE1,
	VOICE_SUPPORT_YES,
	VOICE_LAST_GROUP,

	VOICE_MISC_LEAVE_ME1, //TODO: unused.
	VOICE_MISC_NO, //TODO: unused.

	NUM_SOUNDS
} SoundID_t; //mxd. Add missing typedef name.

extern void plagueelf_cinematic_spell(edict_t* self);

extern void plagueelf_death_squeal(edict_t* self);
extern void plagueelf_squeal(edict_t* self);
extern void plagueelf_growl(edict_t* self);
extern void plagueelf_strike(edict_t* self);
extern void plagueelf_pause(edict_t* self);
extern void plagueelf_attack(edict_t* self);

extern void plagueelf_knockback_death_loop(edict_t* self);
extern void plagueelf_knockback_death_check_land(edict_t* self);
extern void plagueelf_spell(edict_t* self);
extern void plagueelf_apply_jump(edict_t* self);

extern void plagueelf_check_distance(edict_t* self);
extern void plagueelf_land(edict_t* self);
extern void plagueelf_inair_go(edict_t* self);
extern void plagueelf_run_go(edict_t* self);

#define BIT_BASE	0
#define BIT_HANDLE	1
#define BIT_HOE		2
#define BIT_GAFF	4
#define BIT_HAMMER	8
#define BIT_BODY	16
#define BIT_L_LEG	32
#define BIT_R_LEG	64
#define BIT_R_ARM	128
#define BIT_L_ARM	256
#define BIT_HEAD	512
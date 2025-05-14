//
// m_tcheckrik_shared.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

typedef enum AnimID_e
{	
	ANIM_BACK,
	ANIM_DEATHFR,
	ANIM_IDLE,
	ANIM_LAND,
	ANIM_INAIR,
	ANIM_FORCED_JUMP,
	ANIM_FINAIR,
	ANIM_FJUMP,	
	ANIM_PAINA,
	ANIM_PAINC,
	ANIM_RUN,
	ANIM_SPEAR,
	ANIM_SWORD,
	ANIM_SPELL,
	ANIM_SPELL2,
	ANIM_WALK,
	ANIM_DELAY,
	ANIM_KNOCK1_GO,
	ANIM_KNOCK1_LOOP,
	ANIM_KNOCK1_END,
	ANIM_TWITCH,

	ANIM_C_ACTION1,
	ANIM_C_ACTION2,
	ANIM_C_ACTION3,
	ANIM_C_ACTION4,
	ANIM_C_ATTACK1,
	ANIM_C_ATTACK2,
	ANIM_C_ATTACK3,
	ANIM_C_BACKPEDAL,
	ANIM_C_DEATH1,
	ANIM_C_IDLE1,
	ANIM_C_IDLE2,
	ANIM_C_IDLE3,
	ANIM_C_PAIN1,
	ANIM_C_RUN1,
	ANIM_C_WALK1,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAINM,
	SND_PAINF,
	SND_DIEM,
	SND_DIEF,
	SND_GIB,
	SND_SWIPE,
	SND_SWIPEHITF,
	SND_SWIPEHITW,
	SND_SPELLM,
	SND_SPELLM2,
	SND_SPLPWRUPF,
	SND_SPELLF,
	SND_GROWLM1,
	SND_GROWLM2,
	SND_GROWLF1,
	SND_GROWLF2,
	SND_THUD,

	NUM_SOUNDS
} SoundID_t;

extern const animmove_t insect_move_back;
extern const animmove_t insect_move_deathfr;
extern const animmove_t insect_move_idle;
extern const animmove_t insect_move_land;
extern const animmove_t insect_move_inair;
extern const animmove_t insect_move_forcedjump;
extern const animmove_t insect_move_finair;
extern const animmove_t insect_move_fjump;
extern const animmove_t insect_move_paina;
extern const animmove_t insect_move_painc;
extern const animmove_t insect_move_run;
extern const animmove_t insect_move_spear;
extern const animmove_t insect_move_sword;
extern const animmove_t insect_move_spell;
extern const animmove_t insect_move_spell2;
extern const animmove_t insect_move_walk;
extern const animmove_t insect_delay;

extern const animmove_t insect_move_c_action1;
extern const animmove_t insect_move_c_action2;
extern const animmove_t insect_move_c_action3;
extern const animmove_t insect_move_c_action4;
extern const animmove_t insect_move_c_idle1;
extern const animmove_t insect_move_c_idle2;
extern const animmove_t insect_move_c_idle3;
extern const animmove_t insect_move_c_walk;
extern const animmove_t insect_move_c_run;
extern const animmove_t insect_move_c_backpedal;
extern const animmove_t insect_move_c_attack1;
extern const animmove_t insect_move_c_attack2;
extern const animmove_t insect_move_c_attack3;
extern const animmove_t insect_move_c_death1;
extern const animmove_t insect_move_c_pain1;
extern const animmove_t insect_move_knock1_go;
extern const animmove_t insect_move_knock1_loop;
extern const animmove_t insect_move_knock1_end;
extern const animmove_t insect_move_twitch;

extern void tcheckrik_c_dead (edict_t *self);

extern void tcheckrik_release_spell (edict_t *self);

extern void tcheckrik_attack (edict_t *self, float attack_type);
extern void tcheckrik_spell_attack(edict_t *self, float spell_type);
extern void tcheckrik_staff_attack(edict_t *self);
extern void insectCheckLoop (edict_t *self, float frame);

extern void tcheckrik_dead(edict_t *self);
extern void tcheckrik_growl(edict_t *self);
extern void tcheckrik_pause (edict_t *self);
extern void tcheckrik_inair_go(edict_t *self);
extern void tcheckrik_sound(edict_t *self, float channel, float sound_num, float attenuation);
extern void tcheckrik_wait_twitch (edict_t *self);
extern void tcheckrik_flyback_loop(edict_t *self);
extern void tcheckrik_flyback_move(edict_t *self);
extern void tcheckrik_idle_sound (edict_t *self);
extern void tcheckrik_forced_inair_go(edict_t *self);

#define BIT_MASTER		0
#define BIT_LLEG		1
#define BIT_HEAD		2
#define BIT_LMANDIBLE	4
#define BIT_RMANDIBLE	8
#define BIT_CROWN		16
#define BIT_L2NDARM		32
#define BIT_SPEAR		64
#define BIT_FEMHAND		128
#define BIT_SWORD		256
#define BIT_STAFF		512
#define BIT_GEM			1024
#define BIT_R2NDARM		2048
#define BIT_RWINGS		4096
#define BIT_LWINGS		8192
#define BIT_RLEG		16384

#define TC_ATK_STAB		1 //TODO: never used?
#define TC_ATK_HACK		2

#define TC_SPL_FIRE		1
#define TC_SPL_GLOW		2
#define TC_SPL_FIRE2	3
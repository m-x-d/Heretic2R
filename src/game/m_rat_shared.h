//
// m_rat.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h"

typedef enum AnimID_e
{
	ANIM_EATING1,
	ANIM_EATING2,
	ANIM_EATING3,
	ANIM_STAND1,
	ANIM_STAND2,
	ANIM_STAND3,
	ANIM_STAND4,
	ANIM_STAND5,
	ANIM_STAND6,
	ANIM_STAND7,
	ANIM_STAND8,
	ANIM_WATCH1,
	ANIM_WATCH2,
	ANIM_WALK1,
	ANIM_RUN1,
	ANIM_RUN2,
	ANIM_RUN3,
	ANIM_MELEE1,
	ANIM_MELEE2,
	ANIM_MELEE3,
	ANIM_PAIN1,
	ANIM_DIE1,
	ANIM_DIE2,

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_BITEHIT1,
	SND_BITEMISS1,
	SND_BITEMISS2,

	SND_SCRATCH,

	SND_HISS,

	SND_PAIN1,
	SND_PAIN2,

	SND_CHATTER1,
	SND_CHATTER2,
	SND_CHATTER3,

	SND_CHEW1,
	SND_CHEW2,
	SND_CHEW3,

	SND_SWALLOW,

	SND_DIE,
	SND_GIB,

	NUM_SOUNDS
} SoundID_t;

extern const animmove_t rat_move_eat1;
extern const animmove_t rat_move_eat2;
extern const animmove_t rat_move_eat3;
extern const animmove_t rat_move_stand1;
extern const animmove_t rat_move_stand2;
extern const animmove_t rat_move_stand3;
extern const animmove_t rat_move_stand4;
extern const animmove_t rat_move_stand5;
extern const animmove_t rat_move_stand6;
extern const animmove_t rat_move_stand7;
extern const animmove_t rat_move_stand8;
extern const animmove_t rat_move_watch1;
extern const animmove_t rat_move_watch2;
extern const animmove_t rat_move_walk1;
extern const animmove_t rat_move_run1;
extern const animmove_t rat_move_run2;
extern const animmove_t rat_move_run3;
extern const animmove_t rat_move_melee1;
extern const animmove_t rat_move_melee2;
extern const animmove_t rat_move_melee3;
extern const animmove_t rat_move_pain1;
extern const animmove_t rat_move_death1;
extern const animmove_t rat_move_death2;

extern void rat_use(edict_t* self, edict_t* other, edict_t* activator);
extern void rat_touch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf);

extern void ratdeathsqueal(edict_t* self);
extern void ratsqueal(edict_t* self);
extern void ratbite(edict_t* self);
extern void rat_pain_init(edict_t* self);
extern void rat_runorder(edict_t* self);
extern void rat_standorder(edict_t* self);
extern void rat_pause(edict_t* self);

extern void rat_eatorder(edict_t* self);
extern void rathiss(edict_t* self);
extern void ratscratch(edict_t* self);
extern void ratchatter(edict_t* self);
extern void ratchew(edict_t* self);
extern void ratswallow(edict_t* self);
extern void ratjump(edict_t* self);

extern void rat_pain(edict_t* self, G_Message_t* msg);
extern void rat_death(edict_t* self, G_Message_t* msg);
extern void rat_run(edict_t* self, G_Message_t* msg);
extern void rat_walk(edict_t* self, G_Message_t* msg);
extern void rat_melee(edict_t* self, G_Message_t* msg);
extern void rat_watch(edict_t* self, G_Message_t* msg);
extern void rat_stand(edict_t* self, G_Message_t* msg);
extern void rat_eat(edict_t* self, G_Message_t* msg);

extern void rat_ai_eat(edict_t* self, float dist);
extern void rat_ai_stand(edict_t* self, float dist);
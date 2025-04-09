//
// m_elflord_shared.h -- Data and function declarations shared between m_elflord.c and m_elflord_anim.c.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_local.h" //mxd

typedef enum AnimID_e
{
	ANIM_HOVER,
	ANIM_FLOAT_FORWARD,
	ANIM_CHARGE,			// Charge forward.
	ANIM_CHARGE_BTRANS,		// Transition to charge. //TODO: unused.
	ANIM_FLOAT_BACK,		// Float backwards (w / attack). //TODO: unused.
	ANIM_DODGE_RIGHT, //TODO: unused.
	ANIM_DODGE_LEFT, //TODO: unused.
	ANIM_ATTACK_SOA_BTRANS,	// Attack 1 (lightning sphere) beginning.
	ANIM_ATTACK_SOA_LOOP,	// Attack 1 (lightning sphere) loop.
	ANIM_ATTACK_SOA_END,	// Attack 1 (lightning sphere) ending.
	ANIM_ATTACK_LS,			// Attack 2 (light surge).
	ANIM_PAIN1, //TODO: unused.
	ANIM_DIE_BTRANS,		// Death beginning. //TODO: unused.
	ANIM_DIE_LOOP,			// Death loop.
	ANIM_SHIELD, //TODO: unused.
	ANIM_ATTACK,
	ANIM_MOVE,
	ANIM_WAIT, //TODO: unused.
	ANIM_COME_TO_LIFE, //TODO: unused.

	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_PAIN1,
	SND_PAIN2,
	SND_DIE,

	SND_SACHARGE,
	SND_SAFIRE,
	SND_SAHIT, //TODO: unused.

	SND_PROJ1,
	SND_BEAM,

	NUM_SOUNDS
} SoundID_t;

extern const animmove_t elflord_move_idle;
extern const animmove_t elflord_move_run;
extern const animmove_t elflord_move_charge;
extern const animmove_t elflord_move_charge_trans;
extern const animmove_t elflord_move_floatback;
extern const animmove_t elflord_move_dodgeright;
extern const animmove_t elflord_move_dodgeleft;
extern const animmove_t elflord_move_soa_begin;
extern const animmove_t elflord_move_soa_loop;
extern const animmove_t elflord_move_soa_end;
extern const animmove_t elflord_move_ls;
extern const animmove_t elflord_move_pain;
extern const animmove_t elflord_move_death_btrans;
extern const animmove_t elflord_move_death_loop;
extern const animmove_t elflord_move_shield;
extern const animmove_t elflord_move_attack;
extern const animmove_t elflord_move_move;
extern const animmove_t elflord_move_wait;
extern const animmove_t elflord_move_come_to_life;

qboolean elfLordCheckAttack(edict_t* self);

void elflord_decell(edict_t* self, float value);
void elflord_decide_movement(edict_t* self);
void elflord_ai_stand(edict_t* self, float dist);
void elflord_run(edict_t* self, G_Message_t* msg);
void elflord_death_start(edict_t* self, G_Message_t* msg);
void elflord_soa_start(edict_t* self, G_Message_t* msg);
void elflordRandomRushSound(edict_t* self);
void elflordSound(edict_t* self, float channel, float sndindex, float atten);
void elflord_flymove(edict_t* self, float dist);
void elfLordPause(edict_t* self);
void elflord_finish_death(edict_t* self); //mxd
void elfLordGoCharge(edict_t* self);
void elflord_soa_end(edict_t* self);

void elflord_StartBeam(edict_t* self);
void elflord_EndBeam(edict_t* self);
void elford_Attack(edict_t* self);

void elflord_face(edict_t* self);
void elflord_track(edict_t* self);
void elflord_SlideMeter(edict_t* self);
void elflord_soa_go(edict_t* self);
void elflord_soa_charge(edict_t* self);
void elflord_FixAngles(edict_t* self);
//
// m_elflord_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_elflord_anim.h"
#include "m_elflord_shared.h"
#include "g_monster.h"
#include "g_local.h"

// Idle - sit and float.
static const animframe_t elflord_frames_idle[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle9,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle16,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle19,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle21,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle22,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle23,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle24,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t elflord_move_idle = ANIMMOVE(elflord_frames_idle, NULL);

// Run.
static const animframe_t elflord_frames_run[] =
{
	{ FRAME_ftforwd1,	NULL, 0, 0, 0, elflord_flymove, 7, NULL },
	{ FRAME_ftforwd2,	NULL, 0, 0, 0, elflord_flymove, 7, NULL }, //mxd. Removed elflordRandomRushSound() thinkfunc.
	{ FRAME_ftforwd3,	NULL, 0, 0, 0, elflord_flymove, 7, NULL },
	{ FRAME_ftforwd4,	NULL, 0, 0, 0, elflord_flymove, 8, NULL },
	{ FRAME_ftforwd5,	NULL, 0, 0, 0, elflord_flymove, 8, NULL }, //mxd. Removed elflordRandomRushSound() thinkfunc.
	{ FRAME_ftforwd6,	NULL, 0, 0, 0, elflord_flymove, 8, NULL },
	{ FRAME_ftforwd7,	NULL, 0, 0, 0, elflord_flymove, 8, NULL },
	{ FRAME_ftforwd8,	NULL, 0, 0, 0, elflord_flymove, 7, NULL }, //mxd. Removed elflordRandomRushSound() thinkfunc.
	{ FRAME_ftforwd9,	NULL, 0, 0, 0, elflord_flymove, 7, NULL },
	{ FRAME_ftforwd10,	NULL, 0, 0, 0, elflord_flymove, 7, NULL }, //mxd. Removed elflordRandomRushSound() thinkfunc.
	{ FRAME_ftforwd11,	NULL, 0, 0, 0, elflord_flymove, 7, NULL },
};
const animmove_t elflord_move_run = ANIMMOVE(elflord_frames_run, NULL);

// Charge forward.
static const animframe_t elflord_frames_charge[] =
{
	{ FRAME_charge3,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge5,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge6,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge8,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge10,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge11,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge12,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
	{ FRAME_charge13,	NULL, 0, 0, 0, elflord_flymove, 12, NULL },
};
const animmove_t elflord_move_charge = ANIMMOVE(elflord_frames_charge, elflord_check_attack);

// Transition to charge.
static const animframe_t elflord_frames_charge_trans[] =
{
	{ FRAME_charge1,	NULL, 0, 0, 0, elflord_flymove, 8, NULL },
	{ FRAME_charge2,	NULL, 0, 0, 0, elflord_flymove, 8, NULL },
};
const animmove_t elflord_move_charge_trans = ANIMMOVE(elflord_frames_charge_trans, elflord_charge);

// Float backwards.
static const animframe_t elflord_frames_floatback[] =
{
	{ FRAME_ftback1,	NULL, 0, 0, 0, elflord_flymove, -1,  NULL },
	{ FRAME_ftback2,	NULL, 0, 0, 0, elflord_flymove, -2,  NULL },
	{ FRAME_ftback3,	NULL, 0, 0, 0, elflord_flymove, -4,  NULL },
	{ FRAME_ftback4,	NULL, 0, 0, 0, elflord_flymove, -8,  NULL },
	{ FRAME_ftback5,	NULL, 0, 0, 0, elflord_flymove, -16, NULL },
	{ FRAME_ftback6,	NULL, 0, 0, 0, elflord_flymove, -24, NULL },
	{ FRAME_ftback7,	NULL, 0, 0, 0, elflord_flymove, -16, NULL },
	{ FRAME_ftback8,	NULL, 0, 0, 0, elflord_flymove, -8,  NULL },
	{ FRAME_ftback9,	NULL, 0, 0, 0, elflord_flymove, -4,  NULL },
	{ FRAME_ftback10,	NULL, 0, 0, 0, elflord_flymove, -2,  NULL },
	{ FRAME_ftback11,	NULL, 0, 0, 0, elflord_flymove, -1,  NULL },
};
const animmove_t elflord_move_floatback = ANIMMOVE(elflord_frames_floatback, elflord_check_attack);

// Dodge right.
static const animframe_t elflord_frames_dodgeright[] =
{
	{ FRAME_dgrite1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgrite2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgrite3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgrite4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgrite5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgrite6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgrite7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t elflord_move_dodgeright = ANIMMOVE(elflord_frames_dodgeright, elflord_check_attack);

// Dodge left.
static const animframe_t elflord_frames_dodgeleft[] =
{
	{ FRAME_dgleft1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgleft2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgleft3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgleft4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgleft5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgleft6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_dgleft7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t elflord_move_dodgeleft = ANIMMOVE(elflord_frames_dodgeleft, elflord_check_attack);

// Attack 1 (lightning sphere) beginning.
static const animframe_t elflord_frames_soa_begin[] =
{
	{ FRAME_attkb1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb4,		NULL, 0, 0, 0, NULL, 0, elflord_soa_charge },
	{ FRAME_attkb5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb13,	NULL, 0, 0, 0, NULL, 0, elflord_soa_go },
	{ FRAME_attkb14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb20,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t elflord_move_soa_begin = ANIMMOVE(elflord_frames_soa_begin, elflord_check_attack);

// Attack 1 (lightning sphere) loop.
static const animframe_t elflord_frames_soa_loop[] =
{
	{ FRAME_attka1,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t elflord_move_soa_loop = ANIMMOVE(elflord_frames_soa_loop, elflord_soa_end);

// Attack 1 (lightning sphere) ending.
static const animframe_t elflord_frames_soa_end[] =
{
	{ FRAME_attka1,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t elflord_move_soa_end = ANIMMOVE(elflord_frames_soa_end, elflord_check_attack); //mxd. numframes:2 in original logic.

// Attack 2 (light surge).
static const animframe_t elflord_frames_ls[] =
{
	{ FRAME_attkb1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_start_beam },

	// Held for 3 seconds (30 frames).
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },

	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },

	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },
	{ FRAME_attkb16,	NULL, 0, 0, 0, NULL, 0, elflord_track },

	{ FRAME_attkb17,	NULL, 0, 0, 0, NULL, 0, elflord_reset_pitch },
	{ FRAME_attkb18,	NULL, 0, 0, 0, NULL, 0, elflord_reset_pitch },
	{ FRAME_attkb19,	NULL, 0, 0, 0, NULL, 0, elflord_end_beam },
	{ FRAME_attkb20,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t elflord_move_ls = ANIMMOVE(elflord_frames_ls, elflord_check_attack);

// Pain.
static const animframe_t elflord_frames_pain[] =
{
	{ FRAME_pain1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain7,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t elflord_move_pain = ANIMMOVE(elflord_frames_pain, elflord_check_attack);

// Death beginning.
static const animframe_t elflord_frames_death_btrans[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death6,		NULL, 0, 0, 0, NULL, 0, elflord_finish_death },
};
const animmove_t elflord_move_death_btrans = ANIMMOVE(elflord_frames_death_btrans, NULL);

// Death loop.
static const animframe_t elflord_frames_death_loop[] =
{
	{ FRAME_death7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death16,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t elflord_move_death_loop = ANIMMOVE(elflord_frames_death_loop, M_EndDeath);

// Shield.
static const animframe_t elflord_frames_shield[] =
{
	{ FRAME_shield1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shield23,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t elflord_move_shield = ANIMMOVE(elflord_frames_shield, elflord_check_attack);

// Attack.
static const animframe_t elflord_frames_attack[] =
{
	{ FRAME_newatk1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk8,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk9,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk10,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk11,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk12,	NULL, 0, 0, 0, ai_charge2, 0, elflord_attack },
	{ FRAME_newatk13,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk14,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk15,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newatk16,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t elflord_move_attack = ANIMMOVE(elflord_frames_attack, elflord_check_attack);

#define ELFLORD_DECELERATE	0.8f

// Move - hover to a nearby waypoint.
static const animframe_t elflord_frames_move[] =
{
	{ FRAME_idle1,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle2,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle3,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle4,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle5,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle6,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle7,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle8,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle9,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle10,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle11,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle12,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle13,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle14,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle15,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle16,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle17,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle18,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle19,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle20,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle21,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle22,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle23,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
	{ FRAME_idle24,		NULL, 0, 0, 0, elflord_decelerate, ELFLORD_DECELERATE, elflord_try_charge },
};
const animmove_t elflord_move_move = ANIMMOVE(elflord_frames_move, elflord_check_attack);

// Idle - sit and float.
static const animframe_t elflord_frames_wait[] =
{
	{ FRAME_idle1,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle2,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle3,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle4,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle5,		NULL, 0, 0, 0, ai_charge2, 0, elflord_check_attack },
	{ FRAME_idle6,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle7,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle8,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle9,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle10,		NULL, 0, 0, 0, ai_charge2, 0, elflord_check_attack },
	{ FRAME_idle11,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle12,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle13,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle14,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle15,		NULL, 0, 0, 0, ai_charge2, 0, elflord_check_attack },
	{ FRAME_idle16,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle17,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle18,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle19,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle20,		NULL, 0, 0, 0, ai_charge2, 0, elflord_check_attack },
	{ FRAME_idle21,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle22,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle23,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_idle24,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t elflord_move_wait = ANIMMOVE(elflord_frames_wait, elflord_check_attack);

static const animframe_t elflord_frames_come_to_life[] =
{
	{ FRAME_attka1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },

	// Charge.
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },

	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },
	{ FRAME_attka12,	NULL, 0, 0, 0, NULL, 0, elflord_update_charge_meter },

	{ FRAME_attka13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attka15,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t elflord_move_come_to_life = ANIMMOVE(elflord_frames_come_to_life, elflord_check_attack);
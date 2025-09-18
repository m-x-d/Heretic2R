//
// m_beast_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_beast_anim.h"
#include "m_beast_shared.h"
#include "g_monster.h"
#include "g_local.h"

// TB bite way up.
static const animframe_t tbeast_frames_biteup2[] =
{
	{ FRAME_atkc1, NULL, 0, 0, 0, ai_charge2, 0, tbeast_growl },
	{ FRAME_atkc2, NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atkc3, tbeast_check_snatch, TB_HIBITE_F - 32, TB_HIBITE_R, TB_HIBITE_U + 128, NULL, 0, NULL },
	{ FRAME_atkc4, tbeast_bite, TB_HIBITE_F - 32, TB_HIBITE_R, TB_HIBITE_U + 128, NULL, 0, NULL },
	{ FRAME_atkc5, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atkc6, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atkc7, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atkc8, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_biteup2 = ANIMMOVE(tbeast_frames_biteup2, tbeast_pause);

// TB bite up.
static const animframe_t tbeast_frames_biteup[] =
{
	{ FRAME_atka1, NULL, 0, 0, 0, ai_charge2, 0, tbeast_growl },
	{ FRAME_atka2, NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atka3, tbeast_check_snatch, TB_HIBITE_F, TB_HIBITE_R, TB_HIBITE_U, NULL, 0, NULL },
	{ FRAME_atka4, tbeast_bite, TB_HIBITE_F, TB_HIBITE_R, TB_HIBITE_U, NULL, 0, NULL },
	{ FRAME_atka5, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atka6, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atka7, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atka8, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atka9, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_biteup = ANIMMOVE(tbeast_frames_biteup, tbeast_pause);

// TB bite low.
static const animframe_t tbeast_frames_bitelow[] =
{
	{ FRAME_atkb1,	NULL, 0, 0, 0, ai_charge2, 0, tbeast_growl },
	{ FRAME_atkb2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atkb3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atkb4,	tbeast_check_snatch, TB_LOBITE_F, TB_LOBITE_R, TB_LOBITE_U, NULL, 0, NULL },
	{ FRAME_atkb5,	tbeast_bite, TB_LOBITE_F, TB_LOBITE_R, TB_LOBITE_U, NULL, 0, NULL },
	{ FRAME_atkb6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atkb7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atkb8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atkb9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atkb10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_atkb11,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_bitelow = ANIMMOVE(tbeast_frames_bitelow, tbeast_pause);

// TB eating twitch?
static const animframe_t tbeast_frames_eating_twitch[] =
{
	{ FRAME_eatingb1, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb2, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb3, NULL, 0, 0, 0, NULL, 0, tbeast_snort },
	{ FRAME_eatingb4, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb5, NULL, 0, 0, 0, NULL, 0, tbeast_gibs },
	{ FRAME_eatingb6, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb7, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb8, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_eating_twitch = ANIMMOVE(tbeast_frames_eating_twitch, tbeast_eat_order);

// TB eating.
static const animframe_t tbeast_frames_eating[] =
{
	{ FRAME_eatingc1, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingc2, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingc3, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingc4, NULL, 0, 0, 0, NULL, 0, tbeast_gibs },
	{ FRAME_eatingc5, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingc6, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_eating = ANIMMOVE(tbeast_frames_eating, tbeast_eat_order);

// TB bending down, eating.
static const animframe_t tbeast_frames_eatdown[] =
{
	{ FRAME_eatran1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran4,	NULL, 0, 0, 0, NULL, 0, tbeast_snort },
	{ FRAME_eatran5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatran12,	NULL, 0, 0, 0, NULL, 0, tbeast_gibs },
	{ FRAME_eatran13,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_eatdown = ANIMMOVE(tbeast_frames_eatdown, tbeast_eat_order);

// TB walking.
static const animframe_t tbeast_frames_walk[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, tbeast_run, 32, tbeast_footstep },
	{ FRAME_walk2,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, tbeast_run, 32, NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, tbeast_run, 24, NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, tbeast_run, 20, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_walk7,	NULL, 0, 0, 0, tbeast_run, 20, tbeast_growl },
	{ FRAME_walk8,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_walk9,	NULL, 0, 0, 0, tbeast_run, 20, NULL },
	{ FRAME_walk10,	NULL, 0, 0, 0, tbeast_run, 32, NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, tbeast_run, 28, tbeast_footstep },
	{ FRAME_walk12,	NULL, 0, 0, 0, tbeast_run, 12, NULL },
	{ FRAME_walk13,	NULL, 0, 0, 0, tbeast_run, 16, tbeast_snort },
	{ FRAME_walk14,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_walk15,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_walk16,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_walk17,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_walk18,	NULL, 0, 0, 0, tbeast_run, 24, NULL },
};
const animmove_t tbeast_move_walk = ANIMMOVE(tbeast_frames_walk, tbeast_walk_order);

// TB turning left while walking.
static const animframe_t tbeast_frames_walkleft[] =
{
	{ FRAME_wlklft1,	NULL, 0, 0, 0, tbeast_run, 32, tbeast_footstep },
	{ FRAME_wlklft2,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlklft3,	NULL, 0, 0, 0, tbeast_run, 32, NULL },
	{ FRAME_wlklft4,	NULL, 0, 0, 0, tbeast_run, 24, NULL },
	{ FRAME_wlklft5,	NULL, 0, 0, 0, tbeast_run, 20, tbeast_snort },
	{ FRAME_wlklft6,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlklft7,	NULL, 0, 0, 0, tbeast_run, 20, NULL },
	{ FRAME_wlklft8,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlklft9,	NULL, 0, 0, 0, tbeast_run, 20, NULL },
	{ FRAME_wlklft10,	NULL, 0, 0, 0, tbeast_run, 32, NULL },
	{ FRAME_wlklft11,	NULL, 0, 0, 0, tbeast_run, 28, tbeast_footstep },
	{ FRAME_wlklft12,	NULL, 0, 0, 0, tbeast_run, 12, NULL },
	{ FRAME_wlklft13,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlklft14,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlklft15,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlklft16,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlklft17,	NULL, 0, 0, 0, tbeast_run, 16, tbeast_growl },
	{ FRAME_wlklft18,	NULL, 0, 0, 0, tbeast_run, 24, NULL },
};
const animmove_t tbeast_move_walkleft = ANIMMOVE(tbeast_frames_walkleft, tbeast_walk_order);

// TB turning right while walking.
static const animframe_t tbeast_frames_walkrt[] =
{
	{ FRAME_wlkrt1,		NULL, 0, 0, 0, tbeast_run, 32, tbeast_footstep },
	{ FRAME_wlkrt2,		NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlkrt3,		NULL, 0, 0, 0, tbeast_run, 32, NULL },
	{ FRAME_wlkrt4,		NULL, 0, 0, 0, tbeast_run, 24, NULL },
	{ FRAME_wlkrt5,		NULL, 0, 0, 0, tbeast_run, 20, NULL },
	{ FRAME_wlkrt6,		NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlkrt7,		NULL, 0, 0, 0, tbeast_run, 20, NULL },
	{ FRAME_wlkrt8,		NULL, 0, 0, 0, tbeast_run, 16, tbeast_growl },
	{ FRAME_wlkrt9,		NULL, 0, 0, 0, tbeast_run, 20, NULL },
	{ FRAME_wlkrt10,	NULL, 0, 0, 0, tbeast_run, 32, NULL },
	{ FRAME_wlkrt11,	NULL, 0, 0, 0, tbeast_run, 28, tbeast_footstep },
	{ FRAME_wlkrt12,	NULL, 0, 0, 0, tbeast_run, 12, NULL },
	{ FRAME_wlkrt13,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlkrt14,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlkrt15,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlkrt16,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlkrt17,	NULL, 0, 0, 0, tbeast_run, 16, NULL },
	{ FRAME_wlkrt18,	NULL, 0, 0, 0, tbeast_run, 24, tbeast_snort },
};
const animmove_t tbeast_move_walkrt = ANIMMOVE(tbeast_frames_walkrt, tbeast_walk_order);

// TB while jumping.
static const animframe_t tbeast_frames_inair[] =
{
	{ FRAME_jumpb16,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
};
const animmove_t tbeast_move_inair = ANIMMOVE(tbeast_frames_inair, NULL);

// TB land.
static const animframe_t tbeast_frames_land[] =
{
	{ FRAME_jumpb17,	NULL, 0, 0, 0, NULL, 0, tbeast_land },
	{ FRAME_jumpb18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb19,	NULL, 0, 0, 0, NULL, 0, tbeast_snort },
	{ FRAME_jumpb20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb23,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_land = ANIMMOVE(tbeast_frames_land, tbeast_pause);

// TB jump.
static const animframe_t tbeast_frames_jump[] =
{
	{ FRAME_jumpb1,		NULL, 0, 0, 0, NULL, 0, tbeast_growl },
	{ FRAME_jumpb2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb7,		tbeast_leap, 250, 0, 400, NULL, 0, NULL },
	{ FRAME_jumpb8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb11,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
	{ FRAME_jumpb12,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
	{ FRAME_jumpb13,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
	{ FRAME_jumpb14,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
	{ FRAME_jumpb15, 	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
};
const animmove_t tbeast_move_jump = ANIMMOVE(tbeast_frames_jump, tbeast_inair);

// TB forced jump.
static const animframe_t tbeast_frames_forced_jump[] =
{
	{ FRAME_jumpb1,		NULL, 0, 0, 0, NULL, 0, tbeast_growl },
	{ FRAME_jumpb2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb7,		NULL, 0, 0, 0, NULL, 0, tbeast_apply_jump },
	{ FRAME_jumpb8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb11,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
	{ FRAME_jumpb12,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
	{ FRAME_jumpb13,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
	{ FRAME_jumpb14,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
	{ FRAME_jumpb15,	NULL, 0, 0, 0, NULL, 0, tbeast_check_landed },
};
const animmove_t tbeast_move_forced_jump = ANIMMOVE(tbeast_frames_forced_jump, tbeast_inair);

// TB standing.
static const animframe_t tbeast_frames_stand[] =
{
	{ FRAME_wait1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait3,	NULL, 0, 0, 0, ai_stand, 0, tbeast_snort },
	{ FRAME_wait4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait9,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait12,	NULL, 0, 0, 0, ai_stand, 0, tbeast_snort },
	{ FRAME_wait13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t tbeast_move_stand = ANIMMOVE(tbeast_frames_stand, tbeast_stand_order);

// TB waiting.
static const animframe_t tbeast_frames_delay[] =
{
	{ FRAME_wait1,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait2,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait3,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait4,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait5,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait6,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait7,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait8,	NULL, 0, 0, 0, NULL, 0, tbeast_snort },
	{ FRAME_wait9,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait10,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait11,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait12,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait13,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
	{ FRAME_wait14,	NULL, 0, 0, 0, NULL, 0, tbeast_check_mood },
};
const animmove_t tbeast_move_delay = ANIMMOVE(tbeast_frames_delay, tbeast_pause);

// TB dying.
static const animframe_t tbeast_frames_die[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death6,		NULL, 0, 0, 0, NULL, 0, NULL },
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
	{ FRAME_death17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death20,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_die = ANIMMOVE(tbeast_frames_die, tbeast_dead);

// TB dying (normal).
static const animframe_t tbeast_frames_die_norm[] =
{
	{ FRAME_deatha1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha31,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_die_norm = ANIMMOVE(tbeast_frames_die_norm, tbeast_dead);

// TB charging.
static const animframe_t tbeast_frames_charge[] =
{
	{ FRAME_roar1,		NULL, 0, 0, 0, NULL, 0, tbeast_roar_short },
	{ FRAME_roar3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar11,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar13,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar15,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar17,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar19,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar21,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar23,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar25,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar27,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar29,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar31,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar33,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar35,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar37,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar39,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar41,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar43,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar45,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar47,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar49,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_footstep },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 32, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 24, tbeast_growl },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 32, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 28, tbeast_footstep },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 12, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 16, tbeast_growl },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 24, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_footstep },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 32, NULL },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 24, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 20, tbeast_growl },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_growl },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 28, tbeast_footstep },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 12, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 16, tbeast_snort },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 24, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_footstep },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 32, NULL },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 24, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_growl },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 28, tbeast_footstep },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 12, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 24, tbeast_walk_order },
};
const animmove_t tbeast_move_charge = ANIMMOVE(tbeast_frames_charge, tbeast_walk_order);

// TB roar.
static const animframe_t tbeast_frames_roar[] =
{
	{ FRAME_roar1,	NULL, 0, 0, 0, NULL, 0, tbeast_roar },
	{ FRAME_roar2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar10,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar11,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar12,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar14,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar15,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar17,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar18,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar20,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar23,	NULL, 0, 0, 0, NULL, 0, tbeast_roar_knockdown },
	{ FRAME_roar24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar33,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar34,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar35,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar36,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar37,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar38,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar39,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar40,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar41,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar42,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar43,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar44,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar45,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar46,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar47,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar48,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar49,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar50,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_roar = ANIMMOVE(tbeast_frames_roar, tbeast_pause);

// TB walking attack.
static const animframe_t tbeast_frames_walkatk[] =
{
	{ FRAME_wlkatk1,	NULL, 0, 0, 0, tbeast_run, 36, tbeast_footstep },
	{ FRAME_wlkatk2,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk3,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk4,	tbeast_bite, TB_WLKBITE_F, -4, TB_WLKBITE_U, tbeast_run, 36, NULL },
	{ FRAME_wlkatk5,	NULL, 0, 0, 0, tbeast_run, 36, tbeast_growl },
	{ FRAME_wlkatk6,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk7,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk8,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk9,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk10,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk11,	NULL, 0, 0, 0, tbeast_run, 36, tbeast_footstep }, // Bite.
	{ FRAME_wlkatk12,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk13,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk14,	tbeast_bite, TB_WLKBITE_F, 16, TB_WLKBITE_U, tbeast_run, 36, NULL },
	{ FRAME_wlkatk15,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk16,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
	{ FRAME_wlkatk17,	NULL, 0, 0, 0, tbeast_run, 36, tbeast_growl },
	{ FRAME_wlkatk18,	NULL, 0, 0, 0, tbeast_run, 36, NULL },
};
const animmove_t tbeast_move_walkatk = ANIMMOVE(tbeast_frames_walkatk, tbeast_pause);

// TB stunned.
static const animframe_t tbeast_frames_stun[] =
{
	//FIXME: don't walk back so long
	{ FRAME_walk18,	NULL, 0, 0, 0, ai_move, -40, NULL },
	{ FRAME_walk16,	NULL, 0, 0, 0, ai_move, -32, NULL },
	{ FRAME_walk14,	NULL, 0, 0, 0, ai_move, -32, tbeast_snort },
	{ FRAME_walk12,	NULL, 0, 0, 0, ai_move, -12, NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, ai_move, -28, tbeast_footstep },
	{ FRAME_walk10,	NULL, 0, 0, 0, ai_move, -52, NULL },
	{ FRAME_walk8,	NULL, 0, 0, 0, ai_move, -36, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, ai_move, -36, NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, ai_move, -24, tbeast_snort },
	{ FRAME_stun1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun3,	NULL, 0, 0, 0, NULL, 0, tbeast_footstep },
	{ FRAME_stun4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_stun19,	NULL, 0, 0, 0, NULL, 0, tbeast_snort },
};
const animmove_t tbeast_move_stun = ANIMMOVE(tbeast_frames_stun, tbeast_pause);

// TB bite way up success finish
static const animframe_t tbeast_frames_biteup2_sfin[] =
{
	{ FRAME_atkc4, tbeast_shake_toy, 200,  0,  132, NULL, 0, tbeast_anger_sound },
	{ FRAME_atkc5, tbeast_shake_toy, 176, -4,  96,  NULL, 0, NULL },
	{ FRAME_atkc6, tbeast_shake_toy, 160, -12, 80,  NULL, 0, NULL },
	{ FRAME_atkc7, tbeast_shake_toy, 152, -4,  72,  NULL, 0, NULL },
};
const animmove_t tbeast_move_biteup2_sfin = ANIMMOVE(tbeast_frames_biteup2_sfin, tbeast_snatch_go);

// TB bite up success finish.
static const animframe_t tbeast_frames_biteup_sfin[] =
{
	{ FRAME_atka4, tbeast_shake_toy, 200,  0,  132, NULL, 0, tbeast_anger_sound },
	{ FRAME_atka5, tbeast_shake_toy, 176, -4,  96,  NULL, 0, NULL },
	{ FRAME_atka6, tbeast_shake_toy, 160, -12, 80,  NULL, 0, NULL },
	{ FRAME_atka7, tbeast_shake_toy, 152, -4,  72,  NULL, 0, NULL },
};
const animmove_t tbeast_move_biteup_sfin = ANIMMOVE(tbeast_frames_biteup_sfin, tbeast_snatch_go);

// TB bite low success finish.
static const animframe_t tbeast_frames_bitelow_sfin[] =
{
	{ FRAME_atkb5,	tbeast_shake_toy, 216, 0,  64, NULL, 0, NULL },
	{ FRAME_atkb6,	tbeast_shake_toy, 208, 0,  62, NULL, 0, tbeast_anger_sound },
	{ FRAME_atkb7,	tbeast_shake_toy, 196, 0,  56, NULL, 0, NULL },
	{ FRAME_atkb8,	tbeast_shake_toy, 184, 0,  54, NULL, 0, NULL },
	{ FRAME_atkb9,	tbeast_shake_toy, 176, 2,  60, NULL, 0, tbeast_anger_sound },
	{ FRAME_atkb10,	tbeast_shake_toy, 178, 10, 64, NULL, 0, NULL },
};
const animmove_t tbeast_move_bitelow_sfin = ANIMMOVE(tbeast_frames_bitelow_sfin, tbeast_snatch_go);

// TB snatch throw & catch.
static const animframe_t tbeast_frames_snatch[] =
{
	{ FRAME_eatinga6,	tbeast_shake_toy, 164, -16,  94,  NULL, 0,  NULL },
	{ FRAME_eatinga7,	tbeast_shake_toy, 152, -68,  60,  NULL, 0,  NULL },
	{ FRAME_eatinga8,	tbeast_shake_toy, 112, -88,  56,  NULL, 0,  tbeast_anger_sound },
	{ FRAME_eatinga9,	tbeast_shake_toy, 104, -80,  64,  NULL, 0,  NULL },
	{ FRAME_eatinga10,	tbeast_shake_toy, 128, -64,  80,  NULL, 0,  NULL },
	{ FRAME_eatinga11,	tbeast_shake_toy, 160, -16,  84,  NULL, 0,  NULL },
	{ FRAME_eatinga12,	tbeast_shake_toy, 152,  104, 112, NULL, 0,  tbeast_anger_sound },
	{ FRAME_eatinga13,	tbeast_shake_toy, 128,  116, 128, NULL, 0,  NULL },
	{ FRAME_eatinga14,	tbeast_shake_toy, 136,  124, 132, NULL, 0,  NULL },
	{ FRAME_eatinga15,	tbeast_shake_toy, 160,  48,  124, NULL, 0,  NULL },
	{ FRAME_eatinga16,	tbeast_shake_toy, 168, -56,  98,  NULL, 0,  tbeast_anger_sound },
	{ FRAME_eatinga17,	tbeast_shake_toy, 136, -100, 56,  NULL, 0,  NULL },
	{ FRAME_eatinga18,	tbeast_shake_toy, 104, -96,  52,  NULL, 0,  NULL },
	{ FRAME_eatinga19,	tbeast_shake_toy, 88,  -92,  49,  NULL, 0,  NULL },
	{ FRAME_eatinga20,	tbeast_shake_toy, 96,  -88,  64,  NULL, 0,  tbeast_anger_sound },
	{ FRAME_eatinga21,	tbeast_shake_toy, 126, -80,  64,  NULL, 0,  NULL },
	{ FRAME_eatinga22,	tbeast_shake_toy, 142, -56,  53,  NULL, 0,  NULL },
	{ FRAME_eatinga23,	tbeast_shake_toy, 146, -18,  34,  NULL, 0,  tbeast_anger_sound },
	{ FRAME_eatinga24,	tbeast_shake_toy, 136,  24,  16,  NULL, 0,  NULL },
	{ FRAME_eatinga25,	tbeast_shake_toy, 120,  48, -8,   NULL, 0,  NULL },
	{ FRAME_jumpb21,	tbeast_shake_toy, 96,  -32,  8,   NULL, 0, tbeast_anger_sound },
	{ FRAME_jumpb20,	tbeast_shake_toy, 128, -34,  12,  NULL, 0, tbeast_anger_sound },
	{ FRAME_jumpb19,	tbeast_shake_toy, 164,  0,   128, NULL, 0, tbeast_anger_sound },
	{ FRAME_jumpb18,	tbeast_shake_toy, 148,  0,   216, NULL, 0, tbeast_throw_toy },
	{ FRAME_jumpb19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb3,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_snatch = ANIMMOVE(tbeast_frames_snatch, tbeast_ready_catch); //mxd. numframes:29 in original logic.

// TB awaiting catch.
static const animframe_t tbeast_frames_ready_catch[] =
{
	{ FRAME_jumpb4, 	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_ready_catch = ANIMMOVE(tbeast_frames_ready_catch, tbeast_ready_catch);

// TB catching.
static const animframe_t tbeast_frames_catch[] =
{
	{ FRAME_jumpb5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb6,		NULL, 0, 0, 0, tbeast_gore_toy, 150, NULL },
	{ FRAME_jumpb7,		NULL, 0, 0, 0, tbeast_gore_toy, 0, NULL },
	{ FRAME_jumpb8,		NULL, 0, 0, 0, tbeast_gore_toy, 0, NULL },
	{ FRAME_jumpb9,		NULL, 0, 0, 0, tbeast_gore_toy, 0, NULL },
	{ FRAME_jumpb10,	NULL, 0, 0, 0, tbeast_gore_toy, 0, NULL },
	{ FRAME_jumpb11,	NULL, 0, 0, 0, tbeast_gore_toy, -1, NULL },
	{ FRAME_jumpb12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb15, 	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t tbeast_move_catch = ANIMMOVE(tbeast_frames_catch, tbeast_ginair); //mxd. numframes:10 in original logic.

// TB while jumping.
static const animframe_t tbeast_frames_ginair[] =
{
	{ FRAME_jumpb16, 	NULL, 0, 0, 0, NULL, 0, tbeast_gcheck_landed },
};
const animmove_t tbeast_move_ginair = ANIMMOVE(tbeast_frames_ginair, NULL);

// TB landing.
static const animframe_t tbeast_frames_gland[] =
{
	{ FRAME_jumpb17,	NULL, 0, 0, 0, NULL, 0, tbeast_land },
	{ FRAME_jumpb18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb19,	NULL, 0, 0, 0, NULL, 0, tbeast_snort },
	{ FRAME_jumpb20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb23,	NULL, 0, 0, 0, NULL, 0, tbeast_done_gore },
};
const animmove_t tbeast_move_gland = ANIMMOVE(tbeast_frames_gland, tbeast_done_gore);

// TB quick charge.
static const animframe_t tbeast_frames_quick_charge[] =
{
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_footstep },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 32, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 24, tbeast_growl },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 32, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 28, tbeast_footstep },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 12, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 16, tbeast_growl },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 24, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_footstep },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 32, NULL },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 24, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 20, tbeast_growl },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_growl },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 28, tbeast_footstep },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 12, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 16, tbeast_snort },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 24, NULL },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_footstep },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 32, NULL },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 24, NULL },
	{ FRAME_charge1,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge2,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge3,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge4,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge5,	NULL, 0, 0, 0, tbeast_charge, 20, NULL },
	{ FRAME_charge6,	NULL, 0, 0, 0, tbeast_charge, 32, tbeast_growl },
	{ FRAME_charge7,	NULL, 0, 0, 0, tbeast_charge, 28, tbeast_footstep },
	{ FRAME_charge8,	NULL, 0, 0, 0, tbeast_charge, 12, NULL },
	{ FRAME_charge9,	NULL, 0, 0, 0, tbeast_charge, 16, NULL },
	{ FRAME_charge10,	NULL, 0, 0, 0, tbeast_charge, 24, tbeast_walk_order },
};
const animmove_t tbeast_move_quick_charge = ANIMMOVE(tbeast_frames_quick_charge, tbeast_walk_order);
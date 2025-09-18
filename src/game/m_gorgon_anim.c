//
// m_gorgon_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_gorgon_anim.h"
#include "m_gorgon_shared.h"
#include "g_ai.h"

// Gorgon Stand1 - gorgon standing and wagging it's tail.
static const animframe_t gorgon_frames_stand1[] =
{
	{ FRAME_wait1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t gorgon_move_stand1 = ANIMMOVE(gorgon_frames_stand1, gorgon_check_mood);

// Gorgon Stand2 - gorgon standing and looking left.
static const animframe_t gorgon_frames_stand2[] =
{
	{ FRAME_painb1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_painb2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_painb3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_painb4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_painb5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t gorgon_move_stand2 = ANIMMOVE(gorgon_frames_stand2, gorgon_check_mood);

// Gorgon Stand3 - gorgon standing and looking right.
static const animframe_t gorgon_frames_stand3[] =
{
	{ FRAME_painc1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_painc2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_painc3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_painc4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_painc5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t gorgon_move_stand3 = ANIMMOVE(gorgon_frames_stand3, gorgon_check_mood);

// Gorgon Stand4 - gorgon standing and wagging it's tail.
static const animframe_t gorgon_frames_stand4[] =
{
	{ FRAME_wait1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t gorgon_move_stand4 = ANIMMOVE(gorgon_frames_stand4, gorgon_check_mood); //TODO: gorgon_frames_stand1 duplicate. Remove?

// Gorgon walking forward.
static const animframe_t gorgon_frames_walk[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, ai_walk, 7, gorgon_footstep },
	{ FRAME_walk2,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, ai_walk, 6, gorgon_footstep },
	{ FRAME_walk7,	NULL, 0, 0, 0, ai_walk, 7, NULL },
	{ FRAME_walk8,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk9,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk10,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk12,	NULL, 0, 0, 0, ai_walk, 6, gorgon_growl },
};
const animmove_t gorgon_move_walk = ANIMMOVE(gorgon_frames_walk, gorgon_check_mood);

// Gorgon turning left while walking.
static const animframe_t gorgon_frames_walk2[] =
{
	{ FRAME_wlklft1,	NULL, 0, 0, 0, ai_walk, 8, gorgon_footstep },
	{ FRAME_wlklft2,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft3,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft4,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft5,	NULL, 0, 0, 0, ai_walk, 8, gorgon_footstep },
	{ FRAME_wlklft6,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft7,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft8,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft9,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft10,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft11,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlklft12,	NULL, 0, 0, 0, ai_walk, 8, NULL },
};
const animmove_t gorgon_move_walk2 = ANIMMOVE(gorgon_frames_walk2, gorgon_check_mood);

// Gorgon turning right while walking.
static const animframe_t gorgon_frames_walk3[] =
{
	{ FRAME_wlkrt1,		NULL, 0, 0, 0, ai_walk, 8, gorgon_footstep },
	{ FRAME_wlkrt2,		NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt3,		NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt4,		NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt5,		NULL, 0, 0, 0, ai_walk, 8, gorgon_footstep },
	{ FRAME_wlkrt6,		NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt7,		NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt8,		NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt9,		NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt10,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt11,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_wlkrt12,	NULL, 0, 0, 0, ai_walk, 8, NULL },
};
const animmove_t gorgon_move_walk3 = ANIMMOVE(gorgon_frames_walk3, gorgon_check_mood);

// Forced Jump - jump from a buoy.
static const animframe_t gorgon_frames_fjump[] =
{
	{ FRAME_jumpb1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb3,		NULL, 0, 0, 0, NULL, 0, gorgon_growl },
	{ FRAME_jumpb4,		NULL, 0, 0, 0, NULL, 0, gorgon_apply_jump },
	{ FRAME_jumpb5,		NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb6,		NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb7,		NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb8,		NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb9,		NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb10,	NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb11,	NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb12,	NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb13,	NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
	{ FRAME_jumpb14,	NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
};
const animmove_t gorgon_move_fjump = ANIMMOVE(gorgon_frames_fjump, gorgon_inair_go);

// Gorgon Land 1.
static const animframe_t gorgon_frames_land[] =
{
	{ FRAME_jumpa15,	NULL, 0, 0, 0, NULL, 0, gorgon_land },
	{ FRAME_jumpa16,	NULL, 0, 0, 0, NULL, 0, gorgon_growl },
	{ FRAME_jumpa17,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_land = ANIMMOVE(gorgon_frames_land, gorgon_check_mood);

// Gorgon Land 2.
static const animframe_t gorgon_frames_land2[] =
{
	{ FRAME_jumpa15,	NULL, 0, 0, 0, NULL, 0, gorgon_land },
	{ FRAME_jumpa16,	NULL, 0, 0, 0, NULL, 0, gorgon_growl },
	{ FRAME_jumpa17,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_land2 = ANIMMOVE(gorgon_frames_land2, gorgon_check_mood);

// Gorgon In air.
static const animframe_t gorgon_frames_inair[] =
{
	{ FRAME_jumpa14,	NULL, 0, 0, 0, NULL, 0, gorgon_check_landed },
};
const animmove_t gorgon_move_inair = ANIMMOVE(gorgon_frames_inair, NULL);

// Gorgon Melee1 - gorgon attack left.
static const animframe_t gorgon_frames_melee1[] =
{
	{ FRAME_atka1, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL },
	{ FRAME_atka2, NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_bite },
	{ FRAME_atka3, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL },
	{ FRAME_atka4, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL },
};
const animmove_t gorgon_move_melee1 = ANIMMOVE(gorgon_frames_melee1, gorgon_check_mood);

// Gorgon Melee2 - gorgon attack right.
static const animframe_t gorgon_frames_melee2[] =
{
	{ FRAME_atkb1,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_atkb2,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_bite },
	{ FRAME_atkb3,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_atkb4,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
};
const animmove_t gorgon_move_melee2 = ANIMMOVE(gorgon_frames_melee2, gorgon_check_mood);

// Gorgon Melee3 - gorgon attack up.
static const animframe_t gorgon_frames_melee3[] =
{
	{ FRAME_atkd1,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_atkd2,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_atkd3,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_bite },
	{ FRAME_atkd4,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
};
const animmove_t gorgon_move_melee3 = ANIMMOVE(gorgon_frames_melee3, gorgon_check_mood);

// Gorgon Melee4 - gorgon attack pullback.
static const animframe_t gorgon_frames_melee4[] =
{
	{ FRAME_atkc1,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_atkc2,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_atkc3,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_atkc4,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
};
const animmove_t gorgon_move_melee4 = ANIMMOVE(gorgon_frames_melee4, gorgon_check_mood);

// Gorgon Melee5 - running attack.
static const animframe_t gorgon_frames_melee5[] =
{
	{ FRAME_runatk1,	NULL, 0, 0, 0, ai_goal_charge, 20, gorgon_melee5check },
	{ FRAME_runatk2,	NULL, 0, 0, 0, ai_goal_charge, 20, NULL },
	{ FRAME_runatk3,	NULL, 0, 0, 0, ai_goal_charge, 22, NULL },
	{ FRAME_runatk4,	NULL, 0, 0, 0, ai_goal_charge, 21, gorgon_bite },
	{ FRAME_runatk5,	NULL, 0, 0, 0, ai_goal_charge, 20, gorgon_melee5check },
	{ FRAME_runatk6,	NULL, 0, 0, 0, ai_goal_charge, 20, NULL },
	{ FRAME_runatk7,	NULL, 0, 0, 0, ai_goal_charge, 22, NULL },
	{ FRAME_runatk8,	NULL, 0, 0, 0, ai_goal_charge, 21, gorgon_check_mood },
};
const animmove_t gorgon_move_melee5 = ANIMMOVE(gorgon_frames_melee5, gorgon_check_mood);

// Gorgon Melee6 - hop left.
static const animframe_t gorgon_frames_melee6[] =
{
	{ FRAME_hop1,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop2,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop3,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop4,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_hop },
	{ FRAME_hop5,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop6,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop7,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop8,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_hop },
	{ FRAME_hop9,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop10,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
};
const animmove_t gorgon_move_melee6 = ANIMMOVE(gorgon_frames_melee6, gorgon_check_mood);

// Gorgon Melee7 - hop right.
static const animframe_t gorgon_frames_melee7[] =
{
	{ FRAME_hop1,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop2,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop3,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop4,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_hop },
	{ FRAME_hop5,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop6,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop7,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop8,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_hop },
	{ FRAME_hop9,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop10,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
};
const animmove_t gorgon_move_melee7 = ANIMMOVE(gorgon_frames_melee7, gorgon_check_mood); //TODO: gorgon_frames_melee6 duplicate.

// Gorgon Melee8 - hop forwards.
static const animframe_t gorgon_frames_melee8[] =
{
	{ FRAME_hop1,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop2,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop3,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop4,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_hop },
	{ FRAME_hop5,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop6,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop7,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop8,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_hop },
	{ FRAME_hop9,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop10,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
};
const animmove_t gorgon_move_melee8 = ANIMMOVE(gorgon_frames_melee8, gorgon_check_mood); //TODO: gorgon_frames_melee6 duplicate.

// Gorgon Melee9 - hop backwards.
static const animframe_t gorgon_frames_melee9[] =
{
	{ FRAME_hop1,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop2,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop3,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop4,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_hop },
	{ FRAME_hop5,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop6,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop7,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop8,	NULL, 0, 0, 0, ai_goal_charge, 0, gorgon_hop },
	{ FRAME_hop9,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
	{ FRAME_hop10,	NULL, 0, 0, 0, ai_goal_charge, 0, NULL },
};
const animmove_t gorgon_move_melee9 = ANIMMOVE(gorgon_frames_melee9, gorgon_check_mood); //TODO: gorgon_frames_melee6 duplicate.

// Gorgon Melee10 - jump up at player.
static const animframe_t gorgon_frames_melee10[] =
{
	{ FRAME_jumpa1,		NULL, 0, 0, 0, gorgon_ai_charge2,	0, NULL },
	{ FRAME_jumpa2,		NULL, 0, 0, 0, gorgon_ai_charge2,	0, NULL },
	{ FRAME_jumpa3,		NULL, 0, 0, 0, gorgon_ai_charge2,	0, gorgon_growl },
	{ FRAME_jumpa4,		NULL, 0, 0, 0, NULL,				0, gorgon_jump },
	{ FRAME_jumpa5,		NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa6,		NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa7,		NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa8,		NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa9,		NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa10,	NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa11,	NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa12,	NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa13,	NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
	{ FRAME_jumpa14,	NULL, 0, 0, 0, NULL,				0, gorgon_check_landed },
};
const animmove_t gorgon_move_melee10 = ANIMMOVE(gorgon_frames_melee10, gorgon_inair_go);

// Gorgon Run1 - gorgon running.
static const animframe_t gorgon_frames_run1[] =
{
	{ FRAME_run1,	NULL, 0, 0, 0, gorgon_ai_run, 30, gorgon_footstep },
	{ FRAME_run2,	NULL, 0, 0, 0, gorgon_ai_run, 31, gorgon_check_mood },
	{ FRAME_run3,	NULL, 0, 0, 0, gorgon_ai_run, 32, gorgon_check_mood },
	{ FRAME_run4,	NULL, 0, 0, 0, gorgon_ai_run, 34, gorgon_check_mood },
	{ FRAME_run5,	NULL, 0, 0, 0, gorgon_ai_run, 30, gorgon_footstep },
	{ FRAME_run6,	NULL, 0, 0, 0, gorgon_ai_run, 31, gorgon_check_mood },
	{ FRAME_run7,	NULL, 0, 0, 0, gorgon_ai_run, 32, gorgon_check_mood },
	{ FRAME_run8,	NULL, 0, 0, 0, gorgon_ai_run, 34, gorgon_growl },
};
const animmove_t gorgon_move_run1 = ANIMMOVE(gorgon_frames_run1, gorgon_check_mood);

// Gorgon Run2 - turning left while running.
static const animframe_t gorgon_frames_run2[] =
{
	{ FRAME_wlklft1,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_footstep },
	{ FRAME_wlklft2,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft3,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft4,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft5,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_footstep },
	{ FRAME_wlklft6,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft7,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft8,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft9,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft10,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft11,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_mood },
	{ FRAME_wlklft12,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_growl },
};
const animmove_t gorgon_move_run2 = ANIMMOVE(gorgon_frames_run2, gorgon_check_mood);

// Gorgon Run3 - turning right while running.
static const animframe_t gorgon_frames_run3[] =
{
	{ FRAME_wlkrt1,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_footstep },
	{ FRAME_wlkrt2,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt3,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt4,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt5,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt6,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt7,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt8,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt9,		NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt10,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt11,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_check_slip },
	{ FRAME_wlkrt12,	NULL, 0, 0, 0, gorgon_ai_run, 16, gorgon_growl },
};
const animmove_t gorgon_move_run3 = ANIMMOVE(gorgon_frames_run3, gorgon_check_mood);

// Gorgon Pain1 - step back while bending head down.
static const animframe_t gorgon_frames_pain1[] =
{
	{ FRAME_pain1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain9,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_pain1 = ANIMMOVE(gorgon_frames_pain1, gorgon_check_mood);

// Gorgon Pain2 - bend head to the left.
static const animframe_t gorgon_frames_pain2[] =
{
	{ FRAME_painb1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painb2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painb3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painb4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painb5,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_pain2 = ANIMMOVE(gorgon_frames_pain2, gorgon_check_mood);

// Gorgon Pain3 - bend head to the right.
static const animframe_t gorgon_frames_pain3[] =
{
	{ FRAME_painc1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painc2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painc3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painc4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painc5,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_pain3 = ANIMMOVE(gorgon_frames_pain3, gorgon_check_mood);

// Gorgon Die1.
static const animframe_t gorgon_frames_die1[] =
{
	{ FRAME_deatha1,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha2,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha3,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha4,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha5,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha6,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha7,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha8,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha9,	NULL, 0, 0, 0, ai_move,	-8, NULL },
	{ FRAME_deatha10,	NULL, 0, 0, 0, NULL,	 0, gorgon_death1_fall },
	{ FRAME_deatha11,	NULL, 0, 0, 0, NULL,	 0, gorgon_death1_fall },
	{ FRAME_deatha12,	NULL, 0, 0, 0, NULL,	 0, gorgon_death1_fall },
	{ FRAME_deatha13,	NULL, 0, 0, 0, NULL,	 0, gorgon_death1_fall },
	{ FRAME_deatha14,	NULL, 0, 0, 0, NULL,	 0, gorgon_death1_fall },
	{ FRAME_deatha15,	NULL, 0, 0, 0, ai_move,	 0, NULL },
	{ FRAME_deatha16,	NULL, 0, 0, 0, ai_move,	 0, NULL },
	{ FRAME_deatha17,	NULL, 0, 0, 0, ai_move,	 0, NULL },
	{ FRAME_deatha18,	NULL, 0, 0, 0, ai_move,	 0, NULL },
	{ FRAME_deatha19,	NULL, 0, 0, 0, ai_move,	 0, NULL },
};
const animmove_t gorgon_move_die1 = ANIMMOVE(gorgon_frames_die1, gorgon_dead);

// Gorgon Die2.
static const animframe_t gorgon_frames_die2[] =
{
	{ FRAME_hit1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hit3,	NULL, 0, 0, 0, NULL, 0, gorgon_death2_throw },
	{ FRAME_hit5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hit7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hit9,	NULL, 0, 0, 0, NULL, 0, gorgon_death2 },
	{ FRAME_hit11,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_die2 = ANIMMOVE(gorgon_frames_die2, gorgon_death2_slide);

// Gorgon Death2 Twitch - fly backwards and twitch.
static const animframe_t gorgon_frames_death2twitch[] =
{
	{ FRAME_twitch,		NULL, 0, 0, 0, NULL, 0, gorgon_start_twitch },
	{ FRAME_twitch_1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_twitch_2,	NULL, 0, 0, 0, NULL, 0, gorgon_next_twitch },
};
const animmove_t gorgon_move_death2twitch = ANIMMOVE(gorgon_frames_death2twitch, NULL);

// Gorgon Death2 Slide.
static const animframe_t gorgon_frames_death2slide[] =
{
	{ FRAME_slide1,		NULL, 0, 0, 0, ai_move, -16, NULL },
	{ FRAME_slide2,		NULL, 0, 0, 0, ai_move, -14, NULL },
	{ FRAME_slide3,		NULL, 0, 0, 0, ai_move, -12, NULL },
	{ FRAME_slide4,		NULL, 0, 0, 0, ai_move, -8,  NULL },
	{ FRAME_slide5,		NULL, 0, 0, 0, ai_move, -8,  NULL },
	{ FRAME_slide6,		NULL, 0, 0, 0, ai_move, -8,  NULL },
	{ FRAME_slide7,		NULL, 0, 0, 0, ai_move, -8,  NULL },
	{ FRAME_slide8,		NULL, 0, 0, 0, ai_move, -8,  NULL },
	{ FRAME_slide9,		NULL, 0, 0, 0, ai_move, -8,  NULL },
	{ FRAME_slide10,	NULL, 0, 0, 0, ai_move, -8,  NULL },
	{ FRAME_slide11,	NULL, 0, 0, 0, ai_move, -4,  NULL },
	{ FRAME_slide12,	NULL, 0, 0, 0, ai_move, -2,  NULL },
	{ FRAME_slide13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide15,	NULL, 0, 0, 0, ai_moveright, -2, NULL },
	{ FRAME_slide16,	NULL, 0, 0, 0, ai_moveright, -4, NULL },
	{ FRAME_slide17,	NULL, 0, 0, 0, ai_moveright, -2, NULL },
	{ FRAME_slide18,	NULL, 0, 0, 0, ai_moveright, -6, NULL },
	{ FRAME_slide19,	NULL, 0, 0, 0, ai_moveright, -2, NULL },
	{ FRAME_slide20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slide31,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_death2slide = ANIMMOVE(gorgon_frames_death2slide, gorgon_dead);

// Gorgon Catch Toy.
static const animframe_t gorgon_frames_catch[] =
{
	{ FRAME_jumpb5,		NULL, 0, 0, 0, gorgon_gore_toy,  150,	NULL },
	{ FRAME_jumpb6,		NULL, 0, 0, 0, gorgon_gore_toy,  0,		NULL },
	{ FRAME_jumpb7,		NULL, 0, 0, 0, gorgon_gore_toy,  0,		NULL },
	{ FRAME_jumpb8,		NULL, 0, 0, 0, gorgon_gore_toy,  0,		NULL },
	{ FRAME_jumpb9,		NULL, 0, 0, 0, gorgon_gore_toy,  0,		NULL },
	{ FRAME_jumpb10,	NULL, 0, 0, 0, gorgon_gore_toy, -1,		NULL },
	{ FRAME_jumpb11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb19,	NULL, 0, 0, 0, NULL, 0, gorgon_anger_sound },
};
const animmove_t gorgon_move_catch = ANIMMOVE(gorgon_frames_catch, gorgon_done_gore);

// Gorgon Miss.
static const animframe_t gorgon_frames_miss[] =
{
	{ FRAME_eatinga4,	NULL, 0, 0, 0, NULL, 0, gorgon_miss_sound },
	{ FRAME_eatinga3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatinga2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatinga1,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_miss = ANIMMOVE(gorgon_frames_miss, gorgon_check_mood);

// Gorgon shake and toss up toy. Chance of throwing it to left or right, maybe carry away?
static const animframe_t gorgon_frames_snatch[] =
{
	{ FRAME_eatingb5,	gorgon_shake_toy, 70,  0,  66,  NULL, 0, gorgon_anger_sound },
	{ FRAME_eatingb6,	gorgon_shake_toy, 68, -44, 66,  NULL, 0, NULL },
	{ FRAME_eatingb7,	gorgon_shake_toy, 56, -64, 66,  NULL, 0, NULL },
	{ FRAME_eatingb8,	gorgon_shake_toy, 60, -56, 66,  NULL, 0, gorgon_anger_sound },
	{ FRAME_eatingb9,	gorgon_shake_toy, 68, -24, 66,  NULL, 0, NULL },
	{ FRAME_eatingb10,	gorgon_shake_toy, 72,  40, 66,  NULL, 0, NULL },
	{ FRAME_eatingb11,	gorgon_shake_toy, 60,  70, 66,  NULL, 0, NULL },
	{ FRAME_eatingb12,	gorgon_shake_toy, 70,  48, 66,  NULL, 0, gorgon_anger_sound },
	{ FRAME_eatingb13,	gorgon_shake_toy, 74, -32, 66,  NULL, 0, NULL },
	{ FRAME_eatingb14,	gorgon_shake_toy, 64, -70, 66,  NULL, 0, NULL },
	{ FRAME_eatingb15,	gorgon_shake_toy, 40, -72, 66,  NULL, 0, NULL },
	{ FRAME_eatingb16,	gorgon_shake_toy, 42, -70, 66,  NULL, 0, gorgon_anger_sound },
	{ FRAME_eatingb17,	gorgon_shake_toy, 60, -60, 62,  NULL, 0, NULL },
	{ FRAME_eatingb18,	gorgon_shake_toy, 66, -40, 56,  NULL, 0, NULL },
	{ FRAME_eatingb19,	gorgon_shake_toy, 72, -10, 32,  NULL, 0, NULL },
	{ FRAME_eatingb20,	gorgon_shake_toy, 62,  16, 12,  NULL, 0, gorgon_anger_sound },
	{ FRAME_eatingb21,	gorgon_shake_toy, 56,  32, 0,   NULL, 0, NULL },
	{ FRAME_eatinga4,	gorgon_shake_toy, 48,  16, 64,  NULL, 0, NULL },
	{ FRAME_eatinga3,	gorgon_shake_toy, 50,  14, 96,  NULL, 0, gorgon_anger_sound },
	{ FRAME_eatinga2,	gorgon_shake_toy, 80,  8,  80,  NULL, 0, NULL },
	{ FRAME_eatinga1,	gorgon_shake_toy, 96,  0,  76,  NULL, 0, NULL },
	{ FRAME_jumpa1,		gorgon_shake_toy, 100, 8,  78,  NULL, 0, gorgon_anger_sound },
	{ FRAME_jumpa2,		gorgon_shake_toy, 90,  6,  64,  NULL, 0, NULL },
	{ FRAME_jumpa3,		gorgon_shake_toy, 76,  4,  48,  NULL, 0, NULL },
	{ FRAME_jumpa4,		gorgon_shake_toy, 96,  6,  140, NULL, 0, gorgon_anger_sound },
	{ FRAME_jumpa5,		gorgon_shake_toy, 90,  6,  208, NULL, 0, gorgon_throw_toy },
	{ FRAME_jumpb4,		NULL,			  0,   0,  0,   NULL, 0, NULL },
};
const animmove_t gorgon_move_snatch = ANIMMOVE(gorgon_frames_snatch, gorgon_ready_catch);

// Gorgon Ready Catch.
static const animframe_t gorgon_frames_readycatch[] =
{
	{ FRAME_jumpb4,		NULL, 0, 0, 0, NULL, 0,  NULL },
};
const animmove_t gorgon_move_readycatch = ANIMMOVE(gorgon_frames_readycatch, gorgon_ready_catch);

// Gorgon Snatch Hi.
static const animframe_t gorgon_frames_snatchhi[] =
{
	{ FRAME_atkd1,	NULL,				 0, 0, 0,		gorgon_ai_charge2, 10,  NULL },
	{ FRAME_atkd2,	NULL,				 0, 0, 0,		gorgon_ai_charge2, 10,  NULL },
	{ FRAME_atkd3,	gorgon_check_snatch, 96, 0, 56,		NULL, 0,  NULL },
	{ FRAME_atkd4,	gorgon_shake_toy,	 96, 16, 160,	NULL, 0,  NULL },
};
const animmove_t gorgon_move_snatchhi = ANIMMOVE(gorgon_frames_snatchhi, gorgon_snatch_go);

// Gorgon Snatch Low.
static const animframe_t gorgon_frames_snatchlow[] =
{
	{ FRAME_eatinga1,	NULL,					0, 0, 0,	gorgon_ai_charge2, 10, NULL },
	{ FRAME_eatinga2,	NULL,					0, 0, 0,	gorgon_ai_charge2, 10, NULL },
	{ FRAME_eatinga3,	NULL,					0, 0, 0,	gorgon_ai_charge2, 10, NULL },
	{ FRAME_eatinga4,	NULL,					0, 0, 0,	gorgon_ai_charge2, 10, NULL },
	{ FRAME_eatinga5,	gorgon_check_snatch,	64, 0, -48,	NULL, 0, NULL },
	{ FRAME_eatingb1,	gorgon_shake_toy,		48, -32, 0,	NULL, 0, gorgon_anger_sound },
	{ FRAME_eatingb2,	gorgon_shake_toy,		48, 10, 10,	NULL, 0, NULL },
	{ FRAME_eatingb3,	gorgon_shake_toy,		48, 0, 20,	NULL, 0, gorgon_anger_sound },
	{ FRAME_eatingb4,	gorgon_shake_toy,		56, 0, 24,	NULL, 0, NULL },
};
const animmove_t gorgon_move_snatchlow = ANIMMOVE(gorgon_frames_snatchlow, gorgon_snatch_go);

// Gorgon Slip.
static const animframe_t gorgon_frames_slip[] =
{
	{ FRAME_deatha1,	NULL, 0, 0, 0, gorgon_slide, -200, NULL },
	{ FRAME_deatha2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deatha10,	NULL, 0, 0, 0, NULL, 0, gorgon_death1_fall },
	{ FRAME_deatha11,	NULL, 0, 0, 0, NULL, 0, gorgon_death1_fall },
	{ FRAME_deatha12,	NULL, 0, 0, 0, NULL, 0, gorgon_death1_fall },
	{ FRAME_deatha13,	NULL, 0, 0, 0, NULL, 0, gorgon_death1_fall },
	{ FRAME_deatha14,	NULL, 0, 0, 0, NULL, 0, gorgon_death1_fall },
	{ FRAME_deatha15,	NULL, 0, 0, 0, gorgon_set_roll, -60,  NULL },
	{ FRAME_deatha16,	NULL, 0, 0, 0, gorgon_set_roll, -120, NULL },
	{ FRAME_deatha17,	NULL, 0, 0, 0, gorgon_set_roll, -180, NULL },
	{ FRAME_deatha18,	NULL, 0, 0, 0, gorgon_set_roll, -240, NULL },
	{ FRAME_deatha19,	NULL, 0, 0, 0, gorgon_set_roll, -300, gorgon_lerp_off },
	{ FRAME_eatingb1,	NULL, 0, 0, 0, gorgon_set_roll,  0,   NULL },
	{ FRAME_eatingb3,	NULL, 0, 0, 0, NULL, 0, gorgon_lerp_on },
	{ FRAME_eatingb4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatinga4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatinga3,	NULL, 0, 0, 0, NULL, 0, gorgon_anger_sound },
	{ FRAME_eatinga2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatinga1,	NULL, 0, 0, 0, gorgon_slide, 0, NULL },
};
const animmove_t gorgon_move_slip = ANIMMOVE(gorgon_frames_slip, gorgon_check_mood);

// Gorgon Slip Pain.
static const animframe_t gorgon_frames_slip_pain[] =
{
	{ FRAME_deatha11,	NULL, 0, 0, 0, gorgon_slide, -200, gorgon_death1_fall },
	{ FRAME_deatha12,	NULL, 0, 0, 0, NULL, 0, gorgon_death1_fall },
	{ FRAME_deatha13,	NULL, 0, 0, 0, NULL, 0, gorgon_death1_fall },
	{ FRAME_deatha14,	NULL, 0, 0, 0, NULL, 0, gorgon_death1_fall },
	{ FRAME_rollover1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rollover2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rollover3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rollover4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_rollover5,	NULL, 0, 0, 0, NULL, 0, gorgon_lerp_off },
	{ FRAME_eatingb1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb3,	NULL, 0, 0, 0, NULL, 0, gorgon_lerp_on },
	{ FRAME_eatingb4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatingb21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatinga4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatinga3,	NULL, 0, 0, 0, NULL, 0, gorgon_anger_sound },
	{ FRAME_eatinga2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_eatinga1,	NULL, 0, 0, 0, gorgon_slide, 0, NULL },
};
const animmove_t gorgon_move_slip_pain = ANIMMOVE(gorgon_frames_slip_pain, gorgon_check_mood);

// Gorgon Delay.
static const animframe_t gorgon_frames_delay[] =
{
	{ FRAME_wait1,	NULL, 0, 0, 0, NULL, 0, gorgon_check_mood },
	{ FRAME_wait2,	NULL, 0, 0, 0, NULL, 0, gorgon_check_mood },
	{ FRAME_wait3,	NULL, 0, 0, 0, NULL, 0, gorgon_check_mood },
	{ FRAME_wait4,	NULL, 0, 0, 0, NULL, 0, gorgon_check_mood },
	{ FRAME_wait5,	NULL, 0, 0, 0, NULL, 0, gorgon_check_mood },
	{ FRAME_wait6,	NULL, 0, 0, 0, NULL, 0, gorgon_check_mood },
	{ FRAME_wait7,	NULL, 0, 0, 0, NULL, 0, gorgon_check_mood },
	{ FRAME_wait8,	NULL, 0, 0, 0, NULL, 0, gorgon_check_mood },
};
const animmove_t gorgon_move_delay = ANIMMOVE(gorgon_frames_delay, gorgon_check_mood);

// Gorgon Roar - make noise, alert others.
static const animframe_t gorgon_frames_roar[] =
{
	{ FRAME_speak1,		NULL, 0, 0, 0, NULL, 0, gorgon_roar },
	{ FRAME_speak2,		NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak7,		NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak13,	NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak17,	NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak19,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_roar = ANIMMOVE(gorgon_frames_roar, gorgon_check_mood);

// Gorgon Roar 2 - make noise in response to main roar.
static const animframe_t gorgon_frames_roar2[] =
{
	{ FRAME_speak1,		NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak4,		NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak8,		NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak10,	NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak15,	NULL, 0, 0, 0, NULL, 0, gorgon_roar_sound },
	{ FRAME_speak16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_speak19,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_roar2 = ANIMMOVE(gorgon_frames_roar2, gorgon_check_mood);

// Gorgon Swim Start.
static const animframe_t gorgon_frames_to_swim[] =
{
	//FIXME: add wake and bubbles and sploosh swim sounds.
	{ FRAME_swim1,	NULL, 0, 0, 0, NULL, 0, gorgon_check_in_water },
	{ FRAME_swim2,	NULL, 0, 0, 0, NULL, 0, gorgon_check_in_water },
	{ FRAME_swim3,	NULL, 0, 0, 0, NULL, 0, gorgon_check_in_water },
	{ FRAME_swim4,	NULL, 0, 0, 0, NULL, 0, gorgon_check_in_water },
	{ FRAME_swim5,	NULL, 0, 0, 0, NULL, 0, gorgon_check_in_water },
};
const animmove_t gorgon_move_to_swim = ANIMMOVE(gorgon_frames_to_swim, gorgon_swim_go);

// Gorgon Swim.
static const animframe_t gorgon_frames_swim[] =
{
	//FIXME: add wake and bubbles and sploosh swim sounds.
	{ FRAME_swim6,	NULL, 0, 0, 0, gorgon_ai_swim, 31, gorgon_under_water_wake }, //mxd. Original logic uses fish_under_water_wake() here.
	{ FRAME_swim7,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL },
	{ FRAME_swim8,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swim9,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swim10,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL },
	{ FRAME_swim11,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL },
	{ FRAME_swim12,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL },
	{ FRAME_swim13,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL },
	{ FRAME_swim14,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swim15,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL },
	{ FRAME_swim16,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL },
	{ FRAME_swim17,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL },
	{ FRAME_swim18,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
};
const animmove_t gorgon_move_swim = ANIMMOVE(gorgon_frames_swim, NULL);

// Gorgon Swim Bite A.
static const animframe_t gorgon_frames_swim_bite_a[] =
{
	//FIXME: add wake and bubbles and sploosh swim sounds.
	{ FRAME_swimata1,	NULL, 0, 0, 0, gorgon_ai_swim, 31, gorgon_under_water_wake }, //mxd. Original logic uses fish_under_water_wake() here.
	{ FRAME_swimata2,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL },
	{ FRAME_swimata3,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swimata4,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swimata5,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL },
	{ FRAME_swimata6,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL },
	{ FRAME_swimata7,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL },
	{ FRAME_swimata8,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL },
	{ FRAME_swimata9,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swimata10,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL },
	{ FRAME_swimata11,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL },
	{ FRAME_swimata12,	NULL, 0, 0, 0, gorgon_ai_swim, 32, gorgon_bite },
	{ FRAME_swimata13,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swimata14,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
};
const animmove_t gorgon_move_swim_bite_a = ANIMMOVE(gorgon_frames_swim_bite_a, NULL);

// Gorgon Swim Bite B.
static const animframe_t gorgon_frames_swim_bite_b[] =
{
	//FIXME: add wake and bubbles and sploosh swim sounds.
	{ FRAME_swimatb1,	NULL, 0, 0, 0, gorgon_ai_swim, 31, gorgon_under_water_wake }, //mxd. Original logic uses fish_under_water_wake() here.
	{ FRAME_swimatb2,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL },
	{ FRAME_swimatb3,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swimatb4,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swimatb5,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL },
	{ FRAME_swimatb6,	NULL, 0, 0, 0, gorgon_ai_swim, 30, gorgon_bite },
	{ FRAME_swimatb7,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL },
	{ FRAME_swimatb8,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL },
	{ FRAME_swimatb9,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swimatb10,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL },
	{ FRAME_swimatb11,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL },
	{ FRAME_swimatb12,	NULL, 0, 0, 0, gorgon_ai_swim, 32, gorgon_bite },
	{ FRAME_swimatb13,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
	{ FRAME_swimatb14,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL },
};
const animmove_t gorgon_move_swim_bite_b = ANIMMOVE(gorgon_frames_swim_bite_b, NULL);

// Gorgon Exit Water.
static const animframe_t gorgon_frames_outwater[] =
{
	//FIXME: add wake and bubbles and sploosh swim sounds.
	{ FRAME_swimata1,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL },
	{ FRAME_swimata3,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL },
	{ FRAME_swimata6,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL },
	{ FRAME_swimata8,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL },
	{ FRAME_swimata10,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL },
	{ FRAME_swimata12,	NULL, 0, 0, 0, gorgon_ai_swim, -1, gorgon_growl },
	{ FRAME_swimata14,	NULL, 0, 0, 0, gorgon_ai_swim, -1, gorgon_jump_out_of_water },
	{ FRAME_jumpb5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb6,		NULL, 0, 0, 0, NULL, 0, gorgon_fix_pitch },
	{ FRAME_jumpb7,		NULL, 0, 0, 0, NULL, 0, gorgon_fix_pitch },
	{ FRAME_jumpb8,		NULL, 0, 0, 0, gorgon_forward, 100, gorgon_fix_pitch },
	{ FRAME_jumpb9,		NULL, 0, 0, 0, gorgon_forward, 100, gorgon_fix_pitch },
	{ FRAME_jumpb10,	NULL, 0, 0, 0, gorgon_forward, 100, gorgon_fix_pitch },
	{ FRAME_jumpb11,	NULL, 0, 0, 0, gorgon_forward, 100, gorgon_fix_pitch },
	{ FRAME_jumpb12,	NULL, 0, 0, 0, gorgon_forward, 100, gorgon_fix_pitch },
	{ FRAME_jumpb13,	NULL, 0, 0, 0, gorgon_forward, 100, gorgon_fix_pitch },
	{ FRAME_jumpb14,	NULL, 0, 0, 0, gorgon_forward, 100, gorgon_fix_pitch },
	{ FRAME_jumpb15,	NULL, 0, 0, 0, gorgon_forward, 100, gorgon_reset_pitch },
	{ FRAME_jumpb16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jumpb17,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gorgon_move_outwater = ANIMMOVE(gorgon_frames_outwater, gorgon_check_mood);

// Gorgon eating transition to down.
static const animframe_t gorgon_frames_eat_down[] =
{
	{ FRAME_eatinga1,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga2,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga3,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga4,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_down = ANIMMOVE(gorgon_frames_eat_down, NULL);

// Gorgon eating transition to up
static const animframe_t gorgon_frames_eat_up[] =
{
	{ FRAME_eatinga4,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga3,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga2,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga1,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_up = ANIMMOVE(gorgon_frames_eat_up, NULL);

// Gorgon eat cycle.
static const animframe_t gorgon_frames_eat_loop[] =
{
	{ FRAME_eatinga5,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga6,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga7,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga8,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga9,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga10,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_loop = ANIMMOVE(gorgon_frames_eat_loop, NULL);

// Gorgon eating - tear.
static const animframe_t gorgon_frames_eat_tear[] =
{
	{ FRAME_eatingb1,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb2,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb3,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb4,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb5,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb6,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb7,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb8,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb9,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb10,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb11,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb12,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb13,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb14,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb15,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb16,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb17,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb18,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb19,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb20,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatingb21,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_tear = ANIMMOVE(gorgon_frames_eat_tear, NULL);

// Gorgon eat - up & down.
static const animframe_t gorgon_frames_eat_pullback[] =
{
	{ FRAME_eatinga5,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga4,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga3,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga4,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_eatinga5,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_pullback = ANIMMOVE(gorgon_frames_eat_pullback, NULL);

// Gorgon look around.
static const animframe_t gorgon_frames_look_around[] =
{
	//FIXME: modify view_ofs so they actually look behind them.
	{ FRAME_idleb1,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb2,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb3,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb4,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb5,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb6,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb7,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb8,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb9,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb10,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb11,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb12,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb13,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb14,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb15,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb16,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb17,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb18,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb19,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb20,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_idleb21,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_look_around = ANIMMOVE(gorgon_frames_look_around, NULL);

// Gorgon looking left from eat.
static const animframe_t gorgon_frames_eat_left[] =
{
	{ FRAME_loklft1,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft2,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft3,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft4,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft5,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft6,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft7,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft8,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft9,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft10,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft11,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft12,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft13,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft14,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft15,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft16,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft17,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft18,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft19,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft20,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft21,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft22,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft21,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft20,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft19,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft18,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft17,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft16,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft15,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft14,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft13,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft12,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft11,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft10,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft9,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft8,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft7,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft6,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft5,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft4,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft3,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft2,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_loklft1,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_left = ANIMMOVE(gorgon_frames_eat_left, NULL);

// Gorgon looking right from eat.
static const animframe_t gorgon_frames_eat_right[] =
{
	{ FRAME_lokrt1,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt2,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt3,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt4,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt5,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt6,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt7,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt8,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt9,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt10,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt11,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt12,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt13,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt14,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt15,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt16,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt17,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt18,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt19,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt20,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt19,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt18,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt17,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt16,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt15,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt14,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt13,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt12,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt11,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt10,	NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt9,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt8,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt7,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt6,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt5,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt4,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt3,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt2,		NULL, 0, 0, 0, gorgon_ai_eat,  0, NULL },
	{ FRAME_lokrt1,		NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_right = ANIMMOVE(gorgon_frames_eat_right, NULL);

// Gorgon snap at something to right.
static const animframe_t gorgon_frames_eat_snap[] =
{
	{ FRAME_snap1, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_snap2, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_snap3, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_snap4, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_snap5, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_snap6, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_snap7, NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_snap = ANIMMOVE(gorgon_frames_eat_snap, NULL);

// Gorgon react tp something to left.
static const animframe_t gorgon_frames_eat_react[] =
{
	{ FRAME_react1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_react10,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL },
};
const animmove_t gorgon_move_eat_react = ANIMMOVE(gorgon_frames_eat_react, NULL);
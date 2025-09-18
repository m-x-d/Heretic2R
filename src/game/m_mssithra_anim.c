//
// m_mssithra_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_mssithra_anim.h"
#include "m_mssithra_shared.h"
#include "g_ai.h" //mxd
#include "mg_ai.h" //mxd

// Mutant Ssithra Idle - looking around and standing.
static const animframe_t mssithra_frames_idle1[] =
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
};
const animmove_t mssithra_move_idle1 = ANIMMOVE(mssithra_frames_idle1, mssithra_check_mood);

// Mutant Ssithra Walk - walking along.
static const animframe_t mssithra_frames_walk1[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, ai_walk, 8,  NULL },
	{ FRAME_walk2,	NULL, 0, 0, 0, ai_walk, 8,  NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, ai_walk, 8,  NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, ai_walk, 8,  NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, ai_walk, 9,  NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, ai_walk, 9,  NULL },
	{ FRAME_walk7,	NULL, 0, 0, 0, ai_walk, 9,  NULL },
	{ FRAME_walk8,	NULL, 0, 0, 0, ai_walk, 10, NULL },
	{ FRAME_walk9,	NULL, 0, 0, 0, ai_walk, 10, NULL },
	{ FRAME_walk10,	NULL, 0, 0, 0, ai_walk, 10, NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, ai_walk, 10, NULL },
	{ FRAME_walk12,	NULL, 0, 0, 0, ai_walk, 10, NULL },
	{ FRAME_walk13,	NULL, 0, 0, 0, ai_walk, 9,  NULL },
	{ FRAME_walk14,	NULL, 0, 0, 0, ai_walk, 9,  NULL },
	{ FRAME_walk15,	NULL, 0, 0, 0, ai_walk, 9,  NULL },
	{ FRAME_walk16,	NULL, 0, 0, 0, ai_walk, 8,  NULL },
};
const animmove_t mssithra_move_walk1 = ANIMMOVE(mssithra_frames_walk1, NULL);

// Mutant Ssithra Run.
static const animframe_t mssithra_frames_run1[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, MG_AI_Run, 8,  NULL },
	{ FRAME_walk2,	NULL, 0, 0, 0, MG_AI_Run, 8,  NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, MG_AI_Run, 8,  NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, MG_AI_Run, 8,  NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, MG_AI_Run, 9,  NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, MG_AI_Run, 9,  NULL },
	{ FRAME_walk7,	NULL, 0, 0, 0, MG_AI_Run, 9,  NULL },
	{ FRAME_walk8,	NULL, 0, 0, 0, MG_AI_Run, 9,  NULL },
	{ FRAME_walk9,	NULL, 0, 0, 0, MG_AI_Run, 10, NULL },
	{ FRAME_walk10,	NULL, 0, 0, 0, MG_AI_Run, 10, NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, MG_AI_Run, 10, NULL },
	{ FRAME_walk12,	NULL, 0, 0, 0, MG_AI_Run, 10, NULL },
	{ FRAME_walk13,	NULL, 0, 0, 0, MG_AI_Run, 9,  NULL },
	{ FRAME_walk14,	NULL, 0, 0, 0, MG_AI_Run, 9,  NULL },
	{ FRAME_walk15,	NULL, 0, 0, 0, MG_AI_Run, 9,  NULL },
	{ FRAME_walk16,	NULL, 0, 0, 0, MG_AI_Run, 9,  NULL },
};
const animmove_t mssithra_move_run1 = ANIMMOVE(mssithra_frames_run1, NULL);

// Mutant Ssithra Backpedal - looking around and standing.
static const animframe_t mssithra_frames_backpedal1[] =
{
	{ FRAME_walk16,	NULL, 0, 0, 0, ai_charge2, -8,  NULL },
	{ FRAME_walk15,	NULL, 0, 0, 0, ai_charge2, -8,  NULL },
	{ FRAME_walk14,	NULL, 0, 0, 0, ai_charge2, -8,  NULL },
	{ FRAME_walk13,	NULL, 0, 0, 0, ai_charge2, -8,  NULL },
	{ FRAME_walk12,	NULL, 0, 0, 0, ai_charge2, -9,  NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, ai_charge2, -9,  NULL },
	{ FRAME_walk10,	NULL, 0, 0, 0, ai_charge2, -9,  NULL },
	{ FRAME_walk9,	NULL, 0, 0, 0, ai_charge2, -9,  NULL },
	{ FRAME_walk8,	NULL, 0, 0, 0, ai_charge2, -10, NULL },
	{ FRAME_walk7,	NULL, 0, 0, 0, ai_charge2, -10, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, ai_charge2, -10, NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, ai_charge2, -10, NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, ai_charge2, -9,  NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, ai_charge2, -9,  NULL },
	{ FRAME_walk2,	NULL, 0, 0, 0, ai_charge2, -9,  NULL },
	{ FRAME_walk1,	NULL, 0, 0, 0, ai_charge2, -9,  NULL },
};
const animmove_t mssithra_move_backpedal1 = ANIMMOVE(mssithra_frames_backpedal1, NULL);

// Mutant Ssithra Death.
static const animframe_t mssithra_frames_death1[] =
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
	{ FRAME_death21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death30, 	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death33,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death34,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death35,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death36,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death37,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death38,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death39,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death40, 	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death41,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death42,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death43,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death44,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death45,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t mssithra_move_death1 = ANIMMOVE(mssithra_frames_death1, mssithra_dead);

// Mutant Ssithra Claw Attack.
static const animframe_t mssithra_frames_claw1[] =
{
	{ FRAME_claw1,	NULL, 0, 0, 0, ai_charge2, 20, NULL },
	{ FRAME_claw2,	NULL, 0, 0, 0, ai_charge2, 10, mssithra_growl },
	{ FRAME_claw3,	NULL, 0, 0, 0, ai_charge2, 8,  NULL },
	{ FRAME_claw4,	NULL, 0, 0, 0, ai_charge2, 6,  NULL },
	{ FRAME_claw5,	NULL, 0, 0, 0, ai_charge2, 4,  mssithra_swipe },
	{ FRAME_claw6,	NULL, 0, 0, 0, ai_charge2, 3,  NULL },
	{ FRAME_claw7,	NULL, 0, 0, 0, ai_charge2, 3,  NULL },
	{ FRAME_claw8,	NULL, 0, 0, 0, ai_charge2, 4,  mssithra_swipe },
	{ FRAME_claw9,	NULL, 0, 0, 0, ai_charge2, 3,  NULL },
	{ FRAME_claw10,	NULL, 0, 0, 0, ai_charge2, 3,  NULL },
};
const animmove_t mssithra_move_claw1 = ANIMMOVE(mssithra_frames_claw1, mssithra_check_mood); //mxd. numframes:7 in original logic. //TODO: check this!

// Mutant Ssithra Jump.
static const animframe_t mssithra_frames_jump1[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t mssithra_move_jump1 = ANIMMOVE(mssithra_frames_jump1, mssithra_check_mood);

// Mutant Ssithra Forced Jump.
static const animframe_t mssithra_frames_fjump[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t mssithra_move_fjump = ANIMMOVE(mssithra_frames_fjump, mssithra_check_mood);

// Mutant Ssithra Shoot A.
static const animframe_t mssithra_frames_shoota1[] =
{
	{ FRAME_shoota1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota4,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_growl },
	{ FRAME_shoota5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota8,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota9,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_arrow }, // 8: arrow here.
	{ FRAME_shoota10,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota11,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota12,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota13,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota14,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota15,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota16,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota17,	NULL, 0, 0, 0, ai_charge2, 0, NULL }, // Check here to loop back to 8.
	{ FRAME_shoota18,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_arrow },
	{ FRAME_shoota19,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota20,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota21,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoota22,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_growl },
	{ FRAME_shoota23,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t mssithra_move_shoota1 = ANIMMOVE(mssithra_frames_shoota1, mssithra_check_mood);

// Mutant Ssithra Shoot B.
static const animframe_t mssithra_frames_shootb1[] =
{
	{ FRAME_shootb1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb7,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_growl },
	{ FRAME_shootb8,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_arrow }, // 7: shoot here
	{ FRAME_shootb9,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb10,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb11,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb12,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb13,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_growl },
	{ FRAME_shootb14,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb15,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb16,	NULL, 0, 0, 0, ai_charge2, 0, NULL }, // Check here to loop back to 7.
	{ FRAME_shootb17,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_arrow },
	{ FRAME_shootb18,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb19,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb20,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb21,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shootb22,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t mssithra_move_shootb1 = ANIMMOVE(mssithra_frames_shootb1, mssithra_check_mood);

// Mutant Ssithra Roar.
static const animframe_t mssithra_frames_roar[] =
{
	{ FRAME_roar1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_roar24,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t mssithra_move_roar = ANIMMOVE(mssithra_frames_roar, mssithra_check_mood);

// Mutant Ssithra Delay.
static const animframe_t mssithra_frames_delay[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle21,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t mssithra_move_delay = ANIMMOVE(mssithra_frames_delay, NULL);

// Mutant Ssithra Shoot Start.
static const animframe_t mssithra_frames_shoot1_trans[] =
{
	{ FRAME_newshot1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newshot2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newshot3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t mssithra_move_shoot1_trans = ANIMMOVE(mssithra_frames_shoot1_trans, mssithra_shoot_loop);

// Mutant Ssithra Shoot Loop.
static const animframe_t mssithra_frames_shoot1_loop[] =
{
	{ FRAME_newshot4,	NULL, 0, 0, 0, ai_charge2, 0, mssithra_arrow },
	{ FRAME_newshot5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newshot6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newshot7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newshot8,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newshot9,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t mssithra_move_shoot1_loop = ANIMMOVE(mssithra_frames_shoot1_loop, mssithra_check_shoot_loop);

// Mutant Ssithra Shoot End.
static const animframe_t mssithra_frames_shoot1_detrans[] =
{
	{ FRAME_newshot3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newshot2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_newshot1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t mssithra_move_shoot1_detrans = ANIMMOVE(mssithra_frames_shoot1_detrans, mssithra_check_mood);
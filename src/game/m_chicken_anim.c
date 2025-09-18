//
// m_chicken_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_chicken_anim.h"
#include "m_chicken_shared.h"
#include "g_ai.h" //mxd
#include "mg_ai.h" //mxd
#include "g_local.h"

#define ENEMY_WALK_SPEED	32.0f
#define ENEMY_RUN_SPEED		64.0f

// FOR BAD GUYS ONLY

// Chicken standing.
static const animframe_t chicken_frames_stand1[] =
{
	{ FRAME_wait1,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_wait2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait4,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_wait5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_wait6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t chicken_move_stand1 = ANIMMOVE(chicken_frames_stand1, chicken_pause);

// Chicken running.
static const animframe_t chicken_frames_run[] =
{
	{ FRAME_run1,	NULL, 0, 0, 0, MG_AI_Run, ENEMY_RUN_SPEED, chicken_check_unmorph },
	{ FRAME_run2,	NULL, 0, 0, 0, MG_AI_Run, ENEMY_RUN_SPEED, NULL },
	{ FRAME_run3,	chicken_sound, CHAN_BODY, SND_CLAW, ATTN_NORM,  MG_AI_Run, ENEMY_RUN_SPEED, NULL },
	{ FRAME_run4,	NULL, 0, 0, 0, MG_AI_Run, ENEMY_RUN_SPEED, chicken_check_unmorph },
	{ FRAME_run5,	NULL, 0, 0, 0, MG_AI_Run, ENEMY_RUN_SPEED, NULL },
	{ FRAME_run6,	chicken_sound, CHAN_BODY, SND_CLAW, ATTN_NORM,  MG_AI_Run, ENEMY_RUN_SPEED, NULL },
};
const animmove_t chicken_move_run = ANIMMOVE(chicken_frames_run, chicken_pause);

// Chicken walking.
static const animframe_t chicken_frames_walk[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, ai_walk, ENEMY_WALK_SPEED, chicken_check_unmorph },
	{ FRAME_walk2,	NULL, 0, 0, 0, ai_walk, ENEMY_WALK_SPEED, NULL },
	{ FRAME_walk3,	chicken_sound, CHAN_BODY, SND_CLAW, ATTN_NORM,  ai_walk, ENEMY_WALK_SPEED, chicken_check_unmorph },
	{ FRAME_walk4,	NULL, 0, 0, 0, ai_walk, ENEMY_WALK_SPEED, NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, ai_walk, ENEMY_WALK_SPEED, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, ai_walk, ENEMY_WALK_SPEED, NULL },
	{ FRAME_walk7,	chicken_sound, CHAN_BODY, SND_CLAW, ATTN_NORM,  ai_walk, ENEMY_WALK_SPEED, chicken_check_unmorph },
	{ FRAME_walk8,	NULL, 0, 0, 0, ai_walk, ENEMY_WALK_SPEED, NULL },
};
const animmove_t chicken_move_walk = ANIMMOVE(chicken_frames_walk, chicken_pause);

// Chicken cluck.
static const animframe_t chicken_frames_cluck[] =
{
	{ FRAME_cluck1,		NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_cluck2,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck3,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck4,		NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_cluck5,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck6,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck7,		NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_cluck8,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck9,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck10,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_cluck11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck13,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_cluck14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck16,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_cluck17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_cluck19,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
};
const animmove_t chicken_move_cluck = ANIMMOVE(chicken_frames_cluck, chicken_eat_again);

// Chicken attacking.
static const animframe_t chicken_frames_attack[] =
{
	{ FRAME_attack1,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_attack2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_attack3,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_attack4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_attack5,	NULL, 0, 0, 0, ai_stand, 0, chicken_bite },
	{ FRAME_attack6,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
};
const animmove_t chicken_move_attack = ANIMMOVE(chicken_frames_attack, chicken_pause);

// Chicken eating.
static const animframe_t chicken_frames_eat[] =
{
	{ FRAME_peck1,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck4,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck7,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck9,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck10,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck13,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck16,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck19,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck21,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck22,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck23,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck24,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck25,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck26,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck27,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_peck28,	NULL, 0, 0, 0, ai_stand, 0, chicken_check_unmorph },
	{ FRAME_peck29,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t chicken_move_eat = ANIMMOVE(chicken_frames_eat, chicken_eat_again);

// Chicken jumping.
static const animframe_t chicken_frames_jump[] = //TODO: no 'chicken_check_unmorph' checks here. Probably intentional.
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_jump5,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_jump6,	NULL, 0, 0, 0, ai_walk, 8, NULL },
};
const animmove_t chicken_move_jump = ANIMMOVE(chicken_frames_jump, chicken_pause);
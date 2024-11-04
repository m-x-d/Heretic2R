//==============================================================================
//
// m_chicken_anim.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "g_local.h"

#pragma hdrstop("g_local.pch")
// PRECOMPILED HEADER ABOVE
// WARNING:  DO NOT CHANGE THE ABOVE HEADERS OR THE PRECOMPILED STUFF WILL BREAK!  
// ADD ANY ADDITIONAL FILES BELOW

#include "m_chicken_anim.h"
#include "m_chicken.h"

#define ENEMY_WALK_SPEED  32
#define ENEMY_RUN_SPEED   64
#define PLAYER_WALK_SPEED 240
#define PLAYER_STRAFE_SPEED 185
#define PLAYER_RUN_SPEED  300

// FOR BAD GUYS ONLY
/*----------------------------------------------------------------------
  Chicken Standing - 
-----------------------------------------------------------------------*/
animframe_t chicken_frames_stand1 [] =
{
	FRAME_wait1,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_wait2,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_wait3,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_wait4,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_wait5,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_wait6,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
};
animmove_t chicken_move_stand1 = {6, chicken_frames_stand1, chicken_pause};

/*----------------------------------------------------------------------
  Chicken running - 
-----------------------------------------------------------------------*/
animframe_t chicken_frames_run [] =
{
	FRAME_run1,	NULL, 0, 0, 0,  ai_run, ENEMY_RUN_SPEED, chicken_check,
	FRAME_run2,	NULL, 0, 0, 0,  ai_run, ENEMY_RUN_SPEED, NULL,
	FRAME_run3,	chickenSound, CHAN_BODY, SND_CLAW, ATTN_NORM,  ai_run, ENEMY_RUN_SPEED, NULL,
	FRAME_run4,	NULL, 0, 0, 0,  ai_run, ENEMY_RUN_SPEED, chicken_check,
	FRAME_run5,	NULL, 0, 0, 0,  ai_run, ENEMY_RUN_SPEED, NULL,
	FRAME_run6,	chickenSound, CHAN_BODY, SND_CLAW, ATTN_NORM,  ai_run, ENEMY_RUN_SPEED, NULL,
};
animmove_t chicken_move_run = {6, chicken_frames_run, chicken_pause};

/*----------------------------------------------------------------------
  Chicken walking - 
-----------------------------------------------------------------------*/
animframe_t chicken_frames_walk [] =
{
	FRAME_walk1,	NULL, 0, 0, 0,  ai_walk, ENEMY_WALK_SPEED, chicken_check,
	FRAME_walk2,	NULL, 0, 0, 0,  ai_walk, ENEMY_WALK_SPEED, NULL,
	FRAME_walk3,	chickenSound, CHAN_BODY, SND_CLAW, ATTN_NORM,  ai_walk, ENEMY_WALK_SPEED, chicken_check,
	FRAME_walk4,	NULL, 0, 0, 0,  ai_walk, ENEMY_WALK_SPEED, NULL,
	FRAME_walk5,	NULL, 0, 0, 0,  ai_walk, ENEMY_WALK_SPEED, NULL,
	FRAME_walk6,	NULL, 0, 0, 0,  ai_walk, ENEMY_WALK_SPEED, NULL,
	FRAME_walk7,	chickenSound, CHAN_BODY, SND_CLAW, ATTN_NORM,  ai_walk, ENEMY_WALK_SPEED, chicken_check,
	FRAME_walk8,	NULL, 0, 0, 0,  ai_walk, ENEMY_WALK_SPEED, NULL,
};
animmove_t chicken_move_walk = {8, chicken_frames_walk, chicken_pause};

/*----------------------------------------------------------------------
  Chicken cluck - 
-----------------------------------------------------------------------*/
animframe_t chicken_frames_cluck [] =
{
	FRAME_cluck1,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_cluck2,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck3,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck4,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_cluck5,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck6,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck7,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_cluck8,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck9,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck10,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_cluck11,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck12,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck13,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_cluck14,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck15,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck16,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_cluck17,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck18,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_cluck19,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
};
animmove_t chicken_move_cluck = {19, chicken_frames_cluck, chicken_eat_again};

/*----------------------------------------------------------------------
  Chicken attacking - 
-----------------------------------------------------------------------*/
animframe_t chicken_frames_attack [] =
{
	FRAME_attack1,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_attack2,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_attack3,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_attack4,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_attack5,	NULL, 0, 0, 0,  ai_stand, 0, chicken_bite,
	FRAME_attack6,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check
};

animmove_t chicken_move_attack = {6, chicken_frames_attack, chicken_pause};

/*----------------------------------------------------------------------
  Chicken eating - 
-----------------------------------------------------------------------*/
animframe_t chicken_frames_eat [] =
{
	FRAME_peck1,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck2,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck3,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck4,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck5,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck6,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck7,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck8,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck9,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck10,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck11,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck12,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck13,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck14,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck15,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck16,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck17,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck18,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck19,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck20,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck21,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck22,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck23,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck24,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck25,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck26,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck27,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_peck28,	NULL, 0, 0, 0,  ai_stand, 0, chicken_check,
	FRAME_peck29,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
};

animmove_t chicken_move_eat = {29, chicken_frames_eat, chicken_eat_again};

/*----------------------------------------------------------------------
  Chicken jumping - 
-----------------------------------------------------------------------*/
animframe_t chicken_frames_jump [] =
{
	FRAME_jump1,	NULL, 0, 0, 0,  ai_walk, 8, NULL,
	FRAME_jump2,	NULL, 0, 0, 0,  ai_walk, 8, NULL,
	FRAME_jump3,	NULL, 0, 0, 0,  ai_walk, 8, NULL,
	FRAME_jump4,	NULL, 0, 0, 0,  ai_walk, 8, NULL,
	FRAME_jump5,	NULL, 0, 0, 0,  ai_walk, 8, NULL,
	FRAME_jump6,	NULL, 0, 0, 0,  ai_walk, 8, NULL,
};

animmove_t chicken_move_jump = {6, chicken_frames_jump, chicken_pause};

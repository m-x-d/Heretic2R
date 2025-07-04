//
// p_chicken_anim.c -- Player chicken animations.
//
// Copyright 1998 Raven Software
//

#include "p_anims.h"
#include "p_actions.h"
#include "p_chicken.h"
#include "p_anim_branch.h"
#include "m_chicken_anim.h"

#define PLAYER_WALK_SPEED	240.0f
#define PLAYER_STRAFE_SPEED	185.0f
#define PLAYER_RUN_SPEED	300.0f

// Should never be used.
static panimframe_t chickenp_frames_dummy[] =
{
	FRAME_wait1, NULL, 0, 0, 0, NULL, 0, PlayerChickenAssert,
};
panimmove_t chickenp_move_dummy = { 1, chickenp_frames_dummy, PlayerAnimLowerUpdate };

// Idle stand animation.
static panimframe_t chickenp_frames_stand[] =
{
	FRAME_wait1, NULL, 0, 0, 0, PlayerChickenCluck, 0, ChickenBranchIdle,
	FRAME_wait2, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_wait3, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_wait4, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_wait5, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_wait6, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
};
panimmove_t chickenp_move_stand = { 6, chickenp_frames_stand, PlayerAnimLowerUpdate };

// Idle peck animation.
static panimframe_t chickenp_frames_stand1[] =
{
	FRAME_peck1,  NULL, 0, 0, 0, PlayerChickenCluck, 0, ChickenBranchIdle,
	FRAME_peck2,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck3,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck4,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck5,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck6,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck7,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck8,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck9,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck10, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck11, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck12, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck13, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck14, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck15, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck16, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck17, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck18, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck19, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck20, NULL, 0, 0, 0, PlayerChickenCluck, 0, ChickenBranchIdle,
	FRAME_peck21, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck22, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck23, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck24, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck25, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck26, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck27, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck28, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_peck29, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
};
panimmove_t chickenp_move_stand1 = { 29, chickenp_frames_stand1, PlayerAnimLowerUpdate };

// Idle clucking animation.
static panimframe_t chickenp_frames_stand2[] =
{
	FRAME_cluck1,  NULL, 0, 0, 0, PlayerChickenCluck, 0, ChickenBranchIdle,
	FRAME_cluck2,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck3,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck4,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck5,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck6,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck7,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck8,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck9,  NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck10, NULL, 0, 0, 0, PlayerChickenCluck, 1, ChickenBranchIdle,
	FRAME_cluck11, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck12, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck13, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck14, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck15, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck16, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck17, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck18, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
	FRAME_cluck19, NULL, 0, 0, 0, NULL, 0, ChickenBranchIdle,
};
panimmove_t chickenp_move_stand2 = { 19, chickenp_frames_stand2, PlayerAnimLowerUpdate };

// Chicken running.
static panimframe_t chickenp_frames_run[] =
{
	FRAME_run1, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run2, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run3, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_run4, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run5, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run6, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
};
panimmove_t chickenp_move_run = { 6, chickenp_frames_run, PlayerAnimLowerUpdate };

// Chicken walking.
static panimframe_t chickenp_frames_walk[] =
{
	FRAME_walk1, PlayerMoveFunc, PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk2, PlayerMoveFunc, PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk3, PlayerMoveFunc, PLAYER_WALK_SPEED, 0, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_walk4, PlayerMoveFunc, PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk5, PlayerMoveFunc, PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk6, PlayerMoveFunc, PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk7, PlayerMoveFunc, PLAYER_WALK_SPEED, 0, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_walk8, PlayerMoveFunc, PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
};
panimmove_t chickenp_move_walk = { 8, chickenp_frames_walk, PlayerAnimLowerUpdate };

// Chicken running backwards.
static panimframe_t chickenp_frames_runb[] =
{
	FRAME_run6, PlayerMoveFunc, -PLAYER_RUN_SPEED, 0, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_run5, PlayerMoveFunc, -PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run4, PlayerMoveFunc, -PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run3, PlayerMoveFunc, -PLAYER_RUN_SPEED, 0, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_run2, PlayerMoveFunc, -PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run1, PlayerMoveFunc, -PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
};
panimmove_t chickenp_move_runb = { 6, chickenp_frames_runb, PlayerAnimLowerUpdate };

// Chicken walking backwards.
static panimframe_t chickenp_frames_back[] =
{
	FRAME_walk8, PlayerMoveFunc, -PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk7, PlayerMoveFunc, -PLAYER_WALK_SPEED, 0, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_walk6, PlayerMoveFunc, -PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk5, PlayerMoveFunc, -PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk4, PlayerMoveFunc, -PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk3, PlayerMoveFunc, -PLAYER_WALK_SPEED, 0, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_walk2, PlayerMoveFunc, -PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_walk1, PlayerMoveFunc, -PLAYER_WALK_SPEED, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
};
panimmove_t chickenp_move_back = { 8, chickenp_frames_back, PlayerAnimLowerUpdate };

// Chicken strafing left.
static panimframe_t chickenp_frames_strafel[] =
{
	FRAME_run1, PlayerMoveFunc, 0, -PLAYER_STRAFE_SPEED, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run2, PlayerMoveFunc, 0, -PLAYER_STRAFE_SPEED, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run3, PlayerMoveFunc, 0, -PLAYER_STRAFE_SPEED, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_run4, PlayerMoveFunc, 0, -PLAYER_STRAFE_SPEED, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run5, PlayerMoveFunc, 0, -PLAYER_STRAFE_SPEED, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run6, PlayerMoveFunc, 0, -PLAYER_STRAFE_SPEED, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
};
panimmove_t chickenp_move_strafel = { 6, chickenp_frames_strafel, PlayerAnimLowerUpdate };

// Chicken strafing right.
static panimframe_t chickenp_frames_strafer[] =
{
	FRAME_run1, PlayerMoveFunc, 0, PLAYER_STRAFE_SPEED, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run2, PlayerMoveFunc, 0, PLAYER_STRAFE_SPEED, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run3, PlayerMoveFunc, 0, PLAYER_STRAFE_SPEED, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
	FRAME_run4, PlayerMoveFunc, 0, PLAYER_STRAFE_SPEED, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run5, PlayerMoveFunc, 0, PLAYER_STRAFE_SPEED, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_run6, PlayerMoveFunc, 0, PLAYER_STRAFE_SPEED, 0, PlayerChickenStepSound, 0, PlayerAnimLowerUpdate,
};
panimmove_t chickenp_move_strafer = { 6, chickenp_frames_strafer, PlayerAnimLowerUpdate };

// Chicken jumping start - standing (mxd. Every 2-nd frame is skipped).
static panimframe_t chickenp_frames_jump[] =
{
	FRAME_jump1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jump3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jump5, NULL, 0, 0, 0, NULL, 0, NULL,
};
panimmove_t chickenp_move_jump = { 3, chickenp_frames_jump, PlayerChickenJump };

// Chicken jumping loop - used for falling too.
static panimframe_t chickenp_frames_jump_loop[] =
{
	FRAME_jump6, NULL, 0, 0, 0, NULL, 0, PlayerChickenCheckFlap,
};
panimmove_t chickenp_move_jump_loop = { 1, chickenp_frames_jump_loop, PlayerAnimLowerUpdate };

// Chicken flap
static panimframe_t chickenp_frames_jump_flap[] =
{
	FRAME_jump7, NULL, 0, 0, 0, NULL, 0, PlayerChickenFlap,
};
panimmove_t chickenp_move_jump_flap = { 1, chickenp_frames_jump_flap, PlayerAnimLowerUpdate };

// Chicken jumping walking start.
static panimframe_t chickenp_frames_wjump[] =
{
	FRAME_jump3, PlayerMoveFunc, PLAYER_WALK_SPEED / 2,	0, 0, NULL, 0, NULL,
	FRAME_jump4, PlayerMoveFunc, PLAYER_WALK_SPEED,		0, 0, NULL, 0, NULL,
	FRAME_jump5, PlayerMoveFunc, PLAYER_WALK_SPEED * 2,	0, 0, NULL, 0, NULL,
};
panimmove_t chickenp_move_wjump = { 3, chickenp_frames_wjump, PlayerChickenJump };

// Chicken jumping running start.
static panimframe_t chickenp_frames_rjump[] =
{
	FRAME_jump3, PlayerMoveFunc, PLAYER_RUN_SPEED / 2, 0, 0, NULL, 0, NULL,
	FRAME_jump4, PlayerMoveFunc, PLAYER_RUN_SPEED,	   0, 0, NULL, 0, NULL,
	FRAME_jump5, PlayerMoveFunc, PLAYER_RUN_SPEED * 2, 0, 0, NULL, 0, NULL,
};
panimmove_t chickenp_move_rjump = { 3, chickenp_frames_rjump, PlayerChickenJump };

// Chicken jumping running back start.
static panimframe_t chickenp_frames_rjumpb[] =
{
	FRAME_jump3, PlayerMoveFunc, -PLAYER_RUN_SPEED / 2,	0, 0, NULL, 0, NULL,
	FRAME_jump4, PlayerMoveFunc, -PLAYER_RUN_SPEED,		0, 0, NULL, 0, NULL,
	FRAME_jump5, PlayerMoveFunc, -PLAYER_RUN_SPEED * 2,	0, 0, NULL, 0, NULL,
};
panimmove_t chickenp_move_rjumpb = { 3, chickenp_frames_rjumpb, PlayerChickenJump };

// Chicken attacking.
static panimframe_t chickenp_frames_attack[] =
{
	FRAME_attack1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_attack2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_attack3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_attack4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_attack5, NULL, 0, 0, 0, NULL, 0, PlayerChickenBite,
	FRAME_attack6, NULL, 0, 0, 0, NULL, 0, NULL
};
panimmove_t chickenp_move_attack = { 6, chickenp_frames_attack, PlayerAnimLowerUpdate };

// Chicken running attacking.
static panimframe_t chickenp_frames_runattack[] =
{
	FRAME_rattack1, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, NULL, 0, PlayerChickenBite,
	FRAME_rattack2, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, NULL, 0, NULL,
	FRAME_rattack3, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, PlayerChickenStepSound, 0, NULL,
	FRAME_rattack4, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, NULL, 0, NULL,
	FRAME_rattack5, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, NULL, 0, NULL,
	FRAME_rattack6, PlayerMoveFunc, PLAYER_RUN_SPEED, 0, 0, PlayerChickenStepSound, 0, NULL,
};
panimmove_t chickenp_move_runattack = { 6, chickenp_frames_runattack, PlayerAnimLowerUpdate };

// Reuses idle stand animation.
static panimframe_t chickenp_frames_swim_idle[] =
{
	FRAME_wait1, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_wait2, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_wait3, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_wait4, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_wait5, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_wait6, NULL, 0, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
};
panimmove_t chickenp_move_swim_idle = { 6, chickenp_frames_swim_idle, PlayerAnimLowerUpdate };

// Reuses jump animation.
static panimframe_t chickenp_frames_swim[] =
{
	FRAME_jump6, PlayerMoveFunc, PLAYER_RUN_SPEED,	   0, 0, NULL, 0, NULL,
	FRAME_jump7, PlayerMoveFunc, PLAYER_RUN_SPEED / 2, 0, 0, NULL, 0, PlayerAnimLowerUpdate,
	FRAME_jump8, PlayerMoveFunc, PLAYER_RUN_SPEED / 4, 0, 0, NULL, 0, NULL,
};
panimmove_t chickenp_move_swim = { 3, chickenp_frames_swim, PlayerAnimLowerUpdate };
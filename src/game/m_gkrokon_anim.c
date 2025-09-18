//
// m_gkrokon_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_gkrokon_anim.h"
#include "m_gkrokon_shared.h"
#include "g_ai.h" //mxd
#include "mg_ai.h" //mxd
#include "g_local.h"

// Stand1 - laid down, resting, still on the floor.
static const animframe_t gkrokon_frames_stand1[] =
{
	{ FRAME_bwait1,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait2,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait3,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait4,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait5,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, gkrokon_idle_sound },
	{ FRAME_bwait6,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait7,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait8,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait9,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait10,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait11,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait12,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait13,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait14,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
};
const animmove_t gkrokon_move_stand1 = ANIMMOVE(gkrokon_frames_stand1, gkrokon_pause);

// Stand2 - getting up off the floor.
static const animframe_t gkrokon_frames_stand2[] =
{
	{ FRAME_birth1,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth2,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth3,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth4,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth5,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth6,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth7,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth8,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth9,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, gkrokon_idle_sound },
	{ FRAME_birth10,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth11,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth12,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
};
const animmove_t gkrokon_move_stand2 = ANIMMOVE(gkrokon_frames_stand2, gkrokon_pause);

// Stand3 - standing fairly still, waiting.
static const animframe_t gkrokon_frames_stand3[] =
{
	{ FRAME_wait1,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_wait2,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_wait3,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_wait4,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, gkrokon_idle_sound },
	{ FRAME_wait5,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_wait6,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_wait7,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_wait8,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
};
const animmove_t gkrokon_move_stand3 = ANIMMOVE(gkrokon_frames_stand3, gkrokon_pause);

// Stand4 - settling down onto the floor.
static const animframe_t gkrokon_frames_stand4[] =
{
	{ FRAME_birth5,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth4,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth3,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_birth2,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, gkrokon_idle_sound },
	{ FRAME_birth1,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
};
const animmove_t gkrokon_move_stand4 = ANIMMOVE(gkrokon_frames_stand4, gkrokon_pause);

// Crouch1 - crouched down on the floor (stalking enemy).
static const animframe_t gkrokon_frames_crouch1[] =
{
	{ FRAME_bwait1,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait2,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait3,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait4,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait5,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait6,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait7,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait8,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait9,		NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait10,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait11,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait12,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait13,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
	{ FRAME_bwait14,	NULL, 0, 0, 0, gkrokon_ai_stand, 0, NULL },
};
const animmove_t gkrokon_move_crouch1 = ANIMMOVE(gkrokon_frames_crouch1, gkrokon_pause);

// Crouch2 - getting up off the floor from crouching (stalking enemy).
static const animframe_t gkrokon_frames_crouch2[] =
{
	{ FRAME_birth1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth4,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gkrokon_move_crouch2 = ANIMMOVE(gkrokon_frames_crouch2, gkrokon_set_stand_anim);

// Crouch3 - settling down into crouching position (stalking enemy).
static const animframe_t gkrokon_frames_crouch3[] =
{
	{ FRAME_birth4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth1,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gkrokon_move_crouch3 = ANIMMOVE(gkrokon_frames_crouch3, gkrokon_set_crouch_anim);

// Walk1 - a leisurely ambling gait.
static const animframe_t gkrokon_frames_walk1[] =
{
	{ FRAME_walkB1,		NULL, 0, 0, 0, gkrokon_ai_stand, 6, NULL },
	{ FRAME_walkB2,		NULL, 0, 0, 0, gkrokon_ai_stand, 7, NULL },
	{ FRAME_walkB3,		NULL, 0, 0, 0, gkrokon_ai_stand, 5, NULL },
	{ FRAME_walkB4,		NULL, 0, 0, 0, gkrokon_ai_stand, 8, NULL },
	{ FRAME_walkB5,		NULL, 0, 0, 0, gkrokon_ai_stand, 7, NULL },
	{ FRAME_walkB6,		NULL, 0, 0, 0, gkrokon_ai_stand, 6, gkrokon_walk_sound },
	{ FRAME_walkB7,		NULL, 0, 0, 0, gkrokon_ai_stand, 5, NULL },
	{ FRAME_walkB8,		NULL, 0, 0, 0, gkrokon_ai_stand, 8, NULL },
};
const animmove_t gkrokon_move_walk1 = ANIMMOVE(gkrokon_frames_walk1, gkrokon_pause);

// Run1 - a galloping run.
static const animframe_t gkrokon_frames_run1[] =
{
	{ FRAME_gallop1,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_gallop2,	NULL, 0, 0, 0, MG_AI_Run, 24, NULL },
	{ FRAME_gallop3,	NULL, 0, 0, 0, MG_AI_Run, 22, NULL },
	{ FRAME_gallop4,	NULL, 0, 0, 0, MG_AI_Run, 18, gkrokon_walk_sound },
	{ FRAME_gallop5,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_gallop6,	NULL, 0, 0, 0, MG_AI_Run, 24, NULL },
};
const animmove_t gkrokon_move_run1 = ANIMMOVE(gkrokon_frames_run1, gkrokon_pause);

// Run2 - a skittering, insectlike run.
static const animframe_t gkrokon_frames_run2[] =
{
	{ FRAME_skittr1,	NULL, 0, 0, 0, MG_AI_Run, 12, NULL },
	{ FRAME_skittr2,	NULL, 0, 0, 0, MG_AI_Run, 12, NULL },
	{ FRAME_skittr3,	NULL, 0, 0, 0, MG_AI_Run, 12, gkrokon_walk_sound },
	{ FRAME_skittr4,	NULL, 0, 0, 0, MG_AI_Run, 12, NULL },
};
const animmove_t gkrokon_move_run2 = ANIMMOVE(gkrokon_frames_run2, gkrokon_pause);

// Run away.
static const animframe_t gkrokon_frames_run_away[] =
{
	{ FRAME_skittr4,	gkrokon_sound, CHAN_VOICE, SND_FLEE, ATTN_NORM, MG_AI_Run, -14, NULL },
	{ FRAME_skittr3,	NULL, 0, 0, 0, MG_AI_Run, -16, NULL },
	{ FRAME_skittr2,	NULL, 0, 0, 0, MG_AI_Run, -14, gkrokon_walk_sound },
	{ FRAME_skittr1,	NULL, 0, 0, 0, MG_AI_Run, -12, NULL },
	{ FRAME_skittr4,	NULL, 0, 0, 0, MG_AI_Run, -14, NULL },
	{ FRAME_skittr3,	NULL, 0, 0, 0, MG_AI_Run, -16, NULL },
	{ FRAME_skittr2,	NULL, 0, 0, 0, MG_AI_Run, -14, gkrokon_walk_sound },
	{ FRAME_skittr1,	NULL, 0, 0, 0, MG_AI_Run, -12, NULL },
};
const animmove_t gkrokon_move_run_away = ANIMMOVE(gkrokon_frames_run_away, gkrokon_pause);

// Jump1 - jumping.
static const animframe_t gkrokon_frames_jump1[] =
{
	{ FRAME_jump1,		gkrokon_sound, CHAN_VOICE, SND_ANGRY, ATTN_NORM, NULL, 0, NULL },
	{ FRAME_jump2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump10,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump12,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump14,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump16,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump18,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump20,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump22,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump23,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gkrokon_move_jump1 = ANIMMOVE(gkrokon_frames_jump1, gkrokon_pause);

// Forced jump.
static const animframe_t gkrokon_frames_forced_jump[] =
{
	{ FRAME_jump1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump10,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump12,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump14,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump16,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump18,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump20,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump22,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump23,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gkrokon_move_forced_jump = ANIMMOVE(gkrokon_frames_forced_jump, gkrokon_pause);

// MeleeAttack1 - A left hand attack.
static const animframe_t gkrokon_frames_melee_attack1[] =
{
	{ FRAME_latack1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_latack2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_latack3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_latack4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_latack5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_latack6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_latack7,	NULL, 0, 0, 0, gkrokon_bite, 0, NULL },
	{ FRAME_latack8,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gkrokon_move_melee_attack1 = ANIMMOVE(gkrokon_frames_melee_attack1, gkrokon_pause);

// MeleeAttack2 - A right hand attack.
static const animframe_t gkrokon_frames_melee_attack2[] =
{
	{ FRAME_ratack1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ratack2,	NULL, 0, 0, 0, NULL, 0,	NULL },
	{ FRAME_ratack3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ratack4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ratack5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ratack6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ratack7,	NULL, 0, 0, 0, gkrokon_bite, 1, NULL },
	{ FRAME_ratack8,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gkrokon_move_melee_attack2 = ANIMMOVE(gkrokon_frames_melee_attack2, gkrokon_pause);

// MissileAttack1 - Firing spoo-goo from spoo launcher.
static const animframe_t gkrokon_frames_missile_attack1[] =
{
	{ FRAME_spoo1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spoo2,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spoo3,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spoo4,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spoo5,		NULL, 0, 0, 0, ai_charge, 0, gkrokon_spoo_attack },
};
const animmove_t gkrokon_move_missile_attack1 = ANIMMOVE(gkrokon_frames_missile_attack1, gkrokon_pause);

// MissileAttack2 - Not firing spoo-goo from spoo launcher. Referenced as ANIM_SNEEZE.
static const animframe_t gkrokon_frames_missile_attack2[] =
{
	{ FRAME_spoo1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spoo2,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spoo3,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spoo4,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spoo5,		NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t gkrokon_move_missile_attack2 = ANIMMOVE(gkrokon_frames_missile_attack2, gkrokon_pause);

// Eat1 - going from ready to eating.
static const animframe_t gkrokon_frames_eat1[] =
{
	{ FRAME_eat1,		NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_eat2,		NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_eat3,		NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_eat4,		NULL, 0, 0, 0, ai_eat, 0, NULL },
};
const animmove_t gkrokon_move_eat1 = ANIMMOVE(gkrokon_frames_eat1, gkrokon_pause);

// Eat2 - The eat cycle.
static const animframe_t gkrokon_frames_eat2[] =
{
	{ FRAME_eat5,		NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_eat6,		NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_eat7,		NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_eat8,		NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_eat9,		NULL, 0, 0, 0, ai_eat, 0, NULL },
};
const animmove_t gkrokon_move_eat2 = ANIMMOVE(gkrokon_frames_eat2, gkrokon_pause);

// Eat3 - going from eating to ready.
static const animframe_t gkrokon_frames_eat3[] =
{
	{ FRAME_EATTRANS1,	NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_EATTRANS2,	NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_EATTRANS3,	NULL, 0, 0, 0, ai_eat, 0, NULL },
	{ FRAME_EATTRANS4,	NULL, 0, 0, 0, ai_eat, 0, NULL },
};
const animmove_t gkrokon_move_eat3 = ANIMMOVE(gkrokon_frames_eat3, gkrokon_pause);

// Pain1.
static const animframe_t gkrokon_frames_pain1[] =
{
	{ FRAME_pain1,		NULL, 0, 0, 0, ai_charge, -1, NULL },
	{ FRAME_pain2,		NULL, 0, 0, 0, ai_charge, -1, NULL },
	{ FRAME_pain3,		NULL, 0, 0, 0, ai_charge, -1, NULL },
	{ FRAME_pain4,		NULL, 0, 0, 0, ai_charge, -1, NULL },
	{ FRAME_pain5,		NULL, 0, 0, 0, ai_charge, -1, NULL },
	{ FRAME_pain6,		NULL, 0, 0, 0, ai_charge, -1, NULL },
	{ FRAME_pain7,		NULL, 0, 0, 0, ai_charge, -1, NULL },
	{ FRAME_pain8,		NULL, 0, 0, 0, ai_charge, -1, NULL },
};
const animmove_t gkrokon_move_pain1 = ANIMMOVE(gkrokon_frames_pain1, gkrokon_pause);

// Death1.
static const animframe_t gkrokon_frames_death1[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death4,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death5,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death6,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death7,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death8,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death9,		NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death10,	NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death11,	NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death12,	NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death13,	NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death14,	NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death15,	NULL, 0, 0, 0, NULL,	0, NULL },
	{ FRAME_death16,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death17,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death18,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death19,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death20,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death21,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death22,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death23,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death24,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death25,	NULL, 0, 0, 0, ai_move,	2, NULL },
	{ FRAME_death26,	NULL, 0, 0, 0, ai_move,	2, NULL },
};
const animmove_t gkrokon_move_death1 = ANIMMOVE(gkrokon_frames_death1, gkrokon_dead);

// Hop.
static const animframe_t gkrokon_frames_hop[] =
{
	{ FRAME_birth5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth10,	NULL, 0, 0, 0, NULL, 0, gkrokon_walk_sound },
	{ FRAME_birth11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_birth12,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t gkrokon_move_hop1 = ANIMMOVE(gkrokon_frames_hop, gkrokon_pause);

// Delay - idling while laying on the ground.
static const animframe_t gkrokon_frames_delay[] =
{
	{ FRAME_bwait1,		NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait2,		NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait3,		NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait4,		NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait5,		gkrokon_sound, CHAN_VOICE, SND_IDLE1, ATTN_NORM, NULL, 0, gkrokon_pause },
	{ FRAME_bwait6,		NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait7,		NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait8,		NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait9,		NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait10,	NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait11,	NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait12,	NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait13,	NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
	{ FRAME_bwait14,	NULL, 0, 0, 0, NULL, 0, gkrokon_pause },
};
const animmove_t gkrokon_move_delay = ANIMMOVE(gkrokon_frames_delay, gkrokon_pause);
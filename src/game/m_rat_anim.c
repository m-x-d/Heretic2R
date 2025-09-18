//
// m_rat_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_rat_anim.h"
#include "m_rat_shared.h"
#include "g_monster.h"

// Rat Death 1 - the big death, flying backwards and flipping over.
static const animframe_t rat_frames_death1[] =
{
	{ FRAME_deathA1,	NULL, 0, 0, 0, ai_move, 0, rat_death_squeal },
	{ FRAME_deathA2,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA3,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA4,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA5,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA6,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA7,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA8,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA9,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA10,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA11,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA12,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA13,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathA14,	NULL, 0, 0, 0, ai_move, 0, NULL },
};
const animmove_t rat_move_death1 = ANIMMOVE(rat_frames_death1, M_EndDeath);

// Rat Death 2 - the little death, flipping over.
static const animframe_t rat_frames_death2[] =
{
	{ FRAME_DeathB1,	NULL, 0, 0, 0, NULL, 0, rat_death_squeal },
	{ FRAME_DeathB2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_DeathB3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_DeathB4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_DeathB5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_DeathB6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_DeathB7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_DeathB9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_DeathB9,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t rat_move_death2 = ANIMMOVE(rat_frames_death2, M_EndDeath);

// Rat Pain - backup and run away.
static const animframe_t rat_frames_pain1[] =
{
	{ FRAME_backup1,	NULL, 0, 0, 0, ai_move, -4, rat_squeal },
	{ FRAME_backup2,	NULL, 0, 0, 0, ai_move, -4, NULL },
	{ FRAME_backup3,	NULL, 0, 0, 0, ai_move, -4, NULL },
	{ FRAME_backup4,	NULL, 0, 0, 0, ai_move, -4, NULL },
	{ FRAME_backup5,	NULL, 0, 0, 0, ai_move, -4, NULL },
	{ FRAME_backup6,	NULL, 0, 0, 0, ai_move, -4, NULL },
};
const animmove_t rat_move_pain1 = ANIMMOVE(rat_frames_pain1, rat_pause);

// Rat Melee 1 - rat attacking at feet.
static const animframe_t rat_frames_melee1[] =
{
	{ FRAME_eat1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_eat2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_eat3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_eat4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_eat5,	NULL, 0, 0, 0, ai_charge, 0, rat_bite },
};
const animmove_t rat_move_melee1 = ANIMMOVE(rat_frames_melee1, rat_pause);

// Rat Melee 2 - rat attacking jumping in the air.
static const animframe_t rat_frames_melee2[] =
{
	{ FRAME_attack1,	NULL, 0, 0, 0, ai_move, 0, rat_jump },
	{ FRAME_attack2,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_attack3,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_attack4,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_attack5,	NULL, 0, 0, 0, ai_move, 0, rat_bite },
	{ FRAME_attack6,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_attack7,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_attack8,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_attack9,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_attack10,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_attack11,	NULL, 0, 0, 0, ai_move, 0, NULL },
};
const animmove_t rat_move_melee2 = ANIMMOVE(rat_frames_melee2, rat_pause);

// Rat Melee 3 - rat attacking 2.
static const animframe_t rat_frames_melee3[] =
{
	{ FRAME_eat12,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat13,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat14,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat15,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat16,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat17,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat18,	NULL, 0, 0, 0, ai_move, 0, rat_bite },
	{ FRAME_eat19,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat20,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat21,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat22,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat23,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_eat24,	NULL, 0, 0, 0, ai_move, 0, NULL },
};
const animmove_t rat_move_melee3 = ANIMMOVE(rat_frames_melee3, rat_pause);

// Rat Running 1 - rat running.
static const animframe_t rat_frames_run1[] =
{
	{ FRAME_run1,	NULL, 0, 0, 0, rat_ai_run, 10, rat_chatter },
	{ FRAME_run2,	NULL, 0, 0, 0, rat_ai_run, 20, NULL },
	{ FRAME_run3,	NULL, 0, 0, 0, rat_ai_run, 10, NULL },
	{ FRAME_run4,	NULL, 0, 0, 0, rat_ai_run, 10, NULL },
	{ FRAME_run5,	NULL, 0, 0, 0, rat_ai_run, 10, NULL },
};
const animmove_t rat_move_run1 = ANIMMOVE(rat_frames_run1, rat_run_order);

// Rat Running 2 - rat running to the left.
static const animframe_t rat_frames_run2[] =
{
	{ FRAME_run_lft1,	NULL, 0, 0, 0, rat_ai_run, 10, rat_chatter },
	{ FRAME_run_lft2,	NULL, 0, 0, 0, rat_ai_run, 10, NULL },
	{ FRAME_run_lft3,	NULL, 0, 0, 0, rat_ai_run, 10, NULL },
	{ FRAME_run_lft4,	NULL, 0, 0, 0, rat_ai_run, 10, NULL },
};
const animmove_t rat_move_run2 = ANIMMOVE(rat_frames_run2, rat_run_order);

// Rat Running 3 - rat running to the right.
static const animframe_t rat_frames_run3[] =
{
	{ FRAME_run_rt1,	NULL, 0, 0, 0, rat_ai_run, 10, rat_chatter },
	{ FRAME_run_rt2,	NULL, 0, 0, 0, rat_ai_run, 10, NULL },
	{ FRAME_run_rt3,	NULL, 0, 0, 0, rat_ai_run, 10, NULL },
	{ FRAME_run_rt4,	NULL, 0, 0, 0, rat_ai_run, 10, rat_run_order },
};
const animmove_t rat_move_run3 = ANIMMOVE(rat_frames_run3, NULL);

// Rat Walking.
static const animframe_t rat_frames_walk1[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, ai_walk, 4, rat_chatter },
	{ FRAME_walk2,	NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, ai_walk, 4, NULL },
};
const animmove_t rat_move_walk1 = ANIMMOVE(rat_frames_walk1, NULL);

// Rat Stand 1.
static const animframe_t rat_frames_stand1[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, rat_ai_stand,	0, rat_chatter },
	{ FRAME_idle2,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_idle3,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_idle4,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_idle5,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_idle6,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_idle7,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_idle8,	NULL, 0, 0, 0, NULL,			0, NULL },
};
const animmove_t rat_move_stand1 = ANIMMOVE(rat_frames_stand1, rat_stand_order);

// Rat Stand 2 - rising up to sit on haunches.
static const animframe_t rat_frames_stand2[] =
{
	{ FRAME_haunch1,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch2,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch3,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch4,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch5,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch6,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch7,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch8,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch9,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch10,	NULL, 0, 0, 0, NULL,			0, NULL },
};
const animmove_t rat_move_stand2 = ANIMMOVE(rat_frames_stand2, rat_stand_order);

// Rat Stand 3 - sitting on haunches.
static const animframe_t rat_frames_stand3[] =
{
	{ FRAME_haunch12,	NULL, 0, 0, 0, rat_ai_stand,	0, rat_chatter },
	{ FRAME_haunch13,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch14,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch15,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch16,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch17,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch18,	NULL, 0, 0, 0, NULL,			0, NULL },
};
const animmove_t rat_move_stand3 = ANIMMOVE(rat_frames_stand3, rat_stand_order);

// Rat Stand 4 - sitting on haunches, looking left.
static const animframe_t rat_frames_stand4[] =
{
	{ FRAME_haunch20,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch21,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch22,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch23,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch24,	NULL, 0, 0, 0, NULL,			0, NULL },
};
const animmove_t rat_move_stand4 = ANIMMOVE(rat_frames_stand4, rat_stand_order);

// Rat Stand 5 - sitting on haunches, look right.
static const animframe_t rat_frames_stand5[] =
{
	{ FRAME_haunch26,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch27,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch28,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch29,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch30,	NULL, 0, 0, 0, NULL,			0, NULL },
};
const animmove_t rat_move_stand5 = ANIMMOVE(rat_frames_stand5, rat_stand_order);

// Rat Stand 6 - sitting on haunches, scratch left.
static const animframe_t rat_frames_stand6[] =
{
	{ FRAME_haunch32,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch33,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch34,	NULL, 0, 0, 0, NULL,			0, rat_scratch },
	{ FRAME_haunch35,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch36,	NULL, 0, 0, 0, NULL,			0, NULL },
};
const animmove_t rat_move_stand6 = ANIMMOVE(rat_frames_stand6, rat_stand_order);

// Rat Stand 7 - sitting on haunches, scratch right.
static const animframe_t rat_frames_stand7[] =
{
	{ FRAME_haunch38,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch39,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch40,	NULL, 0, 0, 0, NULL,			0, rat_scratch },
	{ FRAME_haunch41,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch42,	NULL, 0, 0, 0, NULL,			0, NULL },
};
const animmove_t rat_move_stand7 = ANIMMOVE(rat_frames_stand7, rat_stand_order);

// Rat Stand 8 - from haunches, dropping down to ground
static const animframe_t rat_frames_stand8[] =
{
	{ FRAME_haunch44,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch45,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch46,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch47,	NULL, 0, 0, 0, rat_ai_stand,	0, NULL },
	{ FRAME_haunch48,	NULL, 0, 0, 0, NULL,			0, NULL },
	{ FRAME_haunch49,	NULL, 0, 0, 0, NULL,			0, NULL },
};
const animmove_t rat_move_stand8 = ANIMMOVE(rat_frames_stand8, rat_stand_order); //mxd. numframes:5 in original logic.

// Rat Watch 1 - hiss while on all fours.
static const animframe_t rat_frames_watch1[] =
{
	{ FRAME_hiss14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss15,	NULL, 0, 0, 0, NULL, 0, rat_hiss },
	{ FRAME_hiss16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss29,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t rat_move_watch1 = ANIMMOVE(rat_frames_watch1, rat_eat_order);

// Rat Watch 2 - stand up and hiss then go back to all fours.
static const animframe_t rat_frames_watch2[] =
{
	{ FRAME_hiss1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_hiss13,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t rat_move_watch2 = ANIMMOVE(rat_frames_watch2, rat_eat_order);

// Rat Eat 1 - bite down low.
static const animframe_t rat_frames_eat1[] =
{
	{ FRAME_eat1,	NULL, 0, 0, 0, rat_ai_eat,	0, NULL },
	{ FRAME_eat2,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat3,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat4,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat5,	NULL, 0, 0, 0, NULL,		0, rat_chew },
	{ FRAME_eat6,	NULL, 0, 0, 0, rat_ai_eat,	0, NULL },
	{ FRAME_eat7,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat8,	NULL, 0, 0, 0, NULL,		0, rat_chew },
	{ FRAME_eat9,	NULL, 0, 0, 0, NULL,		0, NULL },
};
const animmove_t rat_move_eat1 = ANIMMOVE(rat_frames_eat1, rat_eat_order);

// Rat Eat 2 - bite low and tear up.
static const animframe_t rat_frames_eat2[] =
{
	{ FRAME_eat12,	NULL, 0, 0, 0, rat_ai_eat,	0, NULL },
	{ FRAME_eat13,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat14,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat15,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat16,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat17,	NULL, 0, 0, 0, NULL,		0, rat_chew },
	{ FRAME_eat18,	NULL, 0, 0, 0, rat_ai_eat,	0, NULL },
	{ FRAME_eat19,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat20,	NULL, 0, 0, 0, NULL,		0, rat_swallow },
	{ FRAME_eat21,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat22,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat23,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat24,	NULL, 0, 0, 0, NULL,		0, NULL },
};
const animmove_t rat_move_eat2 = ANIMMOVE(rat_frames_eat2, rat_eat_order);

// Rat Eat 3 - bite and pull back a little.
static const animframe_t rat_frames_eat3[] =
{
	{ FRAME_eat9,	NULL, 0, 0, 0, rat_ai_eat,	0, NULL },
	{ FRAME_eat10,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_eat11,	NULL, 0, 0, 0, NULL,		0, rat_chew },
	{ FRAME_eat12,	NULL, 0, 0, 0, NULL,		0, NULL },
};
const animmove_t rat_move_eat3 = ANIMMOVE(rat_frames_eat3, rat_eat_order);
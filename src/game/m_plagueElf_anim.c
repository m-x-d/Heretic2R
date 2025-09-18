//
// m_plagueElf_anim.c -- all of the anim frames that used to live in m_plagueElf.h
//
// Copyright 1998 Raven Software
//

#include "m_plagueElf_anim.h"
#include "m_plagueElf_shared.h"
#include "c_ai.h"
#include "mg_ai.h" //mxd
#include "g_monster.h"

// Plague Elf Death 1 - the big death, flying backwards and flipping over.
static const animframe_t plagueElf_frames_death1[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL, 0, plagueelf_death_squeal },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death7,		NULL, 0, 0, 0, NULL, 0, MG_SetNoBlocking },
	{ FRAME_death8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death13,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_death1 = ANIMMOVE(plagueElf_frames_death1, M_EndDeath);

// Plague Elf Death 2.
static const animframe_t plagueElf_frames_death2[] =
{
	{ FRAME_deathb1,	NULL, 0, 0, 0, NULL, 0, plagueelf_death_squeal },
	{ FRAME_deathb2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb7,	NULL, 0, 0, 0, NULL, 0, MG_SetNoBlocking },
	{ FRAME_deathb8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb13,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_death2 = ANIMMOVE(plagueElf_frames_death2, M_EndDeath); //mxd. numframes:14 in original logic.

// Plague Elf Death 3.
static const animframe_t plagueElf_frames_death3[] =
{
	{ FRAME_deathc1,	NULL, 0, 0, 0, NULL, 0, plagueelf_death_squeal },
	{ FRAME_deathc2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc7,	NULL, 0, 0, 0, NULL, 0, MG_SetNoBlocking },
	{ FRAME_deathc8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc13,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_death3 = ANIMMOVE(plagueElf_frames_death3, M_EndDeath); //mxd. numframes:14 in original logic.

// Plague Elf Death 4.
static const animframe_t plagueElf_frames_death4[] =
{
	{ FRAME_deathd1,	NULL, 0, 0, 0, NULL, 0, plagueelf_death_squeal },
	{ FRAME_deathd2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd7,	NULL, 0, 0, 0, NULL, 0, MG_SetNoBlocking },
	{ FRAME_deathd8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd13,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_death4 = ANIMMOVE(plagueElf_frames_death4, M_EndDeath); //mxd. numframes:14 in original logic.

// Plague Elf Pain - plagueElf gets hit <<-- FIXME: this is not a real animation, this is recycling other anims.
static const animframe_t plagueElf_frames_pain1[] =
{
	{ FRAME_painA1,		NULL, 0, 0, 0, NULL, 0, plagueelf_squeal },
	{ FRAME_painA2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painA3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painA4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painA5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painA6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_painA7,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_pain1 = ANIMMOVE(plagueElf_frames_pain1, plagueelf_pause);

// Plague Elf Melee 1 - plagueElf attacking one hand forehand swing.
static const animframe_t plagueElf_frames_melee1[] =
{
	{ FRAME_attckA1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckA2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckA3,	NULL, 0, 0, 0, ai_charge, 0, plagueelf_attack },
	{ FRAME_attckA4,	NULL, 0, 0, 0, ai_charge, 0, plagueelf_strike },
	{ FRAME_attckA5,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckA6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckA7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t plagueElf_move_melee1 = ANIMMOVE(plagueElf_frames_melee1, plagueelf_pause);

// Plague Elf Melee 2 - plagueElf attacking two handed chop.
static const animframe_t plagueElf_frames_melee2[] =
{
	{ FRAME_attckB1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckB2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckB3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckB4,	NULL, 0, 0, 0, ai_charge, 0, plagueelf_attack },
	{ FRAME_attckB5,	NULL, 0, 0, 0, ai_charge, 0, plagueelf_strike },
	{ FRAME_attckB6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckB7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckB8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckB9,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t plagueElf_move_melee2 = ANIMMOVE(plagueElf_frames_melee2, plagueelf_pause);

// Plague Elf Missile.
static const animframe_t plagueElf_frames_missile[] =
{
	{ FRAME_attckA1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckA2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckA3,	NULL, 0, 0, 0, ai_charge, 0, plagueelf_attack },
	{ FRAME_attckA4,	NULL, 0, 0, 0, ai_charge, 0, plagueelf_spell },
	{ FRAME_attckA5,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckA6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_attckA7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t plagueElf_move_missile = ANIMMOVE(plagueElf_frames_missile, plagueelf_pause);

// Plague Elf Running.
static const animframe_t plagueElf_frames_run1[] =
{
	{ FRAME_runA1,		NULL, 0, 0, 0, MG_AI_Run, 12, plagueelf_growl },
	{ FRAME_runA2,		NULL, 0, 0, 0, MG_AI_Run, 13, plagueelf_pause },
	{ FRAME_runA3,		NULL, 0, 0, 0, MG_AI_Run, 14, plagueelf_pause },
	{ FRAME_runA4,		NULL, 0, 0, 0, MG_AI_Run, 14, plagueelf_pause },
	{ FRAME_runA5,		NULL, 0, 0, 0, MG_AI_Run, 14, plagueelf_pause },
	{ FRAME_runA6,		NULL, 0, 0, 0, MG_AI_Run, 14, plagueelf_pause },
	{ FRAME_runA7,		NULL, 0, 0, 0, MG_AI_Run, 14, plagueelf_pause },
	{ FRAME_runA8,		NULL, 0, 0, 0, MG_AI_Run, 11, plagueelf_pause },
};
const animmove_t plagueElf_move_run1 = ANIMMOVE(plagueElf_frames_run1, plagueelf_pause);

// Plague Elf Running & Attack - plagueElf running n swinging.
static const animframe_t plagueElf_frames_runatk1[] =
{
	{ FRAME_runatk1,	NULL, 0, 0, 0, ai_charge, 10, NULL },
	{ FRAME_runatk2,	NULL, 0, 0, 0, ai_charge, 11, plagueelf_attack },
	{ FRAME_runatk3,	NULL, 0, 0, 0, ai_charge, 12, plagueelf_strike },
	{ FRAME_runatk4,	NULL, 0, 0, 0, ai_charge, 12, NULL },
	{ FRAME_runatk5,	NULL, 0, 0, 0, ai_charge, 12, NULL },
	{ FRAME_runatk6,	NULL, 0, 0, 0, ai_charge, 12, NULL },
	{ FRAME_runatk7,	NULL, 0, 0, 0, ai_charge, 12, NULL },
	{ FRAME_runatk8,	NULL, 0, 0, 0, ai_charge,  9, NULL },
};
const animmove_t plagueElf_move_runatk1 = ANIMMOVE(plagueElf_frames_runatk1, plagueelf_pause);

// Plague Elf Landing.
static const animframe_t plagueElf_frames_land[] =
{
	{ FRAME_recover1,	NULL, 0, 0, 0, NULL, 0, plagueelf_land },
	{ FRAME_recover2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_recover5,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_land = ANIMMOVE(plagueElf_frames_land, plagueelf_pause);

// Plague Elf in air.
static const animframe_t plagueElf_frames_inair[] =
{
	{ FRAME_jump20,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, 0 },
};
const animmove_t plagueElf_move_inair = ANIMMOVE(plagueElf_frames_inair, NULL);

// Plague Elf jump from buoy.
static const animframe_t plagueElf_frames_fjump[] =
{
	{ FRAME_jump1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump2,		NULL, 0, 0, 0, NULL, 0, plagueelf_growl },
	{ FRAME_jump3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump5,		NULL, 0, 0, 0, NULL, 0, plagueelf_apply_jump },
	{ FRAME_jump6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump10,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump11,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump12,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump13,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump14,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump15,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump16,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump17,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump18,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump19,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump20,		NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t plagueElf_move_fjump = ANIMMOVE(plagueElf_frames_fjump, plagueelf_inair_go);

// Plague Elf Walking 1.
static const animframe_t plagueElf_frames_walk1[] =
{
	{ FRAME_walkA1,		NULL, 0, 0, 0, ai_walk, 6, plagueelf_growl },
	{ FRAME_walkA2,		NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA3,		NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA4,		NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA5,		NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA6,		NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA7,		NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA8,		NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA9,		NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA10,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA11,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_walkA12,	NULL, 0, 0, 0, ai_walk, 6, NULL },
};
const animmove_t plagueElf_move_walk1 = ANIMMOVE(plagueElf_frames_walk1, plagueelf_pause);

// Plague Elf Walking 2.
static const animframe_t plagueElf_frames_walk2[] =
{
	{ FRAME_walkA1,		NULL, 0, 0, 0, ai_walk, 4, plagueelf_growl },
	{ FRAME_walkA2,		NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA3,		NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA4,		NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA5,		NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA6,		NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA7,		NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA8,		NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA9,		NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA10,	NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA11,	NULL, 0, 0, 0, ai_walk, 4, NULL },
	{ FRAME_walkA12,	NULL, 0, 0, 0, ai_walk, 4, NULL },
};
const animmove_t plagueElf_move_walk2 = ANIMMOVE(plagueElf_frames_walk2, plagueelf_pause);

// Plague Elf Standing.
static const animframe_t plagueElf_frames_stand1[] =
{
	{ FRAME_shake1,		NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t plagueElf_move_stand1 = ANIMMOVE(plagueElf_frames_stand1, plagueelf_pause);

// Plague Elf Shake - standing and having spasms.
static const animframe_t plagueElf_frames_shakeA1[] =
{
	{ FRAME_shake1,		NULL, 0, 0, 0, ai_stand, 0, plagueelf_growl },
	{ FRAME_shake2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake4,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_shake5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake7,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_shake8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_shake11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_shake15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_shake18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_shake21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake23,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_shake24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake25,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_shake1 = ANIMMOVE(plagueElf_frames_shakeA1, plagueelf_pause);

// Plague Elf Fist - beating the wall.
static const animframe_t plagueElf_frames_fist1[] =
{
	{ FRAME_fist1,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fist2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist6,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_fist1 = ANIMMOVE(plagueElf_frames_fist1, plagueelf_pause);

// Plague Elf Leaning - swooning against the wall.
static const animframe_t plagueElf_frames_lean1[] =
{
	{ FRAME_lean1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean3,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_lean4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean9,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_lean10,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean11,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean12,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean13,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean14,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean15,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean16,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean17,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean18,		NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_lean19,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean20,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean21,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean22,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean23,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean24,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean25,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_lean1 = ANIMMOVE(plagueElf_frames_lean1, plagueelf_pause);

// Plague Elf Delay.
static const animframe_t plagueElf_frames_delay[] =
{
	{ FRAME_shake1,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake2,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake3,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake4,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake5,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake6,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake7,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake8,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake9,		NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake10,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake11,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake12,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake13,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake14,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake15,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake16,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake17,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake18,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake19,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake20,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake21,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake22,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake23,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake24,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
	{ FRAME_shake25,	NULL, 0, 0, 0, NULL, 0, plagueelf_pause },
};
const animmove_t plagueElf_delay = ANIMMOVE(plagueElf_frames_delay, plagueelf_pause);

// Plague Elf Knockback Death Start.
static const animframe_t plagueElf_frames_kdeath_go[] =
{
	{ FRAME_death1,		NULL, 0, 0, 0, NULL, 0, plagueelf_death_squeal },
	{ FRAME_death2,		NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death3,		NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_loop },
};
const animmove_t plagueElf_move_kdeath_go = ANIMMOVE(plagueElf_frames_kdeath_go, NULL);

// Plague Elf Knockback Death Loop.
static const animframe_t plagueElf_frames_kdeath_loop[] =
{
	{ FRAME_death4,		NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
};
const animmove_t plagueElf_move_kdeath_loop = ANIMMOVE(plagueElf_frames_kdeath_loop, NULL);

// Plague Elf Knockback Death End.
static const animframe_t plagueElf_frames_kdeath_end[] =
{
	{ FRAME_death5,		NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death6,		NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death7,		NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death8,		NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death9,		NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death10,	NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death11,	NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death12,	NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
	{ FRAME_death13,	NULL, 0, 0, 0, NULL, 0, plagueelf_knockback_death_check_land },
};
const animmove_t plagueElf_move_kdeath_end = ANIMMOVE(plagueElf_frames_kdeath_end, M_EndDeath);

// Plague Elf Crazy A - running with hands in the air.
static const animframe_t plagueElf_frames_crazy_A[] =
{
	{ FRAME_crazyA1,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyA3,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyA5,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyA7,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyA9,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyA11,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
};
const animmove_t plagueElf_crazy_A = ANIMMOVE(plagueElf_frames_crazy_A, plagueelf_pause);

// Plague Elf Crazy B - running with hands in the air, looking down.
static const animframe_t plagueElf_frames_crazy_B[] =
{
	{ FRAME_crazyB1,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyB3,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyB5,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyB7,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyB9,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_crazyB11,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
};
const animmove_t plagueElf_crazy_B = ANIMMOVE(plagueElf_frames_crazy_B, plagueelf_pause);

// Plague Elf Cursing - plague elf doing 'old man yells at cloud' impression.
static const animframe_t plagueElf_frames_cursing[] =
{
	{ FRAME_cursing1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing9,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing13,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing17,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing21,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing25,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing29,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing33,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing37,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing41,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing45,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing49,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing53,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing57,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_cursing61,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t plagueElf_cursing = ANIMMOVE(plagueElf_frames_cursing, plagueelf_run_go);

// Plague Elf Pointing.
static const animframe_t plagueElf_frames_point[] =
{
	{ FRAME_point1,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point2,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point3,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point5,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point6,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point7,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point9,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point10,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point11,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point13,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point14,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point15,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point17,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point18,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point19,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point21,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point22,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point23,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point25,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point26,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point27,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point29,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_point30,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t plagueElf_point = ANIMMOVE(plagueElf_frames_point, plagueelf_run_go);

// Plague Elf Scared.
static const animframe_t plagueElf_frames_scared[] =
{
	{ FRAME_scared1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared5,	NULL, 0, 0, 0, NULL, 0, plagueelf_check_distance },
	{ FRAME_scared6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared10,	NULL, 0, 0, 0, NULL, 0, plagueelf_check_distance },
	{ FRAME_scared11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared15,	NULL, 0, 0, 0, NULL, 0, plagueelf_check_distance },
	{ FRAME_scared16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared20,	NULL, 0, 0, 0, NULL, 0, plagueelf_check_distance },
	{ FRAME_scared21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared25,	NULL, 0, 0, 0, NULL, 0, plagueelf_check_distance },
	{ FRAME_scared26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared30,	NULL, 0, 0, 0, NULL, 0, plagueelf_check_distance },
	{ FRAME_scared31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared33,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared34,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared35,	NULL, 0, 0, 0, NULL, 0, plagueelf_check_distance },
	{ FRAME_scared36,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared37,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared38,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_scared39,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_scared = ANIMMOVE(plagueElf_frames_scared, plagueelf_pause);

// Cinematic Plague Elf Idle 1 - standing and having spasms.
static const animframe_t plagueElf_frames_c_idle1[] =
{
	{ FRAME_shake1,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake2,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake3,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake4,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake5,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake6,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake7,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake8,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake9,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake14,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake15,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake16,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake17,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake18,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake19,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake20,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake21,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake22,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake23,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake24,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shake25,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_idle1 = ANIMMOVE(plagueElf_frames_c_idle1, ai_c_cycleend);

// Cinematic Plague Elf Idle 2 - swooning against the wall.
static const animframe_t plagueElf_frames_c_idle2[] =
{
	{ FRAME_lean1,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean2,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean3,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean4,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean5,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean6,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean7,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean8,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean9,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean10,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean11,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean12,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean13,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean14,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean15,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean16,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean17,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean18,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean19,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean20,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean21,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean22,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean23,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean24,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_lean25,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_idle2 = ANIMMOVE(plagueElf_frames_c_idle2, ai_c_cycleend);

// Cinematic Plague Elf Idle 3 - standing, shaking fist in the air.
static const animframe_t plagueElf_frames_c_idle3[] =
{
	{ FRAME_fist1,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist2,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist3,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist4,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist5,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fist6,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_idle3 = ANIMMOVE(plagueElf_frames_c_idle3, ai_c_cycleend);

// Cinematic Plague Elf Walking.
static const animframe_t plagueElf_frames_c_walk[] =
{
	{ FRAME_walkA1,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA2,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA3,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA4,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA5,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA6,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA7,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA8,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA9,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA10,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA11,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA12,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_walk = ANIMMOVE(plagueElf_frames_c_walk, ai_c_cycleend);

// Cinematic Plague Elf Walking 2.
static const animframe_t plagueElf_frames_c_walk2[] =
{
	{ FRAME_walkA1,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA2,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA3,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA4,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA5,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA6,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA7,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA8,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA9,		ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA10,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA11,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_walkA12,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_walk2 = ANIMMOVE(plagueElf_frames_c_walk2, ai_c_cycleend);

// Cinematic Plague Elf Running.
static const animframe_t plagueElf_frames_c_run[] =
{
	{ FRAME_runA1,		ai_c_move, 12, 0, 0, NULL, 0, NULL },
	{ FRAME_runA2,		ai_c_move, 13, 0, 0, NULL, 0, NULL },
	{ FRAME_runA3,		ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_runA4,		ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_runA5,		ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_runA6,		ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_runA7,		ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_runA8,		ai_c_move, 11, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_run = ANIMMOVE(plagueElf_frames_c_run, ai_c_cycleend);

// Cinematic Plague Elf Attack 1 - running & swinging.
static const animframe_t plagueElf_frames_c_attack1[] =
{
	{ FRAME_runatk1,	ai_c_move, 10, 0, 0, NULL, 0, NULL },
	{ FRAME_runatk2,	ai_c_move, 11, 0, 0, NULL, 0, NULL },
	{ FRAME_runatk3,	ai_c_move, 12, 0, 0, NULL, 0, NULL },
	{ FRAME_runatk4,	ai_c_move, 12, 0, 0, NULL, 0, NULL },
	{ FRAME_runatk5,	ai_c_move, 12, 0, 0, NULL, 0, NULL },
	{ FRAME_runatk6,	ai_c_move, 12, 0, 0, NULL, 0, NULL },
	{ FRAME_runatk7,	ai_c_move, 12, 0, 0, NULL, 0, NULL },
	{ FRAME_runatk8,	ai_c_move,  9, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_attack1 = ANIMMOVE(plagueElf_frames_c_attack1, ai_c_cycleend);

// Cinematic Plague Elf Attack 2 - one-handed forehand swing attack.
static const animframe_t plagueElf_frames_c_attack2[] =
{
	{ FRAME_attckA1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_attack2 = ANIMMOVE(plagueElf_frames_c_attack2, ai_c_cycleend);

// Cinematic Plague Elf Attack 3 - two-handed chop attack.
static const animframe_t plagueElf_frames_c_attack3[] =
{
	{ FRAME_attckB1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckB2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckB3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckB4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckB5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckB6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckB7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckB8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckB9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_attack3 = ANIMMOVE(plagueElf_frames_c_attack3, ai_c_cycleend);

// Cinematic Plague Elf Attack 4.
static const animframe_t plagueElf_c_frames_attack4[] =
{
	{ FRAME_attckA1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA4,	ai_c_move, 0, 0, 0, NULL, 0, plagueelf_cinematic_spell },
	{ FRAME_attckA5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_attckA7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_attack4 = ANIMMOVE(plagueElf_c_frames_attack4,ai_c_cycleend);

// Cinematic Plague Elf Death 1 - the big death, flying backwards and flipping over.
static const animframe_t plagueElf_frames_c_death1[] =
{
	{ FRAME_death1,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death2,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death3,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death4,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death5,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death6,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death7,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death8,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death9,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_death1 = ANIMMOVE(plagueElf_frames_c_death1,  ai_c_cycleend);

// Cinematic Plague Elf Death 2.
static const animframe_t plagueElf_frames_c_death2[] =
{
	{ FRAME_deathb1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathb13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_death2 = ANIMMOVE(plagueElf_frames_c_death2, ai_c_cycleend);

// Cinematic Plague Elf Death 3.
static const animframe_t plagueElf_frames_c_death3[] =
{
	{ FRAME_deathc1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathc13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_death3 = ANIMMOVE(plagueElf_frames_c_death3, ai_c_cycleend);

// Cinematic Plague Elf Death 4.
static const animframe_t plagueElf_frames_c_death4[] =
{
	{ FRAME_deathd1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathd13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_death4 = ANIMMOVE(plagueElf_frames_c_death4,ai_c_cycleend);

// Cinematic Plague Elf Pain <<-- FIXME: this is not a real animation, this is recycling other anims.
static const animframe_t plagueElf_frames_c_pain1[] =
{
	{ FRAME_death3,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death2,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death1,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t plagueElf_move_c_pain1 = ANIMMOVE(plagueElf_frames_c_pain1, ai_c_cycleend);
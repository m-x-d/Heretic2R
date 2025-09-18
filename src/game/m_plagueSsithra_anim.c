//
// m_plagueSsitra_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_plaguessithra_anim.h"
#include "m_plaguesSithra_shared.h"
#include "mg_ai.h" //mxd
#include "g_monster.h"
#include "g_local.h"

// Plague Ssithra Idle - standing and looking around.
static const animframe_t ssithra_frames_idle1[] =
{
	{ FRAME_idle01,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle02,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle03,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle04,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle05,	NULL, 0, 0, 0, ai_stand, 0, ssithra_growl_sound },
	{ FRAME_idle06,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle07,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle08,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle09,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle16,	NULL, 0, 0, 0, ai_stand, 0, ssithra_growl_sound },
	{ FRAME_idle17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle19,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle21,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle22,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle23,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle24,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle25,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle26,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle27,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle28,	NULL, 0, 0, 0, ai_stand, 0, ssithra_growl_sound },
	{ FRAME_idle29,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle30,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle31,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle32,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle33,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle34,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle35,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle36,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle37,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle38,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle39,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle40,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t ssithra_move_idle1 = ANIMMOVE(ssithra_frames_idle1, ssithra_check_mood);

// Plague Ssithra Walk - walking along.
static const animframe_t ssithra_frames_walk1[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk2,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, ai_walk, 3, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk7,	NULL, 0, 0, 0, ai_walk, 5, ssithra_growl_sound },
	{ FRAME_walk8,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk9,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk10,	NULL, 0, 0, 0, ai_walk, 7, NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk12,	NULL, 0, 0, 0, ai_walk, 5, NULL },
	{ FRAME_walk13,	NULL, 0, 0, 0, ai_walk, 2, NULL },
	{ FRAME_walk14,	NULL, 0, 0, 0, ai_walk, 5, NULL },
};
const animmove_t ssithra_move_walk1 = ANIMMOVE(ssithra_frames_walk1, ssithra_check_mood);

// Plague Ssithra Backpedal.
static const animframe_t ssithra_frames_backpedal1[] =
{
	{ FRAME_backpedal1,		NULL, 0, 0, 0, ai_charge2, -5, NULL },
	{ FRAME_backpedal2,		NULL, 0, 0, 0, ai_charge2, -5, NULL },
	{ FRAME_backpedal3,		NULL, 0, 0, 0, ai_charge2, -5, ssithra_arrow },
	{ FRAME_backpedal4,		NULL, 0, 0, 0, ai_charge2, -7, NULL },
	{ FRAME_backpedal5,		NULL, 0, 0, 0, ai_charge2, -7, NULL },
	{ FRAME_backpedal6,		NULL, 0, 0, 0, ai_charge2, -7, NULL },
	{ FRAME_backpedal7,		NULL, 0, 0, 0, ai_charge2, -7, ssithra_arrow },
	{ FRAME_backpedal8,		NULL, 0, 0, 0, ai_charge2, -5, NULL },
	{ FRAME_backpedal9,		NULL, 0, 0, 0, ai_charge2, -5, NULL },
	{ FRAME_backpedal10,	NULL, 0, 0, 0, ai_charge2, -5, NULL },
};
const animmove_t ssithra_move_backpedal1 = ANIMMOVE(ssithra_frames_backpedal1, ssithra_check_mood);

// Plague Ssithra Bound.
static const animframe_t ssithra_frames_bound1[] =
{
	{ FRAME_bound09,	NULL, 0, 0, 0, ssithra_ai_run, 16, NULL },
	{ FRAME_bound10,	NULL, 0, 0, 0, ssithra_ai_run, 16, NULL },
	{ FRAME_bound11,	NULL, 0, 0, 0, ssithra_ai_run, 20, NULL },
	{ FRAME_bound12,	NULL, 0, 0, 0, ssithra_ai_run, 20, NULL },
	{ FRAME_bound13,	NULL, 0, 0, 0, ssithra_ai_run, 24, NULL },
	{ FRAME_bound14,	NULL, 0, 0, 0, ssithra_ai_run, 20, NULL },
	{ FRAME_bound15,	NULL, 0, 0, 0, ssithra_ai_run, 16, NULL },
	{ FRAME_bound16,	NULL, 0, 0, 0, ssithra_ai_run, 12, NULL },
};
const animmove_t ssithra_move_bound1 = ANIMMOVE(ssithra_frames_bound1, ssithra_check_mood);

// Plague Ssithra Death A.
static const animframe_t ssithra_frames_death_a1[] =
{
	{ FRAME_death_a1,	ssithra_sound, SND_DIE, CHAN_VOICE, 0, NULL, 0, NULL },
	{ FRAME_death_a2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a4,	NULL, 0, 0, 0, NULL, 0, MG_SetNoBlocking },
	{ FRAME_death_a5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_a12,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_death_a1 = ANIMMOVE(ssithra_frames_death_a1, ssithra_dead);

// Plague Ssithra Death B.
static const animframe_t ssithra_frames_death_b1[] =
{
	{ FRAME_death_b1,	ssithra_sound, SND_DIE, CHAN_VOICE, 0, NULL, 0, NULL },
	{ FRAME_death_b2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b11,	NULL, 0, 0, 0, NULL, 0, MG_SetNoBlocking },
	{ FRAME_death_b12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b33,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b34,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b35,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b36,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_death_b1 = ANIMMOVE(ssithra_frames_death_b1, ssithra_dead);

// Plague Ssithra Dive.
static const animframe_t ssithra_frames_dive1[] =
{
	{ FRAME_dive1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive4,	ssithra_jump, 400, 100, 0, NULL, 0, NULL },
	{ FRAME_dive5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dive9,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive10,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive11,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive12,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive13,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive14,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive15,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive16,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive17,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive18,	NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_entry_splash }, //FIXME: check to make sure hit water.
	{ FRAME_dive19,	NULL, 0, 0, 0, ai_move, 22, ssithra_try_spawn_water_entry_splash }, // In water, go forward.
	{ FRAME_dive20,	NULL, 0, 0, 0, ai_move, 20, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive21,	NULL, 0, 0, 0, ai_move, 17, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive22,	NULL, 0, 0, 0, ai_move, 15, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive23,	NULL, 0, 0, 0, ai_move, 12, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive24,	NULL, 0, 0, 0, ai_move,  9, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive25,	NULL, 0, 0, 0, ai_move,  6, ssithra_try_spawn_water_entry_splash },
	{ FRAME_dive26,	NULL, 0, 0, 0, ai_move,  3, ssithra_try_spawn_water_entry_splash },
};
const animmove_t ssithra_move_dive1 = ANIMMOVE(ssithra_frames_dive1, ssithra_decide_swimforward);

// Plague Ssithra Unduck.
static const animframe_t ssithra_frames_unduck[] =
{
	{ FRAME_duckshoot3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duckshoot2,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_uncrouch },
	{ FRAME_duckshoot1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t ssithra_move_unduck = ANIMMOVE(ssithra_frames_unduck, ssithra_decide_run);

// Plague Ssithra Duck Loop.
static const animframe_t ssithra_frames_duckloop[] =
{
	{ FRAME_duckshoot6,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_duckloop = ANIMMOVE(ssithra_frames_duckloop, ssithra_check_unduck);

// Plague Ssithra Duck Shoot.
static const animframe_t ssithra_frames_duckshoot1[] =
{
	{ FRAME_duckshoot1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duckshoot2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duckshoot3,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_crouch },
	{ FRAME_duckshoot4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duckshoot5,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_start_duck_arrow },
	{ FRAME_duckshoot5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duckshoot5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duckshoot5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duckshoot5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duckshoot5,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_duck_arrow },
	{ FRAME_duckshoot6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t ssithra_move_duckshoot1 = ANIMMOVE(ssithra_frames_duckshoot1, ssithra_check_unduck);

// Plague Ssithra Duck.
static const animframe_t ssithra_frames_duck1[] =
{
	// Oops, duckframes same as duckshoot?
	{ FRAME_duckshoot1,	ssithra_sound, SND_GROWL3, CHAN_VOICE, ATTN_IDLE, NULL, 0, ssithra_growl_sound },
	{ FRAME_duckshoot2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_duckshoot3,	NULL, 0, 0, 0, NULL, 0, ssithra_crouch },
	{ FRAME_duckshoot4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_duckshoot5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_duckshoot6,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_duck1 = ANIMMOVE(ssithra_frames_duck1, ssithra_decide_run);

// Plague Ssithra Gallop.
static const animframe_t ssithra_frames_gallop1[] =
{
	{ FRAME_gallop1,	ssithra_jump, 100, 50, 0, ssithra_ai_run, 20, NULL },
	{ FRAME_gallop2,	NULL, 0, 0, 0, ssithra_ai_run, 24, NULL },
	{ FRAME_gallop3,	NULL, 0, 0, 0, ssithra_ai_run, 32, NULL },
	{ FRAME_gallop4,	NULL, 0, 0, 0, ssithra_ai_run, 30, NULL },
	{ FRAME_gallop5,	NULL, 0, 0, 0, ssithra_ai_run, 28, NULL },
	{ FRAME_gallop6,	NULL, 0, 0, 0, ssithra_ai_run, 26, NULL },
	{ FRAME_gallop7,	NULL, 0, 0, 0, ssithra_ai_run, 24, NULL },
	{ FRAME_gallop8,	NULL, 0, 0, 0, ssithra_ai_run, 22, NULL },
};
const animmove_t ssithra_move_gallop1 = ANIMMOVE(ssithra_frames_gallop1, ssithra_decide_run);

// Plague Ssithra Jump From Buoy.
static const animframe_t ssithra_frames_fjump[] =
{
	{ FRAME_bound09,	NULL, 0, 0, 0, NULL, 0, ssithra_apply_jump },
	{ FRAME_bound10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bound11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bound12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bound13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bound14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bound15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_bound16,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_fjump = ANIMMOVE(ssithra_frames_fjump, ssithra_decide_run);

// Plague Ssithra Idle Basic.
static const animframe_t ssithra_frames_idlebasic1[] =
{
	{ FRAME_idlebasic01,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic02,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic03,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic04,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic05,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic06,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic07,	NULL, 0, 0, 0, ai_stand, 0, ssithra_growl_sound },
	{ FRAME_idlebasic08,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic09,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic16,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic19,	NULL, 0, 0, 0, ai_stand, 0, ssithra_growl_sound },
	{ FRAME_idlebasic20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic21,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic22,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic23,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic24,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic25,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic26,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic27,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic28,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic29,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic30,	NULL, 0, 0, 0, ai_stand, 0, ssithra_growl_sound },
	{ FRAME_idlebasic31,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic32,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic33,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic34,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic35,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic36,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic37,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic38,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic39,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic40,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t ssithra_move_idlebasic1 = ANIMMOVE(ssithra_frames_idlebasic1, ssithra_check_mood);

// Plague Ssithra Idle Right.
static const animframe_t ssithra_frames_idleright1[] =
{
	{ FRAME_idleright01,	ssithra_sound, SND_GROWL2, CHAN_VOICE, ATTN_IDLE, ai_stand, 0, ssithra_growl_sound },
	{ FRAME_idleright02,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright03,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright04,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright05,	ssithra_set_view_angle_offsets,	0, -20, 0, ai_stand, 0, NULL },
	{ FRAME_idleright06,	ssithra_set_view_angle_offsets,	0, -30, 0, ai_stand, 0, NULL },
	{ FRAME_idleright07,	ssithra_set_view_angle_offsets,	0, -50, 0, ai_stand, 0, NULL },
	{ FRAME_idleright08,	ssithra_set_view_angle_offsets,	0, -70, 0, ai_stand, 0, NULL },
	{ FRAME_idleright09,	ssithra_set_view_angle_offsets,	0, -80, 0, ai_stand, 0, NULL },
	{ FRAME_idleright10,	ssithra_set_view_angle_offsets,	0, -90, 0, ai_stand, 0, NULL },
	{ FRAME_idleright10,	NULL,								0, 0, 0, ai_stand, 0, NULL }, // Keep looking here a bit.
	{ FRAME_idleright10,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright10,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright10,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright11,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright11,	ssithra_set_view_angle_offsets,	0, -70, 0, ai_stand, 0, NULL },
	{ FRAME_idleright12,	ssithra_set_view_angle_offsets,	0, -50, 0, ai_stand, 0, NULL },
	{ FRAME_idleright13,	ssithra_set_view_angle_offsets,	0, -40, 0, ai_stand, 0, NULL },
	{ FRAME_idleright14,	ssithra_set_view_angle_offsets,	0, -30, 0, ai_stand, 0, NULL },
	{ FRAME_idleright15,	ssithra_set_view_angle_offsets,	0, -20, 0, ai_stand, 0, NULL },
	{ FRAME_idleright16,	ssithra_set_view_angle_offsets,	0, -10, 0, ai_stand, 0, NULL },
	{ FRAME_idleright17,	ssithra_set_view_angle_offsets,	0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright18,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright19,	NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idleright20,	NULL,								0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t ssithra_move_idleright1 = ANIMMOVE(ssithra_frames_idleright1, ssithra_check_mood); //mxd. numframes:24 in original logic.

// Plague Ssithra Melee.
static const animframe_t ssithra_frames_melee1[] =
{
	{ FRAME_melee1,			ssithra_sound, SND_SWIPE, CHAN_WEAPON, 0, ai_charge2, 20, NULL },
	{ FRAME_melee2,			NULL, 0, 0, 0, ai_charge2, 10, NULL },
	{ FRAME_melee3,			NULL, 0, 0, 0, ai_charge2,  8, NULL },
	{ FRAME_melee4,			NULL, 0, 0, 0, ai_charge2,  6, NULL },
	{ FRAME_melee5,			NULL, 0, 0, 0, ai_charge2,  4, ssithra_swipe },
	{ FRAME_melee6,			NULL, 0, 0, 0, ai_charge2,  3, NULL },
	{ FRAME_melee7,			NULL, 0, 0, 0, ai_charge2,  3, NULL },
};
const animmove_t ssithra_move_melee1 = ANIMMOVE(ssithra_frames_melee1, ssithra_decide_run);

// Plague Ssithra Melee Stand.
static const animframe_t ssithra_frames_meleest[] =
{
	{ FRAME_Melee_stand1,	ssithra_sound, SND_SWIPE, CHAN_WEAPON, 0, NULL, 0, NULL },
	{ FRAME_Melee_stand2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_Melee_stand3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_Melee_stand4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_Melee_stand5,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_swipe },
	{ FRAME_Melee_stand6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_Melee_stand7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t ssithra_move_meleest = ANIMMOVE(ssithra_frames_meleest, ssithra_decide_run);

// Plague Ssithra Namor.
static const animframe_t ssithra_frames_namor1[] =
{
	{ FRAME_namor1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_namor2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_namor3,		NULL, 0, 0, 0, NULL, 0, ssithra_out_of_water_jump },
	{ FRAME_namor4,		NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor5,		NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor6,		NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor7,		NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor8,		NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor9,		NULL, 0, 0, 0, NULL, 0, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor10,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 120, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor11,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 130, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor12,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 140, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor13,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 150, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor14,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 160, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor15,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 170, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor16,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 180, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor17,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 190, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor18,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 200, ssithra_try_spawn_water_exit_splash }, //FIXME: check to make sure out of water.
	{ FRAME_namor19,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 200, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor20,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 200, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor21,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 200, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor22,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 200, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor23,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 180, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor24,	NULL, 0, 0, 0, ssithra_set_forward_velocity, 120, ssithra_try_spawn_water_exit_splash },
	{ FRAME_namor25,	NULL, 0, 0, 0, ssithra_set_forward_velocity,  60, ssithra_try_spawn_water_exit_splash },
};
const animmove_t ssithra_move_namor1 = ANIMMOVE(ssithra_frames_namor1, ssithra_decide_run);

// Plague Ssithra Pain A.
static const animframe_t ssithra_frames_pain_a1[] =
{
	{ FRAME_pain_a1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain_a2,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_pain_a1 = ANIMMOVE(ssithra_frames_pain_a1, ssithra_pain_react);

// Plague Ssithra Shoot.
static const animframe_t ssithra_frames_shoot1[] =
{
	{ FRAME_shoot1,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot2,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot3,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot4,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot5,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot6,		NULL, 0, 0, 0, ai_charge2, 0, ssithra_arrow },
	{ FRAME_shoot6,		NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_loop },
	{ FRAME_shoot5,		NULL, 0, 0, 0, ai_charge2, 0, NULL },
}; // Frames 7 - 11 not used - weird anim for turning while shooting only!
const animmove_t ssithra_move_shoot1 = ANIMMOVE(ssithra_frames_shoot1, ssithra_decide_run);

// Plague Ssithra Lunge (from shooting).
static const animframe_t ssithra_frames_lunge[] =
{
	{ FRAME_shoot12,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_apply_jump },
	{ FRAME_shoot13,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot14,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot15,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot16,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot17,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot18,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot19,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot20,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_shoot21,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t ssithra_move_lunge = ANIMMOVE(ssithra_frames_lunge, ssithra_decide_run);

// Plague Ssithra Startle.
static const animframe_t ssithra_frames_startle1[] =
{
	{ FRAME_startle2,	ssithra_sound, SND_GROWL3, CHAN_VOICE, ATTN_IDLE, ai_stand, 0, ssithra_growl_sound },
	{ FRAME_startle3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle9,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle16,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle19,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle21,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle22,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle23,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle24,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle25,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle26,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle27,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle28,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle29,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle30,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle31,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle32,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle33,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle34,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle35,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle36,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle37,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle38,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_startle39,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t ssithra_move_startle1 = ANIMMOVE(ssithra_frames_startle1, ssithra_check_mood);

// Plague Ssithra Spin Left Start.
static const animframe_t ssithra_frames_spinleft_go[] =
{
	{ FRAME_idlebasic41,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_spinleft_go = ANIMMOVE(ssithra_frames_spinleft_go, ssithra_decide_run);

// Plague Ssithra Spin Left.
static const animframe_t ssithra_frames_spinleft[] =
{
	{ FRAME_idlebasic41,	ssithra_sound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE, NULL, 0, NULL },
	{ FRAME_idlebasic42,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic43,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic44,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic45,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic46,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic47,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic48,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic49,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic50,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic51,	NULL, 0, 0, 0, ai_spin,  6, NULL },
	{ FRAME_idlebasic52,	NULL, 0, 0, 0, ai_spin,  9, NULL },
	{ FRAME_idlebasic53,	NULL, 0, 0, 0, ai_spin, 13, NULL },
	{ FRAME_idlebasic54,	NULL, 0, 0, 0, ai_spin, 16, NULL },
	{ FRAME_idlebasic55,	NULL, 0, 0, 0, ai_spin, 18, NULL },
	{ FRAME_idlebasic56,	NULL, 0, 0, 0, ai_spin, 20, NULL },
	{ FRAME_idlebasic57,	NULL, 0, 0, 0, ai_spin, 23, NULL },
	{ FRAME_idlebasic58,	NULL, 0, 0, 0, ai_spin, 27, NULL },
	{ FRAME_idlebasic59,	NULL, 0, 0, 0, ai_spin, 30, NULL },
	{ FRAME_idlebasic60,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_spinleft = ANIMMOVE(ssithra_frames_spinleft, ssithra_decide_run);

// Plague Ssithra Spin Right Start.
static const animframe_t ssithra_frames_spinright_go[] =
{
	{ FRAME_idlebasic41,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_spinright_go = ANIMMOVE(ssithra_frames_spinright_go, ssithra_decide_run);

// Plague Ssithra Spin Right.
static const animframe_t ssithra_frames_spinright[] =
{
	{ FRAME_idlebasic61, ssithra_sound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE, NULL, 0, NULL },
	{ FRAME_idlebasic62, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic63, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic64, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic65, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic66, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic67, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic68, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic69, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic70, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idlebasic71, NULL, 0, 0, 0, ai_spin,  -6, NULL },
	{ FRAME_idlebasic72, NULL, 0, 0, 0, ai_spin,  -9, NULL },
	{ FRAME_idlebasic73, NULL, 0, 0, 0, ai_spin, -13, NULL },
	{ FRAME_idlebasic74, NULL, 0, 0, 0, ai_spin, -16, NULL },
	{ FRAME_idlebasic75, NULL, 0, 0, 0, ai_spin, -18, NULL },
	{ FRAME_idlebasic76, NULL, 0, 0, 0, ai_spin, -20, NULL },
	{ FRAME_idlebasic77, NULL, 0, 0, 0, ai_spin, -23, NULL },
	{ FRAME_idlebasic78, NULL, 0, 0, 0, ai_spin, -27, NULL },
	{ FRAME_idlebasic79, NULL, 0, 0, 0, ai_spin, -30, NULL },
	{ FRAME_idlebasic80, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_spinright = ANIMMOVE(ssithra_frames_spinright, ssithra_decide_run);

// Plague Ssithra Swim Forward.
static const animframe_t ssithra_frames_swimforward1[] =
{
	{ FRAME_swimforward01, NULL, 0, 0, 0, ssithra_ai_run, 10, ssithra_check_ripple },
	{ FRAME_swimforward02, NULL, 0, 0, 0, ssithra_ai_run, 10, NULL },
	{ FRAME_swimforward03, NULL, 0, 0, 0, ssithra_ai_run, 10, ssithra_check_ripple },
	{ FRAME_swimforward04, ssithra_sound, SND_SWIM, CHAN_BODY, 0, ssithra_ai_run, 12, ssithra_try_out_of_water_jump },
	{ FRAME_swimforward05, NULL, 0, 0, 0, ssithra_ai_run, 12, ssithra_check_ripple },
	{ FRAME_swimforward06, NULL, 0, 0, 0, ssithra_ai_run, 12, ssithra_check_ripple },
	{ FRAME_swimforward07, NULL, 0, 0, 0, ssithra_ai_run, 12, ssithra_check_ripple },
	{ FRAME_swimforward08, NULL, 0, 0, 0, ssithra_ai_run, 12, ssithra_try_out_of_water_jump },
	{ FRAME_swimforward09, NULL, 0, 0, 0, ssithra_ai_run, 16, ssithra_check_ripple },
	{ FRAME_swimforward10, ssithra_sound, SND_SWIM, CHAN_BODY, 0, ssithra_ai_run, 16, NULL },
	{ FRAME_swimforward11, NULL, 0, 0, 0, ssithra_ai_run, 12, ssithra_check_ripple },
	{ FRAME_swimforward12, NULL, 0, 0, 0, ssithra_ai_run, 12, ssithra_try_out_of_water_jump },
	{ FRAME_swimforward13, NULL, 0, 0, 0, ssithra_ai_run, 12, ssithra_check_ripple },
	{ FRAME_swimforward14, NULL, 0, 0, 0, ssithra_ai_run, 10, ssithra_check_ripple },
	{ FRAME_swimforward15, NULL, 0, 0, 0, ssithra_ai_run, 10, ssithra_check_ripple },
};
const animmove_t ssithra_move_swimforward1 = ANIMMOVE(ssithra_frames_swimforward1, ssithra_decide_swimforward);

// Plague Ssithra Swim Wander.
static const animframe_t ssithra_frames_swimwander[] =
{
	{ FRAME_swimforward01, NULL, 0, 0, 0, ai_walk, 10, ssithra_check_ripple },
	{ FRAME_swimforward02, NULL, 0, 0, 0, ai_walk, 10, ssithra_check_ripple },
	{ FRAME_swimforward03, NULL, 0, 0, 0, ai_walk, 10, ssithra_check_ripple },
	{ FRAME_swimforward04, ssithra_sound, SND_SWIM, CHAN_BODY, 0, ai_walk, 12, ssithra_try_out_of_water_jump },
	{ FRAME_swimforward05, NULL, 0, 0, 0, ai_walk, 12, ssithra_check_ripple },
	{ FRAME_swimforward06, NULL, 0, 0, 0, ai_walk, 12, ssithra_check_ripple },
	{ FRAME_swimforward07, NULL, 0, 0, 0, ai_walk, 12, ssithra_check_ripple },
	{ FRAME_swimforward08, NULL, 0, 0, 0, ai_walk, 12, ssithra_try_out_of_water_jump },
	{ FRAME_swimforward09, NULL, 0, 0, 0, ai_walk, 16, ssithra_check_ripple },
	{ FRAME_swimforward10, ssithra_sound, SND_SWIM, CHAN_BODY, 0, ai_walk, 16, NULL },
	{ FRAME_swimforward11, NULL, 0, 0, 0, ai_walk, 12, ssithra_check_ripple },
	{ FRAME_swimforward12, NULL, 0, 0, 0, ai_walk, 12, ssithra_try_out_of_water_jump },
	{ FRAME_swimforward13, NULL, 0, 0, 0, ai_walk, 12, ssithra_check_ripple },
	{ FRAME_swimforward14, NULL, 0, 0, 0, ai_walk, 10, ssithra_check_ripple },
	{ FRAME_swimforward15, NULL, 0, 0, 0, ai_walk, 10, ssithra_check_ripple },
};
const animmove_t ssithra_move_swimwander = ANIMMOVE(ssithra_frames_swimwander, ssithra_decide_swimforward);

// Plague Ssithra Water Death.
static const animframe_t ssithra_frames_water_death1[] =
{
	{ FRAME_water_death1,	ssithra_sound, SND_DIE, CHAN_VOICE, 0, NULL, 0, NULL },
	{ FRAME_water_death2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death33,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death34,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death35,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death36,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death37,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death38,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death39,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death40,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death41,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death42,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death43,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death44,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death45,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death46,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death47,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death48,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_death49,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_water_death1 = ANIMMOVE(ssithra_frames_water_death1, ssithra_water_dead);

// Plague Ssithra Water Idle.
static const animframe_t ssithra_frames_water_idle1[] =
{
	//FIXME: add water idle sound?
	{ FRAME_water_idle1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle9,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle16,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle19,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_water_idle20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t ssithra_move_water_idle1 = ANIMMOVE(ssithra_frames_water_idle1, ssithra_check_mood);

// Plague Ssithra Water Pain A.
static const animframe_t ssithra_frames_water_pain_a1[] =
{
	{ FRAME_water_pain_a1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_pain_a2,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_water_pain_a1 = ANIMMOVE(ssithra_frames_water_pain_a1, ssithra_pain_react);

// Plague Ssithra Water Pain B.
static const animframe_t ssithra_frames_water_pain_b1[] =
{
	{ FRAME_water_pain_b1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_water_pain_b2,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_water_pain_b1 = ANIMMOVE(ssithra_frames_water_pain_b1, ssithra_pain_react);

// Plague Ssithra Water Shoot.
static const animframe_t ssithra_frames_water_shoot1[] =
{
	{ FRAME_water_shoot01,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot6,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_arrow },
	{ FRAME_water_shoot7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot8,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot9,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_arrow },
	{ FRAME_water_shoot10,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot11,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot12,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_arrow },
	{ FRAME_water_shoot13,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot14,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot15,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_arrow },
	{ FRAME_water_shoot16,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_water_shoot17,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t ssithra_move_water_shoot1 = ANIMMOVE(ssithra_frames_water_shoot1, ssithra_decide_swimforward);

// Plague Ssithra Run.
static const animframe_t ssithra_frames_run1[] =
{
	{ FRAME_run1, NULL, 0, 0, 0, ssithra_ai_run, 16, ssithra_check_bound },
	{ FRAME_run2, NULL, 0, 0, 0, ssithra_ai_run, 18, ssithra_check_dive },
	{ FRAME_run3, NULL, 0, 0, 0, ssithra_ai_run, 20, NULL },
	{ FRAME_run4, NULL, 0, 0, 0, ssithra_ai_run, 18, NULL },
	{ FRAME_run5, NULL, 0, 0, 0, ssithra_ai_run, 16, NULL },
	{ FRAME_run6, NULL, 0, 0, 0, ssithra_ai_run, 18, NULL },
	{ FRAME_run7, NULL, 0, 0, 0, ssithra_ai_run, 20, NULL },
	{ FRAME_run8, NULL, 0, 0, 0, ssithra_ai_run, 18, NULL },
};
const animmove_t ssithra_move_run1 = ANIMMOVE(ssithra_frames_run1, ssithra_decide_run);

// Plague Ssithra Face And Namor.
static const animframe_t ssithra_frames_faceandnamor[] =
{
	{ FRAME_water_idle1,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle2,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle3,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle4,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle5,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle6,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle7,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle8,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle9,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle10,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle11,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle12,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle13,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle14,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle15,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle16,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle17,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle18,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle19,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
	{ FRAME_water_idle20,	NULL, 0, 0, 0, ai_charge2, 0, ssithra_check_faced_out_of_water_jump },
};
const animmove_t ssithra_move_faceandnamor = ANIMMOVE(ssithra_frames_faceandnamor, ssithra_check_mood);

// Plague Ssithra Look Left.
static const animframe_t ssithra_frames_lookleft[] =
{
	{ FRAME_idlebasic41, ssithra_sound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE, ai_stand, 0, NULL },
	{ FRAME_idlebasic42, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic43, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic44, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic45, ssithra_set_view_angle_offsets,	20, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic46, ssithra_set_view_angle_offsets,	40, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic47, ssithra_set_view_angle_offsets,	60, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic48, ssithra_set_view_angle_offsets,	80, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic49, ssithra_set_view_angle_offsets,	100, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic50, ssithra_set_view_angle_offsets,	120, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic51, ssithra_set_view_angle_offsets,	160, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic52, ssithra_set_view_angle_offsets,	120, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic53, ssithra_set_view_angle_offsets,	80, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic54, ssithra_set_view_angle_offsets,	60, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic55, ssithra_set_view_angle_offsets,	40, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic56, ssithra_set_view_angle_offsets,	20, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic57, ssithra_set_view_angle_offsets,	0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic58, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic59, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic60, NULL,								0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t ssithra_move_lookleft = ANIMMOVE(ssithra_frames_lookleft, ssithra_check_mood);

// Plague Ssithra Look Right.
static const animframe_t ssithra_frames_lookright[] =
{
	{ FRAME_idlebasic61, ssithra_sound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE, ai_stand, 0, NULL },
	{ FRAME_idlebasic62, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic63, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic64, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic65, ssithra_set_view_angle_offsets,	-20, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic66, ssithra_set_view_angle_offsets,	-40, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic67, ssithra_set_view_angle_offsets,	-60, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic68, ssithra_set_view_angle_offsets,	-80, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic69, ssithra_set_view_angle_offsets,	-100, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic70, ssithra_set_view_angle_offsets,	-120, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic71, ssithra_set_view_angle_offsets,	-160, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic72, ssithra_set_view_angle_offsets,	-120, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic73, ssithra_set_view_angle_offsets,	-80, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic74, ssithra_set_view_angle_offsets,	-60, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic75, ssithra_set_view_angle_offsets,	-40, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic76, ssithra_set_view_angle_offsets,	-20, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic77, ssithra_set_view_angle_offsets,	0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic78, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic79, NULL,								0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idlebasic80, NULL,								0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t ssithra_move_lookright = ANIMMOVE(ssithra_frames_lookright, ssithra_check_mood);

// Plague Ssithra Yrans Up.
static const animframe_t ssithra_frames_transup[] =
{
	{ FRAME_Water_trans1, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Water_trans2, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Water_trans3, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Water_trans4, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_transup = ANIMMOVE(ssithra_frames_transup, ssithra_water_shoot);

// Plague Ssithra Trans Down.
static const animframe_t ssithra_frames_transdown[] =
{
	{ FRAME_Water_trans4, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Water_trans3, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Water_trans2, NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Water_trans1, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_transdown = ANIMMOVE(ssithra_frames_transdown, ssithra_decide_swimforward);

// Plague Ssithra Death C.
static const animframe_t ssithra_frames_death_c[] =
{
	{ FRAME_shoot5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shoot4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shoot3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shoot2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_shoot1,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b11,	NULL, 0, 0, 0, NULL, 0, MG_SetNoBlocking },
	{ FRAME_death_b12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b33,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b34,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b35,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b36,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_death_c = ANIMMOVE(ssithra_frames_death_c, ssithra_dead);

// Plague Ssithra Headless Loop.
static const animframe_t ssithra_frames_headlessloop[] =
{
	{ FRAME_shoot8,		NULL, 0, 0, 0, ai_spin, -5, NULL },
	{ FRAME_shoot9,		NULL, 0, 0, 0, ai_spin, -4, ssithra_panic_arrow },
	{ FRAME_shoot10,	NULL, 0, 0, 0, ai_spin, -2, NULL },
	{ FRAME_shoot11,	NULL, 0, 0, 0, ai_spin, -2, NULL },
	{ FRAME_shoot12,	NULL, 0, 0, 0, ai_spin, -3, NULL },
	{ FRAME_shoot13,	NULL, 0, 0, 0, ai_spin, -4, NULL },
};
const animmove_t ssithra_move_headlessloop = ANIMMOVE(ssithra_frames_headlessloop, ssithra_collapse);

// Plague Ssithra Headless.
static const animframe_t ssithra_frames_headless[] =
{
	{ FRAME_shoot1,		NULL, 0, 0, 0, ai_spin, -20, NULL },
	{ FRAME_shoot2,		NULL, 0, 0, 0, ai_spin, -12, NULL },
	{ FRAME_shoot3,		NULL, 0, 0, 0, ai_spin, -10, NULL },
	{ FRAME_shoot4,		NULL, 0, 0, 0, ai_spin,  -9, NULL },
	{ FRAME_shoot5,		NULL, 0, 0, 0, ai_spin,  -8, NULL },
	{ FRAME_shoot6,		NULL, 0, 0, 0, ai_spin,  -7, ssithra_panic_arrow },
	{ FRAME_shoot7,		NULL, 0, 0, 0, ai_spin,  -7, NULL },
	{ FRAME_shoot8,		NULL, 0, 0, 0, ai_spin,  -7, ssithra_panic_arrow },
	{ FRAME_shoot9,		NULL, 0, 0, 0, ai_spin,  -6, NULL },
	{ FRAME_shoot10,	NULL, 0, 0, 0, ai_spin,  -6, NULL },
	{ FRAME_shoot11,	NULL, 0, 0, 0, ai_spin,  -6, NULL },
	{ FRAME_shoot12,	NULL, 0, 0, 0, ai_spin,  -5, NULL },
	{ FRAME_shoot13,	NULL, 0, 0, 0, ai_spin,  -5, NULL },
};
const animmove_t ssithra_move_headless = ANIMMOVE(ssithra_frames_headless, ssithra_collapse);

// Plague Ssithra Dead A.
static const animframe_t ssithra_frames_dead_a[] =
{
	{ FRAME_death_a12, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_dead_a = ANIMMOVE(ssithra_frames_dead_a, NULL);

// Plague Ssithra Dead B.
static const animframe_t ssithra_frames_dead_b[] =
{
	{ FRAME_death_b36, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_dead_b = ANIMMOVE(ssithra_frames_dead_b, NULL);

// Plague Ssithra Dead Water.
static const animframe_t ssithra_frames_dead_water[] =
{
	{ FRAME_water_death49, NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_dead_water = ANIMMOVE(ssithra_frames_dead_water, NULL);

// Plague Ssithra Sliced.
static const animframe_t ssithra_frames_sliced[] =
{
	{ FRAME_death_b1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b11,	NULL, 0, 0, 0, NULL, 0, MG_SetNoBlocking },
	{ FRAME_death_b12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b33,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b34,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b35,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_death_b36,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t ssithra_move_sliced = ANIMMOVE(ssithra_frames_sliced, ssithra_kill_self);

// Plague Ssithra Delay - stop and look around.
static const animframe_t ssithra_frames_delay[] =
{
	{ FRAME_startle2,	ssithra_sound, SND_GROWL3, CHAN_VOICE, ATTN_IDLE, NULL, 0, ssithra_check_mood },
	{ FRAME_startle3,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle4,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle5,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle6,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle7,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle8,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle9,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle10,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle11,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle12,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle13,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle14,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle15,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle16,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle17,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle18,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle19,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle20,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle21,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle22,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle23,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle24,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle25,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle26,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle27,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle28,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle29,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle30,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle31,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle32,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle33,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle34,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle35,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle36,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle37,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle38,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
	{ FRAME_startle39,	NULL, 0, 0, 0, NULL, 0, ssithra_check_mood },
};
const animmove_t ssithra_move_delay = ANIMMOVE(ssithra_frames_delay, ssithra_check_mood);
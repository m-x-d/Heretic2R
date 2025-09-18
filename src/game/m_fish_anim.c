//
// m_fish_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_fish_anim.h"
#include "m_fish_shared.h"
#include "g_ai.h" //mxd
#include "g_local.h"

// Fish melee.
static const animframe_t fish_frames_melee[] =
{
	{ FRAME_attfrnzy1,	NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_attfrnzy2,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy3,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy4,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy5,	NULL, 0, 0, 0, NULL, 0, fish_bite },
	{ FRAME_attfrnzy6,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy7,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy8,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy9,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy10,	NULL, 0, 0, 0, NULL, 0, fish_bite },
	{ FRAME_attfrnzy11,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy12,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy13,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attfrnzy14,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
};
const animmove_t fish_move_melee = ANIMMOVE(fish_frames_melee, fish_pause);

// Fish bite.
static const animframe_t fish_frames_bite[] =
{
	{ FRAME_attbite1,	NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_attbite2,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attbite3,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attbite4,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attbite5,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
	{ FRAME_attbite6,	NULL, 0, 0, 0, NULL, 0, fish_bite },
	{ FRAME_attbite7,	NULL, 0, 0, 0, NULL, 0, fish_update_target_movedir },
};
const animmove_t fish_move_bite = ANIMMOVE(fish_frames_bite, fish_pause);

// Fish swim 1.
static const animframe_t fish_frames_run1[] =
{
	{ FRAME_swim1,		NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_swim2,		NULL, 0, 0, 0, NULL, 0, fish_under_water_wake },
	{ FRAME_swim3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swim4,		NULL, 0, 0, 0, fish_swim_sound, 1, NULL },
	{ FRAME_swim5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swim6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swim7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swim8,		NULL, 0, 0, 0, fish_swim_sound, 1, fish_chase },
};
const animmove_t fish_move_run1 = ANIMMOVE(fish_frames_run1, fish_runswim_finished);

// Fish swim 2.
static const animframe_t fish_frames_run2[] =
{
	{ FRAME_swimLEFT1,	NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_swimLEFT2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimLEFT3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimLEFT4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimLEFT5,	NULL, 0, 0, 0, fish_swim_sound, 1, NULL },
	{ FRAME_swimLEFT6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimLEFT7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimLEFT8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimLEFT9,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t fish_move_run2 = ANIMMOVE(fish_frames_run2, fish_run);

// Fish swim 3.
static const animframe_t fish_frames_run3[] =
{
	{ FRAME_swimRIGHT1,	NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_swimRIGHT2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimRIGHT3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimRIGHT4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimRIGHT5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimRIGHT6,	NULL, 0, 0, 0, fish_swim_sound, 1, NULL },
	{ FRAME_swimRIGHT7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimRIGHT8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_swimRIGHT9,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t fish_move_run3 = ANIMMOVE(fish_frames_run3, fish_run);

// Fish walk 1.
static const animframe_t fish_frames_walk1[] =
{
	{ FRAME_fishpat1,	NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_fishpat2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat9,	NULL, 0, 0, 0, fish_swim_sound, 0, NULL },
	{ FRAME_fishpat10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat11,	NULL, 0, 0, 0, NULL, 0, fish_chase },
	{ FRAME_fishpat12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat21,	NULL, 0, 0, 0, fish_swim_sound, 0, NULL },
	{ FRAME_fishpat22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_fishpat24,	NULL, 0, 0, 0, NULL, 0, fish_chase },
};
const animmove_t fish_move_walk1 = ANIMMOVE(fish_frames_walk1, fish_walkswim_finished);

// Fish walk 2 - swim to the left.
static const animframe_t fish_frames_walk2[] =
{
	{ FRAME_slowturnl1,	NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_slowturnl2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnl3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnl4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnl5,	NULL, 0, 0, 0, fish_swim_sound, 0, NULL },
	{ FRAME_slowturnl6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnl7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnl8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnl9,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t fish_move_walk2 = ANIMMOVE(fish_frames_walk2, fish_walk);

// Fish walk 3 - swim to the right.
static const animframe_t fish_frames_walk3[] =
{
	{ FRAME_slowturnr1,	NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_slowturnr2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnr3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnr4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnr5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnr6,	NULL, 0, 0, 0, fish_swim_sound, 0, NULL },
	{ FRAME_slowturnr7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnr8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_slowturnr9,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t fish_move_walk3 = ANIMMOVE(fish_frames_walk3, fish_walk);

// Fish stand.
static const animframe_t fish_frames_stand1[] =
{
	{ FRAME_fishpat1,	NULL, 0, 0, 0, ai_stand, 0, fish_update_yaw },
	{ FRAME_fishpat2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat9,	NULL, 0, 0, 0, fish_swim_sound, 0, NULL },
	{ FRAME_fishpat10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat16,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat19,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat21,	NULL, 0, 0, 0, fish_swim_sound, 0, NULL },
	{ FRAME_fishpat22,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat23,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_fishpat24,	NULL, 0, 0, 0, ai_stand, 0, fish_chase },
};
const animmove_t fish_move_stand1 = ANIMMOVE(fish_frames_stand1, fish_idle);

// Fish pain.
static const animframe_t fish_frames_pain1[] =
{
	{ FRAME_pain1,		NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_pain2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_pain7,		NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t fish_move_pain1 = ANIMMOVE(fish_frames_pain1, fish_pain_finished);

// Fish death.
static const animframe_t fish_frames_death[] =
{
	{ FRAME_Death1,		NULL, 0, 0, 0, NULL, 0, fish_update_yaw },
	{ FRAME_Death2,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death3,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death4,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death5,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death6,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death7,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death8,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death9,		NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death20,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death23,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death24,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death25,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death26,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death27,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death28,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death29,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death30,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death31,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death32,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_Death33,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t fish_move_death = ANIMMOVE(fish_frames_death, fish_dead);
//
// m_spreader_anim.c
//
// Copyright 1998 Raven Software
//

#include "m_spreader_anim.h"
#include "m_spreader_shared.h"
#include "m_spreadermist.h"
#include "g_ai.h" //mxd
#include "mg_ai.h" //mxd
#include "g_local.h"

// Spreader attack 1 - the throw.
static const animframe_t spreader_frames_attack1[] =
{
	{ FRAME_atacka1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka6,	NULL, 0, 0, 0, ai_charge2, 0, spreader_show_grenade },
	{ FRAME_atacka7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka8,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka9,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka10,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka11,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka12,	NULL, 0, 0, 0, ai_charge2, 0, spreader_hide_grenade },
	{ FRAME_atacka13,	NULL, 0, 0, 0, ai_charge2, 0, spreader_toss_grenade },
	{ FRAME_atacka14,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka15,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atacka16,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t spreader_move_attack1 = ANIMMOVE(spreader_frames_attack1, spreader_pause);

// Spreader Attack 2 - the plague effect attack.
// Using the move func for the spreader mist instead of the action func because the action func is already being used as a move func...
static const animframe_t spreader_frames_attack2[] =
{
	{ FRAME_atackb1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atackb2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atackb3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atackb4,	spreader_mist, 14.59f, -0.96f,  16.0f,  ai_charge2, 0, spreader_mist_start_sound },
	{ FRAME_atackb5,	spreader_mist, 12.49f, -5.98f,  16.0f,  ai_charge2, 0, NULL },
	{ FRAME_atackb6,	spreader_mist, 10.35f, -13.15f, 16.0f,  ai_charge2, 0, NULL },
	{ FRAME_atackb7,	spreader_mist, 6.14f,  -17.95f, 16.0f,  ai_charge2, 0, NULL },
	{ FRAME_atackb8,	spreader_mist, 2.4f,   -18.06f, 16.0f,  ai_charge2, 0, NULL },
	{ FRAME_atackb9,	spreader_mist, 6.23f,  -14.4f,  16.0f,  ai_charge2, 0, NULL },
	{ FRAME_atackb10,	spreader_mist, 12.85f, -4.18f,  16.0f,  ai_charge2, 0, NULL },
	{ FRAME_atackb11,	spreader_mist, 13.88f,  10.8f,  16.36f, ai_charge2, 0, NULL },
	{ FRAME_atackb12,	spreader_mist, 7.11f,   23.19f, 16.81f, ai_charge2, 0, spreader_mist_stop_sound },
	{ FRAME_atackb13,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_atackb14,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t spreader_move_attack2 = ANIMMOVE(spreader_frames_attack2, spreader_pause);

// Spreader Back Attack 1 - the spreader backpedals while shooting.
static const animframe_t spreader_frames_backattack1[] =
{
	{ FRAME_bkatck1,	spreader_mist, 14.4f,  10.24f, 16.0f, MG_AI_Run, -5, spreader_mist_start_sound },
	{ FRAME_bkatck2,	spreader_mist, 14.22f, 10.03f, 16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck3,	spreader_mist, 13.79f, 9.79f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck4,	spreader_mist, 13.62f, 9.63f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck5,	spreader_mist, 13.84f, 9.64f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck6,	spreader_mist, 14.23f, 9.75f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck7,	spreader_mist, 14.59f, 9.85f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck8,	spreader_mist, 14.15f, 9.75f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck9,	spreader_mist, 13.83f, 9.66f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck10,	spreader_mist, 13.58f, 9.65f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck11,	spreader_mist, 13.82f, 9.82f,  16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck12,	spreader_mist, 14.2f,  10.06f, 16.0f, MG_AI_Run, -5, NULL },
	{ FRAME_bkatck13,	spreader_mist, 14.47f, 10.38f, 16.0f, MG_AI_Run, -5, spreader_mist_stop_sound },
};
const animmove_t spreader_move_backattack1 = ANIMMOVE(spreader_frames_backattack1, spreader_pause);

// Spreader Backup 1 - the spreader backpedals.
static const animframe_t spreader_frames_backup1[] =
{
	{ FRAME_backup1,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup2,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup3,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup4,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup5,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup6,	NULL, 0, 0, 0, MG_AI_Run, -5, spreader_pause },
	{ FRAME_backup7,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup8,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup9,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup10,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup11,	NULL, 0, 0, 0, MG_AI_Run, -5, spreader_pause },
	{ FRAME_backup12,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
	{ FRAME_backup13,	NULL, 0, 0, 0, MG_AI_Run, -5, NULL },
};
const animmove_t spreader_move_backup1 = ANIMMOVE(spreader_frames_backup1, spreader_pause);

// Spreader Death 1 Start - the spreader spreads no more (big death).
static const animframe_t spreader_frames_death1_go[] =
{
	{ FRAME_death1,	NULL, 0, 0, 0, NULL, 0, spreader_flyback_move },
};
const animmove_t spreader_move_death1_go = ANIMMOVE(spreader_frames_death1_go, spreader_flyback_loop);

// Spreader Death 1 Loop.
static const animframe_t spreader_frames_death1_loop[] =
{
	{ FRAME_death2,	NULL, 0, 0, 0, NULL, 0, spreader_flyback_move },
};
const animmove_t spreader_move_death1_loop = ANIMMOVE(spreader_frames_death1_loop, NULL);

// Spreader Death 1 End.
static const animframe_t spreader_frames_death1_end[] =
{
	{ FRAME_death3,		NULL, 0, 0, 0, ai_move, -4, spreader_flyback_move },
	{ FRAME_death4,		NULL, 0, 0, 0, ai_move, -4, spreader_flyback_move },
	{ FRAME_death5,		NULL, 0, 0, 0, ai_move, -4, spreader_flyback_move },
	{ FRAME_death6,		NULL, 0, 0, 0, ai_move, -3, spreader_flyback_move },
	{ FRAME_death7,		NULL, 0, 0, 0, ai_move, -3, spreader_flyback_move },
	{ FRAME_death8,		NULL, 0, 0, 0, ai_move, -3, spreader_flyback_move },
	{ FRAME_death9,		NULL, 0, 0, 0, ai_move, -2, spreader_flyback_move },
	{ FRAME_death10,	NULL, 0, 0, 0, ai_move, -2, spreader_flyback_move },
	{ FRAME_death11,	NULL, 0, 0, 0, ai_move, -2, spreader_flyback_move },
	{ FRAME_death12,	NULL, 0, 0, 0, ai_move, -1, spreader_flyback_move },
	{ FRAME_death13,	NULL, 0, 0, 0, ai_move, -1, spreader_flyback_move },
	{ FRAME_death14,	NULL, 0, 0, 0, ai_move,  0, spreader_flyback_move },
	{ FRAME_death15,	NULL, 0, 0, 0, ai_move,  0, spreader_flyback_move },
	{ FRAME_death16,	NULL, 0, 0, 0, ai_move,  0, spreader_flyback_move },
	{ FRAME_death17,	NULL, 0, 0, 0, ai_move,  0, spreader_dead },
};
const animmove_t spreader_move_death1_end = ANIMMOVE(spreader_frames_death1_end, NULL);

// Spreader Death 2 - the spreader spreads no more (little death).
static const animframe_t spreader_frames_death2[] =
{
	{ FRAME_deathb1,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb2,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb3,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb4,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb6,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb7,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb9,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb10,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb11,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb12,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb13,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb14,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb16,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_deathb17,	NULL, 0, 0, 0, ai_move, 0, spreader_dead },
};
const animmove_t spreader_move_death2 = ANIMMOVE(spreader_frames_death2, NULL);

// Spreader Duck Attack 1 - spreader ducking, shoots, rises.
static const animframe_t spreader_frames_dkatck1[] =
{
	{ FRAME_dkatck_1,	spreader_mist, 21.39f,  11.26f, -16.0f, ai_move, 0, spreader_mist_start_sound },
	{ FRAME_dkatck_2,	spreader_mist, 12.08f,  24.16f, -16.0f, ai_move, 0, NULL },
	{ FRAME_dkatck_3,	spreader_mist, 26.14f,  1.26f,  -16.0f, ai_move, 0, NULL },
	{ FRAME_dkatck_4,	spreader_mist, 26.65f, -0.43f,  -16.0f, ai_move, 0, spreader_mist_stop_sound },
	{ FRAME_dkatck_5,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_dkatck_6,	NULL, 0, 0, 0, ai_move, 0, NULL },
};
const animmove_t spreader_move_dkatck1 = ANIMMOVE(spreader_frames_dkatck1, spreader_pause);

// Spreader Duck 1 - spreader ducks and rises.
static const animframe_t spreader_frames_duck1[] =
{
	{ FRAME_duck1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t spreader_move_duck1 = ANIMMOVE(spreader_frames_duck1, spreader_pause);

// Spreader Duck Down.
static const animframe_t spreader_frames_duckdown[] =
{
	{ FRAME_duck1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t spreader_move_duckdown = ANIMMOVE(spreader_frames_duckdown, spreader_duck_pause);

// Spreader Duck Still 1 - the spreader ducking, still.
static const animframe_t spreader_frames_duckstill[] =
{
	{ FRAME_dkidle1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_dkidle12,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t spreader_move_duckstill = ANIMMOVE(spreader_frames_duckstill, spreader_duck_pause);

// Spreader Duck Up - the spreader rises from a ducking position.
static const animframe_t spreader_frames_duckup[] =
{
	{ FRAME_duck4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck6,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_duck7,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t spreader_move_duckup = ANIMMOVE(spreader_frames_duckup, spreader_pause);

// Spreader Idle 1 - the spreader stands around.
static const animframe_t spreader_frames_idle1[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, ai_stand,	0, NULL },
	{ FRAME_idle2,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_idle3,	NULL, 0, 0, 0, ai_stand,	0, spreader_idle_sound },
	{ FRAME_idle4,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_idle5,	NULL, 0, 0, 0, ai_stand,	0, NULL },
	{ FRAME_idle6,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_idle7,	NULL, 0, 0, 0, ai_stand,	0, NULL },
	{ FRAME_idle8,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_idle9,	NULL, 0, 0, 0, ai_stand,	0, NULL },
	{ FRAME_idle10,	NULL, 0, 0, 0, NULL,		0, NULL },
	{ FRAME_idle11,	NULL, 0, 0, 0, ai_stand,	0, NULL },
};
const animmove_t spreader_move_idle1 = ANIMMOVE(spreader_frames_idle1, spreader_pause);

// Spreader Pain 1 - spreader recoils from hit.
static const animframe_t spreader_frames_pain1[] =
{
	{ FRAME_pain1,	NULL, 0, 0, 0, ai_move, 0, spreader_pain_sound },
	{ FRAME_pain2,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_pain3,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_pain4,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_pain5,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_pain6,	NULL, 0, 0, 0, ai_move, 0, NULL },
	{ FRAME_pain7,	NULL, 0, 0, 0, ai_move, 0, NULL },
};
const animmove_t spreader_move_pain1 = ANIMMOVE(spreader_frames_pain1, spreader_pause);

// Spreader Pivot Left 1 - spreader turns left.
static const animframe_t spreader_frames_pvtlt1[] =
{
	{ FRAME_pvtlt1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_pvtlt2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_pvtlt3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_pvtlt4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_pvtlt5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t spreader_move_pvtlt1 = ANIMMOVE(spreader_frames_pvtlt1, spreader_pause);

// Spreader Pivot Right 1 - spreader turns right.
static const animframe_t spreader_frames_pvtrt1[] =
{
	{ FRAME_pvtrt1,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_pvtrt2,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_pvtrt3,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_pvtrt4,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
	{ FRAME_pvtrt5,	NULL, 0, 0, 0, ai_charge2, 0, NULL },
};
const animmove_t spreader_move_pvtrt1 = ANIMMOVE(spreader_frames_pvtrt1, spreader_pause);

// Spreader Run 1 - the spreader runs.
static const animframe_t spreader_frames_run1[] =
{
	{ FRAME_run1,	NULL, 0, 0, 0, MG_AI_Run, 20, spreader_pause },
	{ FRAME_run2,	NULL, 0, 0, 0, MG_AI_Run, 18, NULL },
	{ FRAME_run3,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_run4,	NULL, 0, 0, 0, MG_AI_Run, 18, spreader_pause }, // Foot down.
	{ FRAME_run5,	NULL, 0, 0, 0, MG_AI_Run, 20, NULL },
	{ FRAME_run6,	NULL, 0, 0, 0, MG_AI_Run, 18, NULL },
	{ FRAME_run7,	NULL, 0, 0, 0, MG_AI_Run, 16, NULL },
	{ FRAME_run8,	NULL, 0, 0, 0, MG_AI_Run, 18, NULL }, // Foot down.
};
const animmove_t spreader_move_run1 = ANIMMOVE(spreader_frames_run1, spreader_pause);

// Spreader Land.
static const animframe_t spreader_frames_land[] =
{
	{ FRAME_jump17,	NULL, 0, 0, 0, NULL, 0, spreader_land },
	{ FRAME_jump18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump20,	NULL, 0, 0, 0, NULL, 0, NULL }, // Foot down.
	{ FRAME_jump21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump22,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump23,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t spreader_move_land = ANIMMOVE(spreader_frames_land, spreader_pause);

// Spreader In Air.
static const animframe_t spreader_frames_inair[] =
{
	{ FRAME_jump16,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t spreader_move_inair = ANIMMOVE(spreader_frames_inair, NULL);

// Spreader Forward Jump.
static const animframe_t spreader_frames_fjump[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, NULL, 0, NULL }, // Foot down.
	{ FRAME_jump5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, NULL, 0, spreader_jump },
	{ FRAME_jump8,	NULL, 0, 0, 0, NULL, 0, NULL }, // Foot down.
	{ FRAME_jump9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump10,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump11,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump12,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL }, // Foot down.
	{ FRAME_jump13,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump15,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t spreader_move_fjump = ANIMMOVE(spreader_frames_fjump, spreader_inair_go);

// Spreader Run Attack 1 - the spreader runs and shoots.
static const animframe_t spreader_frames_rnatck1[] =
{
	{ FRAME_rnatck1,	spreader_mist_fast, 38.0f * 2.0f,  5.0f, 16.0f, MG_AI_Run, 20, spreader_mist_start_sound },
	{ FRAME_rnatck2,	spreader_mist_fast, 39.27f * 2.0f, 5.0f, 16.0f, MG_AI_Run, 18, NULL },
	{ FRAME_rnatck3,	spreader_mist_fast, 37.43f * 2.0f, 5.0f, 16.0f, MG_AI_Run, 16, NULL },
	{ FRAME_rnatck4,	spreader_mist_fast, 36.68f * 2.0f, 5.0f, 16.0f, MG_AI_Run, 18, spreader_pause },
	{ FRAME_rnatck5,	spreader_mist_fast, 37.02f * 2.0f, 5.0f, 16.0f, MG_AI_Run, 20, NULL },
	{ FRAME_rnatck6,	spreader_mist_fast, 36.41f * 2.0f, 5.0f, 16.0f, MG_AI_Run, 18, NULL },
	{ FRAME_rnatck7,	spreader_mist_fast, 35.69f * 2.0f, 5.0f, 16.0f, MG_AI_Run, 16, spreader_pause },
	{ FRAME_rnatck8,	spreader_mist_fast, 35.68f * 2.0f, 5.0f, 16.0f, MG_AI_Run, 18, spreader_pause },
};
const animmove_t spreader_move_rnatck1 = ANIMMOVE(spreader_frames_rnatck1, spreader_pause);

// Spreader Walk 1 - the spreader walks.
static const animframe_t spreader_frames_walk1[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, ai_walk, 8, spreader_idle_sound },
	{ FRAME_walk2,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk3,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk4,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk5,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk6,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk7,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk8,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk9,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk10,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk11,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk12,	NULL, 0, 0, 0, ai_walk, 8, NULL },
	{ FRAME_walk13,	NULL, 0, 0, 0, ai_walk, 8, NULL },
};
const animmove_t spreader_move_walk1 = ANIMMOVE(spreader_frames_walk1, spreader_pause);

// Spreader Walk 2 - the spreader walks.
static const animframe_t spreader_frames_walk2[] =
{
	{ FRAME_walk1,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk2,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk3,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk4,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk5,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk6,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk7,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk8,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk9,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk10,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk11,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk12,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
	{ FRAME_walk13,	NULL, 0, 0, 0, MG_AI_Run, 6, spreader_pause },
};
const animmove_t spreader_move_walk2 = ANIMMOVE(spreader_frames_walk2, spreader_pause);

// Spreader Fly Loop.
static const animframe_t spreader_frames_flyloop[] =
{
	{ FRAME_pain5,	NULL, 0, 0, 0, NULL, 0, spreader_fly },
};
const animmove_t spreader_move_flyloop = ANIMMOVE(spreader_frames_flyloop, NULL);

// Spreader Fly.
static const animframe_t spreader_frames_fly[] =
{
	{ FRAME_pain1,	NULL, 0, 0, 0, NULL, 0, spreader_fly },
	{ FRAME_pain2,	NULL, 0, 0, 0, NULL, 0, spreader_fly },
	{ FRAME_pain3,	NULL, 0, 0, 0, NULL, 0, spreader_fly },
	{ FRAME_pain4,	NULL, 0, 0, 0, NULL, 0, spreader_fly },
};
const animmove_t spreader_move_fly = ANIMMOVE(spreader_frames_fly, spreader_fly_loop);

// Spreader Dead.
static const animframe_t spreader_frames_dead[] =
{
	{ FRAME_deathb17,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
};
const animmove_t spreader_move_dead = ANIMMOVE(spreader_frames_dead, NULL);

// Spreader Die.
static const animframe_t spreader_frames_fdie[] =
{
	{ FRAME_deathb1,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb2,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb3,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb4,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb6,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb7,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb9,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb10,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb11,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb12,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb13,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb14,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb16,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
	{ FRAME_deathb17,	NULL, 0, 0, 0, NULL, 0, spreader_become_solid },
};
const animmove_t spreader_move_fdie = ANIMMOVE(spreader_frames_fdie, spreader_deadloop_go);

// Spreader Delay.
static const animframe_t spreader_frames_delay[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle2,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle3,	NULL, 0, 0, 0, NULL, 0, spreader_idle_sound },
	{ FRAME_idle4,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle5,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle6,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle7,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle8,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle9,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle10,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
	{ FRAME_idle11,	NULL, 0, 0, 0, NULL, 0, spreader_pause },
};
const animmove_t spreader_move_delay = ANIMMOVE(spreader_frames_delay, spreader_pause);
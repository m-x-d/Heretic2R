//
// m_assassin_anim.c -- All of the anim frames that used to live in m_assassin.h.
//
// Copyright 1998 Raven Software
//

#include "m_assassin_anim.h"
#include "m_assassin_shared.h"
#include "g_monster.h"
#include "c_ai.h"
#include "mg_ai.h" //mxd
#include "g_local.h"

#pragma region ========================== ATTACKS ==========================

// Assassin daggerL - assassin attacking left hand.
static const animframe_t assassin_frames_daggerl[] =
{
	{ FRAME_ataka1,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, assassin_growl },
	{ FRAME_ataka2,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka3,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka4,		NULL, 0.0f, 0.0f, 0.0f, assassin_enable_fmnode, MESH__LKNIFE, NULL }, // Loop in from an attack, no windup.
	{ FRAME_ataka5,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka6,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka7,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka8,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka9,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka10,	NULL, 0.0f, 0.0f, 0.0f, assassin_attack, BIT_LKNIFE, NULL },
	{ FRAME_ataka11,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka12,	NULL, 0.0f, 0.0f, 0.0f, assassin_check_loop, 2.0f, NULL }, // Check for loop to other attack.
	{ FRAME_ataka13,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_ataka14,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL }
};
const animmove_t assassin_move_daggerl = ANIMMOVE(assassin_frames_daggerl, assassin_pause);

// Assassin daggerR - assassin attacking right hand.
static const animframe_t assassin_frames_daggerr[] =
{
	{ FRAME_atakb1,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, assassin_growl },
	{ FRAME_atakb2,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb3,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb4,		NULL, 0.0f, 0.0f, 0.0f, assassin_enable_fmnode, MESH__RKNIFE, NULL },
	{ FRAME_atakb5,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb6,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL }, // Loop in from an attack, no windup.
	{ FRAME_atakb7,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb8,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb9,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb10,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb11,	NULL, 0.0f, 0.0f, 0.0f, assassin_attack, BIT_RKNIFE, NULL },
	{ FRAME_atakb12,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb13,	NULL, 0.0f, 0.0f, 0.0f, assassin_check_loop, 2.0f, NULL }, // Check for loop to other attack.
	{ FRAME_atakb14,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakb15,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL }
};
const animmove_t assassin_move_daggerr = ANIMMOVE(assassin_frames_daggerr, assassin_pause);

// Assassin daggerB - assassin attacking left hand.
static const animframe_t assassin_frames_daggerb[] =
{
	{ FRAME_atakc1,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, assassin_growl },
	{ FRAME_atakc2,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakc3,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakc4,		NULL, 0.0f, 0.0f, 0.0f, assassin_enable_fmnode, MESH__LKNIFE, NULL },
	{ FRAME_atakc5,		NULL, 0.0f, 0.0f, 0.0f, assassin_enable_fmnode, MESH__RKNIFE, NULL },
	{ FRAME_atakc6,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakc7,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL }, // Loop in from an attack, no windup.
	{ FRAME_atakc8,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakc9,		NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakc10,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakc11,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_atakc12,	NULL, 0.0f, 0.0f, 0.0f, assassin_attack, BIT_LKNIFE | BIT_RKNIFE, NULL },
	{ FRAME_atakc13,	NULL, 0.0f, 0.0f, 0.0f, assassin_check_loop, 2.0f, NULL }, // Check for loop to other attack.
	{ FRAME_atakc14,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
};
const animmove_t assassin_move_daggerb = ANIMMOVE(assassin_frames_daggerb, assassin_pause);

// Assassin daggerC - assassin attacking crouched.
static const animframe_t assassin_frames_daggerc[] =
{
	{ FRAME_lndatk1,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, assassin_growl },
	{ FRAME_lndatk2,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_lndatk3,	NULL, 0.0f, 0.0f, 0.0f, assassin_enable_fmnode, MESH__RKNIFE, NULL },
	{ FRAME_lndatk4,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_lndatk5,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL }, // Loop in from an attack, no windup.
	{ FRAME_lndatk6,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_lndatk7,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_lndatk8,	NULL, 0.0f, 0.0f, 0.0f, assassin_crouched_check_attack, 1.0f, NULL },
	{ FRAME_lndatk9,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_lndatk10,	NULL, 0.0f, 0.0f, 0.0f, assassin_crouched_check_attack, 0.0f, NULL }, // Check for loop to other attack.
	{ FRAME_lndatk11,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL }
};
const animmove_t assassin_move_daggerc = ANIMMOVE(assassin_frames_daggerc, assassin_pause);

// Assassin newdagger.
static const animframe_t assassin_frames_newdagger[] =
{
	{ FRAME_newattackA1,	NULL, 0.0f, 0.0f, 0.0f, assassin_enable_fmnode, MESH__RKNIFE, assassin_growl },
	{ FRAME_newattackA2,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA3,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA4,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA5,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA6,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA7,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA8,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA9,	NULL, 0.0f, 0.0f, 0.0f, assassin_attack, BIT_RKNIFE, NULL },
	{ FRAME_newattackA10,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA11,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA12,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA13,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA14,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackA15,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
};
const animmove_t assassin_move_newdagger = ANIMMOVE(assassin_frames_newdagger, assassin_pause);

// Assassin newdaggerB.
static const animframe_t assassin_frames_newdaggerb[] =
{
	{ FRAME_newattackB1,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, assassin_growl },
	{ FRAME_newattackB2,	NULL, 0.0f, 0.0f, 0.0f, assassin_enable_fmnode, MESH__RKNIFE, NULL },
	{ FRAME_newattackB3,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB4,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL }, // Loop in from an attack, no windup.
	{ FRAME_newattackB5,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB6,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB7,	NULL, 0.0f, 0.0f, 0.0f, assassin_enable_fmnode, MESH__LKNIFE, NULL },
	{ FRAME_newattackB8,	NULL, 0.0f, 0.0f, 0.0f, assassin_attack, BIT_RKNIFE, NULL },
	{ FRAME_newattackB9,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB10,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB11,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB12,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL }, // Check for loop to other attack.
	{ FRAME_newattackB13,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB14,	NULL, 0.0f, 0.0f, 0.0f, assassin_attack, BIT_LKNIFE, NULL },
	{ FRAME_newattackB15,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB16,	NULL, 0.0f, 0.0f, 0.0f, MG_AI_Charge, 0.0f, NULL },
	{ FRAME_newattackB17,	NULL, 0.0f, 0.0f, 0.0f, assassin_check_loop, 4, NULL },
};
const animmove_t assassin_move_newdaggerb = ANIMMOVE(assassin_frames_newdaggerb, assassin_pause);

#pragma endregion

#pragma region ========================== ASSASSIN EVASION	 ==========================

// Assassin crouch.
static const animframe_t assassin_frames_crouch[] =
{
	{ FRAME_jump14,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_set_crouched },
	{ FRAME_jump15,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_jump16,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_stop },
};
const animmove_t assassin_move_crouch = ANIMMOVE(assassin_frames_crouch, assassin_pause);

// Assassin uncrouch.
static const animframe_t assassin_frames_uncrouch[] =
{
	{ FRAME_jump17,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_unset_crouched }
};
const animmove_t assassin_move_uncrouch = ANIMMOVE(assassin_frames_uncrouch, assassin_pause);

// Assassin in air.
static const animframe_t assassin_frames_evinair[] =
{
	{ FRAME_jump12,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL } // Hang here until land.
};
const animmove_t assassin_move_evinair = ANIMMOVE(assassin_frames_evinair, NULL);

// Assassin in air.
static const animframe_t assassin_frames_inair[] =
{
	{ FRAME_jump12,	MG_InAirMove, 50, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL } // Hang here until land.
};
const animmove_t assassin_move_inair = ANIMMOVE(assassin_frames_inair, NULL);

// Assassin land.
static const animframe_t assassin_frames_land[] =
{
	{ FRAME_jump13,	assassin_sound, CHAN_BODY, SND_LAND, ATTN_NORM, NULL, 0.0f, NULL },
	{ FRAME_jump14,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_set_crouched },
	{ FRAME_jump15,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_jump16,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_jump17,	NULL, 0.0f, 0.0f, 0.0f, assassin_crouched_check_attack, 2.0f, assassin_unset_crouched }
};
const animmove_t assassin_move_land = ANIMMOVE(assassin_frames_land, assassin_pause);

// Assassin jump.
static const animframe_t assassin_frames_evade_jump[] =
{
	{ FRAME_jump7,	assassin_jump_go, 100.0f, 400.0f, 0.0f, ai_charge, 0.0f, assassin_growl },
	{ FRAME_jump8,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump9,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump10,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump11,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t assassin_move_evade_jump = ANIMMOVE(assassin_frames_evade_jump, assassin_evade_inair_go);

// Assassin backflipping.
static const animframe_t assassin_frames_evade_backflip[] =
{
	{ FRAME_bkflp6,		assassin_jump_go, -150.0f, 400.0f, 0.0f, NULL, 0.0f, assassin_growl },
	{ FRAME_bkflp7,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp8,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp9,		assassin_sound, CHAN_ITEM, SND_FLIP, ATTN_NORM, NULL, 0.0f, NULL },
	{ FRAME_bkflp10,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_BFLAND, NULL },
	{ FRAME_bkflp11,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_BFLAND, NULL },
	{ FRAME_bkflp12,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_BFLAND, NULL },
};
const animmove_t assassin_move_evade_backflip = ANIMMOVE(assassin_frames_evade_backflip, assassin_backflip_inair_go);

// Assassin front flipping.
static const animframe_t assassin_frames_evade_frontflip[] =
{
	{ FRAME_fntflp6,	assassin_jump_go, 150.0f, 400.0f, 0.0f, NULL, 0.0f, assassin_growl },
	{ FRAME_fntflp7,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp8,	assassin_sound, CHAN_ITEM, SND_FLIP, ATTN_NORM, NULL, 0.0f, NULL },
	{ FRAME_fntflp9,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_FFLAND, NULL },
	{ FRAME_fntflp10,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_FFLAND, NULL },
	{ FRAME_fntflp11,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_FFLAND, NULL },
};
const animmove_t assassin_move_evade_frontflip = ANIMMOVE(assassin_frames_evade_frontflip, assassin_fwdflip_inair_go);

// Assassin dodging right.
static const animframe_t assassin_frames_dodge_right[] =
{
	{ FRAME_dgert4,		assassin_sound, CHAN_BODY, SND_SLIDE, ATTN_NORM, ai_moveright, 24.0f, NULL },
	{ FRAME_dgert5,		NULL, 0.0f, 0.0f, 0.0f, ai_moveright, 16.0f, NULL },
	{ FRAME_dgert6,		NULL, 0.0f, 0.0f, 0.0f, ai_moveright, 12.0f, NULL },
	{ FRAME_dgert7,		NULL, 0.0f, 0.0f, 0.0f, ai_moveright, 8.0f, NULL },
	{ FRAME_dgert8,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_dgert9,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_dgert10,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL }
};
const animmove_t assassin_move_dodge_right = ANIMMOVE(assassin_frames_dodge_right, assassin_pause);

// Assassin dodging left.
static const animframe_t assassin_frames_dodge_left[] =
{
	{ FRAME_dgelft5,	assassin_sound, CHAN_BODY, SND_SLIDE, ATTN_NORM, ai_moveright, -24.0f, NULL },
	{ FRAME_dgelft6,	NULL, 0.0f, 0.0f, 0.0f, ai_moveright, -16.0f, NULL },
	{ FRAME_dgelft7,	NULL, 0.0f, 0.0f, 0.0f, ai_moveright, -12.0f, NULL },
	{ FRAME_dgelft8,	NULL, 0.0f, 0.0f, 0.0f, ai_moveright, -8.0f, NULL },
	{ FRAME_dgelft9,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_dgelft10,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL }
};
const animmove_t assassin_move_dodge_left = ANIMMOVE(assassin_frames_dodge_left, assassin_pause);

#pragma endregion

#pragma region ========================== ASSASSIN DEATHS ==========================

// Assassin DeathA
static const animframe_t assassin_frames_deatha[] =
{
	{ FRAME_deatha1,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha2,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha3,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha4,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha5,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha6,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha7,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha8,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, MG_SetNoBlocking },
	{ FRAME_deatha9,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha10,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha11,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha12,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha13,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deatha14,	NULL, 0.0f, 0.0f, 0.0f, NULL, 1.0f, NULL }
};
const animmove_t assassin_move_deatha = ANIMMOVE(assassin_frames_deatha, assassin_dead);

// Assassin DeathB.
static const animframe_t assassin_frames_deathb[] =
{
	{ FRAME_deathb1,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb2,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb3,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb4,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb5,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb6,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, MG_SetNoBlocking },
	{ FRAME_deathb7,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb8,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb9,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb10,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb11,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb12,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb13,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb14,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_deathb15,	NULL, 0.0f, 0.0f, 0.0f, NULL, 1.0f, NULL }
};
const animmove_t assassin_move_deathb = ANIMMOVE(assassin_frames_deathb, assassin_dead);

#pragma endregion

#pragma region ========================== ASSASSIN PURSUING ==========================

// Assassin jump.
static const animframe_t assassin_frames_jump[] =
{
	{ FRAME_jump1,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump2,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump3,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump4,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump5,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump6,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump7,	assassin_jump_go, 50.0f, 500.0f, 0.0f, ai_charge, 0.0f, assassin_growl },
	{ FRAME_jump8,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump9,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump10,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump11,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t assassin_move_jump = ANIMMOVE(assassin_frames_jump, assassin_inair_go);

// Assassin forced jump.
static const animframe_t assassin_frames_forcedjump[] =
{
	{ FRAME_jump1,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump2,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump3,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump4,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump5,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump6,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump7,	assassin_sound, CHAN_VOICE, SND_JUMP, ATTN_NORM, ai_charge, 0.0f, MG_ApplyJump },
	{ FRAME_jump8,	MG_InAirMove, 50.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_jump9,	MG_InAirMove, 50.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump10,	MG_InAirMove, 50.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump11,	MG_InAirMove, 50.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t assassin_move_forcedjump = ANIMMOVE(assassin_frames_forcedjump, assassin_inair_go);

// Assassin forced jump.
static const animframe_t assassin_frames_fjump[] =
{
	{ FRAME_jump1,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump2,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump3,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump4,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump5,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump6,	NULL, 0.0f, 0.0f, 0.0f, ai_charge, 0.0f, NULL },
	{ FRAME_jump7,	assassin_sound, CHAN_VOICE, SND_JUMP, ATTN_NORM, ai_charge, 0.0f, MG_ApplyJump },
	{ FRAME_jump8,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_jump9,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump10,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump11,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t assassin_move_fjump = ANIMMOVE(assassin_frames_fjump, assassin_evade_inair_go);

// Assassin backflipping.
static const animframe_t assassin_frames_bfland[] =
{
	{ FRAME_bkflp14, assassin_sound, CHAN_BODY, SND_LAND, ATTN_NORM, NULL, 0.0f, NULL },
	{ FRAME_bkflp15, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp16, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL }
};
const animmove_t assassin_move_bfland = ANIMMOVE(assassin_frames_bfland, assassin_pause);

// Assassin backflipping.
static const animframe_t assassin_frames_bfinair[] =
{
	{ FRAME_bkflp13, NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_BFLAND, NULL },
};
const animmove_t assassin_move_bfinair = ANIMMOVE(assassin_frames_bfinair, NULL);

// Assassin backflipping.
static const animframe_t assassin_frames_backflip[] =
{
	{ FRAME_bkflp1,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp2,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp3,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp4,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp5,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp6,		assassin_jump_go, -150.0f, 400.0f, 0.0f, NULL, 0.0f, assassin_growl },
	{ FRAME_bkflp7,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp8,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_bkflp9,		assassin_sound, CHAN_ITEM, SND_FLIP, ATTN_NORM, NULL, 0.0f, NULL },
	{ FRAME_bkflp10,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_BFLAND, NULL },
	{ FRAME_bkflp11,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_BFLAND, NULL },
	{ FRAME_bkflp12,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_BFLAND, NULL },
};
const animmove_t assassin_move_backflip = ANIMMOVE(assassin_frames_backflip, assassin_backflip_inair_go);

// Assassin backspring.
static const animframe_t assassin_frames_backspring[] =
{
	{ FRAME_newbackspring1,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -4.0f,  assassin_growl },
	{ FRAME_newbackspring2,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -8.0f,  NULL },
	{ FRAME_newbackspring3,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -12.0f, NULL },
	{ FRAME_newbackspring4,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -12.0f, NULL },
	{ FRAME_newbackspring5,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -16.0f, NULL },
	{ FRAME_newbackspring6,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -18.0f, NULL },
	{ FRAME_newbackspring7,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -24.0f, NULL },
	{ FRAME_newbackspring8,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -16.0f, NULL },
	{ FRAME_newbackspring9,		NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -12.0f, NULL },
	{ FRAME_newbackspring10,	NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -12.0f, NULL },
	{ FRAME_newbackspring11,	NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -10.0f, NULL },
	{ FRAME_newbackspring12,	NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -8.0f,  NULL },
	{ FRAME_newbackspring13,	NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -4.0f,  NULL },
	{ FRAME_newbackspring14,	NULL, 0.0f, 0.0f, 0.0f, ai_charge2, -2.0f,  NULL },
	{ FRAME_newbackspring15,	NULL, 0.0f, 0.0f, 0.0f, ai_charge2,  0.0f,  NULL },
};
const animmove_t assassin_move_backspring = ANIMMOVE(assassin_frames_backspring, assassin_pause);

// Assassin front flipping.
static const animframe_t assassin_frames_ffland[] =
{
	{ FRAME_fntflp12,	assassin_sound, CHAN_BODY, SND_LAND, ATTN_NORM, NULL, 0.0f, NULL },
	{ FRAME_fntflp13,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp14,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp15,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp16,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
};
const animmove_t assassin_move_ffland = ANIMMOVE(assassin_frames_ffland, assassin_pause);

// Assassin front flipping.
static const animframe_t assassin_frames_ffinair[] =
{
	{ FRAME_fntflp11, NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_FFLAND, NULL },
};
const animmove_t assassin_move_ffinair = ANIMMOVE(assassin_frames_ffinair, NULL);

// Assassin front flipping.
static const animframe_t assassin_frames_frontflip[] =
{
	{ FRAME_fntflp1,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp2,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp3,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp4,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp5,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp6,	assassin_jump_go, 150.0f, 400.0f, 0.0f, NULL, 0.0f, assassin_growl },
	{ FRAME_fntflp7,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_fntflp8,	assassin_sound, CHAN_ITEM, SND_FLIP, ATTN_NORM, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_fntflp9,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_fntflp10,	NULL, 0.0f, 0.0f, 0.0f, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t assassin_move_frontflip = ANIMMOVE(assassin_frames_frontflip, assassin_fwdflip_inair_go);

// Assassin running.
static const animframe_t assassin_frames_run[] =
{
	// Recalculate movement values with new anim.
	{ FRAME_run1,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 20.0f, assassin_growl },
	{ FRAME_run2,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 18.0f, assassin_pause },
	{ FRAME_run3,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 12.0f, assassin_pause },
	{ FRAME_run4,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 16.0f, assassin_pause },
	{ FRAME_run5,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 24.0f, assassin_pause },
	{ FRAME_run6,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 18.0f, assassin_pause },
	{ FRAME_run7,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 16.0f, assassin_pause },
	{ FRAME_run8,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 12.0f, assassin_pause },
	{ FRAME_run9,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 18.0f, assassin_pause },
	{ FRAME_run10,	NULL, 0.0f, 0.0f, 0.0f, assassin_run, 26.0f, assassin_pause }
};
const animmove_t assassin_move_run = ANIMMOVE(assassin_frames_run, assassin_pause);

// Assassin walking start.
static const animframe_t assassin_frames_walk[] =
{
	{ FRAME_newwalk1, NULL, 0.0f, 0.0f, 0.0f, ai_walk, 6.0f, NULL },
	{ FRAME_newwalk2, NULL, 0.0f, 0.0f, 0.0f, ai_walk, 6.0f, NULL },
	{ FRAME_newwalk3, NULL, 0.0f, 0.0f, 0.0f, ai_walk, 6.0f, NULL },
};
const animmove_t assassin_move_walk = ANIMMOVE(assassin_frames_walk, assassin_walk_loop_go);

// Assassin walking loop.
static const animframe_t assassin_frames_walk_loop[] =
{
	{ FRAME_newwalk4,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, assassin_growl },
	{ FRAME_newwalk5,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk6,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk7,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk8,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk9,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk10,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk11,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk12,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk13,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
	{ FRAME_newwalk14,	NULL, 0.0f, 0.0f, 0.0f, assassin_ai_walk, 8.0f, NULL },
};
const animmove_t assassin_move_walk_loop = ANIMMOVE(assassin_frames_walk_loop, assassin_pause);

#pragma endregion

#pragma region ========================== ASSASSIN PAINS ==========================

// Assassin Pain - assassin gets hit.
static const animframe_t assassin_frames_pain1[] =
{
	{ FRAME_painb1,	NULL, 0.0f, 0.0f, 0.0f, ai_move, -16.0f, NULL },
};
const animmove_t assassin_move_pain1 = ANIMMOVE(assassin_frames_pain1, assassin_post_pain);

// Assassin Pain - assassin gets hit.
static const animframe_t assassin_frames_pain2[] =
{
	{ FRAME_painb1,	NULL, 0.0f, 0.0f, 0.0f, ai_move, -10.0f, NULL },
	{ FRAME_painb2,	NULL, 0.0f, 0.0f, 0.0f, ai_move, -8.0f, NULL },
	{ FRAME_painb3,	NULL, 0.0f, 0.0f, 0.0f, ai_move, -6.0f, NULL },
	{ FRAME_painb4,	NULL, 0.0f, 0.0f, 0.0f, ai_move, -3.0f, NULL },
	{ FRAME_painb5,	NULL, 0.0f, 0.0f, 0.0f, NULL,     0.0f, NULL }
};
const animmove_t assassin_move_pain2 = ANIMMOVE(assassin_frames_pain2, assassin_post_pain);

#pragma endregion

#pragma region ========================== ASSASSIN WAITING ==========================

// Assassin chillin out.
static const animframe_t assassin_frames_stand[] =
{
	{ FRAME_newidle1,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newidle2,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newidle3,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newidle4,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newidle5,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newidle6,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newidle7,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newidle8,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newidle9,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newidle10,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newidle11,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newidle12,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_stand = ANIMMOVE(assassin_frames_stand, assassin_pause);

// Assassin chillin out.
static const animframe_t assassin_frames_delay[] =
{
	{ FRAME_newidle1,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle2,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle3,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle4,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle5,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle6,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle7,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle8,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle9,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle10,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle11,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
	{ FRAME_newidle12,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_pause },
};
const animmove_t assassin_move_delay = ANIMMOVE(assassin_frames_delay, assassin_pause);

// Assassin crouch start.
static const animframe_t assassin_frames_crouch_trans[] =
{
	{ FRAME_newcrchtrn1, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_newcrchtrn2, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_newcrchtrn3, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_newcrchtrn4, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_newcrchtrn5, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
};
const animmove_t assassin_move_crouch_trans = ANIMMOVE(assassin_frames_crouch_trans, assassin_crouch_idle_decision);

// Assassin crouch idle.
static const animframe_t assassin_frames_crouch_idle[] =
{
	{ FRAME_newcrouchidle1,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newcrouchidle2,		NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrouchidle3,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrouchidle4,		NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrouchidle5,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrouchidle6,		NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrouchidle7,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrouchidle8,		NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrouchidle9,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrouchidle10,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrouchidle11,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrouchidle12,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_idle = ANIMMOVE(assassin_frames_crouch_idle, assassin_crouch_idle_decision);

// Assassin crouch - look left.
static const animframe_t assassin_frames_crouch_look_right[] =
{
	{ FRAME_newcrchlkrit1,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newcrchlkrit2,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit3,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit4,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit5,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit6,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit7,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit8,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit9,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit10,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit11,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit12,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_look_right = ANIMMOVE(assassin_frames_crouch_look_right, assassin_crouch_idle_decision);

// Assassin crouch idle - look left.
static const animframe_t assassin_frames_crouch_look_right_idle[] =
{
	{ FRAME_newcrhlkrtidle1,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newcrhlkrtidle2,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrhlkrtidle3,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrhlkrtidle4,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrhlkrtidle5,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrhlkrtidle6,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrhlkrtidle7,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrhlkrtidle8,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrhlkrtidle9,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrhlkrtidle10,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrhlkrtidle11,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrhlkrtidle12,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_look_right_idle = ANIMMOVE(assassin_frames_crouch_look_right_idle, assassin_crouch_idle_decision);

// Assassin crouch idle - look left to right.
static const animframe_t assassin_frames_crouch_look_l2r[] =
{
	{ FRAME_newcrchlklr1,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newcrchlklr2,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr3,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr4,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr5,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr6,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr7,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr8,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr9,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr10,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr11,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr12,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_look_l2r = ANIMMOVE(assassin_frames_crouch_look_l2r, assassin_crouch_idle_decision);

// Assassin crouch - look left.
static const animframe_t assassin_frames_crouch_look_left[] =
{
	{ FRAME_newcrchlklft1,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newcrchlklft2,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft3,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft4,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft5,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft6,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft7,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft8,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft9,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft10,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft11,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft12,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_look_left = ANIMMOVE(assassin_frames_crouch_look_left, assassin_crouch_idle_decision);

// Assassin crouch - look left idle.
static const animframe_t assassin_frames_crouch_look_left_idle[] =
{
	{ FRAME_newlkleftidle1,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newlkleftidle2,		NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newlkleftidle3,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newlkleftidle4,		NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newlkleftidle5,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newlkleftidle6,		NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newlkleftidle7,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newlkleftidle8,		NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newlkleftidle9,		NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newlkleftidle10,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newlkleftidle11,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newlkleftidle12,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_look_left_idle = ANIMMOVE(assassin_frames_crouch_look_left_idle, assassin_crouch_idle_decision);

// Assassin crouch - look right to left.
static const animframe_t assassin_frames_crouch_look_r2l[] =
{
	{ FRAME_newcrchlklr12,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newcrchlklr11,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr10,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr9,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr8,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr7,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr6,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr5,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr4,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr3,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklr2,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklr1,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_look_r2l = ANIMMOVE(assassin_frames_crouch_look_r2l, assassin_crouch_idle_decision);

// Assassin crouch - look right to center.
static const animframe_t assassin_frames_crouch_look_r2c[] =
{
	{ FRAME_newcrchlkrit12,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newcrchlkrit11,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit10,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit9,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit8,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit7,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit6,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit5,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit4,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit3,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlkrit2,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlkrit1,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_look_r2c = ANIMMOVE(assassin_frames_crouch_look_r2c, assassin_crouch_idle_decision);

// Assassin crouch - look left to center.
static const animframe_t assassin_frames_crouch_look_l2c[] =
{
	{ FRAME_newcrchlklft12,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_newcrchlklft11,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft10,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft9,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft8,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft7,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft6,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft5,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft4,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft3,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_newcrchlklft2,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_newcrchlklft1,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_look_l2c = ANIMMOVE(assassin_frames_crouch_look_l2c, assassin_crouch_idle_decision);

// Assassin crouch end.
static const animframe_t assassin_frames_crouch_end[] =
{
	{ FRAME_newcrchtrn5, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_newcrchtrn4, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_newcrchtrn3, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_newcrchtrn2, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_newcrchtrn1, NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
};
const animmove_t assassin_move_crouch_end = ANIMMOVE(assassin_frames_crouch_end, assassin_crouch_idle_decision);

// Assassin crouch poke.
static const animframe_t assassin_frames_crouch_poke[] =
{
	{ FRAME_poke1,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, assassin_growl },
	{ FRAME_poke2,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke3,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke4,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke5,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke6,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke7,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke8,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke9,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke10,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke11,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke12,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke13,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke14,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke15,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke16,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke17,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke18,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke19,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke20,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke21,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke22,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
	{ FRAME_poke23,	NULL, 0.0f, 0.0f, 0.0f, ai_stand,	0.0f, NULL },
	{ FRAME_poke24,	NULL, 0.0f, 0.0f, 0.0f, NULL,		0.0f, NULL },
};
const animmove_t assassin_move_crouch_poke = ANIMMOVE(assassin_frames_crouch_poke, assassin_crouch_idle_decision);

// Assassin teleport - throws smoke bomb, then gone.
static const animframe_t assassin_frames_teleport[] =
{
	{ FRAME_ataka3,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_skip_frame_skill_check },
	{ FRAME_ataka4,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_skip_frame_skill_check },
	{ FRAME_ataka5,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_skip_frame_skill_check },
	{ FRAME_ataka6,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_skip_frame_skill_check },
	{ FRAME_ataka7,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_skip_frame_skill_check },
	{ FRAME_ataka8,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_skip_frame_skill_check },
	{ FRAME_ataka9,		NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka10,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_ready_teleport },
	{ FRAME_ataka11,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
};
const animmove_t assassin_move_teleport = ANIMMOVE(assassin_frames_teleport, assassin_gone);

// Assassin clock.
static const animframe_t assassin_frames_cloak[] =
{
	{ FRAME_jump14,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_set_crouched },
	{ FRAME_jump15,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_jump16,	NULL, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_init_cloak },
};
const animmove_t assassin_move_cloak = ANIMMOVE(assassin_frames_cloak, assassin_uncrouch);

#pragma endregion

#pragma region ========================== CINEMATIC ANIMATIONS ==========================

// Assassin idling.
static const animframe_t assassin_frames_c_idle1[] =
{
	{ FRAME_ataka1, ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka2, ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka3, ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka4, ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka3, ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka2, ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka1, ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
};
const animmove_t assassin_move_c_idle1 = ANIMMOVE(assassin_frames_c_idle1, ai_c_cycleend);

// Assassin running.
static const animframe_t assassin_frames_c_run1[] =
{
	{ FRAME_run1,	ai_c_move, 20, 0.0f, 0.0f, NULL, 0.0f, assassin_growl },
	{ FRAME_run2,	ai_c_move, 18, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_run3,	ai_c_move, 12, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_run4,	ai_c_move, 16, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_run5,	ai_c_move, 24, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_run6,	ai_c_move, 18, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_run7,	ai_c_move, 16, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_run8,	ai_c_move, 12, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_run9,	ai_c_move, 18, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_run10,	ai_c_move, 26, 0.0f, 0.0f, NULL, 0.0f, NULL }
};
const animmove_t assassin_move_c_run1 = ANIMMOVE(assassin_frames_c_run1, ai_c_cycleend);

// Assassin daggerL - assassin attacking left hand.
static const animframe_t assassin_frames_c_attack1[] =
{
	{ FRAME_ataka1,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka2,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka3,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka4,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka5,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka6,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka7,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka8,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka9,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka10,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka11,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka12,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka13,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_ataka14,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL }
};
const animmove_t assassin_move_c_attack1 = ANIMMOVE(assassin_frames_c_attack1, ai_c_cycleend);

// Assassin daggerR - assassin attacking right hand.
static const animframe_t assassin_frames_c_attack2[] =
{
	{ FRAME_atakb1,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb2,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb3,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb4,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb5,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb6,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb7,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb8,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, assassin_growl },
	{ FRAME_atakb9,		ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb10,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb11,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb12,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb13,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb14,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL },
	{ FRAME_atakb15,	ai_c_move, 0.0f, 0.0f, 0.0f, NULL, 0.0f, NULL }
};
const animmove_t assassin_move_c_attack2 = ANIMMOVE(assassin_frames_c_attack2, ai_c_cycleend);

#pragma endregion
//
// m_tcheckrik_anim.c -- all of the anim frames that used to live in m_insect.h.
//
// Copyright 1998 Raven Software
//

#include "m_tcheckrik_anim.h"
#include "m_tcheckrik_shared.h"
#include "c_ai.h"
#include "g_ai.h"
#include "mg_ai.h" //mxd

// Insect Backpedaling.
static const animframe_t insect_frames_back[] =
{
	{ FRAME_backpeddle1,	NULL, 0, 0, 0, ai_charge, -18, NULL },
	{ FRAME_backpeddle2,	NULL, 0, 0, 0, ai_charge, -16, NULL },
	{ FRAME_backpeddle3,	NULL, 0, 0, 0, ai_charge, -12, NULL },
	{ FRAME_backpeddle4,	NULL, 0, 0, 0, ai_charge, -8,  NULL },
	{ FRAME_backpeddle5,	NULL, 0, 0, 0, ai_charge, -16, NULL },
	{ FRAME_backpeddle6,	NULL, 0, 0, 0, ai_charge, -12, NULL },
	{ FRAME_backpeddle7,	NULL, 0, 0, 0, ai_charge, -20, NULL },
	{ FRAME_backpeddle8,	NULL, 0, 0, 0, ai_charge, -18, NULL },
	{ FRAME_backpeddle9,	NULL, 0, 0, 0, ai_charge, -10, NULL },
	{ FRAME_backpeddle10,	NULL, 0, 0, 0, ai_charge, -6,  NULL },
};
const animmove_t insect_move_back = { 10, insect_frames_back, tcheckrik_pause };

// Insect Death.
static const animframe_t insect_frames_deathfr[] =
{
	{ FRAME_deathfr1,	NULL, 0, 0, 0, ai_move, -24, NULL },
	{ FRAME_deathfr2,	NULL, 0, 0, 0, ai_move, -18, NULL }, // After this frame, goes into knockback?
	{ FRAME_deathfr3,	NULL, 0, 0, 0, ai_move, -12, NULL },
	{ FRAME_deathfr4,	NULL, 0, 0, 0, ai_move, -8,  NULL },
	{ FRAME_deathfr5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr7,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr8,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr9,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr10,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr11,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr16,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr17,	NULL, 0, 0, 0, ai_move, 4,  NULL },
	{ FRAME_deathfr18,	NULL, 0, 0, 0, ai_move, 6,  MG_SetNoBlocking },
	{ FRAME_deathfr19,	NULL, 0, 0, 0, ai_move, 12, NULL },
	{ FRAME_deathfr20,	tcheckrik_sound, CHAN_BODY, SND_THUD, ATTN_NORM, ai_move, 4, NULL },
	{ FRAME_deathfr21,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr22,	NULL, 0, 0, 0, NULL, 0, tcheckrik_dead },
};
const animmove_t insect_move_deathfr = { 22, insect_frames_deathfr, NULL };

// Insect Knockback Start.
static const animframe_t insect_frames_knock1_go[] =
{
	{ FRAME_knock1,	NULL, 0, 0, 0, NULL, 0, tcheckrik_flyback_move },
};
const animmove_t insect_move_knock1_go = { 1, insect_frames_knock1_go, tcheckrik_flyback_loop };

// Insect Knockback Loop.
static const animframe_t insect_frames_knock1_loop[] =
{
	{ FRAME_knock2,	NULL, 0, 0, 0, NULL, 0, tcheckrik_flyback_move },
};
const animmove_t insect_move_knock1_loop = { 1, insect_frames_knock1_loop, NULL };

// Insect Knockback End.
static const animframe_t insect_frames_knock1_end[] =
{
	{ FRAME_knock3,		tcheckrik_sound, CHAN_BODY, SND_THUD, ATTN_NORM, ai_move, -4, tcheckrik_flyback_move },
	{ FRAME_knock4,		NULL, 0, 0, 0, ai_move, -3, tcheckrik_flyback_move },
	{ FRAME_knock5,		NULL, 0, 0, 0, ai_move, -3, tcheckrik_flyback_move },
	{ FRAME_knock6,		NULL, 0, 0, 0, ai_move, -3, tcheckrik_flyback_move },
	{ FRAME_knock7,		NULL, 0, 0, 0, ai_move, -2, tcheckrik_flyback_move },
	{ FRAME_knock8,		NULL, 0, 0, 0, ai_move, -2, tcheckrik_flyback_move },
	{ FRAME_knock9,		NULL, 0, 0, 0, ai_move, -2, tcheckrik_flyback_move },
	{ FRAME_knock10,	NULL, 0, 0, 0, ai_move, -1, tcheckrik_flyback_move },
	{ FRAME_knock11,	NULL, 0, 0, 0, ai_move, -1, tcheckrik_flyback_move },
	{ FRAME_knock12,	NULL, 0, 0, 0, ai_move,  0, tcheckrik_flyback_move },
	{ FRAME_knock13,	NULL, 0, 0, 0, ai_move,  0, tcheckrik_flyback_move },
	{ FRAME_knock14,	NULL, 0, 0, 0, ai_move,  0, tcheckrik_flyback_move },
	{ FRAME_knock15,	NULL, 0, 0, 0, ai_move,  0, MG_SetNoBlocking },
	{ FRAME_knock15,	NULL, 0, 0, 0, NULL,	 0, tcheckrik_wait_twitch },
};
const animmove_t insect_move_knock1_end = { 14, insect_frames_knock1_end, NULL };

// Insect Twitch.
static const animframe_t insect_frames_twitch[] =
{
	// Takes a couple steps back... May clip!
	{ FRAME_knock16,	NULL, 0, 0, 0, NULL, 0, tcheckrik_idle_sound },
	{ FRAME_knock17,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knock18,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knock19,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_knock19,	NULL, 0, 0, 0, NULL, 0, tcheckrik_wait_twitch },
};
const animmove_t insect_move_twitch = { 5, insect_frames_twitch, NULL };

// Insect Idle.
static const animframe_t insect_frames_idle[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle2,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle3,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle4,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle5,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle6,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle7,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle8,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle9,	NULL, 0, 0, 0, ai_stand, 0, tcheckrik_growl },
	{ FRAME_idle10,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle11,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle12,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle13,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle14,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle15,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle16,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle17,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle18,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle19,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle20,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle21,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle22,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle23,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle24,	NULL, 0, 0, 0, ai_stand, 0, tcheckrik_growl },
	{ FRAME_idle25,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle26,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle27,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle28,	NULL, 0, 0, 0, ai_stand, 0, NULL },
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
	{ FRAME_idle41,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle42,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle43,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle44,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle45,	NULL, 0, 0, 0, ai_stand, 0, tcheckrik_growl },
	{ FRAME_idle46,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle47,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle48,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle49,	NULL, 0, 0, 0, ai_stand, 0, NULL },
	{ FRAME_idle50,	NULL, 0, 0, 0, ai_stand, 0, NULL },
};
const animmove_t insect_move_idle = { 50, insect_frames_idle, tcheckrik_pause };

// Insect Land.
static const animframe_t insect_frames_land[] =
{
	{ FRAME_jump11,	tcheckrik_sound, CHAN_BODY, SND_THUD, ATTN_NORM, NULL, 0, NULL }, // Sound.
	{ FRAME_jump12,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump13,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump14,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump15,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump16,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_land = { 6, insect_frames_land, tcheckrik_pause };

// Insect In Air.
static const animframe_t insect_frames_inair[] =
{
	{ FRAME_jump10,	MG_InAirMove, 50, 0, 0, MG_CheckLanded, ANIM_LAND, NULL }, // Hang here until land.
};
const animmove_t insect_move_inair = { 1, insect_frames_inair, NULL };

// Insect Forced Jump.
static const animframe_t insect_frames_forcedjump[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, MG_ApplyJump }, // Off ground.
	{ FRAME_jump5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump7,	MG_InAirMove, 50, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump8,	MG_InAirMove, 50, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump9,	MG_InAirMove, 50, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t insect_move_forcedjump = { 9, insect_frames_forcedjump, tcheckrik_inair_go };

// Insect Forced In Air.
static const animframe_t insect_frames_finair[] =
{
	{ FRAME_jump10,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL }, // Hang here until land.
};
const animmove_t insect_move_finair = { 1, insect_frames_finair, NULL };

// Insect Forced Jump.
static const animframe_t insect_frames_fjump[] =
{
	{ FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, MG_ApplyJump }, // Off ground.
	{ FRAME_jump5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_jump7,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump8,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
	{ FRAME_jump9,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL },
};
const animmove_t insect_move_fjump = { 9, insect_frames_fjump, insect_go_finair };

// Insect Pain A.
static const animframe_t insect_frames_paina[] =
{
	{ FRAME_paina1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina6,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina7,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_paina = { 7, insect_frames_paina, tcheckrik_pause };

// Insect Pain C.
static const animframe_t insect_frames_painc[] =
{
	{ FRAME_paina1,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina2,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina3,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina4,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina5,	NULL, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina6,	NULL, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_painc = { 6, insect_frames_painc, tcheckrik_pause };

// Insect Run.
static const animframe_t insect_frames_run[] =
{
	{ FRAME_Run1,	NULL, 0, 0, 0, MG_AI_Run, 12, tcheckrik_growl },
	{ FRAME_Run2,	NULL, 0, 0, 0, MG_AI_Run, 13, tcheckrik_pause },
	{ FRAME_Run3,	NULL, 0, 0, 0, MG_AI_Run, 14, tcheckrik_pause },
	{ FRAME_Run4,	NULL, 0, 0, 0, MG_AI_Run, 14, tcheckrik_pause },
	{ FRAME_Run5,	NULL, 0, 0, 0, MG_AI_Run, 14, tcheckrik_pause },
	{ FRAME_Run6,	NULL, 0, 0, 0, MG_AI_Run, 14, tcheckrik_pause },
	{ FRAME_Run7,	NULL, 0, 0, 0, MG_AI_Run, 14, tcheckrik_pause },
	{ FRAME_Run8,	NULL, 0, 0, 0, MG_AI_Run, 11, tcheckrik_pause },
	{ FRAME_Run9,	NULL, 0, 0, 0, MG_AI_Run, 14, tcheckrik_pause },
	{ FRAME_Run10,	NULL, 0, 0, 0, MG_AI_Run, 14, tcheckrik_pause },
	{ FRAME_Run10,	NULL, 0, 0, 0, MG_AI_Run, 14, tcheckrik_pause },
};
const animmove_t insect_move_run = { 11, insect_frames_run, tcheckrik_pause };

// Insect Spear Attack.
static const animframe_t insect_frames_spear[] =
{
	{ FRAME_SpearB1,	NULL, 0, 0, 0, ai_charge, 0, tcheckrik_growl },
	{ FRAME_SpearB2,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_SpearB3,	NULL, 0, 0, 0, ai_charge, 0, tcheckrik_staff_attack },
	{ FRAME_SpearB4,	NULL, 0, 0, 0, ai_charge, 0, tcheckrik_staff_attack },
	{ FRAME_SpearB5,	NULL, 0, 0, 0, ai_charge, 0, tcheckrik_staff_attack },
	{ FRAME_SpearB6,	NULL, 0, 0, 0, ai_charge, 0, tcheckrik_staff_attack },
	{ FRAME_SpearB7,	NULL, 0, 0, 0, insectCheckLoop, 3, NULL },
	{ FRAME_SpearB8,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_SpearB9,	NULL, 0, 0, 0, ai_charge, 0, NULL }, // Check for loop.
	{ FRAME_SpearB10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_SpearB11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t insect_move_spear = { 11, insect_frames_spear, tcheckrik_pause };

// Insect Sword Attack.
static const animframe_t insect_frames_sword[] =
{
	{ FRAME_sword1,	NULL, 0, 0, 0, ai_charge, 0, tcheckrik_growl },
	{ FRAME_sword2,	tcheckrik_sound, CHAN_WEAPON, SND_SWIPE, ATTN_NORM, ai_charge, 0, NULL },
	{ FRAME_sword3,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_sword4,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_sword5,	NULL, 0, 0, 0, tcheckrik_attack, TC_ATK_HACK, NULL },
	{ FRAME_sword6,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_sword7,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_sword8,	NULL, 0, 0, 0, insectCheckLoop, 1, NULL },
	{ FRAME_sword9,	NULL, 0, 0, 0, ai_charge, 0, NULL }, // Check for loop.
};
const animmove_t insect_move_sword = { 9, insect_frames_sword, tcheckrik_pause };

// Insect Spell Attack.
static const animframe_t insect_frames_spell[] =
{
	{ FRAME_spell1,		NULL, 0, 0, 0, tcheckrik_spell_attack, TC_SPL_GLOW, NULL },
	{ FRAME_spell2,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell3,		NULL, 0, 0, 0, tcheckrik_spell_attack, TC_SPL_FIRE, NULL },
	{ FRAME_spell4,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell5,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell6,		NULL, 0, 0, 0, tcheckrik_spell_attack, TC_SPL_FIRE, NULL },
	{ FRAME_spell7,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell8,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell9,		NULL, 0, 0, 0, tcheckrik_spell_attack, TC_SPL_FIRE, NULL },
	{ FRAME_spell10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell11,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell12,	NULL, 0, 0, 0, tcheckrik_spell_attack, TC_SPL_FIRE, NULL },
	{ FRAME_spell13,	NULL, 0, 0, 0, ai_charge, 0, tcheckrik_release_spell }, // Here if forwards.
	{ FRAME_spell14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell16,	NULL, 0, 0, 0, ai_charge, 0, NULL }, // Check for loop?
	{ FRAME_spell17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t insect_move_spell = { 17, insect_frames_spell, tcheckrik_pause };

// Insect Spell 2 Attack.
static const animframe_t insect_frames_spell2[] =
{
	{ FRAME_spell1,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell3,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell5,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell6,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell8,		NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell10,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell12,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell13,	NULL, 0, 0, 0, tcheckrik_spell_attack, TC_SPL_FIRE2, NULL },
	{ FRAME_spell14,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell15,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell16,	NULL, 0, 0, 0, ai_charge, 0, NULL },
	{ FRAME_spell17,	NULL, 0, 0, 0, ai_charge, 0, NULL },
};
const animmove_t insect_move_spell2 = { 12, insect_frames_spell2, tcheckrik_pause };

// Insect Walk.
static const animframe_t insect_frames_walk[] =
{
	{ FRAME_Walk1,	NULL, 0, 0, 0, ai_walk, 6, tcheckrik_growl },
	{ FRAME_Walk2,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk3,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk4,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk5,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk6,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk7,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk8,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk9,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk10,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk11,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk12,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk13,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk14,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk15,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk16,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk17,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk18,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk19,	NULL, 0, 0, 0, ai_walk, 6, NULL },
	{ FRAME_Walk20,	NULL, 0, 0, 0, ai_walk, 6, NULL },
};
const animmove_t insect_move_walk = { 20, insect_frames_walk, tcheckrik_pause };

// Insect Delay - insect standing by, not thinking, just waiting for a door or plat or something.
static const animframe_t insect_frames_delay[] =
{
	{ FRAME_idle1,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle2,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle3,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle4,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle5,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle6,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle7,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle8,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle9,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle10,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle11,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle12,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle13,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle14,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle15,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle16,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle17,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle18,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle19,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle20,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle21,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle22,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle23,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle24,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle25,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle26,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle27,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle28,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle29,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle30,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle31,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle32,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle33,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle34,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle35,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle36,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle37,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle38,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle39,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle40,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle41,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle42,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle43,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle44,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle45,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle46,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle47,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle48,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle49,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
	{ FRAME_idle50,	NULL, 0, 0, 0, NULL, 0, tcheckrik_pause },
};
const animmove_t insect_delay = { 50, insect_frames_delay, tcheckrik_pause };

// Insect Cinematic Action 1.
static const animframe_t insect_frames_c_action1[] =
{
	{ FRAME_ts_lean2talk1,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_lean2talk2,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_lean2talk3,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_lean2talk4,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_lean2talk5,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_lean2talk6,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_lean2talk7,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_lean2talk8,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_lean2talk9,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_action1 = { 9, insect_frames_c_action1, ai_c_cycleend };

// Insect Cinematic Action 2.
static const animframe_t insect_frames_c_action2[] =
{
	{ FRAME_ts_listen1,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen2,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen3,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen4,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen5,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen6,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen7,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen8,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen9,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen14,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen15,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen16,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen17,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen18,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen19,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen20,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen21,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen22,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen23,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen24,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen25,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen26,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen27,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen28,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen29,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen30,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen31,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen32,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen33,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen34,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen35,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen36,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen37,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen38,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen39,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen40,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen41,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen42,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen43,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen44,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen45,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen46,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen47,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen48,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen49,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen50,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen51,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen52,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen53,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen54,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen55,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen56,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen57,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen58,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen59,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_listen60,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_action2 = { 60, insect_frames_c_action2, ai_c_cycleend };

// Insect Cinematic Action 3.
static const animframe_t insect_frames_c_action3[] =
{
	{ FRAME_ts_spear1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_spear10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_action3 = { 10, insect_frames_c_action3, ai_c_cycleend };

// Insect Cinematic Action 4.
static const animframe_t insect_frames_c_action4[] =
{
	{ FRAME_ts_talk1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk14,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk15,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk16,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk17,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk18,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk19,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk20,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk21,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk22,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk23,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk24,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk25,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk26,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk27,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk28,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk29,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk30,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk31,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk32,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk33,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk34,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk35,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk36,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk37,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk38,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk39,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk40,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk41,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk42,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk43,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk44,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk45,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk46,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk47,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk48,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk49,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk50,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk51,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk52,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk53,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk54,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk55,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk56,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk57,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk58,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk59,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_talk60,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_action4 = { 60, insect_frames_c_action4, ai_c_cycleend };

// Insect Cinematic Idle 1.
static const animframe_t insect_frames_c_idle1[] =
{
	{ FRAME_idle1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle14,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle15,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle16,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle17,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle18,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle19,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle20,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle21,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle22,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle23,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle24,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle25,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle26,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle27,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle28,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle29,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle30,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle31,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle32,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle33,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle34,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle35,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle36,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle37,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle38,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle39,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle40,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle41,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle42,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle43,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle44,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle45,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle46,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle47,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle48,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle49,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_idle50,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_idle1 = { 50, insect_frames_c_idle1, ai_c_cycleend };

// Insect Cinematic Idle 2.
static const animframe_t insect_frames_c_idle2[] =
{
	{ FRAME_ts_rdy2idle1, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_rdy2idle2, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_rdy2idle3, ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_rdy2idle4, ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_idle2 = { 4, insect_frames_c_idle2, ai_c_cycleend };

// Insect Cinematic Idle 3.
static const animframe_t insect_frames_c_idle3[] =
{
	{ FRAME_ts_idle1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle14,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle15,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle16,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle17,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle18,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle19,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle20,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle21,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle22,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle23,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle24,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle25,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle26,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle27,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle28,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle29,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle30,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle31,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle32,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle33,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle34,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle35,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle36,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle37,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle38,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle39,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle40,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle41,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle42,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle43,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle44,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle45,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle46,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle47,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle48,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle49,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle50,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle51,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle52,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle53,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle54,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle55,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle56,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle57,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle58,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle59,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_ts_idle60,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_idle3 = { 60, insect_frames_c_idle3, ai_c_cycleend };

// Insect Cinematic Run.
static const animframe_t insect_frames_c_run[] =
{
	{ FRAME_Run1,	ai_c_move, 12, 0, 0, NULL, 0, NULL },
	{ FRAME_Run2,	ai_c_move, 13, 0, 0, NULL, 0, NULL },
	{ FRAME_Run3,	ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_Run4,	ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_Run5,	ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_Run6,	ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_Run7,	ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_Run8,	ai_c_move, 11, 0, 0, NULL, 0, NULL },
	{ FRAME_Run9,	ai_c_move, 14, 0, 0, NULL, 0, NULL },
	{ FRAME_Run10,	ai_c_move, 14, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_run = { 10, insect_frames_c_run, ai_c_cycleend };

// Insect Cinematic Spear Attack.
static const animframe_t insect_frames_c_attack1[] =
{
	{ FRAME_SpearB1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB9,	ai_c_move, 0, 0, 0, NULL, 0, NULL }, // Check for loop.
	{ FRAME_SpearB10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_SpearB11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_attack1 = { 11, insect_frames_c_attack1, ai_c_cycleend };

// Insect Cinematic Spell Attack.
static const animframe_t insect_frames_c_attack2[] =
{
	{ FRAME_spell1,		ai_c_move, 0, 0, 0, NULL, TC_SPL_GLOW, NULL },
	{ FRAME_spell2,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell3,		ai_c_move, 0, 0, 0, NULL, TC_SPL_FIRE, NULL },
	{ FRAME_spell4,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell5,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell6,		ai_c_move, 0, 0, 0, NULL, TC_SPL_FIRE, NULL },
	{ FRAME_spell7,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell8,		ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell9,		ai_c_move, 0, 0, 0, NULL, TC_SPL_FIRE, NULL },
	{ FRAME_spell10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell12,	ai_c_move, 0, 0, 0, NULL, TC_SPL_FIRE, NULL },
	{ FRAME_spell13,	ai_c_move, 0, 0, 0, NULL, 0, NULL }, // Here if forwards.
	{ FRAME_spell14,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell15,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_spell16,	ai_c_move, 0, 0, 0, NULL, 0, NULL }, // Check for loop?
	{ FRAME_spell17,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_attack2 = { 17, insect_frames_c_attack2, ai_c_cycleend };

// Insect Cinematic Sword Attack.
static const animframe_t insect_frames_c_attack3[] =
{
	{ FRAME_sword1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sword2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sword3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sword4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sword5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sword6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sword7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_sword8,	ai_c_move, 0, 0, 0, NULL, 1, NULL },
	{ FRAME_sword9,	ai_c_move, 0, 0, 0, NULL, 0, NULL }, // Check for loop.
};
const animmove_t insect_move_c_attack3 = { 9, insect_frames_c_attack3, ai_c_cycleend };

// Insect Cinematic Backpedal.
static const animframe_t insect_frames_c_backpedal[] =
{
	{ FRAME_backpeddle1,	ai_c_move, -10, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle2,	ai_c_move, -12, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle3,	ai_c_move, -10, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle4,	ai_c_move, -12, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle5,	ai_c_move, -10, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle6,	ai_c_move, -12, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle7,	ai_c_move, -10, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle8,	ai_c_move, -12, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle9,	ai_c_move, -10, 0, 0, NULL, 0, NULL },
	{ FRAME_backpeddle10,	ai_c_move, -12, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_backpedal = { 10, insect_frames_c_backpedal, ai_c_cycleend };

// Insect Cinematic Death.
static const animframe_t insect_frames_c_death1[] =
{
	// Takes a couple steps back... May clip!
	{ FRAME_deathfr1,	ai_c_move, -24, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr2,	ai_c_move, -18, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr3,	ai_c_move, -12, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr4,	ai_c_move, -8,  0, 0, NULL, 0, NULL },
	{ FRAME_deathfr5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr8,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr9,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr10,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr11,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr12,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr13,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr14,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr15,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr16,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr17,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr18,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr19,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr20,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr21,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_deathfr22, tcheckrik_sound, CHAN_BODY, SND_THUD, ATTN_NORM, NULL, 0, NULL },
};
const animmove_t insect_move_c_death1 = { 22, insect_frames_c_death1, tcheckrik_c_dead };

// Insect Cinematic Pain.
static const animframe_t insect_frames_c_pain1[] =
{
	{ FRAME_paina1,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina2,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina3,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina4,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina5,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina6,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
	{ FRAME_paina7,	ai_c_move, 0, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_pain1 = { 7, insect_frames_c_pain1, ai_c_cycleend };

// Insect Cinematic Walk.
static const animframe_t insect_frames_c_walk[] =
{
	{ FRAME_Walk1,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk2,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk3,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk4,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk5,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk6,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk7,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk8,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk9,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk10,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk11,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk12,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk13,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk14,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk15,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk16,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk17,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk18,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk19,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
	{ FRAME_Walk20,	ai_c_move, 4, 0, 0, NULL, 0, NULL },
};
const animmove_t insect_move_c_walk = { 20, insect_frames_c_walk, ai_c_cycleend };
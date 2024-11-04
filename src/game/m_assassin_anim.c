//==============================================================================
//
// m_assassin_anim.c
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

#include "c_ai.h"
#include "m_assassin_anim.h"
#include "m_assassin.h"
#include "g_monster.h"

//all of the anim frames that used to live in m_assassin.h

void ai_moveright(edict_t *self, float dist);


//==========================================================================

//ATTACKS

//==========================================================================

/*----------------------------------------------------------------------
  assassin daggerL - assassin attacking left hand
-----------------------------------------------------------------------*/
animframe_t assassin_frames_daggerl [] =
{
	FRAME_ataka1,	NULL, 0, 0, 0, mg_ai_charge, 0, assassingrowl,
	FRAME_ataka2,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka3,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka4,	NULL, 0, 0, 0, assassinNodeOn, MESH__LKNIFE, NULL,//loop in from an attack, no windup
	FRAME_ataka5,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka6,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka7,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka8,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka9,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka10,	NULL, 0, 0, 0, assassindagger, BIT_LKNIFE, NULL,
	FRAME_ataka11,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka12,	NULL, 0, 0, 0, assassinCheckLoop, 2, NULL,//check for loop to other attack
	FRAME_ataka13,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_ataka14,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL
};
animmove_t assassin_move_daggerl = {14, assassin_frames_daggerl, assassin_pause};

/*----------------------------------------------------------------------
  assassin daggerR - assassin attacking right hand
-----------------------------------------------------------------------*/
animframe_t assassin_frames_daggerr [] =
{
	FRAME_atakb1,	NULL, 0, 0, 0, mg_ai_charge, 0, assassingrowl,
	FRAME_atakb2,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb3,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb4,	NULL, 0, 0, 0, assassinNodeOn, MESH__RKNIFE, NULL,
	FRAME_atakb5,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb6,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,//loop in from an attack, no windup
	FRAME_atakb7,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb8,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb9,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb10,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb11,	NULL, 0, 0, 0, assassindagger, BIT_RKNIFE, NULL,
	FRAME_atakb12,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb13,	NULL, 0, 0, 0, assassinCheckLoop, 2, NULL,//check for loop to other attack
	FRAME_atakb14,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakb15,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL
};
animmove_t assassin_move_daggerr = {15, assassin_frames_daggerr, assassin_pause};

/*----------------------------------------------------------------------
  assassin daggerB - assassin attacking left hand
-----------------------------------------------------------------------*/
animframe_t assassin_frames_daggerb [] =
{
	FRAME_atakc1,	NULL, 0, 0, 0, mg_ai_charge, 0, assassingrowl,
	FRAME_atakc2,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakc3,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakc4,	NULL, 0, 0, 0, assassinNodeOn, MESH__LKNIFE, NULL,
	FRAME_atakc5,	NULL, 0, 0, 0, assassinNodeOn, MESH__RKNIFE, NULL,
	FRAME_atakc6,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakc7,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,//loop in from an attack, no windup
	FRAME_atakc8,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakc9,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakc10,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakc11,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_atakc12,	NULL, 0, 0, 0, assassindagger, BIT_LKNIFE|BIT_RKNIFE, NULL,
	FRAME_atakc13,	NULL, 0, 0, 0, assassinCheckLoop, 2, NULL,//check for loop to other attack
	FRAME_atakc14,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
};
animmove_t assassin_move_daggerb = {14, assassin_frames_daggerb, assassin_pause};

/*----------------------------------------------------------------------
  assassin daggerC - assassin attacking crouched
-----------------------------------------------------------------------*/
animframe_t assassin_frames_daggerc [] =
{
	FRAME_lndatk1,	NULL, 0, 0, 0, mg_ai_charge, 0, assassingrowl,
	FRAME_lndatk2,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_lndatk3,	NULL, 0, 0, 0, assassinNodeOn, MESH__RKNIFE, NULL,
	FRAME_lndatk4,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_lndatk5,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,//loop in from an attack, no windup
	FRAME_lndatk6,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_lndatk7,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_lndatk8,	NULL, 0, 0, 0, assassinCrouchedCheckAttack, true, NULL,
	FRAME_lndatk9,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_lndatk10,	NULL, 0, 0, 0, assassinCrouchedCheckAttack, 0, NULL,//check for loop to other attack
	FRAME_lndatk11,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL
};
animmove_t assassin_move_daggerc = {11, assassin_frames_daggerc, assassin_pause};

animframe_t assassin_frames_newdagger [] =
{
	FRAME_newattackA1,	NULL, 0, 0, 0, assassinNodeOn, MESH__RKNIFE, assassingrowl,
	FRAME_newattackA2,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA3,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA4,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA5,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA6,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA7,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA8,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA9,	NULL, 0, 0, 0, assassindagger, BIT_RKNIFE, NULL,
	FRAME_newattackA10,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA11,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA12,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA13,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA14,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackA15,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
};
animmove_t assassin_move_newdagger = {15, assassin_frames_newdagger, assassin_pause};

animframe_t assassin_frames_newdaggerb [] =
{
	FRAME_newattackB1,	NULL, 0, 0, 0, mg_ai_charge, 0, assassingrowl,
	FRAME_newattackB2,	NULL, 0, 0, 0, assassinNodeOn, MESH__RKNIFE, NULL,
	FRAME_newattackB3,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB4,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,//loop in from an attack, no windup
	FRAME_newattackB5,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB6,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB7,	NULL, 0, 0, 0, assassinNodeOn, MESH__LKNIFE, NULL,
	FRAME_newattackB8,	NULL, 0, 0, 0, assassindagger, BIT_RKNIFE, NULL,
	FRAME_newattackB9,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB10,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB11,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB12,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,//check for loop to other attack
	FRAME_newattackB13,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB14,	NULL, 0, 0, 0, assassindagger, BIT_LKNIFE, NULL,
	FRAME_newattackB15,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB16,	NULL, 0, 0, 0, mg_ai_charge, 0, NULL,
	FRAME_newattackB17,	NULL, 0, 0, 0, assassinCheckLoop, 4, NULL,
};
animmove_t assassin_move_newdaggerb = {17, assassin_frames_newdaggerb, assassin_pause};
//===========================================================================

// ASSASSIN EVASION	

//=============================================================================

/*----------------------------------------------------------------------
  assassin crouch
-----------------------------------------------------------------------*/
animframe_t assassin_frames_crouch [] =
{
	FRAME_jump14,	NULL, 0, 0, 0, NULL, 0, assassinSetCrouched,
	FRAME_jump15,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jump16,	NULL, 0, 0, 0, NULL, 0, assassinStop,
};
animmove_t assassin_move_crouch = {3, assassin_frames_crouch, assassin_pause};

/*----------------------------------------------------------------------
  assassin uncrouch
-----------------------------------------------------------------------*/
animframe_t assassin_frames_uncrouch [] =
{
	FRAME_jump17,	NULL, 0, 0, 0, NULL, 0, assassinUndoCrouched
};
animmove_t assassin_move_uncrouch = {1, assassin_frames_uncrouch, assassin_pause};

/*----------------------------------------------------------------------
  assassin in air
-----------------------------------------------------------------------*/
animframe_t assassin_frames_evinair [] =
{
	FRAME_jump12,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL//hang here until land
};
animmove_t assassin_move_evinair = {1, assassin_frames_evinair, NULL};

/*----------------------------------------------------------------------
  assassin in air
-----------------------------------------------------------------------*/
animframe_t assassin_frames_inair [] =
{
	FRAME_jump12,	MG_InAirMove, 50, 0, 0, MG_CheckLanded, ANIM_LAND, NULL//hang here until land
};
animmove_t assassin_move_inair = {1, assassin_frames_inair, NULL};

/*----------------------------------------------------------------------
  assassin land
-----------------------------------------------------------------------*/
animframe_t assassin_frames_land [] =
{
	FRAME_jump13,	assassin_sound, CHAN_BODY, SND_LAND, ATTN_NORM, NULL, 0, NULL,
	FRAME_jump14,	NULL, 0, 0, 0, NULL, 0, assassinSetCrouched,
	FRAME_jump15,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jump16,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jump17,	NULL, 0, 0, 0, assassinCrouchedCheckAttack, 2, assassinUndoCrouched
};
animmove_t assassin_move_land = {5, assassin_frames_land, assassin_pause};

/*----------------------------------------------------------------------
  assassin jump
-----------------------------------------------------------------------*/
animframe_t assassin_frames_evade_jump [] =
{
	FRAME_jump7,	assassinGoJump, 100, 400, 0, ai_charge, 0, assassingrowl,
	FRAME_jump8,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump9,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_jump10,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_jump11,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
};
animmove_t assassin_move_evade_jump = {5, assassin_frames_evade_jump, assassin_go_evinair};

/*----------------------------------------------------------------------
  assassin backflipping
-----------------------------------------------------------------------*/
animframe_t assassin_frames_evade_backflip [] =
{
	FRAME_bkflp6,	assassinGoJump, -150, 400, 0, NULL, 0, assassingrowl,
	FRAME_bkflp7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp9,	assassin_sound, CHAN_ITEM, SND_FLIP, ATTN_NORM, NULL, 0, NULL,
	FRAME_bkflp10,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_BFLAND, NULL,
	FRAME_bkflp11,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_BFLAND, NULL,
	FRAME_bkflp12,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_BFLAND, NULL,
};
animmove_t assassin_move_evade_backflip = {7, assassin_frames_evade_backflip, assassin_go_bfinair};

/*----------------------------------------------------------------------
  assassin front flipping
-----------------------------------------------------------------------*/
animframe_t assassin_frames_evade_frontflip [] =
{
	FRAME_fntflp6,	assassinGoJump, 150, 400, 0, NULL, 0, assassingrowl,
	FRAME_fntflp7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp8,	assassin_sound, CHAN_ITEM, SND_FLIP, ATTN_NORM, NULL, 0, NULL,
	FRAME_fntflp9,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_FFLAND, NULL,
	FRAME_fntflp10, NULL, 0, 0, 0, MG_CheckLanded, ANIM_FFLAND, NULL,
	FRAME_fntflp11, NULL, 0, 0, 0, MG_CheckLanded, ANIM_FFLAND, NULL,
};
animmove_t assassin_move_evade_frontflip = {6, assassin_frames_evade_frontflip, assassin_go_ffinair};

/*----------------------------------------------------------------------
  assassin dodging right
-----------------------------------------------------------------------*/
animframe_t assassin_frames_dodge_right [] =
{
//	FRAME_dgert1,	NULL, 0, 0, 0, ai_moveright, 10, NULL,
//	FRAME_dgert2,	NULL, 0, 0, 0, ai_moveright, 11, NULL,
//	FRAME_dgert3,	NULL, 0, 0, 0, ai_moveright, 12, NULL,
	FRAME_dgert4,	assassin_sound, CHAN_BODY, SND_SLIDE, ATTN_NORM, ai_moveright, 24, NULL,
	FRAME_dgert5,	NULL, 0, 0, 0, ai_moveright, 16, NULL,
	FRAME_dgert6,	NULL, 0, 0, 0, ai_moveright, 12, NULL,
	FRAME_dgert7,	NULL, 0, 0, 0, ai_moveright, 8, NULL,
	FRAME_dgert8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dgert9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dgert10, NULL, 0, 0, 0, NULL, 0, NULL
};
animmove_t assassin_move_dodge_right = {7, assassin_frames_dodge_right, assassin_pause};

/*----------------------------------------------------------------------
  assassin dodging left
-----------------------------------------------------------------------*/
animframe_t assassin_frames_dodge_left [] =
{
//	FRAME_dgelft1,	NULL, 0, 0, 0, ai_moveright, -10, NULL,
//	FRAME_dgelft2,	NULL, 0, 0, 0, ai_moveright, -11, NULL,
//	FRAME_dgelft3,	NULL, 0, 0, 0, ai_moveright, -12, NULL,
//	FRAME_dgelft4,	NULL, 0, 0, 0, ai_moveright, -12, NULL,
	FRAME_dgelft5,	assassin_sound, CHAN_BODY, SND_SLIDE, ATTN_NORM, ai_moveright, -24, NULL,
	FRAME_dgelft6,	NULL, 0, 0, 0, ai_moveright, -16, NULL,
	FRAME_dgelft7,	NULL, 0, 0, 0, ai_moveright, -12, NULL,
	FRAME_dgelft8,	NULL, 0, 0, 0, ai_moveright, -8, NULL,
	FRAME_dgelft9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dgelft10, NULL, 0, 0, 0, NULL, 0, NULL
};
animmove_t assassin_move_dodge_left = {6, assassin_frames_dodge_left, assassin_pause};

//==============================================================================

// ASSASSIN DEATHS

//==============================================================================

/*----------------------------------------------------------------------
  assassin DeathA 
-----------------------------------------------------------------------*/
animframe_t assassin_frames_deatha [] =
{
	FRAME_deatha1, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deatha2, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deatha3, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deatha4, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deatha5, NULL, 0, 0, 0, NULL, 0, NULL,    
	FRAME_deatha6, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deatha7, NULL, 0, 0, 0, NULL, 0, NULL,    
	FRAME_deatha8, NULL, 0, 0, 0, NULL, 0, MG_NoBlocking,    
	FRAME_deatha9, NULL, 0, 0, 0, NULL, 0, NULL,    
	FRAME_deatha10, NULL, 0, 0, 0, NULL, 0, NULL,   
	FRAME_deatha11, NULL, 0, 0, 0, NULL, 0, NULL,   
	FRAME_deatha12, NULL, 0, 0, 0, NULL, 0, NULL,   
	FRAME_deatha13, NULL, 0, 0, 0, NULL, 0, NULL,   
	FRAME_deatha14, NULL, 0, 0, 0, NULL, 1, NULL   
};
animmove_t assassin_move_deatha = {14, assassin_frames_deatha, assassin_dead};

/*-------------------------------------------------------------------------
	assassin Death B
-------------------------------------------------------------------------*/
animframe_t assassin_frames_deathb [] =
{
	FRAME_deathb1, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deathb2, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deathb3, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deathb4, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deathb5, NULL, 0, 0, 0, NULL, 0, NULL,    
	FRAME_deathb6, NULL, 0, 0, 0, NULL, 0, MG_NoBlocking,  
	FRAME_deathb7, NULL, 0, 0, 0, NULL, 0, NULL,    
	FRAME_deathb8, NULL, 0, 0, 0, NULL, 0, NULL,    
	FRAME_deathb9, NULL, 0, 0, 0, NULL, 0, NULL,    
	FRAME_deathb10, NULL, 0, 0, 0, NULL, 0, NULL,   
	FRAME_deathb11, NULL, 0, 0, 0, NULL, 0, NULL,   
	FRAME_deathb12, NULL, 0, 0, 0, NULL, 0, NULL,   
	FRAME_deathb13, NULL, 0, 0, 0, NULL, 0, NULL,  
	FRAME_deathb14, NULL, 0, 0, 0, NULL, 0, NULL,   
	FRAME_deathb15, NULL, 0, 0, 0, NULL, 1, NULL
};
animmove_t assassin_move_deathb = {15, assassin_frames_deathb, assassin_dead};

//=============================================================================

// ASSASSIN PURSUING

//=============================================================================

/*----------------------------------------------------------------------
  assassin jump
-----------------------------------------------------------------------*/
animframe_t assassin_frames_jump [] =
{
	FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump7,	assassinGoJump, 50, 500, 0, ai_charge, 0, assassingrowl,
	FRAME_jump8,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump9,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_jump10,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_jump11,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
};
animmove_t assassin_move_jump = {11, assassin_frames_jump, assassin_go_inair};

/*----------------------------------------------------------------------
  assassin forced jump
-----------------------------------------------------------------------*/
animframe_t assassin_frames_forcedjump [] =
{
	FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump7,	assassin_sound, CHAN_VOICE, SND_JUMP, ATTN_NORM, ai_charge, 0, MG_ApplyJump,
	FRAME_jump8,	MG_InAirMove, 50, 0, 0, NULL, 0, NULL,
	FRAME_jump9,	MG_InAirMove, 50, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_jump10,	MG_InAirMove, 50, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_jump11,	MG_InAirMove, 50, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
};
animmove_t assassin_move_forcedjump = {11, assassin_frames_forcedjump, assassin_go_inair};


/*----------------------------------------------------------------------
  assassin forced jump
-----------------------------------------------------------------------*/
animframe_t assassin_frames_fjump [] =
{
	FRAME_jump1,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump2,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump3,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump4,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump5,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump6,	NULL, 0, 0, 0, ai_charge, 0, NULL,
	FRAME_jump7,	assassin_sound, CHAN_VOICE, SND_JUMP, ATTN_NORM, ai_charge, 0, MG_ApplyJump,
	FRAME_jump8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jump9,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_jump10,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_jump11,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
};
animmove_t assassin_move_fjump = {11, assassin_frames_fjump, assassin_go_evinair};
//BACKFLIP
/*----------------------------------------------------------------------
  assassin backflipping
-----------------------------------------------------------------------*/
animframe_t assassin_frames_bfland [] =
{
	FRAME_bkflp14,	assassin_sound, CHAN_BODY, SND_LAND, ATTN_NORM, NULL, 0, NULL,
	FRAME_bkflp15,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp16,	NULL, 0, 0, 0, NULL, 0, NULL
};
animmove_t assassin_move_bfland = {3, assassin_frames_bfland, assassin_pause};

/*----------------------------------------------------------------------
  assassin backflipping
-----------------------------------------------------------------------*/
animframe_t assassin_frames_bfinair [] =
{
	FRAME_bkflp13,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_BFLAND, NULL,
};
animmove_t assassin_move_bfinair = {1, assassin_frames_bfinair, NULL};

/*----------------------------------------------------------------------
  assassin backflipping
-----------------------------------------------------------------------*/
animframe_t assassin_frames_backflip [] =
{
	FRAME_bkflp1,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp6,	assassinGoJump, -150, 400, 0, NULL, 0, assassingrowl,
	FRAME_bkflp7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp8,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_bkflp9,	assassin_sound, CHAN_ITEM, SND_FLIP, ATTN_NORM, NULL, 0, NULL,
	FRAME_bkflp10,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_BFLAND, NULL,
	FRAME_bkflp11,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_BFLAND, NULL,
	FRAME_bkflp12,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_BFLAND, NULL,
};
animmove_t assassin_move_backflip = {12, assassin_frames_backflip, assassin_go_bfinair};


animframe_t assassin_frames_backspring [] =
{
	FRAME_newbackspring1, NULL, 0, 0, 0, ai_charge2, -4, assassingrowl, 
	FRAME_newbackspring2, NULL, 0, 0, 0, ai_charge2, -8, NULL, 
	FRAME_newbackspring3, NULL, 0, 0, 0, ai_charge2, -12, NULL, 
	FRAME_newbackspring4, NULL, 0, 0, 0, ai_charge2, -12, NULL, 
	FRAME_newbackspring5, NULL, 0, 0, 0, ai_charge2, -16, NULL, 
	FRAME_newbackspring6, NULL, 0, 0, 0, ai_charge2, -18, NULL, 
	FRAME_newbackspring7, NULL, 0, 0, 0, ai_charge2, -24, NULL, 
	FRAME_newbackspring8, NULL, 0, 0, 0, ai_charge2, -16, NULL, 
	FRAME_newbackspring9, NULL, 0, 0, 0, ai_charge2, -12, NULL, 
	FRAME_newbackspring10, NULL, 0, 0, 0, ai_charge2, -12, NULL, 
	FRAME_newbackspring11, NULL, 0, 0, 0, ai_charge2, -10, NULL, 
	FRAME_newbackspring12, NULL, 0, 0, 0, ai_charge2, -8, NULL, 
	FRAME_newbackspring13, NULL, 0, 0, 0, ai_charge2, -4, NULL, 
	FRAME_newbackspring14, NULL, 0, 0, 0, ai_charge2, -2, NULL, 
	FRAME_newbackspring15, NULL, 0, 0, 0, ai_charge2, 0, NULL, 
};
animmove_t assassin_move_backspring = {15, assassin_frames_backspring, assassin_pause};

//FRONT FLIP
/*----------------------------------------------------------------------
  assassin front flipping
-----------------------------------------------------------------------*/
animframe_t assassin_frames_ffland [] =
{
	FRAME_fntflp12,	assassin_sound, CHAN_BODY, SND_LAND, ATTN_NORM, NULL, 0, NULL,
	FRAME_fntflp13,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp14,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp15,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp16,	NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t assassin_move_ffland = {5, assassin_frames_ffland, assassin_pause};

/*----------------------------------------------------------------------
  assassin front flipping
-----------------------------------------------------------------------*/
animframe_t assassin_frames_ffinair [] =
{
	FRAME_fntflp11, NULL, 0, 0, 0, MG_CheckLanded, ANIM_FFLAND, NULL,
};
animmove_t assassin_move_ffinair = {1, assassin_frames_ffinair, NULL};

/*----------------------------------------------------------------------
  assassin front flipping
-----------------------------------------------------------------------*/
animframe_t assassin_frames_frontflip [] =
{
	FRAME_fntflp1,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp2,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp3,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp4,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp5,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp6,	assassinGoJump, 150, 400, 0, NULL, 0, assassingrowl,
	FRAME_fntflp7,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_fntflp8,	assassin_sound, CHAN_ITEM, SND_FLIP, ATTN_NORM, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_fntflp9,	NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
	FRAME_fntflp10, NULL, 0, 0, 0, MG_CheckLanded, ANIM_LAND, NULL,
};
animmove_t assassin_move_frontflip = {10, assassin_frames_frontflip, assassin_go_ffinair};









/*----------------------------------------------------------------------
  assassin Running - assassin running 
-----------------------------------------------------------------------*/
animframe_t assassin_frames_run [] =
{//recalc movement values with new anim
	FRAME_run1,	NULL, 0, 0, 0, assassin_go_run, 20, assassingrowl,
	FRAME_run2,	NULL, 0, 0, 0, assassin_go_run, 18, assassin_pause,
	FRAME_run3,	NULL, 0, 0, 0, assassin_go_run, 12, assassin_pause,
	FRAME_run4,	NULL, 0, 0, 0, assassin_go_run, 16, assassin_pause,
	FRAME_run5,	NULL, 0, 0, 0, assassin_go_run, 24, assassin_pause,
	FRAME_run6,	NULL, 0, 0, 0, assassin_go_run, 18, assassin_pause,
	FRAME_run7,	NULL, 0, 0, 0, assassin_go_run, 16, assassin_pause,
	FRAME_run8,	NULL, 0, 0, 0, assassin_go_run, 12, assassin_pause,
	FRAME_run9,	NULL, 0, 0, 0, assassin_go_run, 18, assassin_pause,
	FRAME_run10, NULL, 0, 0, 0, assassin_go_run, 26, assassin_pause
};
animmove_t assassin_move_run = {10, assassin_frames_run, assassin_pause};

animframe_t assassin_frames_walk [] =
{
	FRAME_newwalk1, NULL, 0, 0, 0, ai_walk, 6, NULL, 
	FRAME_newwalk2, NULL, 0, 0, 0, ai_walk, 6, NULL, 
	FRAME_newwalk3, NULL, 0, 0, 0, ai_walk, 6, NULL, 
};
animmove_t assassin_move_walk = {3, assassin_frames_walk, assasin_walk_loop_go};

animframe_t assassin_frames_walk_loop [] =
{
	FRAME_newwalk4, NULL, 0, 0, 0, assassin_ai_walk, 8, assassingrowl, 
	FRAME_newwalk5, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk6, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk7, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk8, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk9, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk10, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk11, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk12, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk13, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
	FRAME_newwalk14, NULL, 0, 0, 0, assassin_ai_walk, 8, NULL, 
};
animmove_t assassin_move_walk_loop = {11, assassin_frames_walk_loop, assassin_pause};

//=============================================================================

// ASSASSIN PAINS

//=============================================================================

/*----------------------------------------------------------------------
  assassin Pain - assassin gets hit 
-----------------------------------------------------------------------*/

animframe_t assassin_frames_pain1 [] =
{	
	FRAME_painb1,	NULL, 0, 0, 0, ai_move, -16, assassinsqueal,
};
animmove_t assassin_move_pain1 = {1, assassin_frames_pain1, assassin_post_pain};


/*----------------------------------------------------------------------
  assassin Pain - assassin gets hit 
-----------------------------------------------------------------------*/
animframe_t assassin_frames_pain2 [] =
{	
	FRAME_painb1,	NULL, 0, 0, 0, ai_move, -10, assassinsqueal,
	FRAME_painb2,	NULL, 0, 0, 0, ai_move, -8, NULL,
	FRAME_painb3,	NULL, 0, 0, 0, ai_move, -6, NULL,
	FRAME_painb4,	NULL, 0, 0, 0, ai_move, -3, NULL,
	FRAME_painb5,	NULL, 0, 0, 0, NULL, 0, NULL
};
animmove_t assassin_move_pain2 = {5, assassin_frames_pain2, assassin_post_pain};

//======================================================================

//ASSASSIN WAITING

//=======================================================================
/*----------------------------------------------------------------------
  assassin chillin out
-----------------------------------------------------------------------*/

animframe_t assassin_frames_stand [] =
{
	FRAME_newidle1, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newidle2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newidle3, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newidle4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newidle5, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newidle6, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newidle7, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newidle8, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newidle9, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newidle10, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newidle11, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newidle12, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_stand = {12, assassin_frames_stand, assassin_pause};

animframe_t assassin_frames_delay [] =
{
	FRAME_newidle1, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle2, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle3, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle4, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle5, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle6, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle7, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle8, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle9, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle10, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle11, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
	FRAME_newidle12, NULL, 0, 0, 0, NULL, 0, assassin_pause, 
};
animmove_t assassin_move_delay = {12, assassin_frames_delay, assassin_pause};

//crouches

animframe_t assassin_frames_crouch_trans [] =
{
	FRAME_newcrchtrn1, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchtrn2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchtrn3, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchtrn4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchtrn5, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_trans = {5, assassin_frames_crouch_trans, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_idle [] =
{
	FRAME_newcrouchidle1, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newcrouchidle2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrouchidle3, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrouchidle4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrouchidle5, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrouchidle6, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrouchidle7, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrouchidle8, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrouchidle9, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrouchidle10, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrouchidle11, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrouchidle12, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_idle = {12, assassin_frames_crouch_idle, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_look_right[] =
{
	FRAME_newcrchlkrit1, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newcrchlkrit2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit3, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit5, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit6, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit7, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit8, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit9, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit10, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit11, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit12, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_look_right = {12, assassin_frames_crouch_look_right, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_look_right_idle[] =
{
	FRAME_newcrhlkrtidle1, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newcrhlkrtidle2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrhlkrtidle3, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrhlkrtidle4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrhlkrtidle5, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrhlkrtidle6, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrhlkrtidle7, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrhlkrtidle8, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrhlkrtidle9, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrhlkrtidle10, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrhlkrtidle11, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrhlkrtidle12, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_look_right_idle = {12, assassin_frames_crouch_look_right_idle, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_look_l2r[] =
{
	FRAME_newcrchlklr1, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newcrchlklr2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr3, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr5, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr6, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr7, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr8, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr9, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr10, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr11, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr12, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_look_l2r = {12, assassin_frames_crouch_look_l2r, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_look_left[] =
{
	FRAME_newcrchlklft1, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newcrchlklft2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft3, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft5, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft6, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft7, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft8, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft9, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft10, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft11, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft12, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_look_left = {12, assassin_frames_crouch_look_left, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_look_left_idle[] =
{
	FRAME_newlkleftidle1, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newlkleftidle2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newlkleftidle3, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newlkleftidle4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newlkleftidle5, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newlkleftidle6, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newlkleftidle7, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newlkleftidle8, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newlkleftidle9, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newlkleftidle10, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newlkleftidle11, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newlkleftidle12, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_look_left_idle = {12, assassin_frames_crouch_look_left_idle, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_look_r2l[] =
{
	FRAME_newcrchlklr12, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newcrchlklr11, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr10, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr9, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr8, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr7, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr6, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr5, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr4, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr3, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklr2, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklr1, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_look_r2l = {12, assassin_frames_crouch_look_r2l, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_look_r2c[] =
{
	FRAME_newcrchlkrit12, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newcrchlkrit11, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit10, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit9, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit8, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit7, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit6, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit5, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit4, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit3, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlkrit2, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlkrit1, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_look_r2c = {12, assassin_frames_crouch_look_r2c, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_look_l2c[] =
{
	FRAME_newcrchlklft12, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_newcrchlklft11, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft10, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft9, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft8, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft7, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft6, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft5, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft4, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft3, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchlklft2, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_newcrchlklft1, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_look_l2c = {12, assassin_frames_crouch_look_l2c, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_end[] =
{
	FRAME_newcrchtrn5, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchtrn4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchtrn3, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchtrn2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_newcrchtrn1, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_end = {5, assassin_frames_crouch_end, assassin_crouch_idle_decision};

animframe_t assassin_frames_crouch_poke[] =
{
	FRAME_poke1, NULL, 0, 0, 0, ai_stand, 0, assassingrowl, 
	FRAME_poke2, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke3, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke4, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke5, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke6, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke7, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke8, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke9, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke10, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke11, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke12, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke13, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke14, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke15, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke16, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke17, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke18, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke19, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke20, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke21, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke22, NULL, 0, 0, 0, NULL, 0, NULL, 
	FRAME_poke23, NULL, 0, 0, 0, ai_stand, 0, NULL, 
	FRAME_poke24, NULL, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_crouch_poke = {24, assassin_frames_crouch_poke, assassin_crouch_idle_decision};
/*----------------------------------------------------------------------
  assassin teleport - throws smoke bomb, then gone
-----------------------------------------------------------------------*/
animframe_t assassin_frames_teleport [] =
{
	FRAME_ataka3,	NULL, 0, 0, 0, NULL, 0, assassinSkipFrameSkillCheck,
	FRAME_ataka4,	NULL, 0, 0, 0, NULL, 0, assassinSkipFrameSkillCheck,
	FRAME_ataka5,	NULL, 0, 0, 0, NULL, 0, assassinSkipFrameSkillCheck,
	FRAME_ataka6,	NULL, 0, 0, 0, NULL, 0, assassinSkipFrameSkillCheck,
	FRAME_ataka7,	NULL, 0, 0, 0, NULL, 0, assassinSkipFrameSkillCheck,
	FRAME_ataka8,	NULL, 0, 0, 0, NULL, 0, assassinSkipFrameSkillCheck,
	FRAME_ataka9,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka10,	NULL, 0, 0, 0, NULL, 0, assassinReadyTeleport,
	FRAME_ataka11,	NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t assassin_move_teleport = {9, assassin_frames_teleport, assassinGone};

animframe_t assassin_frames_cloak [] =
{
	FRAME_jump14,	NULL, 0, 0, 0, NULL, 0, assassinSetCrouched,
	FRAME_jump15,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jump16,	NULL, 0, 0, 0, NULL, 0, assassinInitCloak,
};
animmove_t assassin_move_cloak = {3, assassin_frames_cloak, assassinUnCrouch};



/************************************************************************
/************************************************************************
// 
//  Cinematic Frames
// 
/************************************************************************
/*************************************************************************/

animframe_t assassin_frames_c_idle1 [] =
{
	FRAME_ataka1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ataka2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ataka3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ataka4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ataka3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ataka2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_ataka1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
};
animmove_t assassin_move_c_idle1 = {7, assassin_frames_c_idle1, ai_c_cycleend};


/*----------------------------------------------------------------------
  assassin Running - assassin running 
-----------------------------------------------------------------------*/
animframe_t assassin_frames_c_run1 [] =
{
	FRAME_run1,	ai_c_move, 20, 0, 0, NULL, 0, assassingrowl,
	FRAME_run2,	ai_c_move, 18, 0, 0, NULL, 0, NULL,
	FRAME_run3,	ai_c_move, 12, 0, 0, NULL, 0, NULL,
	FRAME_run4,	ai_c_move, 16, 0, 0, NULL, 0, NULL,
	FRAME_run5,	ai_c_move, 24, 0, 0, NULL, 0, NULL,
	FRAME_run6,	ai_c_move, 18, 0, 0, NULL, 0, NULL,
	FRAME_run7,	ai_c_move, 16, 0, 0, NULL, 0, NULL,
	FRAME_run8,	ai_c_move, 12, 0, 0, NULL, 0, NULL,
	FRAME_run9,	ai_c_move, 18, 0, 0, NULL, 0, NULL,
	FRAME_run10, ai_c_move, 26, 0, 0, NULL, 0, NULL
};
animmove_t assassin_move_c_run1 = {10, assassin_frames_c_run1, ai_c_cycleend};


/*----------------------------------------------------------------------
  assassin daggerL - assassin attacking left hand
-----------------------------------------------------------------------*/
animframe_t assassin_frames_c_attack1 [] =
{
	FRAME_ataka1,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka2,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka3,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka4,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka5,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka6,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka7,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka8,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka9,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka10,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka11,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka12,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka13,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_ataka14,	ai_c_move, 0, 0, 0, NULL, 0, NULL
};
animmove_t assassin_move_c_attack1 = {14, assassin_frames_c_attack1,ai_c_cycleend};


/*----------------------------------------------------------------------
  assassin daggerR - assassin attacking right hand
-----------------------------------------------------------------------*/
animframe_t assassin_frames_c_attack2 [] =
{
	FRAME_atakb1,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb2,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb3,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb4,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb5,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb6,	ai_c_move, 0, 0, 0, NULL, 0, NULL,//loop in from an attack, no windup
	FRAME_atakb7,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb8,	ai_c_move, 0, 0, 0, NULL, 0, assassingrowl,
	FRAME_atakb9,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb10,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb11,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb12,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb13,	ai_c_move, 0, 0, 0, NULL, 0, NULL,//check for loop to other attack
	FRAME_atakb14,	ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_atakb15,	ai_c_move, 0, 0, 0, NULL, 0, NULL
};
animmove_t assassin_move_c_attack2 = {15, assassin_frames_c_attack2, ai_c_cycleend};

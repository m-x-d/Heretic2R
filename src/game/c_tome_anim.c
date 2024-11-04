//==============================================================================
//
// c_tome_anim.c
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

#include "c_tome_anim.h"
#include "c_tome.h"

#include "g_monster.h"
#include "c_ai.h"


/************************************************************************
/************************************************************************
// 
//  Cinematic Frames
// 
/************************************************************************
/*************************************************************************/

/*----------------------------------------------------------------------
  Tome of Power
-----------------------------------------------------------------------*/
animframe_t tome_frames_c_idle1 [] =
{
	FRAME_poly000, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly001, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly002, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly003, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly004, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly005, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly006, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly007, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly008, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly009, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly010, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly011, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly012, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly013, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly014, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly015, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly016, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly017, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly018, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly019, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly020, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly021, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly022, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly023, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly024, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly025, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly026, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly027, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly028, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly029, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_poly030, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t tome_move_c_idle1 = {31, tome_frames_c_idle1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Tome of Power
-----------------------------------------------------------------------*/
animframe_t tome_frames_c_idle2 [] =
{
	FRAME_poly000, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t tome_move_c_idle2 = {1, tome_frames_c_idle2, ai_c_cycleend};



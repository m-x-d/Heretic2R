//==============================================================================
//
// c_corvus2_anim.c
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

#include "c_corvus2_anim.h"
#include "c_corvus2.h"

#include "c_ai.h"


/************************************************************************
/************************************************************************
// 
//  Cinematic Frames
// 
/************************************************************************
/*************************************************************************/


/*----------------------------------------------------------------------
  Corvus Standing - 
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_idle1 [] =
{
	FRAME_Breath1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Breath23, ai_c_move, 0, 0, 0,  NULL, 0, NULL 
};

animmove_t corvus2_move_c_idle1 = {23, corvus2_frames_c_idle1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus - 
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_idle2 [] =
{
	FRAME_Wchrtk1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Wchrtk1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus2_move_c_idle2 = {2, corvus2_frames_c_idle2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus - 
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_idle3 [] =
{
	FRAME_Wchrtk106, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_Wchrtk106, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus2_move_c_idle3 = {2, corvus2_frames_c_idle3, ai_c_cycleend};


animframe_t corvus2_frames_c_action1 [] =
{
	FRAME_Wchrtk1, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk2, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk3, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk4, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk5, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk6, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk7, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk8, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk9, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk10, ai_c_move, 0, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk11, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk12, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk13, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk14, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk15, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk16, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk17, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk18, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk19, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk20, ai_c_move, 4, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk21, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk22, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk23, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk24, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk25, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk26, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk27, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk28, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk29, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk30, ai_c_move, 4, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk31, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk32, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk33, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk34, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk35, ai_c_move, 4, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk36, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk37, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk38, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk39, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk40, ai_c_move, 0, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk41, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk42, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk43, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk44, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk45, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk46, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk47, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk48, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk49, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk50, ai_c_move, 0, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk51, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk52, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk53, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk54, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk55, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk56, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk57, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk58, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk59, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk60, ai_c_move, 0, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk61, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk62, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk63, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk64, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk65, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk66, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk67, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk68, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk69, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk70, ai_c_move, 0, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk71, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk72, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk73, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk74, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk75, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk76, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk77, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk78, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk79, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk80, ai_c_move, 0, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk81, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk82, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk83, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk84, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk85, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk86, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk87, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk88, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk89, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk90, ai_c_move, 0, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk91, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk92, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk93, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk94, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk95, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk96, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk97, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk98, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk99, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk100, ai_c_move, 0, 0, 0, NULL, 0, NULL, 

	FRAME_Wchrtk101, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk102, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk103, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk104, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk105, ai_c_move, 0, 0, 0, NULL, 0, NULL, 
	FRAME_Wchrtk106, ai_c_move, 0, 0, 0, NULL, 0, NULL 
};

animmove_t corvus2_move_c_action1 = { 106, corvus2_frames_c_action1, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - starting his pivot to the left
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_pivotleftgo [] =
{	
	FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_pivotleftgo = {2, corvus2_frames_c_pivotleftgo, ai_c_cycleend};

animframe_t corvus2_frames_c_pivotleft [] =
{	
	FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_pivotleft = {2, corvus2_frames_c_pivotleft, ai_c_cycleend};


animframe_t corvus2_frames_c_pivotleftstop [] =
{	
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_pivotleftstop = {1, corvus2_frames_c_pivotleftstop, ai_c_cycleend};



/*----------------------------------------------------------------------
  Corvus - starting his pivot to the right
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_pivotrightgo [] =
{	
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_pivotrightgo = {2, corvus2_frames_c_pivotrightgo, ai_c_cycleend};

animframe_t corvus2_frames_c_pivotright [] =
{	
	FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_pivotright = {2, corvus2_frames_c_pivotright, ai_c_cycleend};


animframe_t corvus2_frames_c_pivotrightstop [] =
{	
	FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_pivotrightstop = {1, corvus2_frames_c_pivotrightstop, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_walk1 [] =
{
	FRAME_jog1, ai_c_move, 10, 0, 0, NULL, 0, NULL,
	FRAME_jog2, ai_c_move, 16, 0, 0, NULL, 0, NULL,
	FRAME_jog3, ai_c_move, 15, 0, 0, NULL, 0, NULL,
	FRAME_jog4, ai_c_move, 16, 0, 0, NULL, 0, NULL,
	FRAME_jog5, ai_c_move, 17, 0, 0, NULL, 0, NULL,
	FRAME_jog6, ai_c_move, 16, 0, 0, NULL, 0, NULL,
	FRAME_jog7, ai_c_move, 15, 0, 0, NULL, 0, NULL,
	FRAME_jog8, ai_c_move, 16, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_walk1 = {8, corvus2_frames_c_walk1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_walk2 [] =
{
	FRAME_cinewalk1, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk2, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk3, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk4, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk5, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk6, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk7, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk8, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk9, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk10, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk11, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk12, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk13, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk14, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk15, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_cinewalk16, ai_c_move, 4, 0, 0, NULL, 0, NULL,
};

animmove_t corvus2_move_c_walk2 = {16, corvus2_frames_c_walk2, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - starting his walk anims
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_walkstart [] =
{
	FRAME_gorun2, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_gorun3, ai_c_move, 4, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_walkstart = {2, corvus2_frames_c_walkstart, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - stopping his walk with right foot in front
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_walkstop1 [] =
{
	FRAME_jog1, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_jog2, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_walkstop1 = {2, corvus2_frames_c_walkstop1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus - stopping his walk with left foot in front
-----------------------------------------------------------------------*/
animframe_t corvus2_frames_c_walkstop2 [] =
{
	FRAME_jog5, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_jog6, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t corvus2_move_c_walkstop2 = {2, corvus2_frames_c_walkstop2, ai_c_cycleend};


//==============================================================================
//
// m_corvus_anim.c
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

#include "c_corvus1_anim.h"
#include "c_corvus1.h"

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
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_action1 [] =
{
	FRAME_c_wheelA1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelA2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelA3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelA4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus_move_c_action1 = {4, corvus_frames_c_action1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_action2 [] =
{
	FRAME_c_wheelB1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_wheelB22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus_move_c_action2 = {22, corvus_frames_c_action2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_action3 [] =
{
	FRAME_c_idleA1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleA10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus_move_c_action3 = {10, corvus_frames_c_action3, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_action4 [] =
{
	FRAME_plagued1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued23, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued24, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued25, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued26, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued27, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued28, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued29, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued30, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued31, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued32, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued33, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued34, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued35, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued36, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued37, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued38, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued39, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_plagued40, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus_move_c_action4 = {40, corvus_frames_c_action4, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus Standing - 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_idle1 [] =
{
	FRAME_breath1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_breath23, ai_c_move, 0, 0, 0,  NULL, 0, NULL 
};

animmove_t corvus_move_c_idle1 = {23, corvus_frames_c_idle1, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_idle2 [] =
{
	FRAME_c_idleB1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_c_idleB20, ai_c_move, 0, 0, 0,  NULL, 0, NULL 
}; 

animmove_t corvus_move_c_idle2 = {20, corvus_frames_c_idle2, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_walk1 [] =
{
	FRAME_jog1, ai_c_move, 10, 10, 0, NULL, 0, NULL,
	FRAME_jog2, ai_c_move, 16, 10, 0, NULL, 0, NULL,
	FRAME_jog3, ai_c_move, 15, 10, 0, NULL, 0, NULL,
	FRAME_jog4, ai_c_move, 16, 10, 0, NULL, 0, NULL,
	FRAME_jog5, ai_c_move, 17, 10, 0, NULL, 0, NULL,
	FRAME_jog6, ai_c_move, 16, 10, 0, NULL, 0, NULL,
	FRAME_jog7, ai_c_move, 15, 10, 0, NULL, 0, NULL,
	FRAME_jog8, ai_c_move, 16, 10, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_walk1 = {8, corvus_frames_c_walk1, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_strafeleft [] =
{
	FRAME_Lstep1, ai_c_move, 0, 6, 0, NULL, 0, NULL,
	FRAME_Lstep2, ai_c_move, 0, 6, 0, NULL, 0, NULL,
	FRAME_Lstep3, ai_c_move, 0, 6, 0, NULL, 0, NULL,
	FRAME_Lstep4, ai_c_move, 0, 6, 0, NULL, 0, NULL,
	FRAME_Lstep5, ai_c_move, 0, 6, 0, NULL, 0, NULL,
};

animmove_t corvus_move_c_strafeleft = {5, corvus_frames_c_strafeleft, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_straferight [] =
{
	FRAME_Rstep1, ai_c_move, 0, 6, 0, NULL, 0, NULL,
	FRAME_Rstep2, ai_c_move, 0, 6, 0, NULL, 0, NULL,
	FRAME_Rstep3, ai_c_move, 0, 6, 0, NULL, 0, NULL,
	FRAME_Rstep4, ai_c_move, 0, 6, 0, NULL, 0, NULL,
	FRAME_Rstep5, ai_c_move, 0, 6, 0, NULL, 0, NULL,
};


animmove_t corvus_move_c_straferight = {5, corvus_frames_c_straferight, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_walk2 [] =
{
	FRAME_cinewalk1, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk2, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk3, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk4, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk5, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk6, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk7, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk8, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk9, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk10, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk11, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk12, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk13, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk14, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk15, ai_c_move, 4, 6, 0, NULL, 0, NULL,
	FRAME_cinewalk16, ai_c_move, 4, 6, 0, NULL, 0, NULL,
};

animmove_t corvus_move_c_walk2 = {16, corvus_frames_c_walk2, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - starting his walk anims
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_walkstart [] =
{
	FRAME_gorun2, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_gorun3, ai_c_move, 4, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_walkstart = {2, corvus_frames_c_walkstart, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - stopping his walk with right foot in front
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_walkstop1 [] =
{
	FRAME_jog1, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_jog2, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_walkstop1 = {2, corvus_frames_c_walkstop1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus - stopping his walk with left foot in front
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_walkstop2 [] =
{
	FRAME_jog5, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_jog6, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_walkstop2 = {2, corvus_frames_c_walkstop2, ai_c_cycleend};





/*----------------------------------------------------------------------
  Corvus - starting his pivot to the left
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_pivotleftgo [] =
{	
	FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_pivotleftgo = {2, corvus_frames_c_pivotleftgo, ai_c_cycleend};

animframe_t corvus_frames_c_pivotleft [] =
{	
	FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_pivotleft = {2, corvus_frames_c_pivotleft, ai_c_cycleend};


animframe_t corvus_frames_c_pivotleftstop [] =
{	
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_pivotleftstop = {1, corvus_frames_c_pivotleftstop, ai_c_cycleend};



/*----------------------------------------------------------------------
  Corvus - starting his pivot to the right
-----------------------------------------------------------------------*/
animframe_t corvus_frames_c_pivotrightgo [] =
{	
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_pivotrightgo = {2, corvus_frames_c_pivotrightgo, ai_c_cycleend};

animframe_t corvus_frames_c_pivotright [] =
{	
	FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_pivotright = {2, corvus_frames_c_pivotright, ai_c_cycleend};


animframe_t corvus_frames_c_pivotrightstop [] =
{	
	FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus_move_c_pivotrightstop = {1, corvus_frames_c_pivotrightstop, ai_c_cycleend};



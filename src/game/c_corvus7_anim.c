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

#include "c_corvus7_anim.h"
#include "c_corvus7.h"

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
  Corvus - 
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_action1 [] =
{
	FRAME_MCinaa1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa23, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa24, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa25, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa26, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa27, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa28, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa29, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa30, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa31, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa32, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa33, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa34, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa35, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa36, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa37, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa38, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa39, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinaa40, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus7_move_c_action1 = {40, corvus7_frames_c_action1, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - 
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_action2 [] =
{
	FRAME_MCinab1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinab16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus7_move_c_action2 = {16, corvus7_frames_c_action2, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - 
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_action3 [] =
{
	FRAME_MCinac1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_MCinac14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus7_move_c_action3 = {14, corvus7_frames_c_action3, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus Standing - 
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_idle1 [] =
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

animmove_t corvus7_move_c_idle1 = {23, corvus7_frames_c_idle1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus Standing - 
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_idle2 [] =
{
	FRAME_idleA1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleA12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus7_move_c_idle2 = {12, corvus7_frames_c_idle2, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus Standing - 
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_idle3 [] =
{
	FRAME_idleB1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idleB12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t corvus7_move_c_idle3 = {12, corvus7_frames_c_idle3, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus - starting his pivot to the left
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_pivotleftgo [] =
{	
	FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_pivotleftgo = {2, corvus7_frames_c_pivotleftgo, ai_c_cycleend};

animframe_t corvus7_frames_c_pivotleft [] =
{	
	FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_pivotleft = {2, corvus7_frames_c_pivotleft, ai_c_cycleend};


animframe_t corvus7_frames_c_pivotleftstop [] =
{	
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_pivotleftstop = {1, corvus7_frames_c_pivotleftstop, ai_c_cycleend};



/*----------------------------------------------------------------------
  Corvus - starting his pivot to the right
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_pivotrightgo [] =
{	
	FRAME_Lpivot4, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot3, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_pivotrightgo = {2, corvus7_frames_c_pivotrightgo, ai_c_cycleend};

animframe_t corvus7_frames_c_pivotright [] =
{	
	FRAME_Lpivot2, ai_c_move, 0, 0, 0, NULL, 0, NULL,
	FRAME_Lpivot1, ai_c_move, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_pivotright = {2, corvus7_frames_c_pivotright, ai_c_cycleend};


animframe_t corvus7_frames_c_pivotrightstop [] =
{	
	FRAME_Lpivot1, NULL, 0, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_pivotrightstop = {1, corvus7_frames_c_pivotrightstop, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_walk1 [] =
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
animmove_t corvus7_move_c_walk1 = {8, corvus7_frames_c_walk1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus 
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_walk2 [] =
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

animmove_t corvus7_move_c_walk2 = {16, corvus7_frames_c_walk2, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - starting his walk anims
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_walkstart [] =
{
	FRAME_gorun2, ai_c_move, 4, 0, 0, NULL, 0, NULL,
	FRAME_gorun3, ai_c_move, 4, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_walkstart = {2, corvus7_frames_c_walkstart, ai_c_cycleend};


/*----------------------------------------------------------------------
  Corvus - stopping his walk with right foot in front
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_walkstop1 [] =
{
	FRAME_jog1, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_jog2, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_walkstop1 = {2, corvus7_frames_c_walkstop1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Corvus - stopping his walk with left foot in front
-----------------------------------------------------------------------*/
animframe_t corvus7_frames_c_walkstop2 [] =
{
	FRAME_jog5, ai_c_move, 2, 0, 0, NULL, 0, NULL,
	FRAME_jog6, ai_c_move, 2, 0, 0, NULL, 0, NULL,
};
animmove_t corvus7_move_c_walkstop2 = {2, corvus7_frames_c_walkstop2, ai_c_cycleend};


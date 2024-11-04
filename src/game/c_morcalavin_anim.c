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

#include "c_morcalavin_anim.h"
#include "c_morcalavin.h"

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
  morcalavin 
-----------------------------------------------------------------------*/
animframe_t morcalavin_frames_c_idle1 [] =
{
	FRAME_talka1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t morcalavin_move_c_idle1 = {3, morcalavin_frames_c_idle1, ai_c_cycleend};


/*----------------------------------------------------------------------
  morcalavin 
-----------------------------------------------------------------------*/
animframe_t morcalavin_frames_c_idle2 [] =
{
	FRAME_talkb1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t morcalavin_move_c_idle2 = {1, morcalavin_frames_c_idle2, ai_c_cycleend};



/*----------------------------------------------------------------------
  morcalavin 
-----------------------------------------------------------------------*/
animframe_t morcalavin_frames_c_idle3 [] =
{
	FRAME_talkc66, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t morcalavin_move_c_idle3 = {1, morcalavin_frames_c_idle3, ai_c_cycleend};

/*----------------------------------------------------------------------
  morcalavin 
-----------------------------------------------------------------------*/
animframe_t morcalavin_frames_c_idle4 [] =
{
	FRAME_talkc1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t morcalavin_move_c_idle4 = {1, morcalavin_frames_c_idle4, ai_c_cycleend};


/*----------------------------------------------------------------------
  morcalavin 
-----------------------------------------------------------------------*/
animframe_t morcalavin_frames_c_action1 [] =
{
	FRAME_talka1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka23, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka24, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka25, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka26, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka27, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka28, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka29, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka30, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka31, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka32, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka33, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka34, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka35, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka36, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka37, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka38, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka39, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka40, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka41, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka42, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka43, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka44, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka45, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka46, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka47, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka48, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka49, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka50, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka51, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka52, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka53, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka54, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka55, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka56, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka57, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka58, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka59, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka60, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka61, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka62, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka63, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka64, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka65, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka66, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka67, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka68, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka69, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka70, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka71, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka72, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka73, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka74, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka75, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka76, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka77, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka78, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka79, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka80, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka81, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka82, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka83, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka84, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka85, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka86, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka87, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka88, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka89, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka90, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka91, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka92, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka93, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka94, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka95, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka96, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka97, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka98, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka99, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka100, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka101, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka102, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka103, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka104, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka105, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka106, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka107, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka108, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka109, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka110, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka111, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka112, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka113, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka114, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka115, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka116, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka117, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka118, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka119, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka120, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka121, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka122, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka123, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka124, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka125, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka126, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka127, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka128, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka129, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka130, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka131, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka132, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka133, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka134, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka135, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka136, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka137, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka138, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka139, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka140, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka141, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka142, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka143, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka144, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka145, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka146, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka147, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka148, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka149, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka150, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka151, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka152, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka153, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka154, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka155, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka156, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka157, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka158, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka159, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka160, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka161, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka162, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka163, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka164, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka165, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka166, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka167, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka168, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka169, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka170, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka171, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka172, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka173, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka174, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka175, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka176, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka177, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka178, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka179, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka180, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka181, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka182, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka183, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka184, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka185, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka186, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka187, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka188, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka189, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka190, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka191, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka192, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka193, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka194, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka195, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka196, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka197, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka198, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka199, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka200, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka201, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka202, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka203, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka204, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka205, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka206, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka207, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka208, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka209, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka210, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka211, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka212, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka213, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka214, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka215, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka216, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka217, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka218, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka219, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka220, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka221, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka222, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka223, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka224, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka225, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka226, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka227, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka228, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka229, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka230, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka231, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka232, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka233, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka234, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka235, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka236, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka237, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka238, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka239, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka240, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka241, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka242, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka243, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka244, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka245, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka246, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka247, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka248, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka249, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka250, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka251, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka252, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka253, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka254, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka255, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka256, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka257, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka258, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka259, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka260, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka261, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka262, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka263, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka264, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka265, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka266, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka267, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka268, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka269, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka270, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka271, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka272, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka273, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka274, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka275, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka276, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka277, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka278, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka279, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka280, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka281, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka282, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka283, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka284, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka285, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka286, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka287, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka288, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka289, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka290, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka291, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka292, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka293, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka294, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka295, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka296, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka297, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka298, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka299, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talka300, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t morcalavin_move_c_action1 = {300, morcalavin_frames_c_action1, ai_c_cycleend};


/*----------------------------------------------------------------------
  morcalavin 
-----------------------------------------------------------------------*/
animframe_t morcalavin_frames_c_action2 [] =
{
	FRAME_talkb1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb23, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb24, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb25, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb26, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb27, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb28, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb29, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb30, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb31, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb32, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb33, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb34, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb35, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb36, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb37, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb38, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb39, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb40, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb41, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb42, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb43, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb44, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb45, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb46, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb47, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb48, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb49, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb50, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb51, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb52, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb53, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb54, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb55, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb56, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb57, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb58, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb59, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb60, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb61, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb62, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb63, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb64, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb65, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb66, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb67, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb68, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb69, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb70, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb71, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb72, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb73, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb74, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb75, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb76, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb77, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb78, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb79, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb80, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb81, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb82, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb83, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb84, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb85, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb86, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb87, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb88, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb89, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb90, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb91, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb92, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb93, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb94, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb95, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb96, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb97, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb98, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb99, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb100, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkb101, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t morcalavin_move_c_action2 = {101, morcalavin_frames_c_action2, ai_c_cycleend};


/*----------------------------------------------------------------------
  morcalavin 
-----------------------------------------------------------------------*/
animframe_t morcalavin_frames_c_action3 [] =
{
	FRAME_talkc1, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc2, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc3, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc4, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc5, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc6, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc7, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc8, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc9, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc10, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc11, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc12, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc13, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc14, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc15, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc16, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc17, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc18, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc19, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc20, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc21, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc22, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc23, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc24, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc25, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc26, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc27, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc28, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc29, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc30, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc31, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc32, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc33, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc34, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc35, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc36, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc37, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc38, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc39, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc40, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc41, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc42, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc43, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc44, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc45, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc46, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc47, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc48, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc49, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc50, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc51, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc52, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc53, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc54, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc55, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc56, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc57, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc58, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc59, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc60, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc61, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc62, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc63, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc64, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc65, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc66, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc67, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc68, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc69, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc70, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc71, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc72, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc73, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc74, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc75, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc76, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc77, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc78, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc79, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc80, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc81, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_talkc82, ai_c_move, 0, 0, 0,  NULL, 0, NULL, 
};

animmove_t morcalavin_move_c_action3 = {82, morcalavin_frames_c_action3, ai_c_cycleend};

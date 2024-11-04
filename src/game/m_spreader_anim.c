//==============================================================================
//
// m_spreader_anim.c
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

#include "m_spreader_anim.h"
#include "m_spreadermist.h"
#include "m_spreader.h"

void ai_charge2 (edict_t *self, float dist);

/*------------------------------------------------------------------
	spreader attack 1 -- the throw
-------------------------------------------------------------------*/
animframe_t spreader_frames_attack1 [] =
{
	FRAME_atacka1,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka2,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka3,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka4,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka5,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka6,  NULL, 0, 0, 0,  ai_charge2, 0, spreader_showgrenade, 
	FRAME_atacka7,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka8,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka9,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka10,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka11,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka12,  NULL, 0, 0, 0,  ai_charge2, 0, spreader_hidegrenade, 
	FRAME_atacka13,  NULL, 0, 0, 0,  ai_charge2, 0, spreader_toss_grenade,
	FRAME_atacka14,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka15,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atacka16,  NULL, 0, 0, 0,  ai_charge2, 0, NULL 
};
animmove_t spreader_move_attack1 = { 16, spreader_frames_attack1, spreader_pause };

/*------------------------------------------------------------------
	spreader attack 2 -- the plague effect attack

	Using the move func for the spreader mist instead of the action 
	func because the action func is already being used as a move 
	func...
-------------------------------------------------------------------*/
animframe_t spreader_frames_attack2 [] =
{
	FRAME_atackb1,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atackb2,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atackb3,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atackb4,  spreader_mist, 14.59, -0.96, 16,   ai_charge2, 0, spreader_miststartsound, 
	FRAME_atackb5,  spreader_mist, 12.49, -5.98, 16,   ai_charge2, 0, NULL, 
	FRAME_atackb6,  spreader_mist, 10.35, -13.15, 16,  ai_charge2, 0, NULL, 
	FRAME_atackb7,  spreader_mist, 6.14, -17.95, 16,  ai_charge2, 0, NULL, 
	FRAME_atackb8,  spreader_mist, 2.4, -18.06, 16,  ai_charge2, 0, NULL, 
	FRAME_atackb9,  spreader_mist, 6.23, -14.4, 16,  ai_charge2, 0, NULL, 
	FRAME_atackb10,  spreader_mist, 12.85, -4.18, 16,  ai_charge2, 0, NULL, 
	FRAME_atackb11,  spreader_mist, 13.88, 10.8, 16.36,  ai_charge2, 0, NULL, 
	FRAME_atackb12,  spreader_mist, 7.11, 23.19, 16.81,  ai_charge2, 0, spreader_miststopsound, 
	FRAME_atackb13,  NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_atackb14,  NULL, 0, 0, 0,  ai_charge2, 0, NULL
};
animmove_t spreader_move_attack2 = { 14, spreader_frames_attack2, spreader_pause };

/*------------------------------------------------------------------
	spreader backattack 1 -- the spreader backpedals while shooting
-------------------------------------------------------------------*/
animframe_t spreader_frames_backattack1 [] = 
{	
	FRAME_bkatck1, spreader_mist, 14.4, 10.24, 16, ai_run, -5, spreader_miststartsound,
	FRAME_bkatck2, spreader_mist, 14.22, 10.03, 16, ai_run, -5, NULL,
	FRAME_bkatck3, spreader_mist, 13.79, 9.79, 16, ai_run, -5, NULL,
	FRAME_bkatck4, spreader_mist, 13.62, 9.63, 16, ai_run, -5, NULL,
	FRAME_bkatck5, spreader_mist, 13.84, 9.64, 16, ai_run, -5, NULL,
	FRAME_bkatck6, spreader_mist, 14.23, 9.75, 16,  ai_run, -5, NULL,
	FRAME_bkatck7, spreader_mist, 14.59, 9.85, 16, ai_run, -5, NULL,
	FRAME_bkatck8, spreader_mist, 14.15, 9.75, 16,  ai_run, -5, NULL,
	FRAME_bkatck9, spreader_mist, 13.83, 9.66, 16, ai_run, -5, NULL,
	FRAME_bkatck10, spreader_mist, 13.58, 9.65, 16,  ai_run, -5, NULL,
	FRAME_bkatck11, spreader_mist, 13.82, 9.82, 16, ai_run, -5, NULL,
	FRAME_bkatck12, spreader_mist, 14.2, 10.06, 16, ai_run, -5, NULL,
	FRAME_bkatck13, spreader_mist, 14.47, 10.38, 16, ai_run, -5, spreader_miststopsound
};
animmove_t spreader_move_backattack1 = { 13, spreader_frames_backattack1, spreader_pause };

/*------------------------------------------------------------------
	spreader backup 1 -- the spreader backpedals
-------------------------------------------------------------------*/
animframe_t spreader_frames_backup1 [] = 
{	
	FRAME_backup1, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup2, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup3, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup4, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup5, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup6, NULL, 0, 0, 0, ai_run, -5, spreader_pause,
	FRAME_backup7, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup8, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup9, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup10, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup11, NULL, 0, 0, 0, ai_run, -5, spreader_pause,
	FRAME_backup12, NULL, 0, 0, 0, ai_run, -5, NULL,
	FRAME_backup13, NULL, 0, 0, 0, ai_run, -5, NULL
};
animmove_t spreader_move_backup1 = { 13, spreader_frames_backup1, spreader_pause };

/*------------------------------------------------------------------
	spreader death -- the spreader spreads no more (big death)
-------------------------------------------------------------------*/
animframe_t spreader_frames_death1_go [] =
{
	FRAME_death1, NULL, 0, 0, 0, NULL, 0, spreader_flyback_move, 
};
animmove_t spreader_move_death1_go = { 1, spreader_frames_death1_go, spreader_flyback_loop };

animframe_t spreader_frames_death1_loop [] =
{
	FRAME_death2, NULL, 0, 0, 0, NULL, 0, spreader_flyback_move,
};
animmove_t spreader_move_death1_loop = { 1, spreader_frames_death1_loop, NULL };

animframe_t spreader_frames_death1_end [] =
{
	FRAME_death3, NULL, 0, 0, 0, ai_move, -4, spreader_flyback_move, 
	FRAME_death4, NULL, 0, 0, 0, ai_move, -4, spreader_flyback_move, 
	FRAME_death5, NULL, 0, 0, 0, ai_move, -4, spreader_flyback_move, 
	FRAME_death6, NULL, 0, 0, 0, ai_move, -3, spreader_flyback_move, 
	FRAME_death7, NULL, 0, 0, 0, ai_move, -3, spreader_flyback_move, 
	FRAME_death8, NULL, 0, 0, 0, ai_move, -3, spreader_flyback_move, 
	FRAME_death9, NULL, 0, 0, 0, ai_move, -2, spreader_flyback_move, 
	FRAME_death10, NULL, 0, 0, 0, ai_move, -2, spreader_flyback_move, 
	FRAME_death11, NULL, 0, 0, 0, ai_move, -2, spreader_flyback_move, 
	FRAME_death12, NULL, 0, 0, 0, ai_move, -1, spreader_flyback_move, 
	FRAME_death13, NULL, 0, 0, 0, ai_move, -1, spreader_flyback_move, 
	FRAME_death14, NULL, 0, 0, 0, ai_move, 0, spreader_flyback_move, 
	FRAME_death15, NULL, 0, 0, 0, ai_move, 0, spreader_flyback_move, 
	FRAME_death16, NULL, 0, 0, 0, ai_move, 0, spreader_flyback_move, 
	FRAME_death17, NULL, 0, 0, 0, ai_move, 0, spreader_dead
};
animmove_t spreader_move_death1_end = { 15, spreader_frames_death1_end, NULL };

/*------------------------------------------------------------------
	spreader death -- the spreader spreads no more (little death)
-------------------------------------------------------------------*/
animframe_t spreader_frames_death2 [] =
{
	FRAME_deathb1, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb2, NULL, 0, 0, 0, ai_move, 0, NULL,
	FRAME_deathb3, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb4, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb6, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb7, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb9, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb10, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb11, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb12, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb13, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb14, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb16, NULL, 0, 0, 0, ai_move, 0, NULL, 
	FRAME_deathb17, NULL, 0, 0, 0, ai_move, 0, spreader_dead 
};
animmove_t spreader_move_death2 = { 14, spreader_frames_death2, NULL };


/*------------------------------------------------------------------
	spreader duck attack 1 -- spreader ducking, shoots, rises
-------------------------------------------------------------------*/
animframe_t spreader_frames_dkatck1 [] =
{
	FRAME_dkatck_1, spreader_mist, 21.39, 11.26, -16, ai_move, 0, spreader_miststartsound, 
	FRAME_dkatck_2, spreader_mist, 12.08, 24.16, -16, ai_move, 0, NULL, 
	FRAME_dkatck_3, spreader_mist, 26.14, 1.26, -16, ai_move, 0, NULL, 
	FRAME_dkatck_4, spreader_mist, 26.65, -0.43, -16, ai_move, 0, spreader_miststopsound, 
	FRAME_dkatck_5, NULL, 0, 0, 0,  ai_move, 0, NULL, 
	FRAME_dkatck_6, NULL, 0, 0, 0,  ai_move, 0, NULL 		
};
animmove_t spreader_move_dkatck1 = { 6, spreader_frames_dkatck1, spreader_pause };

/*------------------------------------------------------------------
	spreader duck  -- spreader ducks and rises
-------------------------------------------------------------------*/
animframe_t spreader_frames_duck1 [] =
{
	FRAME_duck1, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_duck2, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_duck3, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_duck4, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_duck5, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_duck6, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 		
	FRAME_duck7, NULL, 0, 0, 0,  ai_charge2, 0, NULL 		
};
animmove_t spreader_move_duck1 = { 7, spreader_frames_duck1, spreader_pause };

/*-------------------------------------------------------------------------
	spreader ducks 
-------------------------------------------------------------------------*/
animframe_t spreader_frames_duckdown [] =
{
	FRAME_duck1, NULL, 0,0,0, ai_charge2, 0, NULL, 
	FRAME_duck2, NULL, 0,0,0, ai_charge2, 0, NULL, 
	FRAME_duck3, NULL, 0,0,0, ai_charge2, 0, NULL, 
	FRAME_duck4, NULL, 0,0,0, ai_charge2, 0, NULL
};
animmove_t spreader_move_duckdown = { 4, spreader_frames_duckdown, spreader_duckpause };

/*------------------------------------------------------------------
	spreader duck still 1 -- the spreader ducking, still
-------------------------------------------------------------------*/
animframe_t spreader_frames_duckstill [] =
{
	FRAME_dkidle1, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle2, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle3, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle4, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle5, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle6, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle7, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle8, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle9, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle10, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle11, NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_dkidle12, NULL, 0, 0, 0, NULL, 0, NULL
};
animmove_t spreader_move_duckstill = { 12, spreader_frames_duckstill, spreader_duckpause };


/*-------------------------------------------------------------------------
	spreader rises from a ducking pos
-------------------------------------------------------------------------*/
animframe_t spreader_frames_duckup [] = 
{
	FRAME_duck4, NULL, 0, 0, 0, ai_charge2, 0, NULL,
	FRAME_duck5, NULL, 0, 0, 0, ai_charge2, 0, NULL, 
	FRAME_duck6, NULL, 0, 0, 0, ai_charge2, 0, NULL, 
	FRAME_duck7, NULL, 0, 0, 0, ai_charge2, 0, NULL
};
animmove_t spreader_move_duckup = { 4, spreader_frames_duckup, spreader_pause };

/*------------------------------------------------------------------
	spreader idle 1 -- the spreader stands around
-------------------------------------------------------------------*/
animframe_t spreader_frames_idle1 [] =
{
	FRAME_idle1, NULL, 0, 0, 0,  ai_stand, 0, NULL, 
	FRAME_idle2, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idle3, NULL, 0, 0, 0,  ai_stand, 0, spreader_idlenoise, 
	FRAME_idle4, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idle5, NULL, 0, 0, 0,  ai_stand, 0, NULL, 
	FRAME_idle6, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idle7, NULL, 0, 0, 0,  ai_stand, 0, NULL, 
	FRAME_idle8, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idle9, NULL, 0, 0, 0,  ai_stand, 0, NULL, 
	FRAME_idle10, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_idle11, NULL, 0, 0, 0,  ai_stand, 0, NULL 	
};
animmove_t spreader_move_idle1 = { 11, spreader_frames_idle1, spreader_pause };

/*------------------------------------------------------------------
	spreader pain  -- spreader recoils from hit
-------------------------------------------------------------------*/
animframe_t spreader_frames_pain1 [] =
{
	FRAME_pain1, NULL, 0, 0, 0,  ai_move, 0, spreader_pain_sound, 
	FRAME_pain2, NULL, 0, 0, 0,  ai_move, 0, NULL, 
	FRAME_pain3, NULL, 0, 0, 0,  ai_move, 0, NULL, 
	FRAME_pain4, NULL, 0, 0, 0,  ai_move, 0, NULL, 
	FRAME_pain5, NULL, 0, 0, 0,  ai_move, 0, NULL, 
	FRAME_pain6, NULL, 0, 0, 0,  ai_move, 0, NULL, 		
	FRAME_pain7, NULL, 0, 0, 0,  ai_move, 0, NULL 		
};
animmove_t spreader_move_pain1 = { 7, spreader_frames_pain1, spreader_pause };

/*------------------------------------------------------------------
	spreader pivot left  -- spreader turns left
-------------------------------------------------------------------*/
animframe_t spreader_frames_pvtlt1 [] =
{
	FRAME_pvtlt1, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_pvtlt2, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_pvtlt3, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_pvtlt4, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_pvtlt5, NULL, 0, 0, 0,  ai_charge2, 0, NULL 
};
animmove_t spreader_move_pvtlt1 = { 5, spreader_frames_pvtlt1, spreader_pause };

/*------------------------------------------------------------------
	spreader pivot right  -- spreader turns right
-------------------------------------------------------------------*/
animframe_t spreader_frames_pvtrt1 [] =
{
	FRAME_pvtrt1, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_pvtrt2, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_pvtrt3, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_pvtrt4, NULL, 0, 0, 0,  ai_charge2, 0, NULL, 
	FRAME_pvtrt5, NULL, 0, 0, 0,  ai_charge2, 0, NULL 
};
animmove_t spreader_move_pvtrt1 = { 5, spreader_frames_pvtrt1, spreader_pause };

/*------------------------------------------------------------------
	spreader run 1 -- the spreader runs
-------------------------------------------------------------------*/
animframe_t spreader_frames_run1 [] =
{
	FRAME_run1, NULL, 0, 0, 0,  ai_run, 20, spreader_pause, 
	FRAME_run2, NULL, 0, 0, 0,  ai_run, 18, NULL, 
	FRAME_run3, NULL, 0, 0, 0,  ai_run, 16, NULL, 
	FRAME_run4, NULL, 0, 0, 0,  ai_run, 18, spreader_pause,	//foot down
	FRAME_run5, NULL, 0, 0, 0,  ai_run, 20, NULL, 
	FRAME_run6, NULL, 0, 0, 0,  ai_run, 18, NULL,
	FRAME_run7, NULL, 0, 0, 0,  ai_run, 16, NULL,
	FRAME_run8, NULL, 0, 0, 0,  ai_run, 18, NULL	//foot down
};
animmove_t spreader_move_run1 = { 8, spreader_frames_run1, spreader_pause };

/*------------------------------------------------------------------
	spreader land
-------------------------------------------------------------------*/
animframe_t spreader_frames_land [] =
{
	FRAME_jump17, NULL, 0, 0, 0,  NULL, 0, spreader_land,
	FRAME_jump18, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump19, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump20, NULL, 0, 0, 0,  NULL, 0, NULL,	//foot down
	FRAME_jump21, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump22, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump23, NULL, 0, 0, 0,  NULL, 0, NULL, 
};
animmove_t spreader_move_land = {7, spreader_frames_land, spreader_pause};

/*------------------------------------------------------------------
	spreader jump from buoy
-------------------------------------------------------------------*/
animframe_t spreader_frames_inair [] =
{
	FRAME_jump16, NULL, 0, 0, 0,  MG_CheckLanded, ANIM_LAND, NULL, 
};
animmove_t spreader_move_inair = {1, spreader_frames_inair, NULL};

/*------------------------------------------------------------------
	spreader jump from buoy
-------------------------------------------------------------------*/
animframe_t spreader_frames_fjump [] =
{
	FRAME_jump1, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump2, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump3, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump4, NULL, 0, 0, 0,  NULL, 0, NULL,	//foot down
	FRAME_jump5, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump6, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump7, NULL, 0, 0, 0,  NULL, 0, spreaderApplyJump, 
	FRAME_jump8, NULL, 0, 0, 0,  NULL, 0, NULL,	//foot down
	FRAME_jump9, NULL, 0, 0, 0,  NULL, 0, NULL, 
	FRAME_jump10, NULL, 0, 0, 0,  MG_CheckLanded, ANIM_LAND, NULL, 
	FRAME_jump11, NULL, 0, 0, 0,  MG_CheckLanded, ANIM_LAND, NULL, 
	FRAME_jump12, NULL, 0, 0, 0,  MG_CheckLanded, ANIM_LAND, NULL,	//foot down
	FRAME_jump13, NULL, 0, 0, 0,  MG_CheckLanded, ANIM_LAND, NULL, 
	FRAME_jump14, NULL, 0, 0, 0,  MG_CheckLanded, ANIM_LAND, NULL, 
	FRAME_jump15, NULL, 0, 0, 0,  MG_CheckLanded, ANIM_LAND, NULL, 
};
animmove_t spreader_move_fjump = {15, spreader_frames_fjump, spreader_go_inair};

/*------------------------------------------------------------------
	spreader run attack -- the spreader runs and shoots
-------------------------------------------------------------------*/
animframe_t spreader_frames_rnatck1 [] =
{
	FRAME_rnatck1, spreader_mist_fast, 38.0*2,  5, 16,  ai_run, 20, spreader_miststartsound, 
	FRAME_rnatck2, spreader_mist_fast, 39.27*2, 5, 16, ai_run, 18, NULL, 
	FRAME_rnatck3, spreader_mist_fast, 37.43*2, 5, 16,  ai_run, 16, NULL, 
	FRAME_rnatck4, spreader_mist_fast, 36.68*2, 5, 16, ai_run, 18, spreader_pause, 
	FRAME_rnatck5, spreader_mist_fast, 37.02*2, 5, 16, ai_run, 20, NULL, 
	FRAME_rnatck6, spreader_mist_fast, 36.41*2, 5, 16, ai_run, 18, NULL, 
	FRAME_rnatck7, spreader_mist_fast, 35.69*2, 5, 16, ai_run, 16, spreader_pause, 
	FRAME_rnatck8, spreader_mist_fast, 35.68*2, 5, 16,  ai_run, 18, spreader_pause,//spreader_mist_faststopsound		
};
animmove_t spreader_move_rnatck1 = { 8, spreader_frames_rnatck1, spreader_pause };

/*------------------------------------------------------------------
	spreader walk 1 -- the spreader walks
-------------------------------------------------------------------*/
animframe_t spreader_frames_walk1 [] =
{
	FRAME_walk1, NULL, 0, 0, 0,  ai_walk, 8, spreader_idlenoise, 
	FRAME_walk2, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk3, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk4, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk5, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk6, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk7, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk8, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk9, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk10, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk11, NULL, 0, 0, 0,  ai_walk, 8, NULL, 	
	FRAME_walk12, NULL, 0, 0, 0,  ai_walk, 8, NULL, 
	FRAME_walk13, NULL, 0, 0, 0,  ai_walk, 8, NULL,
};
animmove_t spreader_move_walk1 = { 13, spreader_frames_walk1, spreader_pause };

/*------------------------------------------------------------------
	spreader walk 2 -- the spreader walks
-------------------------------------------------------------------*/
animframe_t spreader_frames_walk2 [] =
{
	FRAME_walk1, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk2, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk3, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk4, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk5, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk6, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk7, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk8, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk9, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk10, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk11, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 	
	FRAME_walk12, NULL, 0, 0, 0,  ai_run, 6, spreader_pause, 
	FRAME_walk13, NULL, 0, 0, 0,  ai_run, 6, spreader_pause 	
};
animmove_t spreader_move_walk2 = { 13, spreader_frames_walk2, spreader_pause };

/*------------------------------------------------------------------
	spreader fly into air- ouch!
-------------------------------------------------------------------*/
animframe_t spreader_frames_flyloop [] =
{
	FRAME_pain5, NULL, 0, 0, 0,  NULL, 0, spreaderFly, 
};
animmove_t spreader_move_flyloop = { 1, spreader_frames_flyloop, NULL};

/*------------------------------------------------------------------
	spreader fly into air- ouch!
-------------------------------------------------------------------*/
animframe_t spreader_frames_fly [] =
{
	FRAME_pain1, NULL, 0, 0, 0,  NULL, 0, spreaderFly, 
	FRAME_pain2, NULL, 0, 0, 0,  NULL, 0, spreaderFly, 
	FRAME_pain3, NULL, 0, 0, 0,  NULL, 0, spreaderFly, 
	FRAME_pain4, NULL, 0, 0, 0,  NULL, 0, spreaderFly, 
};
animmove_t spreader_move_fly = { 4, spreader_frames_fly, spreaderFlyLoop};


animframe_t spreader_frames_dead [] =
{
	FRAME_deathb17, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain 
};
animmove_t spreader_move_dead = { 1, spreader_frames_dead, NULL};

animframe_t spreader_frames_fdie [] =
{
	FRAME_deathb1, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb2, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain,
	FRAME_deathb3, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb4, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb6, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb7, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb9, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb10, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb11, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb12, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb13, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb14, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb16, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain, 
	FRAME_deathb17, NULL, 0, 0, 0, NULL, 0, spreaderSolidAgain
};
animmove_t spreader_move_fdie = { 14, spreader_frames_fdie, spreader_go_deadloop};

/*------------------------------------------------------------------
	spreader delay
-------------------------------------------------------------------*/
animframe_t spreader_frames_delay [] =
{
	FRAME_idle1, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle2, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle3, NULL, 0, 0, 0,  NULL, 0, spreader_idlenoise, 
	FRAME_idle4, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle5, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle6, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle7, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle8, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle9, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle10, NULL, 0, 0, 0,  NULL, 0, spreader_pause, 
	FRAME_idle11, NULL, 0, 0, 0,  NULL, 0, spreader_pause 	
};
animmove_t spreader_move_delay = { 11, spreader_frames_delay, spreader_pause };

//==============================================================================
//
// m_plagueSsitra_anim.c
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

#include "m_plaguessithra_anim.h"
#include "m_plaguessithra.h"
#include "g_monster.h"
#include "c_ai.h"
#include "vector.h"

void old_ai_run (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);
void ai_charge2 (edict_t *self, float dist);
qboolean ssithraCheckInWater (edict_t *self);
void MG_Pathfind(edict_t *self, qboolean check_clear_path);
void MG_SwimFlyToGoal (edict_t *self, float dist);


void ssithra_ai_run (edict_t *self, float dist)
{
	if(ssithraCheckInWater(self))
	{
		MG_SwimFlyToGoal(self, dist);//really need to get rid of this!
		MG_Pathfind(self, false);
	}
	else
		ai_run(self, dist);
}

/*----------------------------------------------------------------------
  Ssithra Idle - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_idle1 [] =
{
	FRAME_idle01,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle02,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle03,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle04,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle05,	NULL, 0, 0, 0,  ai_stand, 0, ssithraGrowlSound,
	FRAME_idle06,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle07,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle08,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle09,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle10, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle11,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle12,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle13,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle14,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle15,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle16,	NULL, 0, 0, 0,  ai_stand, 0, ssithraGrowlSound,
	FRAME_idle17,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle18,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle19,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle20, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle21,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle22,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle23,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle24,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle25,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle26,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle27,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle28,	NULL, 0, 0, 0,  ai_stand, 0, ssithraGrowlSound,
	FRAME_idle29,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle30, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle31,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle32,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle33,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle34,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle35,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle36,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle37,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle38,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle39,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idle40, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
};
animmove_t ssithra_move_idle1 = {40, ssithra_frames_idle1, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra Walk - walking along
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_walk1 [] =
{
	FRAME_walk1, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk2, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk3, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk4, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk5, 	NULL, 0, 0, 0,  ai_walk, 3, NULL,
	FRAME_walk6, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk7, 	NULL, 0, 0, 0,  ai_walk, 5, ssithraGrowlSound,
	FRAME_walk8, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk9, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk10, 	NULL, 0, 0, 0,  ai_walk, 7, NULL,
	FRAME_walk11, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk12, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
	FRAME_walk13, 	NULL, 0, 0, 0,  ai_walk, 2, NULL,
	FRAME_walk14, 	NULL, 0, 0, 0,  ai_walk, 5, NULL,
};
animmove_t ssithra_move_walk1 = {14, ssithra_frames_walk1, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra backpedal - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_backpedal1 [] =
{
	FRAME_backpedal1,	NULL, 0, 0, 0,  ai_charge2, -5, NULL,
	FRAME_backpedal2,	NULL, 0, 0, 0,  ai_charge2, -5, NULL,
	FRAME_backpedal3,	NULL, 0, 0, 0,  ai_charge2, -5, ssithraArrow,
	FRAME_backpedal4,	NULL, 0, 0, 0,  ai_charge2, -7, NULL,
	FRAME_backpedal5,	NULL, 0, 0, 0,  ai_charge2, -7, NULL,
	FRAME_backpedal6,	NULL, 0, 0, 0,  ai_charge2, -7, NULL,
	FRAME_backpedal7,	NULL, 0, 0, 0,  ai_charge2, -7, ssithraArrow,
	FRAME_backpedal8,	NULL, 0, 0, 0,  ai_charge2, -5, NULL,
	FRAME_backpedal9,	NULL, 0, 0, 0,  ai_charge2, -5, NULL,
	FRAME_backpedal10, 	NULL, 0, 0, 0,  ai_charge2, -5, NULL,
};
animmove_t ssithra_move_backpedal1 = {10, ssithra_frames_backpedal1, ssithra_decide_backpedal};

/*----------------------------------------------------------------------
  Ssithra bound - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_bound1 [] =
{
	FRAME_bound09,	NULL, 0, 0, 0,  ssithra_ai_run, 16, NULL,//ssithraBoundCheck,
	FRAME_bound10,	NULL, 0, 0, 0,  ssithra_ai_run, 16, NULL,
	FRAME_bound11,	NULL, 0, 0, 0,  ssithra_ai_run, 20, NULL,
	FRAME_bound12,	NULL, 0, 0, 0,  ssithra_ai_run, 20, NULL,
	FRAME_bound13,	NULL, 0, 0, 0,  ssithra_ai_run, 24, NULL,
	FRAME_bound14,	NULL, 0, 0, 0,  ssithra_ai_run, 20, NULL,
	FRAME_bound15,	NULL, 0, 0, 0,  ssithra_ai_run, 16, NULL,
	FRAME_bound16,	NULL, 0, 0, 0,  ssithra_ai_run, 12, NULL,
};
animmove_t ssithra_move_bound1 = {8, ssithra_frames_bound1, SsithraCheckMood};


/*----------------------------------------------------------------------
  Ssithra death_a
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_death_a1 [] =
{
	FRAME_death_a1,	ssithraSound, SND_DIE, CHAN_VOICE, 0,  NULL, 0, NULL,
	FRAME_death_a2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a4,	NULL, 0, 0, 0,  NULL, 0, MG_NoBlocking,
	FRAME_death_a5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a6,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a7,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a8,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a9,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a10, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a11,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a12,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_death_a1 = {12, ssithra_frames_death_a1, ssithra_dead};

/*----------------------------------------------------------------------
  Ssithra death_b - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_death_b1 [] =
{
	FRAME_death_b1,	ssithraSound, SND_DIE, CHAN_VOICE, 0,  NULL, 0, NULL,
	FRAME_death_b2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b4,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b6,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b7,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b8,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b9,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b10, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b11,	NULL, 0, 0, 0,  NULL, 0, MG_NoBlocking,
	FRAME_death_b12,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b13,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b14,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b15,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b16,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b17,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b18,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b19,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b20, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b21,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b22,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b23,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b24,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b25,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b26,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b27,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b28,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b29,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b30, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b31,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b32,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b33,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b34,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b35,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b36,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_death_b1 = {36, ssithra_frames_death_b1, ssithra_dead};

/*----------------------------------------------------------------------
  Ssithra dive - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_dive1 [] =
{
	FRAME_dive1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive4,	ssithraJump, 400, 100, 0,  NULL, 0, NULL,
	FRAME_dive5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive6,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive7,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive8,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive9,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive10, 	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive11,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive12,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive13,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive14,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive15,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive16,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive17,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive18,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,//fixme- check to make sure hit water
	FRAME_dive19,	NULL, 0, 0, 0,  ai_move , 22, ssithraCheckHitWaterSplash,//in water, go forward
	FRAME_dive20, 	NULL, 0, 0, 0,  ai_move , 20, ssithraCheckHitWaterSplash,
	FRAME_dive21,	NULL, 0, 0, 0,  ai_move , 17, ssithraCheckHitWaterSplash,
	FRAME_dive22,	NULL, 0, 0, 0,  ai_move , 15, ssithraCheckHitWaterSplash,
	FRAME_dive23,	NULL, 0, 0, 0,  ai_move , 12, ssithraCheckHitWaterSplash,
	FRAME_dive24,	NULL, 0, 0, 0,  ai_move , 9, ssithraCheckHitWaterSplash,
	FRAME_dive25,	NULL, 0, 0, 0,  ai_move , 6, ssithraCheckHitWaterSplash,
	FRAME_dive26,	NULL, 0, 0, 0,  ai_move , 3, ssithraCheckHitWaterSplash,
};
animmove_t ssithra_move_dive1 = {26, ssithra_frames_dive1, ssithra_decide_swimforward};

/*----------------------------------------------------------------------
  Ssithra duckshoot 
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_unduck [] =
{
	FRAME_duckshoot3,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_duckshoot2,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraUnCrouch,
	FRAME_duckshoot1,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
};
animmove_t ssithra_move_unduck = {3, ssithra_frames_unduck, ssithra_decide_gallop};

animframe_t ssithra_frames_duckloop [] =
{
	FRAME_duckshoot6,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_duckloop = {1, ssithra_frames_duckloop, ssithraCheckUnDuck};

animframe_t ssithra_frames_duckshoot1 [] =
{
	FRAME_duckshoot1,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_duckshoot2,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_duckshoot3,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCrouch,
	FRAME_duckshoot4,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	//Wait
	FRAME_duckshoot5,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraStartDuckArrow,
	FRAME_duckshoot5,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_duckshoot5,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_duckshoot5,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_duckshoot5,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	//Fire
	FRAME_duckshoot5,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckDuckArrow,
	FRAME_duckshoot6,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
};
animmove_t ssithra_move_duckshoot1 = {11, ssithra_frames_duckshoot1, ssithraCheckUnDuck};

/*----------------------------------------------------------------------
  Ssithra duck 
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_duck1 [] =
{//oops, duckframes same as duckshoot?
	FRAME_duckshoot1,	ssithraSound, SND_GROWL3, CHAN_VOICE, ATTN_IDLE,  NULL, 0, ssithraGrowlSound,
	FRAME_duckshoot2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot3,	NULL, 0, 0, 0,  NULL, 0, ssithraCrouch,
	FRAME_duckshoot4,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot6,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_duck1 = {6, ssithra_frames_duck1, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra gallop - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_gallop1 [] =
{
	FRAME_gallop1,	ssithraJump, 100, 50, 0,  ssithra_ai_run, 20, NULL,
//	FRAME_gallop1,	NULL, 0, 0, 0,  ssithra_ai_run, 20, NULL,
	FRAME_gallop2,	NULL, 0, 0, 0,  ssithra_ai_run, 24, NULL,
	FRAME_gallop3,	NULL, 0, 0, 0,  ssithra_ai_run, 32, NULL,
	FRAME_gallop4,	NULL, 0, 0, 0,  ssithra_ai_run, 30, NULL,
	FRAME_gallop5,	NULL, 0, 0, 0,  ssithra_ai_run, 28, NULL,
	FRAME_gallop6,	NULL, 0, 0, 0,  ssithra_ai_run, 26, NULL,
	FRAME_gallop7,	NULL, 0, 0, 0,  ssithra_ai_run, 24, NULL,
	FRAME_gallop8,	NULL, 0, 0, 0,  ssithra_ai_run, 22, NULL,
};
animmove_t ssithra_move_gallop1 = {8, ssithra_frames_gallop1, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra jump from buoy
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_fjump[] =
{
	FRAME_bound09,	NULL, 0, 0, 0,  NULL, 0, ssithraApplyJump,
	FRAME_bound10,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound11,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound12,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound13,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound14,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound15,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound16,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_fjump = {8, ssithra_frames_fjump, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra idlebasic - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_idlebasic1 [] =
{
	FRAME_idlebasic01,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic02,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic03,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic04,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic05,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic06,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic07,	NULL, 0, 0, 0,  ai_stand, 0, ssithraGrowlSound,
	FRAME_idlebasic08,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic09,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic10, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic11,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic12,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic13,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic14,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic15,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic16,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic17,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic18,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic19,	NULL, 0, 0, 0,  ai_stand, 0, ssithraGrowlSound,
	FRAME_idlebasic20, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic21,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic22,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic23,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic24,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic25,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic26,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic27,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic28,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic29,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic30, 	NULL, 0, 0, 0,  ai_stand, 0, ssithraGrowlSound,
	FRAME_idlebasic31,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic32,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic33,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic34,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic35,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic36,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic37,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic38,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic39,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic40, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
};
animmove_t ssithra_move_idlebasic1 = {40, ssithra_frames_idlebasic1, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra idleright - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_idleright1 [] =
{
	FRAME_idleright01,	ssithraSound, SND_GROWL2, CHAN_VOICE, ATTN_IDLE,  ai_stand, 0, ssithraGrowlSound,
	FRAME_idleright02,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright03,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright04,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright05,	ssithraVOfs, 0, -20, 0,  ai_stand, 0, NULL,
	FRAME_idleright06,	ssithraVOfs, 0, -30, 0,  ai_stand, 0, NULL,
	FRAME_idleright07,	ssithraVOfs, 0, -50, 0,  ai_stand, 0, NULL,
	FRAME_idleright08,	ssithraVOfs, 0, -70, 0,  ai_stand, 0, NULL,
	FRAME_idleright09,	ssithraVOfs, 0, -80, 0,  ai_stand, 0, NULL,
	FRAME_idleright10, 	ssithraVOfs, 0, -90, 0,  ai_stand, 0, NULL,//ssithraPanicArrow,
	FRAME_idleright10,	NULL, 0, 0, 0,  ai_stand, 0, NULL,//keep looking here a bit
	FRAME_idleright10,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright10,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright10,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright11,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright11,	ssithraVOfs, 0, -70, 0,  ai_stand, 0, NULL,
	FRAME_idleright12,	ssithraVOfs, 0, -50, 0,  ai_stand, 0, NULL,
	FRAME_idleright13,	ssithraVOfs, 0, -40, 0,  ai_stand, 0, NULL,
	FRAME_idleright14,	ssithraVOfs, 0, -30, 0,  ai_stand, 0, NULL,
	FRAME_idleright15,	ssithraVOfs, 0, -20, 0,  ai_stand, 0, NULL,
	FRAME_idleright16,	ssithraVOfs, 0, -10, 0,  ai_stand, 0, NULL,
	FRAME_idleright17,	ssithraVOfs, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright18,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright19,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idleright20,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
};
animmove_t ssithra_move_idleright1 = {24, ssithra_frames_idleright1, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra melee - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_melee1 [] =
{
	FRAME_melee1,	ssithraSound, SND_SWIPE, CHAN_WEAPON, 0,  ai_charge2, 20, NULL,
	FRAME_melee2,	NULL, 0, 0, 0,  ai_charge2, 10, NULL,
	FRAME_melee3,	NULL, 0, 0, 0,  ai_charge2, 8, NULL,
	FRAME_melee4,	NULL, 0, 0, 0,  ai_charge2, 6, NULL,
	FRAME_melee5,	NULL, 0, 0, 0,  ai_charge2, 4, ssithraSwipe,
	FRAME_melee6,	NULL, 0, 0, 0,  ai_charge2, 3, NULL,
	FRAME_melee7,	NULL, 0, 0, 0,  ai_charge2, 3, NULL,
};
animmove_t ssithra_move_melee1 = {7, ssithra_frames_melee1, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra melee - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_meleest [] =
{
	FRAME_Melee_stand1,	ssithraSound, SND_SWIPE, CHAN_WEAPON, 0,  NULL, 0, NULL,
	FRAME_Melee_stand2,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_Melee_stand3,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_Melee_stand4,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_Melee_stand5,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraSwipe,
	FRAME_Melee_stand6,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_Melee_stand7,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
};
animmove_t ssithra_move_meleest = {7, ssithra_frames_meleest, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra namor - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_namor1 [] =
{
	FRAME_namor1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_namor2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_namor3,	NULL, 0, 0, 0,  NULL, 0, ssithraNamorJump,
	FRAME_namor4,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor5,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor6,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor7,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor8,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor9,	NULL, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor10, 	NULL, 0, 0, 0,  ssithraForward, 120, ssithraCheckLeaveWaterSplash,
	FRAME_namor11,	NULL, 0, 0, 0,  ssithraForward, 130, ssithraCheckLeaveWaterSplash,
	FRAME_namor12,	NULL, 0, 0, 0,  ssithraForward, 140, ssithraCheckLeaveWaterSplash,
	FRAME_namor13,	NULL, 0, 0, 0,  ssithraForward, 150, ssithraCheckLeaveWaterSplash,
	FRAME_namor14,	NULL, 0, 0, 0,  ssithraForward, 160, ssithraCheckLeaveWaterSplash,
	FRAME_namor15,	NULL, 0, 0, 0,  ssithraForward, 170, ssithraCheckLeaveWaterSplash,
	FRAME_namor16,	NULL, 0, 0, 0,  ssithraForward, 180, ssithraCheckLeaveWaterSplash,
	FRAME_namor17,	NULL, 0, 0, 0,  ssithraForward, 190, ssithraCheckLeaveWaterSplash,
	FRAME_namor18,	NULL, 0, 0, 0,  ssithraForward, 200, ssithraCheckLeaveWaterSplash,//fixme- check to make sure out of water
	FRAME_namor19,	NULL, 0, 0, 0,  ssithraForward, 200, ssithraCheckLeaveWaterSplash,
	FRAME_namor20, 	NULL, 0, 0, 0,  ssithraForward, 200, ssithraCheckLeaveWaterSplash,
	FRAME_namor21,	NULL, 0, 0, 0,  ssithraForward, 200, ssithraCheckLeaveWaterSplash,
	FRAME_namor22,	NULL, 0, 0, 0,  ssithraForward, 200, ssithraCheckLeaveWaterSplash,
	FRAME_namor23,	NULL, 0, 0, 0,  ssithraForward, 180, ssithraCheckLeaveWaterSplash,
	FRAME_namor24,	NULL, 0, 0, 0,  ssithraForward, 120, ssithraCheckLeaveWaterSplash,
	FRAME_namor25,	NULL, 0, 0, 0,  ssithraForward, 60, ssithraCheckLeaveWaterSplash,
};
animmove_t ssithra_move_namor1 = {25, ssithra_frames_namor1, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra pain_a - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_pain_a1 [] =
{
	FRAME_pain_a1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_pain_a2,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_pain_a1 = {2, ssithra_frames_pain_a1, ssithra_pain_react};

/*----------------------------------------------------------------------
  Ssithra shoot 
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_shoot1 [] =
{
	FRAME_shoot1,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot2,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot3,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot4,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot5,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot6,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraArrow,
	FRAME_shoot6,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckLoop,
	FRAME_shoot5,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
};//7 - 11 not used- weird anim for turning while shooting only!
animmove_t ssithra_move_shoot1 = {8, ssithra_frames_shoot1, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra lunge from shooting
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_lunge [] =
{
	FRAME_shoot12,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraApplyJump,
	FRAME_shoot13,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot14,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot15,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot16,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot17,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot18,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot19,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot20, 	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot21,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
};
animmove_t ssithra_move_lunge = {10, ssithra_frames_lunge, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra startle - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_startle1 [] =
{
	FRAME_startle2,	ssithraSound, SND_GROWL3, CHAN_VOICE, ATTN_IDLE,  ai_stand, 0, ssithraGrowlSound,
	FRAME_startle3,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle4,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle5,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle6,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle7,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle8,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle9,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle10, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle11,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle12,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle13,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle14,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle15,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle16,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle17,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle18,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle19,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle20, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle21,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle22,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle23,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle24,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle25,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle26,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle27,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle28,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle29,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle30, 	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle31,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle32,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle33,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle34,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle35,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle36,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle37,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle38,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_startle39,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
};
animmove_t ssithra_move_startle1 = {38, ssithra_frames_startle1, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra startle - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_spinleft_go [] =
{
	FRAME_idlebasic41,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_spinleft_go = {1, ssithra_frames_spinleft_go, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra startle - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_spinleft [] =
{
	FRAME_idlebasic41,	ssithraSound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE,  NULL, 0, NULL,
	FRAME_idlebasic42,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic43,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic44,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic45,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic46,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic47,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic48,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic49,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic50, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic51,	NULL, 0, 0, 0,  ai_spin, 6, NULL,
	FRAME_idlebasic52,	NULL, 0, 0, 0,  ai_spin, 9, NULL,
	FRAME_idlebasic53,	NULL, 0, 0, 0,  ai_spin, 13, NULL,
	FRAME_idlebasic54,	NULL, 0, 0, 0,  ai_spin, 16, NULL,
	FRAME_idlebasic55,	NULL, 0, 0, 0,  ai_spin, 18, NULL,
	FRAME_idlebasic56,	NULL, 0, 0, 0,  ai_spin, 20, NULL,
	FRAME_idlebasic57,	NULL, 0, 0, 0,  ai_spin, 23, NULL,
	FRAME_idlebasic58,	NULL, 0, 0, 0,  ai_spin, 27, NULL,
	FRAME_idlebasic59,	NULL, 0, 0, 0,  ai_spin, 30, NULL,
	FRAME_idlebasic60,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_spinleft = {20, ssithra_frames_spinleft, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra startle - turn left as you aniimate turn around
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_spinright_go [] =
{
	FRAME_idlebasic41,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_spinright_go = {1, ssithra_frames_spinright_go, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra startle - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_spinright [] =
{
	FRAME_idlebasic61,	ssithraSound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE,  NULL, 0, NULL,
	FRAME_idlebasic62,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic63,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic64,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic65,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic66,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic67,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic68,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic69,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic70, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic71,	NULL, 0, 0, 0,  ai_spin, -6, NULL,
	FRAME_idlebasic72,	NULL, 0, 0, 0,  ai_spin, -9, NULL,
	FRAME_idlebasic73,	NULL, 0, 0, 0,  ai_spin, -13, NULL,
	FRAME_idlebasic74,	NULL, 0, 0, 0,  ai_spin, -16, NULL,
	FRAME_idlebasic75,	NULL, 0, 0, 0,  ai_spin, -18, NULL,
	FRAME_idlebasic76,	NULL, 0, 0, 0,  ai_spin, -20, NULL,
	FRAME_idlebasic77,	NULL, 0, 0, 0,  ai_spin, -23, NULL,
	FRAME_idlebasic78,	NULL, 0, 0, 0,  ai_spin, -27, NULL,
	FRAME_idlebasic79,	NULL, 0, 0, 0,  ai_spin, -30, NULL,
	FRAME_idlebasic80,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_spinright = {20, ssithra_frames_spinright, ssithra_decide_gallop};


/*----------------------------------------------------------------------
  Ssithra swimforward - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_swimforward1 [] =
{
	FRAME_swimforward01,	NULL, 0, 0, 0,  ssithra_ai_run, 10, ssithraCheckRipple,
	FRAME_swimforward02,	NULL, 0, 0, 0,  ssithra_ai_run, 10, NULL,
	FRAME_swimforward03,	NULL, 0, 0, 0,  ssithra_ai_run, 10, ssithraCheckRipple,
	FRAME_swimforward04,	ssithraSound, SND_SWIM, CHAN_BODY, 0,  ssithra_ai_run, 12, ssithra_check_namor,
	FRAME_swimforward05,	NULL, 0, 0, 0,  ssithra_ai_run, 12, ssithraCheckRipple,
	FRAME_swimforward06,	NULL, 0, 0, 0,  ssithra_ai_run, 12, ssithraCheckRipple,
	FRAME_swimforward07,	NULL, 0, 0, 0,  ssithra_ai_run, 12, ssithraCheckRipple,
	FRAME_swimforward08,	NULL, 0, 0, 0,  ssithra_ai_run, 12, ssithra_check_namor,
	FRAME_swimforward09,	NULL, 0, 0, 0,  ssithra_ai_run, 16, ssithraCheckRipple,
	FRAME_swimforward10, 	ssithraSound, SND_SWIM, CHAN_BODY, 0,  ssithra_ai_run, 16, NULL,
	FRAME_swimforward11,	NULL, 0, 0, 0,  ssithra_ai_run, 12, ssithraCheckRipple,
	FRAME_swimforward12,	NULL, 0, 0, 0,  ssithra_ai_run, 12, ssithra_check_namor,
	FRAME_swimforward13,	NULL, 0, 0, 0,  ssithra_ai_run, 12, ssithraCheckRipple,
	FRAME_swimforward14,	NULL, 0, 0, 0,  ssithra_ai_run, 10, ssithraCheckRipple,
	FRAME_swimforward15,	NULL, 0, 0, 0,  ssithra_ai_run, 10, ssithraCheckRipple,
};
animmove_t ssithra_move_swimforward1 = {15, ssithra_frames_swimforward1, ssithra_decide_swimforward};

/*----------------------------------------------------------------------
  Ssithra swimwander - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_swimwander [] =
{
	FRAME_swimforward01,	NULL, 0, 0, 0,  ai_walk, 10, ssithraCheckRipple,
	FRAME_swimforward02,	NULL, 0, 0, 0,  ai_walk, 10, ssithraCheckRipple,
	FRAME_swimforward03,	NULL, 0, 0, 0,  ai_walk, 10, ssithraCheckRipple,
	FRAME_swimforward04,	ssithraSound, SND_SWIM, CHAN_BODY, 0,  ai_walk, 12, ssithra_check_namor,
	FRAME_swimforward05,	NULL, 0, 0, 0,  ai_walk, 12, ssithraCheckRipple,
	FRAME_swimforward06,	NULL, 0, 0, 0,  ai_walk, 12, ssithraCheckRipple,
	FRAME_swimforward07,	NULL, 0, 0, 0,  ai_walk, 12, ssithraCheckRipple,
	FRAME_swimforward08,	NULL, 0, 0, 0,  ai_walk, 12, ssithra_check_namor,
	FRAME_swimforward09,	NULL, 0, 0, 0,  ai_walk, 16, ssithraCheckRipple,
	FRAME_swimforward10, 	ssithraSound, SND_SWIM, CHAN_BODY, 0,  ai_walk, 16, NULL,
	FRAME_swimforward11,	NULL, 0, 0, 0,  ai_walk, 12, ssithraCheckRipple,
	FRAME_swimforward12,	NULL, 0, 0, 0,  ai_walk, 12, ssithra_check_namor,
	FRAME_swimforward13,	NULL, 0, 0, 0,  ai_walk, 12, ssithraCheckRipple,
	FRAME_swimforward14,	NULL, 0, 0, 0,  ai_walk, 10, ssithraCheckRipple,
	FRAME_swimforward15,	NULL, 0, 0, 0,  ai_walk, 10, ssithraCheckRipple,
};
animmove_t ssithra_move_swimwander = {15, ssithra_frames_swimwander, ssithra_decide_swimforward};

/*----------------------------------------------------------------------
  Ssithra water_death - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_water_death1 [] =
{
	FRAME_water_death1,	ssithraSound, SND_DIE, CHAN_VOICE, 0,  NULL, 0, NULL,
	FRAME_water_death2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death4,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death6,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death7,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death8,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death9,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death10, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death11,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death12,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death13,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death14,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death15,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death16,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death17,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death18,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death19,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death20, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death21,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death22,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death23,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death24,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death25,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death26,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death27,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death28,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death29,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death30, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death31,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death32,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death33,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death34,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death35,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death36,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death37,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death38,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death39,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death40, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death41,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death42,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death43,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death44,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death45,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death46,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death47,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death48,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death49,	NULL, 0, 0, 0,  NULL, 0, NULL
};
animmove_t ssithra_move_water_death1 = {49, ssithra_frames_water_death1, ssithraWaterDead};

/*----------------------------------------------------------------------
  Ssithra water_idle - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_water_idle1 [] =
{//water idle sound?
	FRAME_water_idle1,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle2,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle3,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle4,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle5,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle6,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle7,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle8,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle9,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle10, NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle11,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle12,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle13,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle14,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle15,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle16,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle17,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle18,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle19,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_water_idle20, NULL, 0, 0, 0,  ai_stand, 0, NULL,
};
animmove_t ssithra_move_water_idle1 = {20, ssithra_frames_water_idle1, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra water_pain_a - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_water_pain_a1 [] =
{
	FRAME_water_pain_a1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_pain_a2,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_water_pain_a1 = {2, ssithra_frames_water_pain_a1, ssithra_pain_react};

/*----------------------------------------------------------------------
  Ssithra water_pain_b - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_water_pain_b1 [] =
{
	FRAME_water_pain_b1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_pain_b2,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_water_pain_b1 = {2, ssithra_frames_water_pain_b1, ssithra_pain_react};

/*----------------------------------------------------------------------
  Ssithra water_shoot - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_water_shoot1 [] =
{
	FRAME_water_shoot01,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot2,		NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot3,		NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot4,		NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot5,		NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot6,		NULL, 0, 0, 0,  ai_charge2, 0, ssithraArrow,
	FRAME_water_shoot7,		NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot8,		NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot9,		NULL, 0, 0, 0,  ai_charge2, 0, ssithraArrow,
	FRAME_water_shoot10, 	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot11,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot12,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraArrow,
	FRAME_water_shoot13,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot14,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot15,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraArrow,
	FRAME_water_shoot16,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_water_shoot17,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
};
animmove_t ssithra_move_water_shoot1 = {17, ssithra_frames_water_shoot1, ssithra_decide_swimforward};

animframe_t ssithra_frames_run1 [] =
{
	FRAME_run1,	NULL, 0, 0, 0,  ssithra_ai_run, 16, ssithraBoundCheck,
	FRAME_run2,	NULL, 0, 0, 0,  ssithra_ai_run, 18, ssithraDiveCheck,
	FRAME_run3,	NULL, 0, 0, 0,  ssithra_ai_run, 20, NULL,
	FRAME_run4,	NULL, 0, 0, 0,  ssithra_ai_run, 18, NULL,
	FRAME_run5,	NULL, 0, 0, 0,  ssithra_ai_run, 16, NULL,
	FRAME_run6,	NULL, 0, 0, 0,  ssithra_ai_run, 18, NULL,
	FRAME_run7,	NULL, 0, 0, 0,  ssithra_ai_run, 20, NULL,
	FRAME_run8,	NULL, 0, 0, 0,  ssithra_ai_run, 18, NULL,
};
animmove_t ssithra_move_run1 = {8, ssithra_frames_run1, ssithra_decide_gallop};

/*----------------------------------------------------------------------
  Ssithra face and namor
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_faceandnamor [] =
{
	FRAME_water_idle1,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle2,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle3,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle4,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle5,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle6,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle7,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle8,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle9,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle10, NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle11,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle12,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle13,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle14,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle15,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle16,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle17,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle18,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle19,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
	FRAME_water_idle20, NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckFacedNamor,
};
animmove_t ssithra_move_faceandnamor = {20, ssithra_frames_faceandnamor, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra look left
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_lookleft [] =
{
	FRAME_idlebasic41,	ssithraSound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE,  ai_stand, 0, NULL,
	FRAME_idlebasic42,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic43,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic44,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic45,	ssithraVOfs, 20, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic46,	ssithraVOfs, 40, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic47,	ssithraVOfs, 60, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic48,	ssithraVOfs, 80, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic49,	ssithraVOfs, 100, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic50, 	ssithraVOfs, 120, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic51,	ssithraVOfs, 160, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic52,	ssithraVOfs, 120, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic53,	ssithraVOfs, 80, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic54,	ssithraVOfs, 60, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic55,	ssithraVOfs, 40, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic56,	ssithraVOfs, 20, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic57,	ssithraVOfs, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic58,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic59,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic60,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
};
animmove_t ssithra_move_lookleft = {20, ssithra_frames_lookleft, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra look right
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_lookright [] =
{
	FRAME_idlebasic61,	ssithraSound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE,  ai_stand, 0, NULL,
	FRAME_idlebasic62,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic63,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic64,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic65,	ssithraVOfs, -20, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic66,	ssithraVOfs, -40, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic67,	ssithraVOfs, -60, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic68,	ssithraVOfs, -80, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic69,	ssithraVOfs, -100, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic70, 	ssithraVOfs, -120, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic71,	ssithraVOfs, -160, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic72,	ssithraVOfs, -120, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic73,	ssithraVOfs, -80, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic74,	ssithraVOfs, -60, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic75,	ssithraVOfs, -40, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic76,	ssithraVOfs, -20, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic77,	ssithraVOfs, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic78,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic79,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
	FRAME_idlebasic80,	NULL, 0, 0, 0,  ai_stand, 0, NULL,
};
animmove_t ssithra_move_lookright = {20, ssithra_frames_lookright, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra trans up
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_transup [] =
{
	FRAME_Water_trans1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Water_trans2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Water_trans3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Water_trans4,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_transup = {4, ssithra_frames_transup, ssithra_water_shoot};

/*----------------------------------------------------------------------
  Ssithra trans down
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_transdown [] =
{
	FRAME_Water_trans4,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Water_trans3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Water_trans2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Water_trans1,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_transdown = {4, ssithra_frames_transdown, ssithra_decide_swimforward};

/*
Headless
*/

/*----------------------------------------------------------------------
  Ssithra death_b - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_death_c [] =
{
	FRAME_shoot5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot4,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b4,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b6,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b7,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b8,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b9,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b10, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b11,	NULL, 0, 0, 0,  NULL, 0, MG_NoBlocking,
	FRAME_death_b12,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b13,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b14,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b15,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b16,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b17,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b18,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b19,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b20, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b21,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b22,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b23,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b24,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b25,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b26,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b27,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b28,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b29,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b30, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b31,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b32,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b33,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b34,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b35,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b36,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_death_c = {41, ssithra_frames_death_c, ssithra_dead};

animframe_t ssithra_frames_headlessloop [] =
{
	FRAME_shoot8,	NULL, 0, 0, 0,  ai_spin, -5, NULL,
	FRAME_shoot9,	NULL, 0, 0, 0,  ai_spin, -4, ssithraPanicArrow,
	FRAME_shoot10,	NULL, 0, 0, 0,  ai_spin, -2, NULL,
	FRAME_shoot11,	NULL, 0, 0, 0,  ai_spin, -2, NULL,
	FRAME_shoot12, 	NULL, 0, 0, 0,  ai_spin, -3, NULL,
	FRAME_shoot13,	NULL, 0, 0, 0,  ai_spin, -4, NULL,
};
animmove_t ssithra_move_headlessloop = {6, ssithra_frames_headlessloop, ssithraCollapse};

animframe_t ssithra_frames_headless [] =
{
	FRAME_shoot1,	NULL, 0, 0, 0,  ai_spin, -20, NULL,
	FRAME_shoot2,	NULL, 0, 0, 0,  ai_spin, -12, NULL,
	FRAME_shoot3,	NULL, 0, 0, 0,  ai_spin, -10, NULL,
	FRAME_shoot4,	NULL, 0, 0, 0,  ai_spin, -9, NULL,
	FRAME_shoot5,	NULL, 0, 0, 0,  ai_spin, -8, NULL,
	FRAME_shoot6,	NULL, 0, 0, 0,  ai_spin, -7, ssithraPanicArrow,
	FRAME_shoot7,	NULL, 0, 0, 0,  ai_spin, -7, NULL,
	FRAME_shoot8,	NULL, 0, 0, 0,  ai_spin, -7, ssithraPanicArrow,
	FRAME_shoot9,	NULL, 0, 0, 0,  ai_spin, -6, NULL,
	FRAME_shoot10, 	NULL, 0, 0, 0,  ai_spin, -6, NULL,
	FRAME_shoot11,	NULL, 0, 0, 0,  ai_spin, -6, NULL,
	FRAME_shoot12,	NULL, 0, 0, 0,  ai_spin, -5, NULL,
	FRAME_shoot13,	NULL, 0, 0, 0,  ai_spin, -5, NULL,
};
animmove_t ssithra_move_headless = {13, ssithra_frames_headless, ssithraCollapse};

/*----------------------------------------------------------------------
  Ssithra dead
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_dead_a [] =
{
	FRAME_death_a12,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_dead_a = {1, ssithra_frames_dead_a, NULL};

animframe_t ssithra_frames_dead_b [] =
{
	FRAME_death_b36,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_dead_b = {1, ssithra_frames_dead_b, NULL};

animframe_t ssithra_frames_dead_water [] =
{
	FRAME_water_death49,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_dead_water = {1, ssithra_frames_dead_water, NULL};

/*----------------------------------------------------------------------
  Ssithra sliced
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_sliced [] =
{
	FRAME_death_b1,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b2,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b3,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b4,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b5,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b6,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b7,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b8,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b9,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b10, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b11,	NULL, 0, 0, 0,  NULL, 0, MG_NoBlocking,
	FRAME_death_b12,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b13,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b14,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b15,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b16,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b17,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b18,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b19,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b20, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b21,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b22,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b23,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b24,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b25,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b26,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b27,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b28,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b29,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b30, 	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b31,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b32,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b33,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b34,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b35,	NULL, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b36,	NULL, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_sliced = {36, ssithra_frames_sliced, ssithraKillSelf};


/*----------------------------------------------------------------------
  Ssithra delay - stop and look around
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_delay [] =
{
	FRAME_startle2,	ssithraSound, SND_GROWL3, CHAN_VOICE, ATTN_IDLE,  NULL, 0, SsithraCheckMood,
	FRAME_startle3,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle4,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle5,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle6,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle7,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle8,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle9,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle10, 	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle11,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle12,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle13,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle14,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle15,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle16,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle17,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle18,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle19,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle20, 	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle21,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle22,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle23,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle24,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle25,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle26,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle27,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle28,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle29,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle30, 	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle31,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle32,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle33,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle34,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle35,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle36,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle37,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle38,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
	FRAME_startle39,	NULL, 0, 0, 0,  NULL, 0, SsithraCheckMood,
};
animmove_t ssithra_move_delay = {38, ssithra_frames_delay, SsithraCheckMood};

/*----------------------------------------------------------------------
  Ssithra Idle - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_idle1 [] =
{
	FRAME_idle01,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle02,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle03,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle04,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle05,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraGrowlSound,
	FRAME_idle06,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle07,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle08,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle09,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle10, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle11,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle12,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle16,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraGrowlSound,
	FRAME_idle17,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle18,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle19,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle20, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle21,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle22,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle23,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle24,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle25,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle26,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle27,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle28,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraGrowlSound,
	FRAME_idle29,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle30, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle31,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle32,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle33,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle34,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle35,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle36,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle37,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle38,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle39,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idle40, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL
};
animmove_t ssithra_c_move_idle1 = {40, ssithra_c_frames_idle1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra Walk - walking along
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_walk1 [] =
{
	FRAME_walk1, 	ai_c_move, 0, 0, 0,  NULL, 2, NULL,
	FRAME_walk2, 	ai_c_move, 0, 0, 0,  NULL, 3, NULL,
	FRAME_walk3, 	ai_c_move, 0, 0, 0,  NULL, 3, NULL,
	FRAME_walk4, 	ai_c_move, 0, 0, 0,  NULL, 3, NULL,
	FRAME_walk5, 	ai_c_move, 0, 0, 0,  NULL, 2, NULL,
	FRAME_walk6, 	ai_c_move, 0, 0, 0,  NULL, 2, NULL,
	FRAME_walk7, 	ai_c_move, 0, 0, 0,  NULL, 2, ssithraGrowlSound,
	FRAME_walk8, 	ai_c_move, 0, 0, 0,  NULL, 3, NULL,
	FRAME_walk9, 	ai_c_move, 0, 0, 0,  NULL, 3, NULL,
	FRAME_walk10, 	ai_c_move, 0, 0, 0,  NULL, 3, NULL,
	FRAME_walk11, 	ai_c_move, 0, 0, 0,  NULL, 3, NULL,
	FRAME_walk12, 	ai_c_move, 0, 0, 0,  NULL, 3, NULL,
	FRAME_walk13, 	ai_c_move, 0, 0, 0,  NULL, 2, NULL,
	FRAME_walk14, 	ai_c_move, 0, 0, 0,  NULL, 2, NULL
};
animmove_t ssithra_c_move_walk1 = {14, ssithra_c_frames_walk1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra backpedal - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_backpedal1 [] =
{
	FRAME_backpedal1,	ai_c_move, -5, 0, 0,  NULL, 0, NULL,
	FRAME_backpedal2,	ai_c_move, -5, 0, 0,  NULL, 0, NULL,
	FRAME_backpedal3,	ai_c_move, -5, 0, 0,  NULL, 0, ssithraArrow,
	FRAME_backpedal4,	ai_c_move, -7, 0, 0,  NULL, 0, NULL,
	FRAME_backpedal5,	ai_c_move, -7, 0, 0,  NULL, 0, NULL,
	FRAME_backpedal6,	ai_c_move, -7, 0, 0,  NULL, 0, NULL,
	FRAME_backpedal7,	ai_c_move, -7, 0, 0,  NULL, 0, ssithraArrow,
	FRAME_backpedal8,	ai_c_move, -5, 0, 0,  NULL, 0, NULL,
	FRAME_backpedal9,	ai_c_move, -5, 0, 0,  NULL, 0, NULL,
	FRAME_backpedal10, 	ai_c_move, -5, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_backpedal1 = {10, ssithra_c_frames_backpedal1, ai_c_cycleend};


/*----------------------------------------------------------------------
  Ssithra bound - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action1 [] =
{
	FRAME_bound09,	ai_c_move, 16, 0, 0,  NULL, 0, NULL,
	FRAME_bound10,	ai_c_move, 16, 0, 0,  NULL, 0, NULL,
	FRAME_bound11,	ai_c_move, 20, 0, 0,  NULL, 0, NULL,
	FRAME_bound12,	ai_c_move, 20, 0, 0,  NULL, 0, NULL,
	FRAME_bound13,	ai_c_move, 24, 0, 0,  NULL, 0, NULL,
	FRAME_bound14,	ai_c_move, 20, 0, 0,  NULL, 0, NULL,
	FRAME_bound15,	ai_c_move, 16, 0, 0,  NULL, 0, NULL,
	FRAME_bound16,	ai_c_move, 12, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action1 = {8, ssithra_c_frames_action1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra death_a
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_death2 [] =
{
	FRAME_death_a1,	ssithraSound, SND_DIE, CHAN_VOICE, 0,  NULL, 0, NULL,
	FRAME_death_a2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a4,	ai_c_move, 0, 0, 0,  NULL, 0, MG_NoBlocking,
	FRAME_death_a5,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a7,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a8,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a9,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a10,ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a11,ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_a12,ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_death2 = {12, ssithra_c_frames_death2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra death_b - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_death3 [] =
{
	FRAME_death_b1,		ssithraSound, SND_DIE, CHAN_VOICE, 0,  NULL, 0, NULL,
	FRAME_death_b2,		ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b3,		ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b4,		ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b5,		ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b6,		ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b7,		ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b8,		ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b9,		ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b10, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b11,	ai_c_move, 0, 0, 0,  NULL, 0, MG_NoBlocking,
	FRAME_death_b12,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b16,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b17,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b18,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b19,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b20, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b21,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b22,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b23,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b24,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b25,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b26,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b27,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b28,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b29,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b30, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b31,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b32,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b33,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b34,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b35,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_death_b36,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_death3 = {36, ssithra_c_frames_death3, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra dive - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action2 [] =
{
	FRAME_dive1,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive4,	ssithraJump, 400, 100, 0,  NULL, 0, NULL,
	FRAME_dive5,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive7,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive8,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_dive9,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive10, 	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive11,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive12,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive13,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive14,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive15,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive16,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive17,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,
	FRAME_dive18,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckHitWaterSplash,//fixme- check to make sure hit water
	FRAME_dive19,	ai_c_move, 0, 0, 0,  ai_move , 22, ssithraCheckHitWaterSplash,//in water, go forward
	FRAME_dive20, 	ai_c_move, 0, 0, 0,  ai_move , 20, ssithraCheckHitWaterSplash,
	FRAME_dive21,	ai_c_move, 0, 0, 0,  ai_move , 17, ssithraCheckHitWaterSplash,
	FRAME_dive22,	ai_c_move, 0, 0, 0,  ai_move , 15, ssithraCheckHitWaterSplash,
	FRAME_dive23,	ai_c_move, 0, 0, 0,  ai_move , 12, ssithraCheckHitWaterSplash,
	FRAME_dive24,	ai_c_move, 0, 0, 0,  ai_move , 9, ssithraCheckHitWaterSplash,
	FRAME_dive25,	ai_c_move, 0, 0, 0,  ai_move , 6, ssithraCheckHitWaterSplash,
	FRAME_dive26,	ai_c_move, 0, 0, 0,  ai_move , 3, ssithraCheckHitWaterSplash,
};
animmove_t ssithra_c_move_action2 = {26, ssithra_c_frames_action2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra duckshoot 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action3 [] =
{
	FRAME_duckshoot3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot2,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraUnCrouch,
	FRAME_duckshoot1,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action3 = {3, ssithra_c_frames_action3, ai_c_cycleend};

animframe_t ssithra_c_frames_action4 [] =
{
	FRAME_duckshoot6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action4 = {1, ssithra_c_frames_action4, ai_c_cycleend};

animframe_t ssithra_c_frames_action5 [] =
{
	FRAME_duckshoot1,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot4,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot5,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckDuckArrow,
	FRAME_duckshoot6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action5 = {6, ssithra_c_frames_action5, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra duck 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action6 [] =
{//oops, duckframes same as duckshoot?
	FRAME_duckshoot1,	ssithraSound, SND_GROWL3, CHAN_VOICE, ATTN_IDLE,  NULL, 0, NULL,
	FRAME_duckshoot2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot4,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot5,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_duckshoot6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action6 = {6, ssithra_c_frames_action6, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra gallop - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_run1 [] =
{
	FRAME_gallop1,	ai_c_move, 20, 0, 0,  NULL, 0, NULL,
	FRAME_gallop2,	ai_c_move, 24, 0, 0,  NULL, 0, NULL,
	FRAME_gallop3,	ai_c_move, 32, 0, 0,  NULL, 0, NULL,
	FRAME_gallop4,	ai_c_move, 30, 0, 0,  NULL, 0, NULL,
	FRAME_gallop5,	ai_c_move, 28, 0, 0,  NULL, 0, NULL,
	FRAME_gallop6,	ai_c_move, 26, 0, 0,  NULL, 0, NULL,
	FRAME_gallop7,	ai_c_move, 24, 0, 0,  NULL, 0, NULL,
	FRAME_gallop8,	ai_c_move, 22, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_run1 = {8, ssithra_c_frames_run1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra jump from buoy
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action7[] =
{
	FRAME_bound09,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraApplyJump,
	FRAME_bound10,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound11,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound12,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_bound16,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action7 = {8, ssithra_c_frames_action7, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra idle2 - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_idle2 [] =
{
	FRAME_idlebasic01,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic02,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic03,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic04,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic05,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic06,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic07,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraGrowlSound,
	FRAME_idlebasic08,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic09,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic10, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic11,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic12,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic16,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic17,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic18,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic19,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraGrowlSound,
	FRAME_idlebasic20, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic21,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic22,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic23,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic24,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic25,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic26,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic27,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic28,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic29,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic30, 	ai_c_move, 0, 0, 0,  NULL, 0, ssithraGrowlSound,
	FRAME_idlebasic31,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic32,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic33,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic34,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic35,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic36,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic37,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic38,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic39,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic40, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_idle2 = {40, ssithra_c_frames_idle2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra idle3 - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_idle3 [] =
{
	FRAME_idleright01,	ssithraSound, SND_GROWL2, CHAN_VOICE, ATTN_IDLE,  NULL, 0, NULL,
	FRAME_idleright02,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright03,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright04,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright05,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright06,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright07,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright08,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright09,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright10, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,//ssithraPanicArrow,
	FRAME_idleright10,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,//keep looking here a bit
	FRAME_idleright10,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright10,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright10,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright11,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright11,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright12,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright16,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright17,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright18,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright19,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idleright20,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_idle3 = {24, ssithra_c_frames_idle3, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra attack1 - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_attack1 [] =
{
	FRAME_melee1,	ai_c_move, 20, 0, 0,  NULL, 0, NULL,
	FRAME_melee2,	ai_c_move, 10, 0, 0,  NULL, 0, NULL,
	FRAME_melee3,	ai_c_move, 8,  0, 0,  NULL, 0, NULL,
	FRAME_melee4,	ai_c_move, 6,  0, 0,  NULL, 0, NULL,
	FRAME_melee5,	ai_c_move, 4,  0, 0,  NULL, 0, ssithraSwipe,
	FRAME_melee6,	ai_c_move, 3,  0, 0,  NULL, 0, NULL,
	FRAME_melee7,	ai_c_move, 3,  0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_attack1 = {7, ssithra_c_frames_attack1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra melee - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_attack2 [] =
{
	FRAME_Melee_stand1,	ssithraSound, SND_SWIPE, CHAN_WEAPON, 0,  NULL, 0, NULL,
	FRAME_Melee_stand2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Melee_stand3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Melee_stand4,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Melee_stand5,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraSwipe,
	FRAME_Melee_stand6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_Melee_stand7,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_attack2 = {7, ssithra_c_frames_attack2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra namor - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action8 [] =
{
	FRAME_namor1,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_namor2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_namor3,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraNamorJump,
	FRAME_namor4,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor5,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor6,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor7,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor8,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor9,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor10, 	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor11,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor12,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor13,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor14,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor15,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor16,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor17,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor18,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor19,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor20, 	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor21,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor22,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor23,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor24,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
	FRAME_namor25,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraCheckLeaveWaterSplash,
};
animmove_t ssithra_c_move_action8 = {25, ssithra_c_frames_action8, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra pain_a - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_pain1 [] =
{
	FRAME_pain_a1,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_pain_a2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_pain1 = {2, ssithra_c_frames_pain1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra shoot 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_attack3 [] =
{
	FRAME_shoot1,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot2,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot3,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot4,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot5,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
	FRAME_shoot6,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraArrow,
	FRAME_shoot6,	NULL, 0, 0, 0,  ai_charge2, 0, ssithraCheckLoop,
	FRAME_shoot5,	NULL, 0, 0, 0,  ai_charge2, 0, NULL,
};
animmove_t ssithra_c_move_attack3 = {8, ssithra_c_frames_attack3, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra lunge from shooting
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action9 [] =
{
	FRAME_shoot12,	ai_c_move, 0, 0, 0,  NULL, 0, ssithraApplyJump,
	FRAME_shoot13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot16,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot17,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot18,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot19,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot20, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_shoot21,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action9 = {10, ssithra_c_frames_action9, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra startle - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action10 [] =
{
	FRAME_startle2,	ssithraSound, SND_GROWL3, CHAN_VOICE, ATTN_IDLE,  NULL, 0, NULL,
	FRAME_startle3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle4,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle5,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle7,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle8,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle9,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle10, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle11,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle12,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle16,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle17,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle18,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle19,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle20, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle21,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle22,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle23,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle24,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle25,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle26,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle27,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle28,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle29,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle30, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle31,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle32,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle33,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle34,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle35,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle36,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle37,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle38,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_startle39,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action10 = {38, ssithra_c_frames_action10, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra startle - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action11 [] =
{
	FRAME_idlebasic41,	ssithraSound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE,  NULL, 0, NULL,
	FRAME_idlebasic42,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic43,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic44,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic45,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic46,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic47,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic48,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic49,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic50, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic51,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic52,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic53,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic54,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic55,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic56,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic57,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic58,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic59,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic60,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action11 = {20, ssithra_c_frames_action11, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra startle - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_action12 [] =
{
	FRAME_idlebasic61,	ssithraSound, SND_GROWL1, CHAN_VOICE, ATTN_IDLE,  NULL, 0, NULL,
	FRAME_idlebasic62,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic63,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic64,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic65,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic66,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic67,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic68,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic69,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic70, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic71,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic72,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic73,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic74,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic75,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic76,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic77,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic78,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic79,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_idlebasic80,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_action12 = {20, ssithra_c_frames_action12, ai_c_cycleend};


/*----------------------------------------------------------------------
  Ssithra swimforward - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_swim1 [] =
{
	FRAME_swimforward01,	ai_c_move, 16, 0, 0,  NULL, 16, ssithraCheckRipple,
	FRAME_swimforward02,	ai_c_move, 16, 0, 0,  NULL, 16, NULL,
	FRAME_swimforward03,	ai_c_move, 16, 0, 0,  NULL, 16, ssithraCheckRipple,
	FRAME_swimforward04,	ssithraSound, SND_SWIM, CHAN_BODY, 0,  NULL, 20, ssithra_check_namor,
	FRAME_swimforward05,	ai_c_move, 20, 0, 0,  NULL, 20, ssithraCheckRipple,
	FRAME_swimforward06,	ai_c_move, 20, 0, 0,  NULL, 20, ssithraCheckRipple,
	FRAME_swimforward07,	ai_c_move, 20, 0, 0,  NULL, 20, ssithraCheckRipple,
	FRAME_swimforward08,	ai_c_move, 24, 0, 0,  NULL, 24, ssithra_check_namor,
	FRAME_swimforward09,	ai_c_move, 24, 0, 0,  NULL, 24, ssithraCheckRipple,
	FRAME_swimforward10, 	ssithraSound, SND_SWIM, CHAN_BODY, 0,  NULL, 24, NULL,
	FRAME_swimforward11,	ai_c_move, 20, 0, 0,  NULL, 20, ssithraCheckRipple,
	FRAME_swimforward12,	ai_c_move, 20, 0, 0,  NULL, 20, ssithra_check_namor,
	FRAME_swimforward13,	ai_c_move, 20, 0, 0,  NULL, 20, ssithraCheckRipple,
	FRAME_swimforward14,	ai_c_move, 16, 0, 0,  NULL, 16, ssithraCheckRipple,
	FRAME_swimforward15,	ai_c_move, 16, 0, 0,  NULL, 16, ssithraCheckRipple,
};
animmove_t ssithra_c_move_swim1 = {15, ssithra_c_frames_swim1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra water_death -  
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_death1 [] =
{
	FRAME_water_death1,	ssithraSound, SND_DIE, CHAN_VOICE, 0,  NULL, 0, NULL,
	FRAME_water_death2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death4,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death5,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death7,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death8,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death9,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death10, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death11,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death12,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death16,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death17,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death18,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death19,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death20, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death21,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death22,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death23,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death24,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death25,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death26,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death27,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death28,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death29,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death30, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death31,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death32,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death33,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death34,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death35,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death36,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death37,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death38,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death39,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death40, 	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death41,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death42,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death43,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death44,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death45,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death46,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death47,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death48,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_death49,	ai_c_move, 0, 0, 0,  NULL, 0, NULL
};
animmove_t ssithra_c_move_death1 = {49, ssithra_c_frames_death1, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra water_idle - looking around and standing
-----------------------------------------------------------------------*/
animframe_t ssithra_frames_idle4 [] =
{//water idle sound?
	FRAME_water_idle1,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle3,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle4,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle5,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle6,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle7,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle8,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle9,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle10, ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle11,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle12,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle13,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle14,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle15,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle16,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle17,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle18,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle19,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_idle20, ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_move_idle4 = {20, ssithra_frames_idle4, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra water_pain_a - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_pain2 [] =
{
	FRAME_water_pain_a1,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_pain_a2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_pain2 = {2, ssithra_c_frames_pain2, ai_c_cycleend};

/*----------------------------------------------------------------------
  Ssithra water_pain_b - 
-----------------------------------------------------------------------*/
animframe_t ssithra_c_frames_pain3 [] =
{
	FRAME_water_pain_b1,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
	FRAME_water_pain_b2,	ai_c_move, 0, 0, 0,  NULL, 0, NULL,
};
animmove_t ssithra_c_move_pain3 = {2, ssithra_c_frames_pain3, ai_c_cycleend};


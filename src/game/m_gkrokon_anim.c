//==============================================================================
//
// m_gkrokon_anim.c
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

#include "m_gkrokon_anim.h"
#include "m_gkrokon.h"
#include "random.h"

// ****************************************************************************
// Stand1 - Laid down, resting, still on the floor.
// ****************************************************************************

animframe_t GkrokonFramesStand1[]=
{
	FRAME_bwait1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	beetle_idle_sound,
	FRAME_bwait6,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait7,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait8,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait9,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait10,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait11,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait12,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait13,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait14,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
};
animmove_t GkrokonMoveStand1={14,GkrokonFramesStand1, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// Stand2 - Getting up off the floor.
// ****************************************************************************

animframe_t GkrokonFramesStand2[]=
{
	FRAME_birth1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,	
	FRAME_birth6,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth7,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth8,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth9,	NULL, 0, 0, 0, beetle_ai_stand,	0,	beetle_idle_sound,	
	FRAME_birth10,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth11,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth12,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
};

animmove_t GkrokonMoveStand2={12,GkrokonFramesStand2, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// Stand3 - Standing fairly still, waiting.
// ****************************************************************************

animframe_t GkrokonFramesStand3[]=
{
	FRAME_wait1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_wait2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_wait3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_wait4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	beetle_idle_sound,
	FRAME_wait5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_wait6,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_wait7,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_wait8,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
};

animmove_t GkrokonMoveStand3={8,GkrokonFramesStand3, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// Stand4 - Settling down onto the floor.
// ****************************************************************************

animframe_t GkrokonFramesStand4[]=
{
	FRAME_birth5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,	
	FRAME_birth3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_birth2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	beetle_idle_sound,
	FRAME_birth1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
};

animmove_t GkrokonMoveStand4={5,GkrokonFramesStand4, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// Crouch1 - Crouched down on the floor (stalking enemy).
// ****************************************************************************

animframe_t GkrokonFramesCrouch1[]=
{
	FRAME_bwait1,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait2,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait3,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait4,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait5,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait6,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait7,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait8,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait9,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait10,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait11,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait12,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait13,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
	FRAME_bwait14,	NULL, 0, 0, 0, beetle_ai_stand,	0,	NULL,
};

animmove_t GkrokonMoveCrouch1={14,GkrokonFramesCrouch1, GkrokonPause};//GkrokonOrderCrouch};

// ****************************************************************************
// Crouch2 - Getting up off the floor from crouching (stalking enemy).
// ****************************************************************************

animframe_t GkrokonFramesCrouch2[]=
{
	FRAME_birth1,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth2,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth3,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth4,	NULL, 0, 0, 0, 	NULL,	0,	NULL,
};

animmove_t GkrokonMoveCrouch2={4,GkrokonFramesCrouch2, beetle_to_stand};//GkrokonOrderCrouch};

// ****************************************************************************
// Crouch3 - Settling down into crouching position (stalking enemy).
// ****************************************************************************

animframe_t GkrokonFramesCrouch3[]=
{
	FRAME_birth4,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth3,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth2,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth1,	NULL, 0, 0, 0, NULL,	0,	NULL,
};

animmove_t GkrokonMoveCrouch3={4,GkrokonFramesCrouch3, beetle_to_crouch};//GkrokonOrderCrouch};

// ****************************************************************************
// Walk1 - A leisurely ambling gait.
// ****************************************************************************

animframe_t GkrokonFramesWalk1[]=
{
	FRAME_walkB1,	NULL, 0, 0, 0, beetle_ai_stand,	6,	NULL,
	FRAME_walkB2,	NULL, 0, 0, 0, beetle_ai_stand,	7,	NULL,
	FRAME_walkB3,	NULL, 0, 0, 0, beetle_ai_stand,	5,	NULL,
	FRAME_walkB4,	NULL, 0, 0, 0, beetle_ai_stand,	8,	NULL,
	FRAME_walkB5,	NULL, 0, 0, 0, beetle_ai_stand,	7,	NULL,
	FRAME_walkB6,	NULL, 0, 0, 0, beetle_ai_stand,	6,	gkrokonRandomWalkSound,
	FRAME_walkB7,	NULL, 0, 0, 0, beetle_ai_stand,	5,	NULL,
	FRAME_walkB8,	NULL, 0, 0, 0, beetle_ai_stand,	8,	NULL,
};

animmove_t GkrokonMoveWalk1={8,GkrokonFramesWalk1,GkrokonPause};

// ****************************************************************************
// Run1 - A galloping run.
// ****************************************************************************

animframe_t GkrokonFramesRun1[]=
{
	FRAME_gallop1,	NULL, 0, 0, 0, ai_run,	16,	NULL,
	FRAME_gallop2,	NULL, 0, 0, 0, ai_run,	24,	NULL,
	FRAME_gallop3,	NULL, 0, 0, 0, ai_run,	22,	NULL,
	FRAME_gallop4,	NULL, 0, 0, 0, ai_run,	18,	gkrokonRandomWalkSound,
	FRAME_gallop5,	NULL, 0, 0, 0, ai_run,	16,	NULL,
	FRAME_gallop6,	NULL, 0, 0, 0, ai_run,	24,	NULL,
};

animmove_t GkrokonMoveRun1={6,GkrokonFramesRun1, GkrokonPause};//GkrokonOrderRun};

// ****************************************************************************
// Run2 - A skittering, insectlike run.
// ****************************************************************************

animframe_t GkrokonFramesRun2[]=
{
	FRAME_skittr1,	NULL, 0, 0, 0, ai_run,	12,	NULL,
	FRAME_skittr2,	NULL, 0, 0, 0, ai_run,	12,	NULL,
	FRAME_skittr3,	NULL, 0, 0, 0, ai_run,	12,	gkrokonRandomWalkSound,
	FRAME_skittr4,	NULL, 0, 0, 0, ai_run,	12,	NULL,
};

animmove_t GkrokonMoveRun2={4,GkrokonFramesRun2, GkrokonPause};//GkrokonOrderRun};

animframe_t GkrokonFramesRunAway[]=
{
	FRAME_skittr4,	gkrokonSound, CHAN_VOICE, SND_FLEE, ATTN_NORM, ai_run,	-14,	NULL,
	FRAME_skittr3,	NULL, 0, 0, 0, ai_run,	-16,	NULL,
	FRAME_skittr2,	NULL, 0, 0, 0, ai_run,	-14,	gkrokonRandomWalkSound,
	FRAME_skittr1,	NULL, 0, 0, 0, ai_run,	-12,	NULL,
	FRAME_skittr4,	NULL, 0, 0, 0, ai_run,	-14,	NULL,
	FRAME_skittr3,	NULL, 0, 0, 0, ai_run,	-16,	NULL,
	FRAME_skittr2,	NULL, 0, 0, 0, ai_run,	-14,	gkrokonRandomWalkSound,
	FRAME_skittr1,	NULL, 0, 0, 0, ai_run,	-12,	NULL,
};

animmove_t GkrokonMoveRunAway={8,GkrokonFramesRunAway, GkrokonPause};//GkrokonOrderRun};

// ****************************************************************************
// Jump1 - Jumping.
// ****************************************************************************

animframe_t GkrokonFramesJump1[]=
{
	FRAME_jump1,	gkrokonSound, CHAN_VOICE, SND_ANGRY, ATTN_NORM, NULL,	0,	NULL,
	FRAME_jump2,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump4,	NULL, 0, 0, 0, NULL,	0,	/*GkrokonJump*/NULL,
	FRAME_jump6,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump10,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump12,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump14,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump16,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump18,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump20,	NULL, 0, 0, 0, NULL,	0,	/*GkrokonBite*/NULL,
	FRAME_jump22,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump23,	NULL, 0, 0, 0, NULL,	0,	NULL,
};

animmove_t GkrokonMoveJump1={12,GkrokonFramesJump1, GkrokonPause}; //GkrokonOrderRun};

animframe_t GkrokonFramesForcedJump[]=
{
	FRAME_jump1,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump2,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump4,	NULL, 0, 0, 0, NULL,	0,	/*GkrokonJump*/NULL,
	FRAME_jump6,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump10,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump12,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump14,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump16,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump18,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump20,	NULL, 0, 0, 0, NULL,	0,	/*GkrokonBite*/NULL,
	FRAME_jump22,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_jump23,	NULL, 0, 0, 0, NULL,	0,	NULL,
};
animmove_t GkrokonMoveForcedJump={12,GkrokonFramesForcedJump, GkrokonPause}; //GkrokonOrderRun};

// ****************************************************************************
// GkrokonFramesMeleeAttack1 - A bite attack on my enemy.
// ****************************************************************************

animframe_t GkrokonFramesMeleeAttack1[]=
{
	FRAME_latack1,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_latack2,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_latack3,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_latack4,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_latack5,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_latack6,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_latack7,	NULL, 0, 0, 0, GkrokonBite,	0, NULL,
	FRAME_latack8,	NULL, 0, 0, 0, NULL,	0,	NULL,
};

animmove_t GkrokonMoveMeleeAttack1={8,GkrokonFramesMeleeAttack1, GkrokonPause};//GkrokonOrderStand};

animframe_t GkrokonFramesMeleeAttack2[]=
{
	FRAME_ratack1,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_ratack2,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_ratack3,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_ratack4,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_ratack5,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_ratack6,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_ratack7,	NULL, 0, 0, 0, GkrokonBite,	1, NULL,
	FRAME_ratack8,	NULL, 0, 0, 0, NULL,	0,	NULL,
};

animmove_t GkrokonMoveMeleeAttack2={8,GkrokonFramesMeleeAttack2, GkrokonPause};//GkrokonOrderStand};

// ****************************************************************************
// GkrokonFramesMissileAttack1 - Firing spoo-goo from spoo launcher.
// ****************************************************************************

animframe_t GkrokonFramesMissileAttack1[]=
{
	FRAME_spoo1,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
	FRAME_spoo2,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
	FRAME_spoo3,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
	FRAME_spoo4,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
	FRAME_spoo5,	NULL, 0, 0, 0, ai_charge,	0,	GkrokonSpoo,
};
animmove_t GkrokonMoveMissileAttack1={5,GkrokonFramesMissileAttack1, GkrokonPause};//GkrokonOrderRun};

animframe_t GkrokonFramesMissileAttack2[]=
{
	FRAME_spoo1,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
	FRAME_spoo2,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
	FRAME_spoo3,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
	FRAME_spoo4,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
	FRAME_spoo5,	NULL, 0, 0, 0, ai_charge,	0,	NULL,
};

animmove_t GkrokonMoveMissileAttack2={5,GkrokonFramesMissileAttack2, GkrokonPause};//GkrokonOrderRun};

// ****************************************************************************
// GkrokonFramesEat1 - Going from ready to eating.
// ****************************************************************************

animframe_t GkrokonFramesEat1[]=
{
	FRAME_eat1,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_eat2,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_eat3,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_eat4,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
};

animmove_t GkrokonMoveEat1={4,GkrokonFramesEat1, GkrokonPause};//GkrokonOrderEat};

// ****************************************************************************
// GkrokonFramesEat2 - The eat cycle.
// ****************************************************************************

animframe_t GkrokonFramesEat2[]=
{
	FRAME_eat5,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_eat6,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_eat7,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_eat8,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_eat9,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
};

animmove_t GkrokonMoveEat2={5,GkrokonFramesEat2, GkrokonPause};//GkrokonOrderEat};

// ****************************************************************************
// GkrokonFramesEat3 - Going from eating to ready.
// ****************************************************************************

animframe_t GkrokonFramesEat3[]=
{
	FRAME_EATTRANS1,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_EATTRANS2,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_EATTRANS3,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
	FRAME_EATTRANS4,	NULL, 0, 0, 0, ai_eat,	0,	NULL,
};

animmove_t GkrokonMoveEat3={4,GkrokonFramesEat3, GkrokonPause};//GkrokonOrderEat};

// ****************************************************************************
// Pain1 - 
// ****************************************************************************

animframe_t GkrokonFramesPain1[]=
{
	FRAME_pain1,	NULL, 0, 0, 0, ai_charge,	-1,	NULL,
	FRAME_pain2,	NULL, 0, 0, 0, ai_charge,	-1,	NULL,
	FRAME_pain3,	NULL, 0, 0, 0, ai_charge,	-1,	NULL,
	FRAME_pain4,	NULL, 0, 0, 0, ai_charge,	-1,	NULL,
	FRAME_pain5,	NULL, 0, 0, 0, ai_charge,	-1,	NULL,
	FRAME_pain6,	NULL, 0, 0, 0, ai_charge,	-1,	NULL,
	FRAME_pain7,	NULL, 0, 0, 0, ai_charge,	-1,	NULL,
	FRAME_pain8,	NULL, 0, 0, 0, ai_charge,	-1,	NULL,
};

animmove_t GkrokonMovePain1={8,GkrokonFramesPain1, GkrokonPause};//GkrokonOrderRun};

// ****************************************************************************
// Death1 - 
// ****************************************************************************

/*
animframe_t GkrokonFramesDeath_hold[]=
{
	FRAME_deathb19,	NULL, 0, 0, 0, NULL,	0,	NULL,
};
animmove_t GkrokonMoveDeath_hold={1,GkrokonFramesDeath_hold,NULL};

*/
  animframe_t GkrokonFramesDeath1[]=
{
	FRAME_death1,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death2,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death3,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death4,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death5,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death6,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death7,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death8,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death9,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death10,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death11,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death12,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death13,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death14,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death15,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_death16,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death17,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death18,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death19,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death20,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death21,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death22,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death23,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death24,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death25,	NULL, 0, 0, 0, ai_move,	2,	NULL,
	FRAME_death26,	NULL, 0, 0, 0, ai_move,	2,	NULL,
};
animmove_t GkrokonMoveDeath1={26,GkrokonFramesDeath1,GkrokonDead};

animframe_t GkrokonStartHop[]=
{
	FRAME_birth5,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth4,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth5,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth6,	NULL, 0, 0, 0, NULL,	0,	/*gkrokon_hopdown*/NULL,
	FRAME_birth7,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth8,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth9,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth10,	NULL, 0, 0, 0, NULL,	0,	gkrokonRandomWalkSound,
	FRAME_birth11,	NULL, 0, 0, 0, NULL,	0,	NULL,
	FRAME_birth12,	NULL, 0, 0, 0, NULL,	0,	/*gkrokon_donehop*/NULL,
};

animmove_t GkrokonMoveHop1={10,GkrokonStartHop,GkrokonPause};

animframe_t GkrokonFramesDelay[]=
{
	FRAME_bwait1,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait2,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait3,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait4,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait5,	gkrokonSound, CHAN_VOICE, SND_IDLE1, ATTN_NORM, NULL,	0,	GkrokonPause,
	FRAME_bwait6,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait7,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait8,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait9,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait10,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait11,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait12,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait13,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
	FRAME_bwait14,	NULL, 0, 0, 0, NULL, 0,	GkrokonPause,
};
animmove_t GkrokonMoveDelay={14,GkrokonFramesDelay, GkrokonPause};//GkrokonOrderStand};

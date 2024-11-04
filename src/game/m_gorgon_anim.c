//==============================================================================
//
// m_gorgon_anim.c
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

#include "m_gorgon_anim.h"
#include "m_gorgon.h"

#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "Random.h"
#include "vector.h"
#include "m_stats.h"


void gorgon_land(edict_t *self);
void M_movetoside (edict_t *self,float yaw, float dist);
void ai_charge2 (edict_t *self, float dist);
float MG_FaceGoal (edict_t *self, qboolean doturn);
void ai_moveright (edict_t *self, float dist);

#define GORGON_STEPS		20//number of steps for whole swerve
#define GORGON_STEPS_INT1	(GORGON_STEPS/4)
#define GORGON_STEPS_INT2	(GORGON_STEPS/2)
#define GORGON_STEPS_INT3	(GORGON_STEPS - GORGON_STEPS_INT1)
#define GORGON_STEPS_INT4	GORGON_STEPS

#define GORGON_SWERVE		20//degree of swerve
#define GORGON_SWERVE_INT1	(GORGON_SWERVE/4)
#define GORGON_SWERVE_INT2	(GORGON_SWERVE/2)
#define GORGON_SWERVE_INT3	(GORGON_SWERVE - GORGON_SWERVE_INT1)
#define GORGON_SWERVE_INT4	GORGON_SWERVE

#define GORGON_SWERVE_MULT	(GORGON_SWERVE/GORGON_STEPS_INT1)

void gorgon_ai_run (edict_t *self, float dist)
{
	float offset;

	MG_FaceGoal(self, false);

	if(self->monsterinfo.idle_time != -1 && self->monsterinfo.idle_time < level.time)
	{
		self->dmg++;
		if(self->dmg<GORGON_STEPS_INT1)//<5
			offset = (float)(self->dmg) * GORGON_SWERVE_MULT;//0->5 * 4 = 0->20
		else if(self->dmg<GORGON_STEPS_INT2)
			offset = GORGON_SWERVE - (((float)(self->dmg) - GORGON_SWERVE_INT1) * GORGON_SWERVE_MULT);//20 - ((0->10 - 5) * 4) = 20->0
		else if(self->dmg<GORGON_STEPS_INT3)
			offset = -(((float)(self->dmg) - GORGON_SWERVE_INT2) * GORGON_SWERVE_MULT);
		else if(self->dmg<GORGON_STEPS_INT4)
			offset = -(GORGON_SWERVE - (((float)(self->dmg) - GORGON_SWERVE_INT3) * GORGON_SWERVE_MULT));
		else
			self->dmg = 0;

		if(self->dmg)
		{
			self->ideal_yaw = anglemod(self->ideal_yaw + offset);
		}
	}

	if(self->spawnflags & MSF_GORGON_SPEEDY)
		ai_run(self, dist*1.25);
	else
		ai_run(self, dist*0.75);
}

/*----------------------------------------------------------------------
	STAND3 -   Gorgon Standing and looking right
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_stand3 [] =
{
	FRAME_painc1,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_painc2,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_painc3,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_painc4,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_painc5,	NULL, 0, 0, 0, ai_stand, 0, NULL
};
animmove_t gorgon_move_stand3 = {5, gorgon_frames_stand3, gorgonCheckMood};

/*----------------------------------------------------------------------
	STAND2 -   Gorgon Standing and looking left
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_stand2 [] =
{
	FRAME_painb1,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_painb2,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_painb3,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_painb4,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_painb5,	NULL, 0, 0, 0, ai_stand, 0, NULL
};
animmove_t gorgon_move_stand2 = {5, gorgon_frames_stand2, gorgonCheckMood};


/*----------------------------------------------------------------------
	STAND1 -   Gorgon Standing and wagging it's tail
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_stand1 [] =
{
	FRAME_wait1,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait2,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait3,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait4,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait5,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait6,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait7,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait8,	NULL, 0, 0, 0, ai_stand, 0, NULL
};
animmove_t gorgon_move_stand1 = {8, gorgon_frames_stand1, gorgonCheckMood};

/*----------------------------------------------------------------------
	STAND1 -   Gorgon Standing and wagging it's tail
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_stand4 [] =
{
	FRAME_wait1,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait2,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait3,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait4,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait5,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait6,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait7,	NULL, 0, 0, 0, ai_stand, 0, NULL,
	FRAME_wait8,	NULL, 0, 0, 0, ai_stand, 0, NULL
};
animmove_t gorgon_move_stand4 = {8, gorgon_frames_stand4, gorgonCheckMood};

/*----------------------------------------------------------------------
  Gorgon Turning right while running
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_walk3 [] =
{ 
	FRAME_wlkrt1, NULL, 0, 0, 0, ai_walk, 8,  gorgon_footstep,
	FRAME_wlkrt2, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt3, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt4, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt5, NULL, 0, 0, 0, ai_walk, 8,  gorgon_footstep,
	FRAME_wlkrt6, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt7, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt8, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt9, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt10, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt11, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlkrt12, NULL, 0, 0, 0, ai_walk, 8,  NULL
};

animmove_t gorgon_move_walk3 = {12, gorgon_frames_walk3, gorgonCheckMood};


/*----------------------------------------------------------------------
  Gorgon Turning left while running
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_walk2 [] =
{ 
	FRAME_wlklft1,NULL, 0, 0, 0, ai_walk, 8,  gorgon_footstep,
	FRAME_wlklft2,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft3,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft4,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft5,NULL, 0, 0, 0, ai_walk, 8,  gorgon_footstep,
	FRAME_wlklft6,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft7,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft8,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft9,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft10,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft11,NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_wlklft12,NULL, 0, 0, 0, ai_walk, 8,  NULL
};

animmove_t gorgon_move_walk2 = {12, gorgon_frames_walk2, gorgonCheckMood};

/*----------------------------------------------------------------------
  Gorgon Walking
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_walk [] =
{
	FRAME_walk1,  NULL, 0, 0, 0, ai_walk, 7,  gorgon_footstep, 
	FRAME_walk2,  NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_walk3,  NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_walk4,  NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_walk5,  NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_walk6,  NULL, 0, 0, 0, ai_walk, 6,  gorgon_footstep,
	FRAME_walk7,  NULL, 0, 0, 0, ai_walk, 7,  NULL,
	FRAME_walk8,  NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_walk9,  NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_walk10, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_walk11, NULL, 0, 0, 0, ai_walk, 8,  NULL,
	FRAME_walk12, NULL, 0, 0, 0, ai_walk, 6,  gorgon_growl
};

animmove_t gorgon_move_walk = {12, gorgon_frames_walk, gorgonCheckMood};

/*----------------------------------------------------------------------
  Gorgon  Melee while Running
-----------------------------------------------------------------------*/
void gorgon_melee5check (edict_t *self)
{
	if (self->monsterinfo.currframeindex == 0)
	{
		gorgon_footstep(self);
	}
	else if (self->monsterinfo.currframeindex == 4)
		gorgon_footstep(self);
}


void gorgon_ai_charge2 (edict_t *self, float dist)
{
	if(!visible(self, self->enemy))
		return;

	ai_charge2(self, dist);
}

/*----------------------------------------------------------------------
	Forced Jump - jump from a buoy
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_fjump [] =
{      
	FRAME_jumpb1,NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_jumpb2,NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_jumpb3,NULL, 0, 0, 0, NULL, 0,  gorgon_growl,
	FRAME_jumpb4,NULL, 0, 0, 0, NULL, 0,  gorgonApplyJump,
	FRAME_jumpb5,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb6,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb7,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb8,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb9,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb10,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb11,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb12,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb13,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
	FRAME_jumpb14,NULL, 0, 0, 0, NULL, 0,  gorgon_check_landed,
};
animmove_t gorgon_move_fjump = {14, gorgon_frames_fjump, gorgon_go_inair};

/*----------------------------------------------------------------------
	land 1
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_land [] =
{      
	FRAME_jumpa15,NULL, 0, 0, 0, NULL, 0,  gorgon_land,
	FRAME_jumpa16,NULL, 0, 0, 0, NULL, 0,  gorgon_growl,
	FRAME_jumpa17,NULL, 0, 0, 0, NULL, 0,  NULL
};
animmove_t gorgon_move_land = {3, gorgon_frames_land, gorgonCheckMood};

/*----------------------------------------------------------------------
	land 2
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_land2 [] =
{      
	FRAME_jumpa15,NULL, 0, 0, 0, NULL, 0,  gorgon_land,
	FRAME_jumpa16,NULL, 0, 0, 0, NULL, 0,  gorgon_growl,
	FRAME_jumpa17,NULL, 0, 0, 0, NULL, 0,  NULL
};
animmove_t gorgon_move_land2 = {3, gorgon_frames_land2, gorgonCheckMood};

/*----------------------------------------------------------------------
	in air
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_inair [] =
{      
	FRAME_jumpa14,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
};
animmove_t gorgon_move_inair = {1, gorgon_frames_inair, NULL};

/*----------------------------------------------------------------------
	MELEE10 - jump up at player
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee10 [] =
{      
	FRAME_jumpa1,NULL, 0, 0, 0, gorgon_ai_charge2, 0,  NULL,
	FRAME_jumpa2,NULL, 0, 0, 0, gorgon_ai_charge2, 0,  NULL,
	FRAME_jumpa3,NULL, 0, 0, 0, gorgon_ai_charge2, 0,  gorgon_growl,
	FRAME_jumpa4,NULL, 0, 0, 0, NULL, 0,  gorgon_jump,
	FRAME_jumpa5,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa6,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa7,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa8,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa9,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa10,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa11,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa12,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa13,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
	FRAME_jumpa14,NULL, 0, 0, 0, NULL, 0, gorgon_check_landed,
};
animmove_t gorgon_move_melee10 = {14, gorgon_frames_melee10, gorgon_go_inair};


/*----------------------------------------------------------------------
	MELEE9 - hop backward
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee9 [] =
{
	FRAME_hop1,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop2,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop3,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop4,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_hop,
	FRAME_hop5,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop6,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop7,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop8,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_hop,
	FRAME_hop9,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop10, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL
};
animmove_t gorgon_move_melee9 = {10, gorgon_frames_melee9, gorgonCheckMood};

/*----------------------------------------------------------------------
	MELEE8 - hop forward
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee8 [] =
{
	FRAME_hop1,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop2,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop3,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop4,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_hop,
	FRAME_hop5,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop6,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop7,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop8,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_hop,
	FRAME_hop9,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop10, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL
};
animmove_t gorgon_move_melee8 = {10, gorgon_frames_melee8, gorgonCheckMood};

/*----------------------------------------------------------------------
  MELEE7 - hop right
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee7 [] =
{
	FRAME_hop1,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop2,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop3,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop4,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_hop,
	FRAME_hop5,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop6,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop7,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop8,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_hop,
	FRAME_hop9,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop10, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL
};
animmove_t gorgon_move_melee7 = {10, gorgon_frames_melee7, gorgonCheckMood};

/*----------------------------------------------------------------------
  MELEE6 - hop left
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee6 [] =
{
	FRAME_hop1,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop2,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop3,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop4,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_hop,
	FRAME_hop5,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop6,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop7,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop8,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgon_hop,
	FRAME_hop9,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_hop10, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL
};
animmove_t gorgon_move_melee6 = {10, gorgon_frames_melee6, gorgonCheckMood};


/*----------------------------------------------------------------------
  Melee5 -  running attack
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee5 [] =
{ 
	FRAME_runatk1,  NULL, 0, 0, 0, ai_goal_charge, 20, gorgon_melee5check,
	FRAME_runatk2,	NULL, 0, 0, 0, ai_goal_charge, 20, NULL,
	FRAME_runatk3,	NULL, 0, 0, 0, ai_goal_charge, 22, NULL,
	FRAME_runatk4,	NULL, 0, 0, 0, ai_goal_charge, 21, gorgonbite,
	FRAME_runatk5,	NULL, 0, 0, 0, ai_goal_charge, 20, gorgon_melee5check,
	FRAME_runatk6,	NULL, 0, 0, 0, ai_goal_charge, 20, NULL,
	FRAME_runatk7,	NULL, 0, 0, 0, ai_goal_charge, 22, NULL,
	FRAME_runatk8,	NULL, 0, 0, 0, ai_goal_charge, 21, gorgonCheckMood
};


animmove_t gorgon_move_melee5 = {8, gorgon_frames_melee5, gorgonCheckMood};


/*----------------------------------------------------------------------
   Melee4 -   Gorgon Attack Pullback
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee4 [] =
{
	FRAME_atkc1, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atkc2, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atkc3, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atkc4, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL
};
animmove_t gorgon_move_melee4 = {4, gorgon_frames_melee4, gorgonCheckMood};


/*----------------------------------------------------------------------
   Melee3 -   Gorgon Attack Up
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee3 [] =
{
	FRAME_atkd1, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atkd2, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atkd3, NULL, 0, 0, 0, ai_goal_charge, 0,  gorgonbite,
	FRAME_atkd4, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL
};

animmove_t gorgon_move_melee3 = {4, gorgon_frames_melee3, gorgonCheckMood};


/*----------------------------------------------------------------------
   Melee2 -   Gorgon Attack Right
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee2 [] =
{
	FRAME_atkb1,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atkb2,  NULL, 0, 0, 0, ai_goal_charge, 0,  gorgonbite,
	FRAME_atkb3,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atkb4,  NULL, 0, 0, 0, ai_goal_charge, 0,  NULL
};

animmove_t gorgon_move_melee2 = {4, gorgon_frames_melee2, gorgonCheckMood};


/*----------------------------------------------------------------------
   Melee1 -   Gorgon Attack Left
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_melee1 [] =
{
	FRAME_atka1, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atka2, NULL, 0, 0, 0, ai_goal_charge, 0,  gorgonbite,
	FRAME_atka3, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL,
	FRAME_atka4, NULL, 0, 0, 0, ai_goal_charge, 0,  NULL
};

animmove_t gorgon_move_melee1 = {4, gorgon_frames_melee1, gorgonCheckMood};


/*----------------------------------------------------------------------
  RUN3 - Turning right while running
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_run3 [] =
{ 
	FRAME_wlkrt1, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgon_footstep,
	FRAME_wlkrt2, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt3, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt4, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt5, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt6, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt7, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt8, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt9, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt10, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt11, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckSlip,
	FRAME_wlkrt12, NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgon_growl
};

animmove_t gorgon_move_run3 = {12, gorgon_frames_run3, gorgonCheckMood};


/*----------------------------------------------------------------------
  RUN2 - Turning left while running
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_run2 [] =
{ 
	FRAME_wlklft1,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgon_footstep,
	FRAME_wlklft2,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft3,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft4,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft5,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgon_footstep,
	FRAME_wlklft6,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft7,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft8,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft9,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft10,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft11,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgonCheckMood,
	FRAME_wlklft12,NULL, 0, 0, 0, gorgon_ai_run, 16,  gorgon_growl
};

animmove_t gorgon_move_run2 = {12, gorgon_frames_run2, gorgonCheckMood};



/*----------------------------------------------------------------------
  Gorgon Running
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_run1 [] =
{ 
	FRAME_run1, NULL, 0, 0, 0, gorgon_ai_run, 30, gorgon_footstep,
	FRAME_run2,	NULL, 0, 0, 0, gorgon_ai_run, 31, gorgonCheckMood,
	FRAME_run3,	NULL, 0, 0, 0, gorgon_ai_run, 32, gorgonCheckMood,
	FRAME_run4,	NULL, 0, 0, 0, gorgon_ai_run, 34, gorgonCheckMood,
	FRAME_run5,	NULL, 0, 0, 0, gorgon_ai_run, 30, gorgon_footstep,
	FRAME_run6,	NULL, 0, 0, 0, gorgon_ai_run, 31, gorgonCheckMood,
	FRAME_run7,	NULL, 0, 0, 0, gorgon_ai_run, 32, gorgonCheckMood,
	FRAME_run8,	NULL, 0, 0, 0, gorgon_ai_run, 34, gorgon_growl
};

animmove_t gorgon_move_run1 = {8, gorgon_frames_run1, gorgonCheckMood};

/*----------------------------------------------------------------------
  Gorgon Pain1 - step back while bending head down
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_pain1 [] =
{
	FRAME_pain1, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_pain2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_pain3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_pain4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_pain5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_pain6, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_pain7, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_pain8, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_pain9, NULL, 0, 0, 0, NULL, 0,  NULL
};
animmove_t gorgon_move_pain1 = {9, gorgon_frames_pain1, gorgonCheckMood};


/*----------------------------------------------------------------------
  Gorgon Pain2 - bend head to the left
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_pain2 [] =
{
	FRAME_painb1, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_painb2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_painb3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_painb4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_painb5, NULL, 0, 0, 0, NULL, 0,  NULL
};
animmove_t gorgon_move_pain2 = {5, gorgon_frames_pain2, gorgonCheckMood};

/*----------------------------------------------------------------------
  Gorgon Pain3 - bend head to the right
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_pain3 [] =
{
	FRAME_painc1, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_painc2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_painc3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_painc4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_painc5, NULL, 0, 0, 0, NULL, 0,  NULL
};
animmove_t gorgon_move_pain3 = {5, gorgon_frames_pain3, gorgonCheckMood};





void gorgon_smoke(edict_t *self)
{
	//CL_SmokeAndFlash(self->s.origin);
	gorgon_land(self);
	//gi.sound(self, CHAN_WEAPON, sounds[SND_GROWL3], 1, ATTN_NORM, 0);
}

void gorgondeath1_fall (edict_t *self)
{
//	vec3_t		forward,right, up,hold_angles;

//	if (self->s.frame == FRAME_deatha11)
//		carnasaur_noshadow(self);

	if (self->s.frame == FRAME_deatha13)
		gorgon_smoke(self);

	M_movetoside(self,flrand(180, 345), flrand(0, -8));
}

animframe_t gorgon_frames_die1 [] =
{
	FRAME_deatha1, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha2, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha3, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha4, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha5, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha6, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha7, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha8, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha9, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_deatha10, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,
	FRAME_deatha11, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha12, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha13, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha14, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha15, NULL, 0, 0, 0, ai_move, 0,  NULL,
	FRAME_deatha16, NULL, 0, 0, 0, ai_move, 0,  NULL,
	FRAME_deatha17, NULL, 0, 0, 0, ai_move, 0,  NULL,  //carnasaur_drip,
	FRAME_deatha18, NULL, 0, 0, 0, ai_move, 0,  NULL,
	FRAME_deatha19, NULL, 0, 0, 0, ai_move, 0,  NULL
};

animmove_t gorgon_move_die1 = {19, gorgon_frames_die1, gorgon_dead};

/*----------------------------------------------------------------------
  Gorgon Death2 - fly backwards and twitch
-----------------------------------------------------------------------*/
void gorgon_starttwitch(edict_t *self)
{
	self->monsterinfo.thinkinc = MONSTER_THINK_INC;//FRAMETIME;
}

void gorgon_nexttwitch(edict_t *self)
{
 	self->monsterinfo.thinkinc = FRAMETIME * 4  + flrand(0.0, 4.0);
}

animframe_t gorgon_frames_death2twitch [] =
{
	FRAME_twitch,  NULL, 0, 0, 0, NULL, 0,  gorgon_starttwitch,
	FRAME_twitch_1,NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_twitch_2,NULL, 0, 0, 0, NULL, 0,  gorgon_nexttwitch
};

animmove_t gorgon_move_death2twitch = {3, gorgon_frames_death2twitch, NULL};

void gorgon_death2twitch (edict_t *self)
{
	VectorSet(self->mins,-24,-24,0);
	VectorSet(self->maxs,24,24,18);
	VectorScale(self->mins, self->s.scale, self->mins);
	VectorScale(self->maxs, self->s.scale, self->maxs);
	self->monsterinfo.currentmove = &gorgon_move_death2twitch;
	self->monsterinfo.nextframeindex = 0;
}


animframe_t gorgon_frames_death2slide [] =
{
	FRAME_slide1, NULL, 0, 0, 0, ai_move, -16,  NULL,
	FRAME_slide2, NULL, 0, 0, 0, ai_move, -14,  NULL,
	FRAME_slide3, NULL, 0, 0, 0, ai_move, -12,  NULL,
	FRAME_slide4, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_slide5, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_slide6, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_slide7, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_slide8, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_slide9, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_slide10, NULL, 0, 0, 0, ai_move, -8,  NULL,
	FRAME_slide11, NULL, 0, 0, 0, ai_move, -4,  NULL,
	FRAME_slide12, NULL, 0, 0, 0, ai_move, -2,  NULL,
	FRAME_slide13, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide14, NULL, 0, 0, 0, NULL, 0,  NULL,//gorgonChooseDeath,
	FRAME_slide15, NULL, 0, 0, 0, ai_moveright, -2,  NULL,
	FRAME_slide16, NULL, 0, 0, 0, ai_moveright, -4,  NULL,
	FRAME_slide17, NULL, 0, 0, 0, ai_moveright, -2,  NULL,
	FRAME_slide18, NULL, 0, 0, 0, ai_moveright, -6,  NULL,
	FRAME_slide19, NULL, 0, 0, 0, ai_moveright, -2,  NULL,
	FRAME_slide20, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide21, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide22, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide23, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide24, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide25, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide26, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide27, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide28, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide29, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide30, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_slide31, NULL, 0, 0, 0, NULL, 0,  NULL,
};
animmove_t gorgon_move_death2slide = {31, gorgon_frames_death2slide, gorgon_dead};

void gorgon_death2_slide (edict_t *self)
{

	self->monsterinfo.currentmove = &gorgon_move_death2slide;
	self->monsterinfo.nextframeindex = 0;
}

void gorgon_death2throw (edict_t *self)
{
	vec3_t		forward;

	AngleVectors (self->s.angles, forward, NULL, NULL);

	VectorScale (forward, -375, self->velocity);
	self->velocity[2] = 200;
//	self->movetype= MOVETYPE_BOUNCE;//elasticity?
}

void gorgon_death2 (edict_t *self)
{

//	self->movetype= MOVETYPE_STEP;
	self->velocity[2] = -120;
}

animframe_t gorgon_frames_die2 [] =
{
	FRAME_hit1, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_hit3, NULL, 0, 0, 0, NULL, 0,  gorgon_death2throw,
	FRAME_hit5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_hit7, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_hit9, NULL, 0, 0, 0, NULL, 0,  gorgon_death2,
	FRAME_hit11, NULL, 0, 0, 0, NULL, 0,  NULL,
};

animmove_t gorgon_move_die2 = {6, gorgon_frames_die2, gorgon_death2_slide};


/*
CHOMP!
*/

/*----------------------------------------------------------------------
   Melee3 -   Gorgon Attack Up
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_catch [] =
{
	FRAME_jumpb5,NULL, 0, 0, 0, gorgon_gore_toy, 150, NULL,
	FRAME_jumpb6,NULL, 0, 0, 0, gorgon_gore_toy, 0, NULL,
	FRAME_jumpb7,NULL, 0, 0, 0, gorgon_gore_toy, 0, NULL,
	FRAME_jumpb8,NULL, 0, 0, 0, gorgon_gore_toy, 0, NULL,
	FRAME_jumpb9,NULL, 0, 0, 0, gorgon_gore_toy, 0, NULL,
	FRAME_jumpb10,NULL, 0, 0, 0, gorgon_gore_toy, -1, NULL,
	FRAME_jumpb11,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jumpb12,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jumpb13,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jumpb14,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jumpb15,NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jumpb16,NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_jumpb17,NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_jumpb18,NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_jumpb19,NULL, 0, 0, 0, NULL, 0,  gorgon_anger_sound,
};
animmove_t gorgon_move_catch = {15, gorgon_frames_catch, gorgon_done_gore};

/*----------------------------------------------------------------------
  Gorgon Eat3 - pull back, then bend back down to eat
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_miss [] =
{
	FRAME_eatinga4, NULL, 0, 0, 0, NULL, 0,  gorgon_miss_sound,
	FRAME_eatinga3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatinga2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatinga1, NULL, 0, 0, 0, NULL, 0,  NULL,
};
animmove_t gorgon_move_miss = {4, gorgon_frames_miss, gorgonCheckMood};

/*----------------------------------------------------------------------
  Shake and toss up toy

  Chance of throwing it to left or right, maybe carry away?
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_snatch [] =
{
	FRAME_eatingb5, gorgon_toy_ofs, 70, 0, 66,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatingb6, gorgon_toy_ofs, 68, -44, 66,  NULL, 0,  NULL,
	FRAME_eatingb7, gorgon_toy_ofs, 56, -64, 66,  NULL, 0,  NULL,
	FRAME_eatingb8, gorgon_toy_ofs, 60, -56, 66,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatingb9, gorgon_toy_ofs, 68, -24, 66,  NULL, 0,  NULL,
	FRAME_eatingb10, gorgon_toy_ofs, 72, 40, 66,  NULL, 0,  NULL,
	FRAME_eatingb11, gorgon_toy_ofs, 60, 70, 66,  NULL, 0,  NULL,
	FRAME_eatingb12, gorgon_toy_ofs, 70, 48, 66,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatingb13, gorgon_toy_ofs, 74, -32, 66,  NULL, 0,  NULL,
	FRAME_eatingb14, gorgon_toy_ofs, 64, -70, 66,  NULL, 0,  NULL,
	FRAME_eatingb15, gorgon_toy_ofs, 40, -72, 66,  NULL, 0,  NULL,
	FRAME_eatingb16, gorgon_toy_ofs, 42, -70, 66,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatingb17, gorgon_toy_ofs, 60, -60, 62,  NULL, 0,  NULL,
	FRAME_eatingb18, gorgon_toy_ofs, 66, -40, 56,  NULL, 0,  NULL,
	FRAME_eatingb19, gorgon_toy_ofs, 72, -10, 32,  NULL, 0,  NULL,
	FRAME_eatingb20, gorgon_toy_ofs, 62, 16, 12,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatingb21, gorgon_toy_ofs, 56, 32, 0,  NULL, 0,  NULL,
	FRAME_eatinga4, gorgon_toy_ofs, 48, 16, 64,  NULL, 0,  NULL,
	FRAME_eatinga3, gorgon_toy_ofs, 50, 14, 96,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatinga2, gorgon_toy_ofs, 80, 8, 80,  NULL, 0,  NULL,
	FRAME_eatinga1, gorgon_toy_ofs, 96, 0, 76,  NULL, 0,  NULL,
	FRAME_jumpa1, gorgon_toy_ofs, 100, 8, 78,  NULL, 0,  gorgon_anger_sound,
	FRAME_jumpa2, gorgon_toy_ofs, 90, 6, 64,  NULL, 0,  NULL,
	FRAME_jumpa3, gorgon_toy_ofs, 76, 4, 48,  NULL, 0,  NULL,
	FRAME_jumpa4, gorgon_toy_ofs, 96, 6, 140,  NULL, 0,  gorgon_anger_sound,
	FRAME_jumpa5, gorgon_toy_ofs, 90, 6, 208,  NULL, 0, gorgon_throw_toy,
	FRAME_jumpb4, NULL, 0, 0, 0, NULL, 0,  NULL,
};
animmove_t gorgon_move_snatch = {27, gorgon_frames_snatch, gorgon_ready_catch};

/*----------------------------------------------------------------------
  Gorgon Eat3 - pull back, then bend back down to eat
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_readycatch [] =
{
	FRAME_jumpb4, NULL, 0, 0, 0, NULL, 0,  NULL,
};
animmove_t gorgon_move_readycatch = {1, gorgon_frames_readycatch, gorgon_ready_catch};

/*----------------------------------------------------------------------
  Gorgon 
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_snatchhi [] =
{
	FRAME_atkd1, NULL, 0, 0, 0, gorgon_ai_charge2, 10,  NULL,
	FRAME_atkd2, NULL, 0, 0, 0, gorgon_ai_charge2, 10,  NULL,
	FRAME_atkd3, gorgon_check_snatch, 96, 0, 56, NULL, 0,  NULL,
	FRAME_atkd4, gorgon_toy_ofs, 96, 16, 160, NULL, 0,  NULL
};
animmove_t gorgon_move_snatchhi = {4, gorgon_frames_snatchhi, gorgon_go_snatch};

/*----------------------------------------------------------------------
  Gorgon 
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_snatchlow [] =
{
	FRAME_eatinga1, NULL, 0, 0, 0, gorgon_ai_charge2, 10,  NULL,
	FRAME_eatinga2, NULL, 0, 0, 0, gorgon_ai_charge2, 10,  NULL,
	FRAME_eatinga3, NULL, 0, 0, 0, gorgon_ai_charge2, 10,  NULL,
	FRAME_eatinga4, NULL, 0, 0, 0, gorgon_ai_charge2, 10,  NULL,
	FRAME_eatinga5, gorgon_check_snatch, 64, 0, -48, NULL, 0,  NULL,
	FRAME_eatingb1, gorgon_toy_ofs, 48, -32, 0, NULL, 0,  gorgon_anger_sound,
	FRAME_eatingb2, gorgon_toy_ofs, 48, 10, 10,  NULL, 0,  NULL,
	FRAME_eatingb3, gorgon_toy_ofs, 48, 0, 20,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatingb4, gorgon_toy_ofs, 56, 0, 24,  NULL, 0,  NULL,
};
animmove_t gorgon_move_snatchlow = {9, gorgon_frames_snatchlow, gorgon_go_snatch};



/*----------------
?
-----------------------*/
animframe_t gorgon_frames_slip [] =
{
	FRAME_deatha1, NULL, 0, 0, 0, gorgonSlide, -200,  NULL,
	FRAME_deatha2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_deatha3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_deatha4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_deatha5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_deatha6, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_deatha7, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_deatha8, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_deatha9, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_deatha10, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,
	FRAME_deatha11, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha12, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha13, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha14, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha15, NULL, 0, 0, 0, gorgonRoll, -60,  NULL,
	FRAME_deatha16, NULL, 0, 0, 0, gorgonRoll, -120,  NULL,
	FRAME_deatha17, NULL, 0, 0, 0, gorgonRoll, -180,  NULL,  //carnasaur_drip,
	FRAME_deatha18, NULL, 0, 0, 0, gorgonRoll, -240,  NULL,
	FRAME_deatha19, NULL, 0, 0, 0, gorgonRoll, -300,  gorgonLerpOff,
	FRAME_eatingb1, NULL, 0, 0, 0, gorgonRoll, 0,  NULL,
	FRAME_eatingb3, NULL, 0, 0, 0, NULL, 0,  gorgonLerpOn,
	FRAME_eatingb4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb6, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb7, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb8, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb10, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb11, NULL, 0, 0, 0, NULL, 0,  NULL,  // carnasaur_fall,
	FRAME_eatingb13, NULL, 0, 0, 0, NULL, 0,  NULL,  // carnasaur_fall,
	FRAME_eatingb15, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb18, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb21, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatinga4, NULL, 0, 0, 0,  NULL, 0,  NULL,
	FRAME_eatinga3, NULL, 0, 0, 0,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatinga2, NULL, 0, 0, 0,  NULL, 0,  NULL,
	FRAME_eatinga1, NULL, 0, 0, 0,  gorgonSlide, 0,  NULL,
};
animmove_t gorgon_move_slip = {36, gorgon_frames_slip, gorgonCheckMood};

/*----------------
?
-----------------------*/
animframe_t gorgon_frames_slip_pain [] =
{
	FRAME_deatha11, NULL, 0, 0, 0, gorgonSlide, -200,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha12, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha13, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_deatha14, NULL, 0, 0, 0, NULL, 0,  gorgondeath1_fall,  // carnasaur_fall,
	FRAME_rollover1, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_rollover2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_rollover3, NULL, 0, 0, 0, NULL, 0,  NULL,  //carnasaur_drip,
	FRAME_rollover4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_rollover5, NULL, 0, 0, 0, NULL, 0,  gorgonLerpOff,
	FRAME_eatingb1, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb3, NULL, 0, 0, 0, NULL, 0,  gorgonLerpOn,
	FRAME_eatingb4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb6, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb7, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb8, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb10, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb11, NULL, 0, 0, 0, NULL, 0,  NULL,  // carnasaur_fall,
	FRAME_eatingb13, NULL, 0, 0, 0, NULL, 0,  NULL,  // carnasaur_fall,
	FRAME_eatingb15, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb18, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatingb21, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_eatinga4, NULL, 0, 0, 0,  NULL, 0,  NULL,
	FRAME_eatinga3, NULL, 0, 0, 0,  NULL, 0,  gorgon_anger_sound,
	FRAME_eatinga2, NULL, 0, 0, 0,  NULL, 0,  NULL,
	FRAME_eatinga1, NULL, 0, 0, 0,  gorgonSlide, 0,  NULL,
};
animmove_t gorgon_move_slip_pain = {26, gorgon_frames_slip_pain, gorgonCheckMood};

/*----------------------------------------------------------------------
	delay
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_delay [] =
{
	FRAME_wait1,	NULL, 0, 0, 0, NULL, 0, gorgonCheckMood,
	FRAME_wait2,	NULL, 0, 0, 0, NULL, 0, gorgonCheckMood,
	FRAME_wait3,	NULL, 0, 0, 0, NULL, 0, gorgonCheckMood,
	FRAME_wait4,	NULL, 0, 0, 0, NULL, 0, gorgonCheckMood,
	FRAME_wait5,	NULL, 0, 0, 0, NULL, 0, gorgonCheckMood,
	FRAME_wait6,	NULL, 0, 0, 0, NULL, 0, gorgonCheckMood,
	FRAME_wait7,	NULL, 0, 0, 0, NULL, 0, gorgonCheckMood,
	FRAME_wait8,	NULL, 0, 0, 0, NULL, 0, gorgonCheckMood,
};
animmove_t gorgon_move_delay = {8, gorgon_frames_delay, gorgonCheckMood};

/*----------------------------------------------------------------------
  Gorgon roar- make noise, alert others
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_roar [] =
{
	FRAME_speak1, NULL, 0, 0, 0, NULL, 0,  gorgonRoar,
	FRAME_speak2, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak6, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak7, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak8, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak9, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak10, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak11, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak12, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak13, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak14, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak15, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak16, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak17, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak18, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak19, NULL, 0, 0, 0, NULL, 0,  NULL
};
animmove_t gorgon_move_roar = {19, gorgon_frames_roar, gorgonCheckMood};

/*----------------------------------------------------------------------
  Gorgon roar2- make noise in response to main roar
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_roar2 [] =
{
	FRAME_speak1, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak4, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak6, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak7, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak8, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak9, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak10, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak11, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak12, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak13, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak14, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak15, NULL, 0, 0, 0, NULL, 0,  gorgon_roar_sound,
	FRAME_speak16, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak17, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak18, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_speak19, NULL, 0, 0, 0, NULL, 0,  NULL
};
animmove_t gorgon_move_roar2 = {19, gorgon_frames_roar2, gorgonCheckMood};

//=====================================================

//SWIMMING

//=====================================================

/*----------------------------------------------------------------------
  Gorgon Swimming
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_to_swim [] =
{ //FIXME: add wake and bubbles and sploosh swim sounds
	FRAME_swim1,	NULL, 0, 0, 0, NULL, 0, gorgonCheckInWater,
	FRAME_swim2,	NULL, 0, 0, 0, NULL, 0, gorgonCheckInWater,
	FRAME_swim3,	NULL, 0, 0, 0, NULL, 0, gorgonCheckInWater,
	FRAME_swim4,	NULL, 0, 0, 0, NULL, 0, gorgonCheckInWater,
	FRAME_swim5,	NULL, 0, 0, 0, NULL, 0, gorgonCheckInWater,
};
animmove_t gorgon_move_to_swim = {5, gorgon_frames_to_swim, gorgonGoSwim};

animframe_t gorgon_frames_swim [] =
{ //FIXME: add wake and bubbles and sploosh swim sounds
	FRAME_swim6,	NULL, 0, 0, 0, gorgon_ai_swim, 31, fish_under_water_wake,
	FRAME_swim7,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL,
	FRAME_swim8,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swim9,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swim10,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL,
	FRAME_swim11,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL,
	FRAME_swim12,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL,
	FRAME_swim13,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL,
	FRAME_swim14,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swim15,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL,
	FRAME_swim16,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL,
	FRAME_swim17,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL,
	FRAME_swim18,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
};
animmove_t gorgon_move_swim = {13, gorgon_frames_swim, NULL};

animframe_t gorgon_frames_swim_bite_a [] =
{ //FIXME: add wake and bubbles and sploosh swim sounds
	FRAME_swimata1,	NULL, 0, 0, 0, gorgon_ai_swim, 31, fish_under_water_wake,
	FRAME_swimata2,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL,
	FRAME_swimata3,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swimata4,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swimata5,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL,
	FRAME_swimata6,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL,
	FRAME_swimata7,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL,
	FRAME_swimata8,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL,
	FRAME_swimata9,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swimata10,NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL,
	FRAME_swimata11,NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL,
	FRAME_swimata12,NULL, 0, 0, 0, gorgon_ai_swim, 32, gorgonbite,
	FRAME_swimata13,NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swimata14,NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
};
animmove_t gorgon_move_swim_bite_a = {14, gorgon_frames_swim_bite_a, NULL};

animframe_t gorgon_frames_swim_bite_b [] =
{ //FIXME: add wake and bubbles and sploosh swim sounds
	FRAME_swimatb1,	NULL, 0, 0, 0, gorgon_ai_swim, 31, fish_under_water_wake,
	FRAME_swimatb2,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL,
	FRAME_swimatb3,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swimatb4,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swimatb5,	NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL,
	FRAME_swimatb6,	NULL, 0, 0, 0, gorgon_ai_swim, 30, gorgonbite,
	FRAME_swimatb7,	NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL,
	FRAME_swimatb8,	NULL, 0, 0, 0, gorgon_ai_swim, 32, NULL,
	FRAME_swimatb9,	NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swimatb10,NULL, 0, 0, 0, gorgon_ai_swim, 30, NULL,
	FRAME_swimatb11,NULL, 0, 0, 0, gorgon_ai_swim, 31, NULL,
	FRAME_swimatb12,NULL, 0, 0, 0, gorgon_ai_swim, 32, gorgonbite,
	FRAME_swimatb13,NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
	FRAME_swimatb14,NULL, 0, 0, 0, gorgon_ai_swim, 34, NULL,
};
animmove_t gorgon_move_swim_bite_b = {14, gorgon_frames_swim_bite_b, NULL};

animframe_t gorgon_frames_outwater [] =
{ //FIXME: add wake and bubbles and sploosh swim sounds
	FRAME_swimata1,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL,
	FRAME_swimata3,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL,
	FRAME_swimata6,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL,
	FRAME_swimata8,	NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL,
	FRAME_swimata10,NULL, 0, 0, 0, gorgon_ai_swim, -1, NULL,
	FRAME_swimata12,NULL, 0, 0, 0, gorgon_ai_swim, -1, gorgon_growl,
	FRAME_swimata14,NULL, 0, 0, 0, gorgon_ai_swim, -1, gorgonJumpOutWater,
	FRAME_jumpb5,	NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_jumpb6,	NULL, 0, 0, 0, NULL, 0,  gorgonFixPitch,
	FRAME_jumpb7,	NULL, 0, 0, 0, NULL, 0,  gorgonFixPitch,
	FRAME_jumpb8,	NULL, 0, 0, 0, gorgonForward, 100,  gorgonFixPitch,
	FRAME_jumpb9,	NULL, 0, 0, 0, gorgonForward, 100,  gorgonFixPitch,
	FRAME_jumpb10,	NULL, 0, 0, 0, gorgonForward, 100,  gorgonFixPitch,
	FRAME_jumpb11,	NULL, 0, 0, 0, gorgonForward, 100,  gorgonFixPitch,
	FRAME_jumpb12,	NULL, 0, 0, 0, gorgonForward, 100,  gorgonFixPitch,
	FRAME_jumpb13,	NULL, 0, 0, 0, gorgonForward, 100,  gorgonFixPitch,
	FRAME_jumpb14,	NULL, 0, 0, 0, gorgonForward, 100,  gorgonFixPitch,
	FRAME_jumpb15,	NULL, 0, 0, 0, gorgonForward, 100,  gorgonZeroPitch,
	FRAME_jumpb16,	NULL, 0, 0, 0, NULL, 0, NULL,
	FRAME_jumpb17,	NULL, 0, 0, 0, NULL, 0, NULL
};
animmove_t gorgon_move_outwater = {20, gorgon_frames_outwater, gorgonCheckMood};

//=============================================================================

//EATING ANIMS

//FIXME: add sounds too...

//=============================================================================

/*----------------------------------------------------------------------
	eating trand to down
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_down [] =
{
	FRAME_eatinga1,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_eatinga2,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_eatinga3,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_eatinga4,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL,
};
animmove_t gorgon_move_eat_down = {4, gorgon_frames_eat_down, NULL};

/*----------------------------------------------------------------------
	eating trans to up
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_up [] =
{
	FRAME_eatinga4,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_eatinga3,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_eatinga2,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_eatinga1,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL,
};
animmove_t gorgon_move_eat_up = {4, gorgon_frames_eat_up, NULL};

/*----------------------------------------------------------------------
  eat cycle
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_loop [] =
{
	FRAME_eatinga5, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga6, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga7, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga8, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga9, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga10, NULL, 0, 0, 0, gorgon_ai_eat, -1,  NULL,
};
animmove_t gorgon_move_eat_loop = {6, gorgon_frames_eat_loop, NULL};

/*----------------------------------------------------------------------
  eating - tear
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_tear [] =
{
	FRAME_eatingb1, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb2, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb3, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb4, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb5, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb6, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb7, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb8, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb9, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb10, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb11, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb12, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb13, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb14, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb15, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb16, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb17, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb18, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb19, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb20, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatingb21, NULL, 0, 0, 0, gorgon_ai_eat, -1,  NULL,
};
animmove_t gorgon_move_eat_tear = {21, gorgon_frames_eat_tear, NULL};


/*----------------------------------------------------------------------
  eat- up & down
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_pullback [] =
{
	FRAME_eatinga5, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga4, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga3, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga4, NULL, 0, 0, 0, gorgon_ai_eat, 0,  NULL,
	FRAME_eatinga5, NULL, 0, 0, 0, gorgon_ai_eat, -1,  NULL,
};
animmove_t gorgon_move_eat_pullback = {5, gorgon_frames_eat_pullback, NULL};

/*----------------------------------------------------------------------
	look around
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_look_around [] =
{//fixme: modify view_ofs so they actually look behind them
	FRAME_idleb1,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb2,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb3,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb4,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb5,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb6,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb7,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb8,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb9,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb10,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb11,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb12,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb13,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb14,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb15,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb16,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb17,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb18,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb19,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb20,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_idleb21,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL,
};
animmove_t gorgon_move_look_around = {21, gorgon_frames_look_around, NULL};

/*----------------------------------------------------------------------
	looking left from eat
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_left [] =
{
	FRAME_loklft1,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft2,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft3,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft4,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft5,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft6,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft7,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft8,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft9,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft10,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft11,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft12,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft13,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft14,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft15,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft16,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft17,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft18,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft19,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft20,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft21,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft22,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft21,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft20,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft19,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft18,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft17,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft16,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft15,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft14,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft13,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft12,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft11,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft10,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft9,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft8,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft7,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft6,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft5,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft4,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft3,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft2,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_loklft1,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL
};
animmove_t gorgon_move_eat_left = {43, gorgon_frames_eat_left, NULL};

/*----------------------------------------------------------------------
	looking right from eat
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_right [] =
{
	FRAME_lokrt1,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt2,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt3,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt4,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt5,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt6,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt7,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt8,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt9,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt10,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt11,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt12,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt13,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt14,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt15,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt16,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt17,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt18,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt19,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt20,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt19,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt18,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt17,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt16,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt15,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt14,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt13,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt12,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt11,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt10,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt9,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt8,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt7,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt6,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt5,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt4,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt3,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt2,	NULL, 0, 0, 0, gorgon_ai_eat, 0, NULL,
	FRAME_lokrt1,	NULL, 0, 0, 0, gorgon_ai_eat, -1, NULL,
};
animmove_t gorgon_move_eat_right = {39, gorgon_frames_eat_right, NULL};

/*----------------------------------------------------------------------
  snap at something to right
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_snap [] =
{
	FRAME_snap1, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_snap2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_snap3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_snap4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_snap5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_snap6, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_snap7, NULL, 0, 0, 0, gorgon_ai_eat, -1,  NULL,
};
animmove_t gorgon_move_eat_snap = {7, gorgon_frames_eat_snap, NULL};

/*----------------------------------------------------------------------
  react tp something to left
-----------------------------------------------------------------------*/
animframe_t gorgon_frames_eat_react [] =
{
	FRAME_react1, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react2, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react3, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react4, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react5, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react6, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react7, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react8, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react9, NULL, 0, 0, 0, NULL, 0,  NULL,
	FRAME_react10, NULL, 0, 0, 0, gorgon_ai_eat, -1,  NULL,
};
animmove_t gorgon_move_eat_react = {10, gorgon_frames_eat_react, NULL};

/*-------------------------------------------------------------------
c_corvus.c

Heretic II
Copyright 1998 Raven Software

  AI:


-------------------------------------------------------------------*/
#include "g_local.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "fx.h"
#include "random.h"
#include "buoy.h"
#include "vector.h"

#include "g_Skeletons.h"
#include "c_corvus2.h"
#include "c_corvus2_anim.h"
#include "g_HitLocation.h"
#include "g_misc.h"
#include "angles.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[NUM_ANIMS] =
{
	// Cinematics
	&corvus2_move_c_action1,
	&corvus2_move_c_idle1,
	&corvus2_move_c_idle2,
	&corvus2_move_c_idle3,
	&corvus2_move_c_walkstart,
	&corvus2_move_c_walk1,
	&corvus2_move_c_walk2,
	&corvus2_move_c_walkstop1,
	&corvus2_move_c_walkstop2,
	&corvus2_move_c_pivotleftgo,
	&corvus2_move_c_pivotleft,
	&corvus2_move_c_pivotleftstop,
	&corvus2_move_c_pivotrightgo,
	&corvus2_move_c_pivotright,
	&corvus2_move_c_pivotrightstop
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	corvus2_c_anims
-------------------------------------------------------------------------*/
void corvus2_c_anims(edict_t *self, G_Message_t *msg)
{
	int int_msg;
	int curr_anim;

	ai_c_readmessage(self, msg);
	int_msg = (int) msg->ID;

	self->monsterinfo.c_anim_flag = 0; 

	switch(int_msg)
	{
		case MSG_C_ACTION1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 64;
			curr_anim = ANIM_C_ACTION1;
			break;
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE2;
			break;
		case MSG_C_IDLE3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE3;
			break;
		case MSG_C_PIVOTLEFTGO:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PIVOTLEFTGO;
			break;
		case MSG_C_PIVOTLEFT:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_PIVOTLEFT;
			break;
		case MSG_C_PIVOTLEFTSTOP:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PIVOTLEFTSTOP;
			break;
		case MSG_C_PIVOTRIGHTGO:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PIVOTRIGHTGO;
			break;
		case MSG_C_PIVOTRIGHT:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_PIVOTRIGHT;
			break;
		case MSG_C_PIVOTRIGHTSTOP:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PIVOTRIGHTSTOP;
			break;
		case MSG_C_WALKSTART:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTART;
			break;
		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;
		case MSG_C_WALK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK2;
			break;
		case MSG_C_WALKSTOP1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTOP1;
			break;
		case MSG_C_WALKSTOP2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTOP2;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_IDLE1;
			break;
	} 

	SetAnim(self, curr_anim);
}


/*-------------------------------------------------------------------------
	PlagueElfStaticsInit
-------------------------------------------------------------------------*/
void Corvus2StaticsInit()
{
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_ACTION1] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_IDLE1] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_IDLE2] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_IDLE3] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALKSTART] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALK1] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALK2] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALKSTOP1] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALKSTOP2] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTLEFTGO] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTLEFT] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTLEFTSTOP] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTRIGHTGO] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTRIGHT] = corvus2_c_anims;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = corvus2_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/player/watcher_scene/tris.fm");

	classStatics[CID_CORVUS2].resInfo = &resInfo;

}

/*QUAKED character_corvus2 (1 .5 0) (-17 -25 -32) (22 12 32)  INVISIBLE
The cinematic Corvus for the celestial watcher scene
*/
void SP_character_corvus2 (edict_t *self)
{
	c_corvus_init(self,CID_CORVUS2);
}

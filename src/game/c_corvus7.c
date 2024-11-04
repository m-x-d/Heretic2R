/*-------------------------------------------------------------------
c_corvus7.c

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

#include "c_corvus7.h"
#include "c_corvus7_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	// Cinematics
	&corvus7_move_c_action1,
	&corvus7_move_c_action2,
	&corvus7_move_c_action3,
	&corvus7_move_c_idle1,
	&corvus7_move_c_idle2,
	&corvus7_move_c_idle3,
	&corvus7_move_c_walkstart,
	&corvus7_move_c_walk1,
	&corvus7_move_c_walk2,
	&corvus7_move_c_walkstop1,
	&corvus7_move_c_walkstop2,
	&corvus7_move_c_pivotleftgo,
	&corvus7_move_c_pivotleft,
	&corvus7_move_c_pivotleftstop,
	&corvus7_move_c_pivotrightgo,
	&corvus7_move_c_pivotright,
	&corvus7_move_c_pivotrightstop
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	corvus7_c_anims
-------------------------------------------------------------------------*/
void corvus7_c_anims(edict_t *self, G_Message_t *msg)
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
			curr_anim = ANIM_C_ACTION1;
			break;
		case MSG_C_ACTION2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION2;
			break;
		case MSG_C_ACTION3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION3;
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
	StaticsInit
-------------------------------------------------------------------------*/
void Corvus7StaticsInit()
{

	classStatics[CID_CORVUS7].msgReceivers[MSG_C_ACTION1] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_ACTION2] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_ACTION3] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_IDLE1] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_IDLE2] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_IDLE3] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_WALKSTART] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_WALK1] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_WALK2] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_WALKSTOP1] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_WALKSTOP2] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_PIVOTLEFTGO] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_PIVOTLEFT] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_PIVOTLEFTSTOP] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_PIVOTRIGHTGO] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_PIVOTRIGHT] = corvus7_c_anims;
	classStatics[CID_CORVUS7].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = corvus7_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	//note that the name is different in the path
	resInfo.modelIndex = gi.modelindex("models/player/morcalavin_scene/tris.fm");

	classStatics[CID_CORVUS7].resInfo = &resInfo;

}

/*QUAKED character_corvus7 (1 .5 0) (-17 -25 -32) (22 12 32) INVISIBLE
The cinematic corvus for the Morcalavin scene
*/
void SP_character_corvus7 (edict_t *self)
{
	c_corvus_init(self,CID_CORVUS7);
}

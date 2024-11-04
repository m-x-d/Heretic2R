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

#include "c_corvus5.h"
#include "c_corvus5_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[NUM_ANIMS] =
{
	// Cinematics
	&corvus5_move_c_action1,
	&corvus5_move_c_action2,
	&corvus5_move_c_action3,
	&corvus5_move_c_action4,
	&corvus5_move_c_action5,
	&corvus5_move_c_action6,
	&corvus5_move_c_action7,
	&corvus5_move_c_action8,
	&corvus5_move_c_idle1,
	&corvus5_move_c_idle2,
	&corvus5_move_c_idle3,
	&corvus5_move_c_walkstart,
	&corvus5_move_c_walk1,
	&corvus5_move_c_walk2,
	&corvus5_move_c_walkstop1,
	&corvus5_move_c_walkstop2,
	&corvus5_move_c_pivotleftgo,
	&corvus5_move_c_pivotleft,
	&corvus5_move_c_pivotleftstop,
	&corvus5_move_c_pivotrightgo,
	&corvus5_move_c_pivotright,
	&corvus5_move_c_pivotrightstop
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	corvus5_c_anims
-------------------------------------------------------------------------*/
void corvus5_c_anims(edict_t *self, G_Message_t *msg)
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
		case MSG_C_ACTION4:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION4;
			break;
		case MSG_C_ACTION5:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION5;
			break;
		case MSG_C_ACTION6:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION6;
			break;
		case MSG_C_ACTION7:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION7;
			break;
		case MSG_C_ACTION8:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION8;
			break;
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;
		case MSG_C_IDLE3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
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
void Corvus5StaticsInit()
{
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_ACTION1] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_ACTION2] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_ACTION3] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_ACTION4] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_ACTION5] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_ACTION6] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_ACTION7] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_ACTION8] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_IDLE1] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_IDLE2] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_IDLE3] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_WALKSTART] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_WALK1] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_WALK2] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_WALKSTOP1] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_WALKSTOP2] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_PIVOTLEFTGO] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_PIVOTLEFT] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_PIVOTLEFTSTOP] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_PIVOTRIGHTGO] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_PIVOTRIGHT] = corvus5_c_anims;
	classStatics[CID_CORVUS5].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = corvus5_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/player/dranor_scene/tris.fm");

	classStatics[CID_CORVUS5].resInfo = &resInfo;

}


/*QUAKED character_corvus5 (1 .5 0) (-16 -16 -34) (16 16 25) INVISIBLE
The cinematic Corvus for the Dranor scene
*/
void SP_character_corvus5 (edict_t *self)
{
	c_corvus_init(self,CID_CORVUS5);
}

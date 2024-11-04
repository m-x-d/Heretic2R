/*-------------------------------------------------------------------
c_corvus8.c

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

#include "c_corvus8.h"
#include "c_corvus8_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	// Cinematics
	&corvus8_move_c_action1,
	&corvus8_move_c_action2,
	&corvus8_move_c_action3,
	&corvus8_move_c_action4,
	&corvus8_move_c_action5,
	&corvus8_move_c_action6,
	&corvus8_move_c_action7,
	&corvus8_move_c_action8,
	&corvus8_move_c_action9,
	&corvus8_move_c_action10,
	&corvus8_move_c_action11,
	&corvus8_move_c_action12,
	&corvus8_move_c_action13,
	&corvus8_move_c_action14,
	&corvus8_move_c_action15,
	&corvus8_move_c_action16,
	&corvus8_move_c_action17,
	&corvus8_move_c_action18,
	&corvus8_move_c_action19,
	&corvus8_move_c_action20,

	&corvus8_move_c_idle1,
	&corvus8_move_c_idle2,
	&corvus8_move_c_walkstart,
	&corvus8_move_c_walk1,
	&corvus8_move_c_walk2,
	&corvus8_move_c_walk3,
	&corvus8_move_c_walkstop1,
	&corvus8_move_c_walkstop2,
	&corvus8_move_c_pivotleftgo,
	&corvus8_move_c_pivotleft,
	&corvus8_move_c_pivotleftstop,
	&corvus8_move_c_pivotrightgo,
	&corvus8_move_c_pivotright,
	&corvus8_move_c_pivotrightstop
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	corvus8_c_anims
-------------------------------------------------------------------------*/
void corvus8_c_anims(edict_t *self, G_Message_t *msg)
{
	int int_msg;
	int curr_anim;

	ai_c_readmessage(self, msg);
	int_msg = (int) msg->ID;

	self->monsterinfo.c_anim_flag = 0; 

	switch(int_msg)
	{
		case MSG_C_ACTION1 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION1;
			break;
		case MSG_C_ACTION2 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION2;
			break;
		case MSG_C_ACTION3 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION3;
			break;
		case MSG_C_ACTION4 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION4;
			break;
		case MSG_C_ACTION5 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION5;
			break;
		case MSG_C_ACTION6 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION6;
			break;
		case MSG_C_ACTION7 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION7;
			break;
		case MSG_C_ACTION8 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION8;
			break;
		case MSG_C_ACTION9 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION9;
			break;
		case MSG_C_ACTION10 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION10;
			break;
		case MSG_C_ACTION11 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION11;
			break;
		case MSG_C_ACTION12 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION12;
			break;
		case MSG_C_ACTION13 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION13;
			break;
		case MSG_C_ACTION14 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION14;
			break;
		case MSG_C_ACTION15 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION15;
			break;
		case MSG_C_ACTION16 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION16;
			break;
		case MSG_C_ACTION17 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION17;
			break;
		case MSG_C_ACTION18 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION18;
			break;
		case MSG_C_ACTION19 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION19;
			break;
		case MSG_C_ACTION20 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION20;
			break;
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
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
		case MSG_C_WALK3:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK3;
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
void Corvus8StaticsInit()
{

	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION1] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION2] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION3] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION4] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION5] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION6] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION7] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION8] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION9] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION10] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION11] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION12] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION13] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION14] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION15] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION16] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION17] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION18] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION19] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION20] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_IDLE1] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_IDLE2] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALKSTART] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALK1] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALK2] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALK3] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALKSTOP1] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALKSTOP2] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTLEFTGO] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTLEFT] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTLEFTSTOP] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTRIGHTGO] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTRIGHT] = corvus8_c_anims;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = corvus8_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	//note that the name is different in the path
	resInfo.modelIndex = gi.modelindex("models/player/siernan_scene/tris.fm");

	classStatics[CID_CORVUS8].resInfo = &resInfo;

}

/*QUAKED character_corvus8 (1 .5 0) (-17 -25 -32) (22 12 32) INVISIBLE
The cinematic corvus for the Siernan scenes
*/
void SP_character_corvus8 (edict_t *self)
{
	c_corvus_init(self,CID_CORVUS8);
}

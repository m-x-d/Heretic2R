/*-------------------------------------------------------------------
c_corvus9.c

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

#include "c_corvus9.h"
#include "c_corvus9_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	// Cinematics
	&corvus9_move_c_action1,
	&corvus9_move_c_action2,
	&corvus9_move_c_action3,
	&corvus9_move_c_action4,
	&corvus9_move_c_action5,
	&corvus9_move_c_action6,
	&corvus9_move_c_action7,
	&corvus9_move_c_action8,
	&corvus9_move_c_action9,
	&corvus9_move_c_action10,
	&corvus9_move_c_action11,
	&corvus9_move_c_idle1,
	&corvus9_move_c_idle2,
	&corvus9_move_c_idle3,
	&corvus9_move_c_walk1,
	&corvus9_move_c_walk2,
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	corvus9_c_anims
-------------------------------------------------------------------------*/
void corvus9_c_anims(edict_t *self, G_Message_t *msg)
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
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION11;
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
		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;
		case MSG_C_WALK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK2;
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
void Corvus9StaticsInit()
{

	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION1] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION2] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION3] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION4] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION5] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION6] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION7] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION8] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION9] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION10] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION11] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_IDLE1] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_IDLE2] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_IDLE3] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_WALK1] = corvus9_c_anims;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_WALK2] = corvus9_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	//note that the name is different in the path
	resInfo.modelIndex = gi.modelindex("models/player/t'chekrikscene/tris.fm");

	classStatics[CID_CORVUS9].resInfo = &resInfo;

}

/*QUAKED character_corvus9 (1 .5 0) (-17 -25 -32) (22 12 32) INVISIBLE
The cinematic corvus for the T'chekrik scenes
*/
void SP_character_corvus9 (edict_t *self)
{
	c_corvus_init(self,CID_CORVUS9);
	self->svflags|=SVF_FLOAT;
}

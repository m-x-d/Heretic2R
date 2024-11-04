/*-------------------------------------------------------------------
c_priestess.c

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

#include "vector.h"
#include "c_priestess.h"
#include "c_priestess_anim.h"
#include "c_ai.h"


/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[NUM_ANIMS] =
{
	// Cinematics
	&priestess_move_c_action1,
	&priestess_move_c_action2,
	&priestess_move_c_action3,
	&priestess_move_c_action4,
	&priestess_move_c_action5,
	&priestess_move_c_action6,
	&priestess_move_c_action7,
	&priestess_move_c_action8,
	&priestess_move_c_action9,
	&priestess_move_c_action10,
	&priestess_move_c_action11,
	&priestess_move_c_action12,
	&priestess_move_c_action13,
	&priestess_move_c_action14,
	&priestess_move_c_action15,
	&priestess_move_c_action16,
	&priestess_move_c_backpedal1,
	&priestess_move_c_idle1,
	&priestess_move_c_idle2,
	&priestess_move_c_walk1,
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	priestess_c_anims
-------------------------------------------------------------------------*/
void priestess_c_anims(edict_t *self, G_Message_t *msg)
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
		case MSG_C_ACTION9:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION9;
			break;
		case MSG_C_ACTION10:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION10;
			break;
		case MSG_C_ACTION11:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION11;
			break;
		case MSG_C_ACTION12:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION12;
			break;
		case MSG_C_ACTION13:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION13;
			break;
		case MSG_C_ACTION14:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION14;
			break;
		case MSG_C_ACTION15:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION15;
			break;
		case MSG_C_ACTION16:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION16;
			break;
		case MSG_C_BACKPEDAL1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_BACKPEDAL1;
			break;
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;
		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
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
void PriestessCStaticsInit()
{
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION1] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION2] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION3] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION4] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION5] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION6] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION7] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION8] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION9] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION10] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION11] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION12] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION13] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION14] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION15] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION16] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_BACKPEDAL1] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_IDLE1] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_IDLE2] = priestess_c_anims;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_WALK1] = priestess_c_anims;
	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/monsters/highpriestess/cinematic/tris.fm");

	classStatics[CID_C_HIGHPRIESTESS].resInfo = &resInfo;

}


/*QUAKED character_highpriestess(1 .5 0) (-24 -24 -36) (24 24 36)  INVISIBLE
The cinematic High Priestess
--------SPAWNFLAG-------------
INVISIBLE - you won't see her
*/
void SP_character_highpriestess (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -32);
	VectorSet (self->maxs, 16, 16, 32);	

	c_character_init(self,CID_C_HIGHPRIESTESS);
}

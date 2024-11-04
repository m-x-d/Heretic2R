/*-------------------------------------------------------------------
c_priestess2.c

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

#include "c_priestess2.h"
#include "c_priestess2_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[NUM_ANIMS] =
{
	// Cinematics
	&priestess2_move_c_action1,
	&priestess2_move_c_action2,
	&priestess2_move_c_action3,
	&priestess2_move_c_action4,
	&priestess2_move_c_action5,
	&priestess2_move_c_action6,
	&priestess2_move_c_action7,
	&priestess2_move_c_idle1,
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	priestess2_c_anims
-------------------------------------------------------------------------*/
void priestess2_c_anims(edict_t *self, G_Message_t *msg)
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
			self->monsterinfo.c_dist = 40;
			curr_anim = ANIM_C_ACTION1;
			break;
		case MSG_C_ACTION2 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 16;
			curr_anim = ANIM_C_ACTION2;
			break;
		case MSG_C_ACTION3 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 158;
			curr_anim = ANIM_C_ACTION3;
			break;
		case MSG_C_ACTION4 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 96;
			curr_anim = ANIM_C_ACTION4;
			break;
		case MSG_C_ACTION5 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 158;
			curr_anim = ANIM_C_ACTION5;
			break;
		case MSG_C_ACTION6 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 72;
			curr_anim = ANIM_C_ACTION6;
			break;
		case MSG_C_ACTION7 :
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION7;
			break;
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
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
void Priestess2CStaticsInit()
{
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION1] = priestess2_c_anims;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION2] = priestess2_c_anims;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION3] = priestess2_c_anims;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION4] = priestess2_c_anims;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION5] = priestess2_c_anims;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION6] = priestess2_c_anims;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION7] = priestess2_c_anims;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_IDLE1] = priestess2_c_anims;
	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/monsters/highpriestess/pod_scene/tris.fm");

	classStatics[CID_C_HIGHPRIESTESS2].resInfo = &resInfo;

}

/*QUAKED character_highpriestess2(1 .5 0) (-24 -24 -36) (24 24 36)  INVISIBLE
The cinematic High Priestess for the Pod Scene
--------SPAWNFLAG-------------
INVISIBLE - you won't see her
*/
void SP_character_highpriestess2 (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -38);
	VectorSet (self->maxs, 16, 16, 38);	

	c_character_init(self,CID_C_HIGHPRIESTESS2);
}

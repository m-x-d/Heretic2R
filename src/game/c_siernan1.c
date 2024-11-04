/*-------------------------------------------------------------------
c_siernan1.c

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

#include "c_siernan1.h"
#include "c_siernan1_anim.h"
#include "c_ai.h"

#define ENT_INVISIBLE 1
#define ENT_LEANING	4

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	// Cinematics
	&siernan1_move_c_action1,
	&siernan1_move_c_action2,
	&siernan1_move_c_action3,
	&siernan1_move_c_action4,
	&siernan1_move_c_action5,
	&siernan1_move_c_action6,
	&siernan1_move_c_action7,
	&siernan1_move_c_action8,
	&siernan1_move_c_action9,
	&siernan1_move_c_action10,
	&siernan1_move_c_action11,
	&siernan1_move_c_action12,
	&siernan1_move_c_action13,
	&siernan1_move_c_idle1,
	&siernan1_move_c_idle2,
	&siernan1_move_c_idle3,
	&siernan1_move_c_idle4,
	&siernan1_move_c_idle5,
	&siernan1_move_c_walkstart,
	&siernan1_move_c_walk1,
	&siernan1_move_c_walkstop1,
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	siernan1_c_anims
-------------------------------------------------------------------------*/
void siernan1_c_anims(edict_t *self, G_Message_t *msg)
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
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			if (self->spawnflags & ENT_LEANING)
				curr_anim = ANIM_C_IDLE3;
			else
				curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;
		case MSG_C_IDLE3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE3;
			break;
		case MSG_C_IDLE4:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE4;
			break;
		case MSG_C_IDLE5:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE5;
			break;
		case MSG_C_WALKSTART:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTART;
			break;
		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALK1;
			break;
		case MSG_C_WALKSTOP1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTOP1;
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
void Siernan1CinStaticsInit()
{


	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION1] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION2] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION3] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION4] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION5] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION6] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION7] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION8] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION9] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION10] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION11] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION12] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION13] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE1] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE2] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE3] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE4] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE5] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_WALKSTART] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_WALK1] = siernan1_c_anims;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_WALKSTOP1] = siernan1_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	//note that the name is different in the path
	resInfo.modelIndex = gi.modelindex("models/monsters/siernan/standing/tris.fm");

	classStatics[CID_C_SIERNAN1].resInfo = &resInfo;

}


/*QUAKED character_siernan1 (1 .5 0) (-10 -10 -20) (10 10 20) INVISIBLE x LEANING
The cinematic siernan standing
INVISIBLE - can't be seen
LEANING - leaning against a wall, idling
*/
void SP_character_siernan1 (edict_t *self)
{
	VectorSet (self->mins, -10, -10, -20);
	VectorSet (self->maxs, 10, 10, 20);	

	self->s.scale = self->monsterinfo.scale = 1.2;

	c_character_init(self,CID_C_SIERNAN1);


}



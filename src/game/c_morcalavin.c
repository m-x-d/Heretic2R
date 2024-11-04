/*-------------------------------------------------------------------
c_morcalavin.c

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

#include "c_morcalavin.h"
#include "c_morcalavin_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	// Cinematics
	&morcalavin_move_c_action1,
	&morcalavin_move_c_action2,
	&morcalavin_move_c_action3,
	&morcalavin_move_c_idle1,
	&morcalavin_move_c_idle2,
	&morcalavin_move_c_idle3,
	&morcalavin_move_c_idle4,
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	morcalavin_c_anims
-------------------------------------------------------------------------*/
void morcalavin_c_anims(edict_t *self, G_Message_t *msg)
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
		case MSG_C_IDLE4:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE4;
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
void MorcalavinCinStaticsInit()
{

	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_ACTION1] = morcalavin_c_anims;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_ACTION2] = morcalavin_c_anims;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_ACTION3] = morcalavin_c_anims;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_IDLE1] = morcalavin_c_anims;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_IDLE2] = morcalavin_c_anims;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_IDLE3] = morcalavin_c_anims;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_IDLE4] = morcalavin_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	//note that the name is different in the path
	resInfo.modelIndex = gi.modelindex("models/monsters/morcalavin/cinematic/tris.fm");

	classStatics[CID_C_MORCALAVIN].resInfo = &resInfo;

}


/*QUAKED character_morcalavin (1 .5 0) (-24 -24 -50) (24 24 50) INVISIBLE
The cinematic morcalavin
*/
void SP_character_morcalavin (edict_t *self)
{
	VectorSet(self->mins, -24, -24, -46);
	VectorSet(self->maxs, 24, 24, 46);

	c_character_init(self,CID_C_MORCALAVIN);

}

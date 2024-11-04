/*-------------------------------------------------------------------
c_siernan2.c

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

#include "c_siernan2.h"
#include "c_siernan2_anim.h"
#include "c_ai.h"

#define ENT_INVISIBLE 1


/*--------------------
--------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	// Cinematics
	&siernan2_move_c_action1,
	&siernan2_move_c_action2,
	&siernan2_move_c_idle1,
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	siernan2_c_anims
-------------------------------------------------------------------------*/
void siernan2_c_anims(edict_t *self, G_Message_t *msg)
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
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;
		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE1;
			break;
	} 

	SetAnim(self, curr_anim);
}


/*-------------------------------------------------------------------------
	StaticsInit
-------------------------------------------------------------------------*/
void Siernan2CinStaticsInit()
{

	classStatics[CID_C_SIERNAN2].msgReceivers[MSG_C_ACTION1] = siernan2_c_anims;
	classStatics[CID_C_SIERNAN2].msgReceivers[MSG_C_ACTION2] = siernan2_c_anims;
	classStatics[CID_C_SIERNAN2].msgReceivers[MSG_C_IDLE1] = siernan2_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/monsters/siernan/laying/tris.fm");

	classStatics[CID_C_SIERNAN2].resInfo = &resInfo;

}



/*QUAKED character_siernan2 (1 .5 0) (-17 -25 0) (22 12 16) INVISIBLE
The cinematic siernan laying down
*/
void SP_character_siernan2 (edict_t *self)
{
	VectorSet (self->mins, -17, -25, 0);
	VectorSet (self->maxs, 22, 12, 16);	

	self->s.scale = self->monsterinfo.scale = 1;

	c_character_init(self,CID_C_SIERNAN2);
}

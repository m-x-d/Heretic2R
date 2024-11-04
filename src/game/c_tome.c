/*-------------------------------------------------------------------
c_tome.c

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

#include "c_tome.h"
#include "c_tome_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	// Cinematics
	&tome_move_c_idle1,
	&tome_move_c_idle2,
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	tome_c_anims
-------------------------------------------------------------------------*/
void tome_c_anims(edict_t *self, G_Message_t *msg)
{
	int int_msg;
	int curr_anim;

	ai_c_readmessage(self, msg);
	int_msg = (int) msg->ID;

	self->monsterinfo.c_anim_flag = 0; 

	switch(int_msg)
	{
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;
		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
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
void TomeStaticsInit()
{

	classStatics[CID_C_TOME].msgReceivers[MSG_C_IDLE1] = tome_c_anims;
	classStatics[CID_C_TOME].msgReceivers[MSG_C_IDLE2] = tome_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/spells/book/tris.fm");

	classStatics[CID_C_TOME].resInfo = &resInfo;

}


/*QUAKED character_tome (1 .5 0) (-4 -8 -12) (4 8 12) INVISIBLE
The talking tome of power (sounds like a hot babe)
*/
void SP_character_tome (edict_t *self)
{
	VectorSet (self->mins, -4, -8, -12);
	VectorSet (self->maxs, 4, 8, 12);	

	c_character_init(self,CID_C_TOME);
}

//==============================================================================
//
// m_victimSsithra.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================
#include "g_local.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "fx.h"
#include "random.h"
#include "vector.h"

#include "c_victimSsithra.h"
#include "c_ai.h"


void BboxYawAndScale(edict_t *self);

/*----------------------------------------------------------------------
  victimSsithra Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	&victimSsithra_move_c_action1,
	&victimSsithra_move_c_action2,
	&victimSsithra_move_c_action3,
	&victimSsithra_move_c_action4,
	&victimSsithra_move_c_action5,
	&victimSsithra_move_c_action6
};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	victimSsithra_c_anims
-------------------------------------------------------------------------*/
void victimSsithra_c_anims(edict_t *self, G_Message_t *msg)
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
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_ACTION6;
			break;
		default:
			break;
	} 

	SetAnim(self, curr_anim);
}

/*-------------------------------------------------------------------------
	PlagueElfStaticsInit
-------------------------------------------------------------------------*/
void victimSsithraStaticsInit()
{

	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_IDLE1] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION1] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION2] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION3] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION4] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION5] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION6] = victimSsithra_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/monsters/Ssithra/cinematics/tris.fm");

	classStatics[CID_SSITHRA_VICTIM].resInfo = &resInfo;

	sounds[SND_PAIN1] = gi.soundindex("monsters/plagueElf/pain1.wav");
	
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_SSITHRA_VICTIM].resInfo = &resInfo;

}

/*QUAKED character_ssithra_victim (1 .5 0) (-40 -16 -2) (40 16 2) INVISIBLE
The Ssithra Victim for use in the torture scene
*/
void SP_character_ssithra_victim (edict_t *self)
{
	VectorSet (self->mins, -40, -16, -2);
	VectorSet (self->maxs, 40, 16, 2);	

	c_character_init(self,CID_SSITHRA_VICTIM);

}


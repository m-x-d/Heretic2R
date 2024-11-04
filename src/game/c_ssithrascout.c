//==============================================================================
//
// m_ssithrascout.c
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

#include "c_ssithrascout_anim.h"
#include "c_ssithrascout.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  victimSsithra Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	&scout_move_c_action1,
	&scout_move_c_action2,
	&scout_move_c_action3,
	&scout_move_c_action4,
	&scout_move_c_action5,
	&scout_move_c_action6,
	&scout_move_c_action7,
	&scout_move_c_action8,
	&scout_move_c_action9,
	&scout_move_c_action10,
	&scout_move_c_death1,
	&scout_move_c_idle1,
	&scout_move_c_idle2,
	&scout_move_c_idle3,
};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	scout_c_anims
-------------------------------------------------------------------------*/
void scout_c_anims(edict_t *self, G_Message_t *msg)
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
		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH1;
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
		default:
			break;
	} 

	SetAnim(self, curr_anim);
}

/*-------------------------------------------------------------------------
	SsithrascoutStaticsInit
-------------------------------------------------------------------------*/
void SsithrascoutStaticsInit()
{
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION1] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION2] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION3] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION4] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION5] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION6] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION7] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION8] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION9] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION10] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_DEATH1] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_IDLE1] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_IDLE2] = scout_c_anims;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_IDLE3] = scout_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/monsters/ssithra/scout_scene/tris.fm");

	classStatics[CID_SSITHRA_SCOUT].resInfo = &resInfo;

	sounds[SND_PAIN1] = gi.soundindex("monsters/plagueElf/pain1.wav");
	
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_SSITHRA_SCOUT].resInfo = &resInfo;

}

/*QUAKED character_ssithra_scout (1 .5 0) (-26 -16 -13) (26 16 13)
The scout
*/
void SP_character_ssithra_scout (edict_t *self)
{
	VectorSet (self->mins, -26, -16, -13);
	VectorSet (self->maxs, 26, 16, 13);	

	c_character_init(self,CID_SSITHRA_SCOUT);
}


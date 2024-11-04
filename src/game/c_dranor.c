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

#include "c_dranor_anim.h"
#include "c_dranor.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  victimSsithra Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[ NUM_ANIMS] =
{
	&dranor_move_c_action1,
	&dranor_move_c_action2,
	&dranor_move_c_action3,
	&dranor_move_c_action4,
	&dranor_move_c_action5,
	&dranor_move_c_action6,
	&dranor_move_c_action7,
	&dranor_move_c_action8,
	&dranor_move_c_action9,
	&dranor_move_c_action10,
	&dranor_move_c_action11,
	&dranor_move_c_action12,
	&dranor_move_c_death1,
	&dranor_move_c_idle1,
	&dranor_move_c_idle2,
	&dranor_move_c_idle3,
};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	victimSsithra_c_anims
-------------------------------------------------------------------------*/
void Dranor_c_anims(edict_t *self, G_Message_t *msg)
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
		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			self->health = 5;
			curr_anim = ANIM_C_DEATH1;
			self->svflags |= SVF_DEADMONSTER;//doesn't block walking
			self->takedamage = DAMAGE_YES;
			self->movetype = PHYSICSTYPE_STOP;
			self->solid = SOLID_BBOX;
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
	DranorStaticsInit
-------------------------------------------------------------------------*/
void DranorStaticsInit()
{
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION1] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION2] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION3] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION4] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION5] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION6] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION7] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION8] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION9] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION10] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION11] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION12] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_DEATH1] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_IDLE1] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_IDLE2] = Dranor_c_anims;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_IDLE3] = Dranor_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/monsters/plaguelf/dranor/tris.fm");

	classStatics[CID_SSITHRA_VICTIM].resInfo = &resInfo;

	sounds[SND_PAIN1] = gi.soundindex("monsters/plagueElf/pain1.wav");
	
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_DRANOR].resInfo = &resInfo;

}

/*QUAKED character_dranor (1 .5 0) (-17 -25 -32) (22 12 32)  INVISIBLE
The elf who talks like Sean Connery
*/
void SP_character_dranor (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -32);
	VectorSet (self->maxs, 16, 16, 32);	

	c_character_init(self,CID_DRANOR);

	self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
	self->health = 30;

}


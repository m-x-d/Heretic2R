/*-------------------------------------------------------------------
c_corvus.c

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

#include "c_corvus6.h"
#include "c_corvus6_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[NUM_ANIMS] =
{
	// Cinematics
	&corvus6_move_c_action1,
	&corvus6_move_c_action2,
	&corvus6_move_c_action3,
	&corvus6_move_c_action4,
	&corvus6_move_c_action5,
	&corvus6_move_c_action6,
	&corvus6_move_c_action7,
	&corvus6_move_c_action8,
	&corvus6_move_c_action9,
	&corvus6_move_c_action10,
	&corvus6_move_c_action11,
	&corvus6_move_c_idle1,
	&corvus6_move_c_idle2,
	&corvus6_move_c_idle3,
	&corvus6_move_c_idle4,
	&corvus6_move_c_idle5,
};

static ClassResourceInfo_t resInfo;


/*-------------------------------------------------------------------------
	corvus6_c_anims
-------------------------------------------------------------------------*/
void corvus6_c_anims(edict_t *self, G_Message_t *msg)
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

		case MSG_C_IDLE5:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE5;
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
void Corvus6StaticsInit()
{
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION1] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION2] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION3] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION4] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION5] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION6] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION7] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION8] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION9] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION10] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION11] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE1] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE2] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE3] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE4] = corvus6_c_anims;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE5] = corvus6_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/player/sewer_scene/tris.fm");

	classStatics[CID_CORVUS6].resInfo = &resInfo;

}

/*QUAKED character_corvus6 (1 .5 0) (-16 -16 -34) (16 16 25) INVISIBLE
The cinematic Corvus for the Dranor scene
*/
void SP_character_corvus6 (edict_t *self)
{
	c_corvus_init(self,CID_CORVUS6);
}

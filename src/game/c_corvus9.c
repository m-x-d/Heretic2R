//
// c_corvus9.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus9.h"
#include "c_corvus9_anim.h"
#include "c_ai.h"
#include "Utilities.h"

// Corvus8 cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&corvus9_move_c_action1,
	&corvus9_move_c_action2,
	&corvus9_move_c_action3,
	&corvus9_move_c_action4,
	&corvus9_move_c_action5,
	&corvus9_move_c_action6,
	&corvus9_move_c_action7,
	&corvus9_move_c_action8,
	&corvus9_move_c_action9,
	&corvus9_move_c_action10,
	&corvus9_move_c_action11,
	&corvus9_move_c_idle1,
	&corvus9_move_c_idle2,
	&corvus9_move_c_idle3,
	&corvus9_move_c_walk1,
	&corvus9_move_c_walk2,
};

static void Corvus9CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'corvus9_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
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
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
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

		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;

		case MSG_C_WALK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK2;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

void Corvus9CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION1] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION2] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION3] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION4] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION5] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION6] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION7] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION8] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION9] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION10] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_ACTION11] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_IDLE1] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_IDLE2] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_IDLE3] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_WALK1] = Corvus9CinematicActionMsgHandler;
	classStatics[CID_CORVUS9].msgReceivers[MSG_C_WALK2] = Corvus9CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/player/t'chekrikscene/tris.fm"); // Note that the name is different in the path.

	classStatics[CID_CORVUS9].resInfo = &res_info;
}

// QUAKED character_corvus9 (1 .5 0) (-17 -25 -32) (22 12 32) INVISIBLE
// The cinematic corvus for the T'chekrik scenes.
void SP_character_corvus9(edict_t* self)
{
	CinematicCorvusInit(self, CID_CORVUS9);
	self->svflags |= SVF_FLOAT;
}
//
// c_corvus6.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus6.h"
#include "c_corvus6_anim.h"
#include "c_corvus6_moves.h"
#include "c_ai.h"
#include "Utilities.h"

// Corvus6 cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
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

static void Corvus6CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'corvus6_c_anims' in original logic.
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

void Corvus6CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION1] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION2] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION3] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION4] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION5] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION6] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION7] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION8] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION9] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION10] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_ACTION11] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE1] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE2] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE3] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE4] = Corvus6CinematicActionMsgHandler;
	classStatics[CID_CORVUS6].msgReceivers[MSG_C_IDLE5] = Corvus6CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/player/sewer_scene/tris.fm");

	classStatics[CID_CORVUS6].resInfo = &res_info;
}

// QUAKED character_corvus6 (1 .5 0) (-16 -16 -34) (16 16 25) INVISIBLE
// The cinematic Corvus for the Dranor scene.
void SP_character_corvus6(edict_t* self)
{
	CinematicCorvusInit(self, CID_CORVUS6);
}
//
// c_priestess.c
//
// Copyright 1998 Raven Software
//

#include "c_priestess.h"
#include "c_priestess_anim.h"
#include "c_priestess_moves.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

// High priestess cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&priestess_move_c_action1,
	&priestess_move_c_action2,
	&priestess_move_c_action3,
	&priestess_move_c_action4,
	&priestess_move_c_action5,
	&priestess_move_c_action6,
	&priestess_move_c_action7,
	&priestess_move_c_action8,
	&priestess_move_c_action9,
	&priestess_move_c_action10,
	&priestess_move_c_action11,
	&priestess_move_c_action12,
	&priestess_move_c_action13,
	&priestess_move_c_action14,
	&priestess_move_c_action15,
	&priestess_move_c_action16,
	&priestess_move_c_backpedal1,
	&priestess_move_c_idle1,
	&priestess_move_c_idle2,
	&priestess_move_c_walk1,
};

static void PriestessCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'priestess_c_anims' in original logic.
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

		case MSG_C_ACTION12:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION12;
			break;

		case MSG_C_ACTION13:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION13;
			break;

		case MSG_C_ACTION14:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION14;
			break;

		case MSG_C_ACTION15:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION15;
			break;

		case MSG_C_ACTION16:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION16;
			break;

		case MSG_C_BACKPEDAL1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_BACKPEDAL1;
			break;

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;

		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;

		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

void PriestessCinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION1] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION2] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION3] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION4] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION5] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION6] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION7] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION8] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION9] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION10] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION11] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION12] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION13] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION14] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION15] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_ACTION16] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_BACKPEDAL1] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_IDLE1] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_IDLE2] = PriestessCinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS].msgReceivers[MSG_C_WALK1] = PriestessCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/highpriestess/cinematic/tris.fm");

	classStatics[CID_C_HIGHPRIESTESS].resInfo = &res_info;
}

// QUAKED character_highpriestess(1 .5 0) (-24 -24 -36) (24 24 36)  INVISIBLE
// The cinematic High Priestess.
void SP_character_highpriestess(edict_t* self)
{
	VectorSet(self->mins, -16.0f, -16.0f, -32.0f);
	VectorSet(self->maxs,  16.0f,  16.0f, 32.0f);

	CinematicCharacterInit(self, CID_C_HIGHPRIESTESS);
}
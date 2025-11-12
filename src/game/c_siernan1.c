//
// c_siernan1.c
//
// Copyright 1998 Raven Software
//

#include "c_siernan1.h"
#include "c_siernan1_anim.h"
#include "c_siernan1_moves.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

#define ENT_LEANING		4

// Siernan cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&siernan1_move_c_action1,
	&siernan1_move_c_action2,
	&siernan1_move_c_action3,
	&siernan1_move_c_action4,
	&siernan1_move_c_action5,
	&siernan1_move_c_action6,
	&siernan1_move_c_action7,
	&siernan1_move_c_action8,
	&siernan1_move_c_action9,
	&siernan1_move_c_action10,
	&siernan1_move_c_action11,
	&siernan1_move_c_action12,
	&siernan1_move_c_action13,
	&siernan1_move_c_idle1,
	&siernan1_move_c_idle2,
	&siernan1_move_c_idle3,
	&siernan1_move_c_idle4,
	&siernan1_move_c_idle5,
	&siernan1_move_c_walkstart,
	&siernan1_move_c_walk1,
	&siernan1_move_c_walkstop1,
};

static void Siernan1CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'siernan1_c_anims' in original logic.
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

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= (C_ANIM_REPEAT | C_ANIM_IDLE);
			curr_anim = ((self->spawnflags & ENT_LEANING) ? ANIM_C_IDLE3 : ANIM_C_IDLE1);
			break;

		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;

		case MSG_C_IDLE3:
			self->monsterinfo.c_anim_flag |= (C_ANIM_REPEAT | C_ANIM_IDLE);
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

		case MSG_C_WALKSTART:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTART;
			break;

		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALK1;
			break;

		case MSG_C_WALKSTOP1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTOP1;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

void Siernan1CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION1] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION2] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION3] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION4] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION5] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION6] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION7] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION8] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION9] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION10] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION11] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION12] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_ACTION13] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE1] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE2] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE3] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE4] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_IDLE5] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_WALKSTART] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_WALK1] = Siernan1CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN1].msgReceivers[MSG_C_WALKSTOP1] = Siernan1CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/siernan/standing/tris.fm"); // Note that the name is different in the path.

	classStatics[CID_C_SIERNAN1].resInfo = &res_info;
}

// QUAKED character_siernan1 (1 .5 0) (-10 -10 -20) (10 10 20) INVISIBLE x LEANING
// The cinematic siernan standing.
// INVISIBLE -	Can't be seen.
// LEANING -	Leaning against a wall, idling.
void SP_character_siernan1(edict_t* self)
{
	VectorSet(self->mins, -10.0f, -10.0f, -20.0f);
	VectorSet(self->maxs,  10.0f,  10.0f,  20.0f);

	self->s.scale = 1.2f;
	self->monsterinfo.scale = 1.2f;

	CinematicCharacterInit(self, CID_C_SIERNAN1);
}
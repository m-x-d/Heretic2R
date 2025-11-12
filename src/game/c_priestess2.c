//
// c_priestess2.c
//
// Copyright 1998 Raven Software
//

#include "c_priestess2.h"
#include "c_priestess2_anim.h"
#include "c_priestess2_moves.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

// High priestess cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&priestess2_move_c_action1,
	&priestess2_move_c_action2,
	&priestess2_move_c_action3,
	&priestess2_move_c_action4,
	&priestess2_move_c_action5,
	&priestess2_move_c_action6,
	&priestess2_move_c_action7,
	&priestess2_move_c_idle1,
};

static void Priestess2CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'priestess2_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
	{
		case MSG_C_ACTION1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 40;
			curr_anim = ANIM_C_ACTION1;
			break;

		case MSG_C_ACTION2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 16;
			curr_anim = ANIM_C_ACTION2;
			break;

		case MSG_C_ACTION3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 158;
			curr_anim = ANIM_C_ACTION3;
			break;

		case MSG_C_ACTION4:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 96;
			curr_anim = ANIM_C_ACTION4;
			break;

		case MSG_C_ACTION5:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 158;
			curr_anim = ANIM_C_ACTION5;
			break;

		case MSG_C_ACTION6:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 72;
			curr_anim = ANIM_C_ACTION6;
			break;

		case MSG_C_ACTION7:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION7;
			break;

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

void Priestess2CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION1] = Priestess2CinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION2] = Priestess2CinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION3] = Priestess2CinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION4] = Priestess2CinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION5] = Priestess2CinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION6] = Priestess2CinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_ACTION7] = Priestess2CinematicActionMsgHandler;
	classStatics[CID_C_HIGHPRIESTESS2].msgReceivers[MSG_C_IDLE1] = Priestess2CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/highpriestess/pod_scene/tris.fm");

	classStatics[CID_C_HIGHPRIESTESS2].resInfo = &res_info;
}

// QUAKED character_highpriestess2(1 .5 0) (-24 -24 -36) (24 24 36)  INVISIBLE
// The cinematic High Priestess for the Pod Scene.
void SP_character_highpriestess2(edict_t* self)
{
	VectorSet(self->mins, -16.0f, -16.0f, -38.0f);
	VectorSet(self->maxs,  16.0f,  16.0f,  38.0f);

	CinematicCharacterInit(self, CID_C_HIGHPRIESTESS2);
}
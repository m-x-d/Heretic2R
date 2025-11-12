//
// c_siernan2.c
//
// Copyright 1998 Raven Software
//

#include "c_siernan2.h"
#include "c_siernan2_anim.h"
#include "c_siernan2_moves.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

// Siernan cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&siernan2_move_c_action1,
	&siernan2_move_c_action2,
	&siernan2_move_c_idle1,
};

static void Siernan2CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'siernan2_c_anims' in original logic.
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

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

void Siernan2CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_C_SIERNAN2].msgReceivers[MSG_C_ACTION1] = Siernan2CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN2].msgReceivers[MSG_C_ACTION2] = Siernan2CinematicActionMsgHandler;
	classStatics[CID_C_SIERNAN2].msgReceivers[MSG_C_IDLE1] = Siernan2CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/siernan/laying/tris.fm");

	classStatics[CID_C_SIERNAN2].resInfo = &res_info;
}

// QUAKED character_siernan2 (1 .5 0) (-17 -25 0) (22 12 16) INVISIBLE
// The cinematic siernan laying down.
void SP_character_siernan2(edict_t* self)
{
	VectorSet(self->mins, -17.0f, -25.0f, 0.0f);
	VectorSet(self->maxs,  22.0f,  12.0f, 16.0f);

	self->s.scale = 1.0f;
	self->monsterinfo.scale = 1.0f;

	CinematicCharacterInit(self, CID_C_SIERNAN2);
}
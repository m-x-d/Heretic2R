//
// c_morcalavin.c
//
// Copyright 1998 Raven Software
//

#include "c_morcalavin.h"
#include "c_morcalavin_anim.h"
#include "c_morcalavin_moves.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

// Morcalavin cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&morcalavin_move_c_action1,
	&morcalavin_move_c_action2,
	&morcalavin_move_c_action3,
	&morcalavin_move_c_idle1,
	&morcalavin_move_c_idle2,
	&morcalavin_move_c_idle3,
	&morcalavin_move_c_idle4,
};

static void MorcalavinCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'morcalavin_c_anims' in original logic.
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

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

void MorcalavinCinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_ACTION1] = MorcalavinCinematicActionMsgHandler;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_ACTION2] = MorcalavinCinematicActionMsgHandler;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_ACTION3] = MorcalavinCinematicActionMsgHandler;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_IDLE1] = MorcalavinCinematicActionMsgHandler;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_IDLE2] = MorcalavinCinematicActionMsgHandler;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_IDLE3] = MorcalavinCinematicActionMsgHandler;
	classStatics[CID_C_MORCALAVIN].msgReceivers[MSG_C_IDLE4] = MorcalavinCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/morcalavin/cinematic/tris.fm"); // Note that the name is different in the path.

	classStatics[CID_C_MORCALAVIN].resInfo = &res_info;
}


// QUAKED character_morcalavin (1 .5 0) (-24 -24 -50) (24 24 50) INVISIBLE
// The cinematic morcalavin.
void SP_character_morcalavin(edict_t* self)
{
	VectorSet(self->mins, -24.0f, -24.0f, -46.0f);
	VectorSet(self->maxs,  24.0f,  24.0f,  46.0f);

	CinematicCharacterInit(self, CID_C_MORCALAVIN);
}
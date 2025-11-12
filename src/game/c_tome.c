//
// c_tome.c
//
// Copyright 1998 Raven Software
//

#include "c_tome.h"
#include "c_tome_anim.h"
#include "c_tome_moves.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

// Tome of Power cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&tome_move_c_idle1,
	&tome_move_c_idle2,
};

static void TomeCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'tome_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
	{
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= (C_ANIM_REPEAT | C_ANIM_IDLE);
			curr_anim = ANIM_C_IDLE1;
			break;

		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

void TomeCinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_C_TOME].msgReceivers[MSG_C_IDLE1] = TomeCinematicActionMsgHandler;
	classStatics[CID_C_TOME].msgReceivers[MSG_C_IDLE2] = TomeCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/spells/book/tris.fm");

	classStatics[CID_C_TOME].resInfo = &res_info;
}

// QUAKED character_tome (1 .5 0) (-4 -8 -12) (4 8 12) INVISIBLE
// The talking tome of power (sounds like a hot babe).
void SP_character_tome(edict_t* self)
{
	VectorSet(self->mins, -4.0f, -8.0f, -12.0f);
	VectorSet(self->maxs,  4.0f,  8.0f,  12.0f);

	CinematicCharacterInit(self, CID_C_TOME);
}
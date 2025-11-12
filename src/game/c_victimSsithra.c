//
// c_victimSsithra.c
//
// Copyright 1998 Raven Software
//

#include "c_victimSsithra.h"
#include "c_victimSsithra_anim.h"
#include "c_victimSsithra_moves.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

// Ssithra victim cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&victimSsithra_move_c_action1,
	&victimSsithra_move_c_action2,
	&victimSsithra_move_c_action3,
	&victimSsithra_move_c_action4,
	&victimSsithra_move_c_action5,
	&victimSsithra_move_c_action6
};

static void VictimSsithraCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'victimSsithra_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
	{
		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= (C_ANIM_REPEAT | C_ANIM_IDLE);
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
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_ACTION6;
			break;

		default:
			assert(0); //mxd
			return; //mxd. 'break' in original version.
	}

	SetAnim(self, curr_anim);
}

void VictimSsithraCinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.
	static int sounds[NUM_SOUNDS]; //mxd. Made local static.

	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_IDLE1] = VictimSsithraCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION1] = VictimSsithraCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION2] = VictimSsithraCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION3] = VictimSsithraCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION4] = VictimSsithraCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION5] = VictimSsithraCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION6] = VictimSsithraCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/Ssithra/cinematics/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/plagueElf/pain1.wav");
	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_SSITHRA_VICTIM].resInfo = &res_info;
}

// QUAKED character_ssithra_victim (1 .5 0) (-40 -16 -2) (40 16 2) INVISIBLE
// The Ssithra Victim for use in the torture scene.
void SP_character_ssithra_victim(edict_t* self)
{
	VectorSet(self->mins, -40.0f, -16.0f, -2.0f);
	VectorSet(self->maxs,  40.0f,  16.0f,  2.0f);

	CinematicCharacterInit(self, CID_SSITHRA_VICTIM);
}
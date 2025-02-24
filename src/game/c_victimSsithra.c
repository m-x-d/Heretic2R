//
// c_victimSsithra.c
//
// Copyright 1998 Raven Software
//

#include "c_victimSsithra.h"
#include "c_victimSsithra_anim.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

typedef enum SoundID_e
{
	SND_PAIN1,
	NUM_SOUNDS
} SoundID_t;

// Ssithra victim cinematic actions.
static animmove_t* animations[NUM_ANIMS] =
{
	&victimSsithra_move_c_action1,
	&victimSsithra_move_c_action2,
	&victimSsithra_move_c_action3,
	&victimSsithra_move_c_action4,
	&victimSsithra_move_c_action5,
	&victimSsithra_move_c_action6
};

static void victimSsithra_c_anims(edict_t* self, G_Message_t* msg)
{
	int curr_anim;

	ai_c_readmessage(self, msg);
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

void VictimSsithraStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.
	static int sounds[NUM_SOUNDS]; //mxd. Made local static.

	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_IDLE1] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION1] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION2] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION3] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION4] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION5] = victimSsithra_c_anims;
	classStatics[CID_SSITHRA_VICTIM].msgReceivers[MSG_C_ACTION6] = victimSsithra_c_anims;

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

	c_character_init(self, CID_SSITHRA_VICTIM);
}
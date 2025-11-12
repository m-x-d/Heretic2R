//
// c_ssithrascout.c
//
// Copyright 1998 Raven Software
//

#include "c_ssithrascout.h"
#include "c_ssithrascout_anim.h"
#include "c_ssithrascout_moves.h"
#include "c_ai.h"
#include "Utilities.h"
#include "Vector.h"

// Ssithra scout cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&scout_move_c_action1,
	&scout_move_c_action2,
	&scout_move_c_action3,
	&scout_move_c_action4,
	&scout_move_c_action5,
	&scout_move_c_action6,
	&scout_move_c_action7,
	&scout_move_c_action8,
	&scout_move_c_action9,
	&scout_move_c_action10,
	&scout_move_c_death1,
	&scout_move_c_idle1,
	&scout_move_c_idle2,
	&scout_move_c_idle3,
};

static void SsithraScoutCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'scout_c_anims' in original logic.
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

		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH1;
			break;

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= (C_ANIM_REPEAT | C_ANIM_IDLE);
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

		default:
			assert(0); //mxd
			return; //mxd. 'break' in original version.
	}

	SetAnim(self, curr_anim);
}

void SsithraScoutCinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.
	static int sounds[NUM_SOUNDS]; //mxd. Made local static.

	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION1] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION2] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION3] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION4] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION5] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION6] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION7] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION8] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION9] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_ACTION10] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_DEATH1] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_IDLE1] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_IDLE2] = SsithraScoutCinematicActionMsgHandler;
	classStatics[CID_SSITHRA_SCOUT].msgReceivers[MSG_C_IDLE3] = SsithraScoutCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/ssithra/scout_scene/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/plagueElf/pain1.wav");
	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_SSITHRA_SCOUT].resInfo = &res_info;
}

// QUAKED character_ssithra_scout (1 .5 0) (-26 -16 -13) (26 16 13)
// The scout.
void SP_character_ssithra_scout(edict_t* self)
{
	VectorSet(self->mins, -26.0f, -16.0f, -13.0f);
	VectorSet(self->maxs,  26.0f,  16.0f,  13.0f);

	CinematicCharacterInit(self, CID_SSITHRA_SCOUT);
}
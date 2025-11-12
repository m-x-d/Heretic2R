//
// c_dranor.c
//
// Copyright 1998 Raven Software
//

#include "c_Dranor.h"
#include "c_dranor_anim.h"
#include "c_dranor_moves.h"
#include "c_ai.h"
#include "Vector.h"
#include "Utilities.h"

// Dranor cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&dranor_move_c_action1,
	&dranor_move_c_action2,
	&dranor_move_c_action3,
	&dranor_move_c_action4,
	&dranor_move_c_action5,
	&dranor_move_c_action6,
	&dranor_move_c_action7,
	&dranor_move_c_action8,
	&dranor_move_c_action9,
	&dranor_move_c_action10,
	&dranor_move_c_action11,
	&dranor_move_c_action12,
	&dranor_move_c_death1,
	&dranor_move_c_idle1,
	&dranor_move_c_idle2,
	&dranor_move_c_idle3,
};

static void DranorCinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'Dranor_c_anims' in original logic.
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

		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			self->health = 5;
			curr_anim = ANIM_C_DEATH1;
			self->svflags |= SVF_DEADMONSTER; // Doesn't block walking.
			self->takedamage = DAMAGE_YES;
			self->movetype = PHYSICSTYPE_STOP;
			self->solid = SOLID_BBOX;
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

		default:
			assert(0); //mxd
			return; //mxd. 'break' in original version.
	}

	SetAnim(self, curr_anim);
}

void DranorCinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.
	static int sounds[NUM_SOUNDS]; //mxd. Made local static.

	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION1] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION2] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION3] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION4] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION5] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION6] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION7] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION8] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION9] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION10] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION11] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_ACTION12] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_DEATH1] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_IDLE1] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_IDLE2] = DranorCinematicActionMsgHandler;
	classStatics[CID_DRANOR].msgReceivers[MSG_C_IDLE3] = DranorCinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/plaguelf/dranor/tris.fm");

	sounds[SND_PAIN1] = gi.soundindex("monsters/plagueElf/pain1.wav");
	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_SSITHRA_VICTIM].resInfo = &res_info;
	classStatics[CID_DRANOR].resInfo = &res_info;
}

// QUAKED character_dranor (1 .5 0) (-17 -25 -32) (22 12 32)  INVISIBLE
// The elf who talks like Sean Connery.
void SP_character_dranor(edict_t* self)
{
	VectorSet(self->mins, -16.0f, -16.0f, -32.0f);
	VectorSet(self->maxs,  16.0f,  16.0f,  32.0f);

	CinematicCharacterInit(self, CID_DRANOR);

	self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
	self->health = 30;
}
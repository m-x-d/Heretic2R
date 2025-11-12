//
// c_corvus8.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus8.h"
#include "c_corvus8_anim.h"
#include "c_corvus8_moves.h"
#include "c_ai.h"
#include "Utilities.h"

// Corvus8 cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&corvus8_move_c_action1,
	&corvus8_move_c_action2,
	&corvus8_move_c_action3,
	&corvus8_move_c_action4,
	&corvus8_move_c_action5,
	&corvus8_move_c_action6,
	&corvus8_move_c_action7,
	&corvus8_move_c_action8,
	&corvus8_move_c_action9,
	&corvus8_move_c_action10,
	&corvus8_move_c_action11,
	&corvus8_move_c_action12,
	&corvus8_move_c_action13,
	&corvus8_move_c_action14,
	&corvus8_move_c_action15,
	&corvus8_move_c_action16,
	&corvus8_move_c_action17,
	&corvus8_move_c_action18,
	&corvus8_move_c_action19,
	&corvus8_move_c_action20,

	&corvus8_move_c_idle1,
	&corvus8_move_c_idle2,
	&corvus8_move_c_walkstart,
	&corvus8_move_c_walk1,
	&corvus8_move_c_walk2,
	&corvus8_move_c_walk3,
	&corvus8_move_c_walkstop1,
	&corvus8_move_c_walkstop2,
	&corvus8_move_c_pivotleftgo,
	&corvus8_move_c_pivotleft,
	&corvus8_move_c_pivotleftstop,
	&corvus8_move_c_pivotrightgo,
	&corvus8_move_c_pivotright,
	&corvus8_move_c_pivotrightstop
};

static void Corvus8CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'corvus8_c_anims' in original logic.
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

		case MSG_C_ACTION17:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION17;
			break;

		case MSG_C_ACTION18:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION18;
			break;

		case MSG_C_ACTION19:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION19;
			break;

		case MSG_C_ACTION20:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ACTION20;
			break;

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;

		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE2;
			break;

		case MSG_C_PIVOTLEFTGO:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PIVOTLEFTGO;
			break;

		case MSG_C_PIVOTLEFT:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_PIVOTLEFT;
			break;

		case MSG_C_PIVOTLEFTSTOP:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PIVOTLEFTSTOP;
			break;

		case MSG_C_PIVOTRIGHTGO:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PIVOTRIGHTGO;
			break;

		case MSG_C_PIVOTRIGHT:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_PIVOTRIGHT;
			break;

		case MSG_C_PIVOTRIGHTSTOP:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PIVOTRIGHTSTOP;
			break;

		case MSG_C_WALKSTART:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTART;
			break;

		case MSG_C_WALK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK1;
			break;

		case MSG_C_WALK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK2;
			break;

		case MSG_C_WALK3:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK3;
			break;

		case MSG_C_WALKSTOP1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTOP1;
			break;

		case MSG_C_WALKSTOP2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_WALKSTOP2;
			break;

		default:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_IDLE1;
			break;
	}

	SetAnim(self, curr_anim);
}

void Corvus8CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION1] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION2] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION3] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION4] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION5] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION6] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION7] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION8] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION9] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION10] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION11] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION12] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION13] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION14] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION15] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION16] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION17] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION18] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION19] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_ACTION20] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_IDLE1] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_IDLE2] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALKSTART] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALK1] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALK2] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALK3] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALKSTOP1] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_WALKSTOP2] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTLEFTGO] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTLEFT] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTLEFTSTOP] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTRIGHTGO] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTRIGHT] = Corvus8CinematicActionMsgHandler;
	classStatics[CID_CORVUS8].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = Corvus8CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/player/siernan_scene/tris.fm"); // Note that the name is different in the path.

	classStatics[CID_CORVUS8].resInfo = &res_info;
}

// QUAKED character_corvus8 (1 .5 0) (-17 -25 -32) (22 12 32) INVISIBLE
// The cinematic corvus for the Siernan scenes.
void SP_character_corvus8(edict_t* self)
{
	CinematicCorvusInit(self, CID_CORVUS8);
}
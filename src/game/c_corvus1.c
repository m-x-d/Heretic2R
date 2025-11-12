//
// c_corvus1.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus1.h"
#include "c_corvus1_anim.h"
#include "c_corvus1_moves.h"
#include "c_ai.h"
#include "Utilities.h"

// Corvus1 cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&corvus_move_c_action1,
	&corvus_move_c_action2,
	&corvus_move_c_action3,
	&corvus_move_c_action4,
	&corvus_move_c_idle1,
	&corvus_move_c_idle2,
	&corvus_move_c_strafeleft,
	&corvus_move_c_straferight,
	&corvus_move_c_walkstart,
	&corvus_move_c_walk1,
	&corvus_move_c_walk2,
	&corvus_move_c_walkstop1,
	&corvus_move_c_walkstop2,
	&corvus_move_c_pivotleftgo,
	&corvus_move_c_pivotleft,
	&corvus_move_c_pivotleftstop,
	&corvus_move_c_pivotrightgo,
	&corvus_move_c_pivotright,
	&corvus_move_c_pivotrightstop,
};

static void Corvus1CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'corvus_c_anims' in original logic.
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

		case MSG_C_STEPLEFT:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_STRAFELEFT;
			break;
		case MSG_C_STEPRIGHT:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_STRAFERIGHT;
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

void Corvus1CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_CORVUS].msgReceivers[MSG_C_ACTION1] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_ACTION2] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_ACTION3] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_ACTION4] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_IDLE1] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_IDLE2] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_WALKSTART] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_WALK1] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_WALK2] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_PIVOTLEFTGO] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_PIVOTLEFT] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_PIVOTLEFTSTOP] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_PIVOTRIGHTGO] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_PIVOTRIGHT] = Corvus1CinematicActionMsgHandler;
	classStatics[CID_CORVUS].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = Corvus1CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/player/victim_scene/tris.fm");

	classStatics[CID_CORVUS].resInfo = &res_info;
}

// QUAKED character_corvus1 (1 .5 0) (-17 -25 -32) (22 12 32) INVISIBLE
// The cinematic corvus for the torture victim.
void SP_character_corvus1(edict_t* self)
{
	CinematicCorvusInit(self, CID_CORVUS);
}
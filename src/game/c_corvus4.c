//
// c_corvus4.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus4.h"
#include "c_corvus4_anim.h"
#include "c_corvus4_moves.h"
#include "c_ai.h"
#include "Utilities.h"

// Corvus4 cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&corvus4_move_c_action1,
	&corvus4_move_c_action2,
	&corvus4_move_c_action3,
	&corvus4_move_c_action4,
	&corvus4_move_c_action5,
	&corvus4_move_c_idle1,
	&corvus4_move_c_idle2,
	&corvus4_move_c_walkstart,
	&corvus4_move_c_walk1,
	&corvus4_move_c_walk2,
	&corvus4_move_c_walkstop1,
	&corvus4_move_c_walkstop2,
	&corvus4_move_c_pivotleftgo,
	&corvus4_move_c_pivotleft,
	&corvus4_move_c_pivotleftstop,
	&corvus4_move_c_pivotrightgo,
	&corvus4_move_c_pivotright,
	&corvus4_move_c_pivotrightstop
};

static void Corvus4CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'corvus4_c_anims' in original logic.
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

void Corvus4CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_CORVUS4].msgReceivers[MSG_C_ACTION1] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_ACTION2] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_ACTION3] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_ACTION4] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_ACTION5] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_IDLE1] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_IDLE2] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_WALKSTART] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_WALK1] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_WALK2] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_WALKSTOP1] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_WALKSTOP2] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_PIVOTLEFTGO] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_PIVOTLEFT] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_PIVOTLEFTSTOP] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_PIVOTRIGHTGO] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_PIVOTRIGHT] = Corvus4CinematicActionMsgHandler;
	classStatics[CID_CORVUS4].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = Corvus4CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/player/scout_scene/tris.fm");

	classStatics[CID_CORVUS4].resInfo = &res_info;
}

// QUAKED character_corvus4 (1 .5 0) (-16 -16 -34) (16 16 25) INVISIBLE
// The cinematic Corvus for the Scout scene.
void SP_character_corvus4(edict_t* self)
{
	CinematicCorvusInit(self, CID_CORVUS4);
}
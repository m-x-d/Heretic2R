//
// c_corvus2.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus2.h"
#include "c_corvus2_anim.h"
#include "c_corvus2_moves.h"
#include "c_ai.h"
#include "Utilities.h"

// Corvus2 cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&corvus2_move_c_action1,
	&corvus2_move_c_idle1,
	&corvus2_move_c_idle2,
	&corvus2_move_c_idle3,
	&corvus2_move_c_walkstart,
	&corvus2_move_c_walk1,
	&corvus2_move_c_walk2,
	&corvus2_move_c_walkstop1,
	&corvus2_move_c_walkstop2,
	&corvus2_move_c_pivotleftgo,
	&corvus2_move_c_pivotleft,
	&corvus2_move_c_pivotleftstop,
	&corvus2_move_c_pivotrightgo,
	&corvus2_move_c_pivotright,
	&corvus2_move_c_pivotrightstop
};

static void Corvus2CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'corvus2_c_anims' in original logic.
{
	int curr_anim;

	ReadCinematicMessage(self, msg);
	self->monsterinfo.c_anim_flag = 0;

	switch (msg->ID)
	{
		case MSG_C_ACTION1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			self->monsterinfo.c_dist = 64;
			curr_anim = ANIM_C_ACTION1;
			break;

		case MSG_C_IDLE1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE1;
			break;

		case MSG_C_IDLE2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE2;
			break;

		case MSG_C_IDLE3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT | C_ANIM_IDLE;
			curr_anim = ANIM_C_IDLE3;
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

void Corvus2CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_CORVUS2].msgReceivers[MSG_C_ACTION1] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_IDLE1] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_IDLE2] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_IDLE3] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALKSTART] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALK1] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALK2] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALKSTOP1] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_WALKSTOP2] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTLEFTGO] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTLEFT] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTLEFTSTOP] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTRIGHTGO] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTRIGHT] = Corvus2CinematicActionMsgHandler;
	classStatics[CID_CORVUS2].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = Corvus2CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/player/watcher_scene/tris.fm");

	classStatics[CID_CORVUS2].resInfo = &res_info;
}

// QUAKED character_corvus2 (1 .5 0) (-17 -25 -32) (22 12 32)  INVISIBLE
// The cinematic Corvus for the celestial watcher scene.
void SP_character_corvus2(edict_t* self)
{
	CinematicCorvusInit(self, CID_CORVUS2);
}
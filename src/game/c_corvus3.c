//
// c_corvus3.c
//
// Copyright 1998 Raven Software
//

#include "c_corvus3.h"
#include "c_corvus3_anim.h"
#include "c_corvus3_moves.h"
#include "c_ai.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"

// Corvus3 cinematic actions.
static const animmove_t* animations[NUM_ANIMS] =
{
	&corvus3_move_c_action1,
	&corvus3_move_c_action2,
	&corvus3_move_c_action3,
	&corvus3_move_c_action4,
	&corvus3_move_c_action5,
	&corvus3_move_c_action6,
	&corvus3_move_c_action7,
	&corvus3_move_c_action8,
	&corvus3_move_c_action9,
	&corvus3_move_c_action10,
	&corvus3_move_c_action11,
	&corvus3_move_c_action12,
	&corvus3_move_c_action13,
	&corvus3_move_c_action14,
	&corvus3_move_c_action15,
	&corvus3_move_c_action16,
	&corvus3_move_c_action17,
	&corvus3_move_c_action18,
	&corvus3_move_c_action19,
	&corvus3_move_c_action20,
	&corvus3_move_c_idle1,
	&corvus3_move_c_idle2,
	&corvus3_move_c_idle3,
	&corvus3_move_c_walkstart,
	&corvus3_move_c_walk1,
	&corvus3_move_c_walk2,
	&corvus3_move_c_walk3,
	&corvus3_move_c_walkstop1,
	&corvus3_move_c_walkstop2,
	&corvus3_move_c_pivotleftgo,
	&corvus3_move_c_pivotleft,
	&corvus3_move_c_pivotleftstop,
	&corvus3_move_c_pivotrightgo,
	&corvus3_move_c_pivotright,
	&corvus3_move_c_pivotrightstop
};

void corvus3_invisible(edict_t* self)
{
	self->s.modelindex = 0;
}

void corvus3_visible(edict_t* self)
{
	self->s.modelindex = (byte)self->count;
}

void corvus3_teleportsmalleffect(edict_t* self)
{
	vec3_t hold_origin = VEC3_INIT(self->s.origin);

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	float holdfor = flrand(5.0f, 10.0f); //mxd. int/irand() in original version.
	VectorMA(hold_origin, holdfor, forward, hold_origin);

	holdfor = flrand(-5.0f, 5.0f); //mxd. irand() in original version.
	VectorMA(hold_origin, holdfor, right, hold_origin);
	hold_origin[2] += flrand(-25.0f, 5.0f);

	gi.sound(self, CHAN_WEAPON, gi.soundindex("player/picup.wav"), 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(NULL, FX_PICKUP, 0, hold_origin, NULL);
}

void corvus3_teleporteffect(edict_t* self)
{
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, self->s.origin, NULL);
}

static void Corvus3CinematicActionMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'corvus3_c_anims' in original logic.
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

void Corvus3CinStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION1] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION2] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION3] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION4] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION5] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION6] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION7] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION8] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION9] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION10] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION11] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION12] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION13] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION14] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION15] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION16] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION17] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION18] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION19] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION20] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_IDLE1] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_IDLE2] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_IDLE3] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALKSTART] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALK1] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALK2] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALK3] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALKSTOP1] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALKSTOP2] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTLEFTGO] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTLEFT] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTLEFTSTOP] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTRIGHTGO] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTRIGHT] = Corvus3CinematicActionMsgHandler;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = Corvus3CinematicActionMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/player/highpriestess_scene/tris.fm");

	classStatics[CID_CORVUS3].resInfo = &res_info;
}

// QUAKED character_corvus3 (1 .5 0) (-17 -25 -32) (22 12 32) INVISIBLE
// The cinematic Corvus for the high priestess scene.
void SP_character_corvus3(edict_t* self)
{
	CinematicCorvusInit(self, CID_CORVUS3);
}
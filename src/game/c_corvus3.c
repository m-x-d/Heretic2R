/*-------------------------------------------------------------------
c_corvus.c

Heretic II
Copyright 1998 Raven Software

  AI:


-------------------------------------------------------------------*/
#include "g_local.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "fx.h"
#include "random.h"
#include "buoy.h"
#include "vector.h"

#include "c_corvus3.h"
#include "c_corvus3_anim.h"
#include "c_ai.h"

/*----------------------------------------------------------------------
  plagueElf Base Info
-----------------------------------------------------------------------*/
static animmove_t *animations[NUM_ANIMS] =
{
	// Cinematics
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

static ClassResourceInfo_t resInfo;

void corvus3_invisible (edict_t *self)
{
	self->s.modelindex = 0;
}

void corvus3_visible (edict_t *self)
{
	self->s.modelindex = self->count;
}

void corvus3_teleportsmalleffect (edict_t *self)
{
	vec3_t holdorigin,forward,right;
	int holdfor;

	VectorCopy(self->s.origin,holdorigin);

	AngleVectors(self->s.angles, forward, right, NULL);

	holdfor = irand(5,10);
	VectorMA(holdorigin, holdfor, forward, holdorigin);
	holdfor = irand(-5,5);
	VectorMA(holdorigin, holdfor, right, holdorigin);
	holdorigin[2] += flrand(-25,5);

	gi.sound(self, CHAN_WEAPON, gi.soundindex("player/picup.wav"), 1, ATTN_NORM, 0);
	gi.CreateEffect(NULL, FX_PICKUP, 0, holdorigin, NULL);
}

void corvus3_teleporteffect (edict_t *self)
{
	gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, self->s.origin, NULL);
}

/*-------------------------------------------------------------------------
	corvus3_c_anims
-------------------------------------------------------------------------*/
void corvus3_c_anims(edict_t *self, G_Message_t *msg)
{
	int int_msg;
	int curr_anim;

	ai_c_readmessage(self, msg);
	int_msg = (int) msg->ID;

	self->monsterinfo.c_anim_flag = 0; 

	switch(int_msg)
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


/*-------------------------------------------------------------------------
	PlagueElfStaticsInit
-------------------------------------------------------------------------*/
void Corvus3StaticsInit()
{
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION1] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION2] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION3] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION4] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION5] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION6] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION7] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION8] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION9] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION10] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION11] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION12] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION13] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION14] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION15] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION16] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION17] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION18] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION19] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_ACTION20] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_IDLE1] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_IDLE2] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_IDLE3] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALKSTART] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALK1] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALK2] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALK3] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALKSTOP1] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_WALKSTOP2] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTLEFTGO] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTLEFT] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTLEFTSTOP] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTRIGHTGO] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTRIGHT] = corvus3_c_anims;
	classStatics[CID_CORVUS3].msgReceivers[MSG_C_PIVOTRIGHTSTOP] = corvus3_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/player/highpriestess_scene/tris.fm");

	classStatics[CID_CORVUS3].resInfo = &resInfo;

}


/*QUAKED character_corvus3 (1 .5 0) (-17 -25 -32) (22 12 32) INVISIBLE
The cinematic Corvus for the high priestess scene
*/
void SP_character_corvus3 (edict_t *self)
{
	c_corvus_init(self,CID_CORVUS3);
}

//==============================================================================
//
// m_ogle.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "g_local.h"
#include "m_ogle.h"
#include "m_ogle_anim.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "Random.h"
#include "vector.h"
#include "fx.h"
#include "g_HitLocation.h"
#include "g_misc.h"
#include "c_ai.h"
#include "m_stats.h"

#define OGLE_WORKER_CHISEL 1
#define OGLE_WORKER_HAMMER 2
#define OGLE_WORKER_PICK   4

#define OF_PUSHING		1
#define OF_PICK_UP		4
#define OF_PICK_DOWN	8
#define OF_CHISEL_UP	16
#define OF_CHISEL_DOWN	32
#define OF_HAMMER_UP	64
#define OF_HAMMER_DOWN	128
#define OF_SONG_LEADER	256
#define	OF_CINEMATIC	8448

//#define OGLE_RADIUS 1000 //FIXME: Tweak this out, activation range


qboolean MG_CheckClearPathToEnemy( edict_t *self );
void MG_Pathfind(edict_t *self, qboolean check_clear_path);

static animmove_t *animations[NUM_ANIMS] =
{
	&ogle_move_walk1,
	&ogle_move_push1,
	&ogle_move_push2,
	&ogle_move_push3,
	&ogle_move_stand1,
	&ogle_move_work1,
	&ogle_move_work2,
	&ogle_move_work3,
	&ogle_move_work4,
	&ogle_move_work5,
	&ogle_move_pain1,
	&ogle_move_pain2,
	&ogle_move_pain3,
	&ogle_move_rest1_trans,
	&ogle_move_rest1_wipe,
	&ogle_move_rest1,
	&ogle_move_rest2_wipe,
	&ogle_move_rest3_wipe,
	&ogle_move_rest4_trans,
	&ogle_move_rest4_trans2,
	&ogle_move_rest4,
	&ogle_move_celebrate1,
	&ogle_move_celebrate2,
	&ogle_move_celebrate3_trans,
	&ogle_move_celebrate3,
	&ogle_move_celebrate4_trans,
	&ogle_move_celebrate4,
	&ogle_move_celebrate5_trans,
	&ogle_move_celebrate5,
	&ogle_move_charge1,
	&ogle_move_charge2,
	&ogle_move_charge3,
	&ogle_move_charge4,
	&ogle_move_charge5,
	&ogle_move_attack1,
	&ogle_move_attack2,
	&ogle_move_attack3,
	&ogle_move_death1,
	&ogle_move_death2,

	&ogle_c_move_action1,
	&ogle_c_move_action2,
	&ogle_c_move_action3,
	&ogle_c_move_action4,
	&ogle_c_move_action5,
	&ogle_c_move_action6,
	&ogle_c_move_action7,
	&ogle_c_move_action8,
	&ogle_c_move_action9,
	&ogle_c_move_action10,
	&ogle_c_move_action11,
	&ogle_c_move_action12,
	&ogle_c_move_action13,
	&ogle_c_move_action14,
	&ogle_c_move_action15,
	&ogle_c_move_attack1,
	&ogle_c_move_attack2,
	&ogle_c_move_attack3,
	&ogle_c_move_death1,
	&ogle_c_move_death2,
	NULL,
	&ogle_c_move_idle1,
	&ogle_c_move_idle2,
	&ogle_c_move_idle3,
	&ogle_c_move_idle4,
	&ogle_c_move_idle5,
	&ogle_c_move_idle6,
	&ogle_c_move_pain1,
	&ogle_c_move_pain2,
	&ogle_c_move_pain3,
	&ogle_move_rest1,
	&ogle_c_move_trans1,
	&ogle_c_move_trans2,
	&ogle_c_move_trans3,
	&ogle_c_move_trans4,
	&ogle_c_move_trans5,
	&ogle_c_move_trans6,
	&ogle_c_move_walk1,
	&ogle_c_move_walk2,
	&ogle_c_move_walk3,
	&ogle_c_move_walk4,

};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;

int MG_SetFirstBuoy(edict_t *self);
qboolean MG_WanderToNextBuoy(edict_t *self);

/*QUAKED obj_corpse_ogle (1 .5 0) (-30 -12 -2) (30 12 2) pushing pick_up pick_down chisel_up chisel_down hammer_up hammer_down 
A dead ogle.
---------- KEYS -----------------  
style - skin of ogle (default 0)
0 - damage skin
1 - normal skin
-------  FIELDS  ------------------
same as monster ogle
-----------------------------------
*/
void SP_obj_corpse_ogle(edict_t *self)
{
	self->s.origin[2] += 22.0;

	VectorSet(self->mins,-30,-30,-2);
	VectorSet(self->maxs,30,30,8);

	self->s.modelindex = gi.modelindex("models/monsters/ogle/tris.fm");

	self->s.frame = FRAME_deatha14;	//Ths is the reason the function can't be put in g_obj.c

	// Setting the skinnum correctly
	if (!self->style)
		self->s.skinnum = 1;
	else
		self->s.skinnum = 0;

	self->svflags |= SVF_DEADMONSTER;//doesn't block walking

	if (self->monsterinfo.ogleflags & OF_PUSHING)
	{
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_PICK_UP)
	{
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_PICK_DOWN)
	{
		SetAnim(self, ANIM_WORK5);
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_CHISEL_UP)
	{
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_HAMMER_UP)
	{
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_HAMMER_DOWN)
	{
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else
	{
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}

	ObjectInit(self,40,80,MAT_FLESH,SOLID_BBOX);
}

/*-------------------------------------------------------------------------
	ogle_c_anims
-------------------------------------------------------------------------*/
void ogle_c_anims(edict_t *self, G_Message_t *msg)
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
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION11;
			break;
		case MSG_C_ACTION12:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION12;
			break;
		case MSG_C_ACTION13:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION13;
			break;
		case MSG_C_ACTION14:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION14;
			break;
		case MSG_C_ACTION15:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_ACTION15;
			break;
		case MSG_C_ATTACK1:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_ATTACK1;
			break;
		case MSG_C_ATTACK2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK2;
			break;
		case MSG_C_ATTACK3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_ATTACK3;
			break;
		case MSG_C_DEATH1:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH1;
			break;
		case MSG_C_DEATH2:
			self->monsterinfo.c_anim_flag |= C_ANIM_DONE;
			curr_anim = ANIM_C_DEATH2;
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
		case MSG_C_IDLE4:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE4;
			break;
		case MSG_C_IDLE5:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE5;
			break;
		case MSG_C_IDLE6:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_IDLE6;
			break;
		case MSG_C_PAIN1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PAIN1;
			break;
		case MSG_C_PAIN2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PAIN2;
			break;
		case MSG_C_PAIN3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_PAIN3;
			break;
		case MSG_C_THINKAGAIN:			// Think for yourself, little ogle man
			self->monsterinfo.c_anim_flag = 0;
			curr_anim = ANIM_C_THINKAGAIN;
			self->monsterinfo.c_mode = 0;
			break;
		case MSG_C_TRANS1:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS1;
			break;
		case MSG_C_TRANS2:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS2;
			break;
		case MSG_C_TRANS3:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS3;
			break;
		case MSG_C_TRANS4:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS4;
			break;
		case MSG_C_TRANS5:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS5;
			break;
		case MSG_C_TRANS6:
			self->monsterinfo.c_anim_flag |= C_ANIM_REPEAT;
			curr_anim = ANIM_C_TRANS6;
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
		case MSG_C_WALK4:
			self->monsterinfo.c_anim_flag |= C_ANIM_MOVE;
			curr_anim = ANIM_C_WALK4;
			break;
		default:
			break;
	} 

	SetAnim(self, curr_anim);
}

void ogle_mood_think ( edict_t *self )
{
	qboolean	coward = self->monsterinfo.aiflags & AI_COWARD;
	vec3_t		v;
	float		len;
	qboolean	path = false;

	if (self->enemy && self->ai_mood == AI_MOOD_WANDER)
		self->ai_mood = AI_MOOD_PURSUE;

	if (!self->enemy)
	{
		if (self->targetEnt && self->targetEnt->health > 0)
		{
			if (self->targetEnt->health < SERAPH_HEALTH/2 || self->targetEnt->ai_mood == AI_MOOD_FLEE)
			{
				if (irand(0,1))
					gi.sound (self, CHAN_BODY, sounds[SND_ENRAGE1], 1, ATTN_NORM, 0);
				else
					gi.sound (self, CHAN_BODY, sounds[SND_ENRAGE2], 1, ATTN_NORM, 0);

				self->enemy = self->targetEnt;
				self->ai_mood = AI_MOOD_PURSUE;
				return;
			}
		}

		switch (self->ai_mood)
		{
		case AI_MOOD_NORMAL:
			if ( (irand(0,100) > 50) && (self->monsterinfo.attack_finished < level.time) )
			{
				self->ai_mood = AI_MOOD_REST;
				self->monsterinfo.attack_finished = level.time + 45;
			}
			break;
		}

		self->enemy = NULL;
		return;
	}

	if(self->monsterinfo.aiflags & AI_COWARD)
		self->ai_mood = AI_MOOD_FLEE;

	if(self->enemy)
		path = MG_CheckClearPathToEnemy(self);
	else
		path = false;

	if ((!path && self->enemy) || self->monsterinfo.aiflags & AI_NO_MELEE || self->ai_mood == AI_MOOD_FLEE)
	{
		MG_Pathfind(self, false);
		return;
	}
	else
	{
		self->monsterinfo.searchType = SEARCH_COMMON;

		if(self->enemy)
		{
			VectorSubtract (self->s.origin, self->enemy->s.origin, v);
			
			if (v[2] <= 40)
				v[2] = 0;

			len = VectorLength (v) - self->enemy->maxs[0];

			// Far enough to run after
			if ((len > self->melee_range) || (self->monsterinfo.aiflags & AI_FLEE))
				self->ai_mood = AI_MOOD_PURSUE;
			else	// Close enough to Attack 
			{
				self->ai_mood = AI_MOOD_ATTACK;
				self->ai_mood_flags |= AI_MOOD_FLAG_MELEE;
				self->ai_mood_flags &= ~AI_MOOD_FLAG_MISSILE;
			}
		}
	}
}

/*
==========================================================

	Ogle Helper functions

==========================================================
*/

//Cast off the tools which have oppressed the ogle people for centuries... or something
void ogle_cast_off_tools_of_oppression ( edict_t *self )
{
	int		throw_nodes = 0;

	if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
	{//Cast off the hammer and nail
		throw_nodes |= BPN_NAIL;
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		ThrowWeapon(self, &vec3_origin, throw_nodes, 0, 0);
		
		throw_nodes |= BPN_HAMMER | BPN_HANDLE;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, &vec3_origin, throw_nodes, 0, 0);
	}
	else if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
	{//Cast off the pick
		throw_nodes |= BPN_PICK | BPN_HANDLE;
		
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, &vec3_origin, throw_nodes, 0, 0);
	}
	else if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
	{//Cast off the hammer
		throw_nodes |= BPN_HAMMER | BPN_HANDLE;
		
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

		ThrowWeapon(self, &vec3_origin, throw_nodes, 0, 0);
	}

	self->monsterinfo.aiflags |= AI_NO_MELEE;
}

//The ogle's been yelled at or struck by the overlord, get back to work!
void ogle_use(edict_t *self, edict_t *other, edict_t *activator)
{
	switch (self->curAnimID)
	{
	case ANIM_REST1:
		SetAnim(self, ANIM_PAIN2);
		break;

	case ANIM_REST4:
		SetAnim(self, ANIM_PAIN3);
		break;

	default:
		SetAnim(self, ANIM_PAIN2);
		break;
	}

	self->ai_mood = AI_MOOD_NORMAL;
}

//When the ogle is spawned, he checks around to figure out who his tormentor is 
void ogle_init_overlord(edict_t *self)
{
	edict_t	*seraph = NULL;
	
	while((seraph = G_Find(seraph, FOFS(targetname), self->target)) != NULL)
	{
		if (seraph->classID != CID_SERAPH_OVERLORD)
			continue;
		
		//Restore what we lost from monsterstart()
		self->targetEnt = seraph;
		self->nextthink = level.time + 0.1;
		self->think = walkmonster_start_go;
		self->use = ogle_use;
		return;
	}		

	self->use = ogle_use;
	self->think = walkmonster_start_go;
	self->nextthink = level.time + 0.1;
}

//Ogle Singing Technology (tm) All Right Reserved
void ogle_sing (edict_t *self)
{
	edict_t *ogle = NULL;

	if(self->monsterinfo.awake)
		return;

	switch( self->noise_index )
	{
	case 0:
		while((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
		{
			if(!ogle->monsterinfo.awake)
				gi.sound(ogle, CHAN_VOICE, sounds[SND_CHORUS1], 0.25, ATTN_NORM, 0);
		}
		
		self->monsterinfo.jump_time = level.time + 16;
		break;

	case 1:
		gi.sound (self, CHAN_VOICE, sounds[SND_SOLO1], 1, ATTN_NORM, 0);
		self->monsterinfo.jump_time = level.time + 4;
		break;
	
	case 2:
		while((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
		{
			if(!ogle->monsterinfo.awake)
				gi.sound (ogle, CHAN_VOICE, sounds[SND_CHORUS2], 0.25, ATTN_NORM, 0);
		}
		self->monsterinfo.jump_time = level.time + 3;
		break;

	case 3:
		gi.sound (self, CHAN_VOICE, sounds[SND_SOLO2], 1, ATTN_NORM, 0);
		self->monsterinfo.jump_time = level.time + 4;
		break;

	case 4:
		while((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
		{
			if(!ogle->monsterinfo.awake)
				gi.sound (ogle, CHAN_VOICE, sounds[SND_CHORUS3], 0.25, ATTN_NORM, 0);
		}
		self->monsterinfo.jump_time = level.time + 4;
		break;
	}

	self->noise_index++;
	
	if (self->noise_index > 4)
		self->noise_index = 0;
}

//High level AI interface
void ogle_pause(edict_t *self)
{	
	edict_t	*ogle = NULL;
	int chance = irand(0,100);

	if ( (self->monsterinfo.ogleflags & OF_SONG_LEADER) && (self->monsterinfo.jump_time < level.time) && (!self->enemy) )
		ogle_sing(self);

	if (!self->enemy)
	{
		//If we're in pain, get back to work!
		if ( self->curAnimID == ANIM_PAIN2)
		{
			SetAnim(self, ANIM_WORK4);
		}
		else if ( self->curAnimID == ANIM_PAIN3)
		{
			SetAnim(self, ANIM_WORK4);
		}

		//Switch up the animation speeds
		if ( (self->curAnimID == ANIM_WORK1) || (self->curAnimID == ANIM_WORK2) ) 
		{
			if (irand(1,10) < 8)
			{
				SetAnim(self, ANIM_WORK1);
			}
			else
			{
				SetAnim(self, ANIM_WORK2);
			}
		}
	}

	if(self->mood_think)
		self->mood_think(self);

	switch (self->ai_mood)
	{
	case AI_MOOD_ATTACK:
		QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		break;
	
	case AI_MOOD_FLEE:
	case AI_MOOD_PURSUE:
	case AI_MOOD_NAVIGATE:
		if (self->enemy)
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);

		break;
	case AI_MOOD_WALK:
		QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
		break;
	case AI_MOOD_STAND:
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		break;

	case AI_MOOD_DELAY:
		break;

	case AI_MOOD_NORMAL:
		break;
	case AI_MOOD_REST:
		if ( (self->curAnimID != ANIM_REST1) && (self->curAnimID != ANIM_REST1_WIPE) && 
			 (self->curAnimID != ANIM_REST1_TRANS) && (self->curAnimID != ANIM_REST2_WIPE) && (self->curAnimID != ANIM_REST3_WIPE) )
		{
			if ( (self->curAnimID == ANIM_WORK4) || (self->curAnimID == ANIM_WORK5) )
			{
				if (chance < 30)
				{
					gi.sound (self, CHAN_BODY, sounds[SND_WIPE_BROW], 1, ATTN_IDLE, 0);
					SetAnim(self, ANIM_REST2_WIPE);
					self->ai_mood = AI_MOOD_NORMAL;
				}
				else if (chance < 60)
				{
					SetAnim(self, ANIM_REST1_TRANS);
				}
				else
				{
					SetAnim(self, ANIM_REST4_TRANS);
				}
			}
			else if (self->curAnimID == ANIM_WORK1 || self->curAnimID == ANIM_WORK2)
			{
				gi.sound (self, CHAN_BODY, sounds[SND_WIPE_BROW], 1, ATTN_IDLE, 0);
				SetAnim(self, ANIM_REST3_WIPE);
				self->ai_mood = AI_MOOD_NORMAL;
			}
		}
		break;

	default :
		break;
	}
}

//The ogle is resting, choose a few different possible things to do
void ogle_rest(edict_t *self)
{
	int chance = irand(0,100);

	ogle_pause(self);

	switch(self->curAnimID)
	{
	case ANIM_REST4_TRANS:
		SetAnim(self, ANIM_REST4);
		break;

	case ANIM_REST2_WIPE:
		SetAnim(self, ANIM_WORK4);
		break;

	case ANIM_REST3_WIPE:
		SetAnim(self, ANIM_WORK2);
		break;

	case ANIM_REST1_TRANS:
		SetAnim(self, ANIM_REST1);
		break;
	
	case ANIM_REST1:
		if (chance < 20)
			SetAnim(self, ANIM_REST1_WIPE);
		break;

	case ANIM_REST1_WIPE:
		SetAnim(self, ANIM_REST1);
		break;
	}
}

//Check to do damage with the ogle's weapon
void ogle_strike(edict_t *self)
{
	trace_t	trace;
	edict_t *victim;
	vec3_t	soff, eoff, mins, maxs, bloodDir, direction;
	float	damage = flrand(OGLE_DMG_MIN, OGLE_DMG_MAX);

	VectorSet(soff, 0,  16, 8);
	VectorSet(eoff, self->melee_range, 2,  8);
	
	VectorSet(mins, -4, -4, -4);
	VectorSet(maxs,  4,  4,  4);

	//Purposely backwards
	VectorSubtract(soff, eoff, bloodDir);
	VectorNormalize(bloodDir);

	victim = M_CheckMeleeLineHit(self, soff, eoff, mins, maxs, &trace, direction);	

	if (victim)
	{
		if (victim == self)
		{
			//Create a puff effect
			gi.CreateEffect(NULL, FX_SPARKS, CEF_FLAG6, trace.endpos, "d", bloodDir);
		}
		else
		{
			//Hurt whatever we were whacking away at
			if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
				gi.sound (self, CHAN_WEAPON, sounds[SND_HAMMER_FLESH], 1, ATTN_NORM, 0);
			else
				gi.sound (self, CHAN_WEAPON, sounds[SND_PICK_FLESH], 1, ATTN_NORM, 0);
				
			T_Damage(victim, self, self, direction, trace.endpos, bloodDir, damage, damage*2, 0,MOD_DIED);
		}
	}
	else
	{
		//Play swoosh sound?
	}
}

//Pick and choose from a wide assortment of happy, joyous dances
void ogle_celebrate(edict_t *self)
{
	int chance = irand(0,100);

	if (!irand(0, 10))
	{
		if (chance < 33)
			gi.sound (self, CHAN_VOICE, sounds[SND_CHEER1], 1, ATTN_IDLE, 0);
		else if (chance < 66)
			gi.sound (self, CHAN_VOICE, sounds[SND_CHEER2], 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_VOICE, sounds[SND_CHEER3], 1, ATTN_IDLE, 0);
	}

	switch (self->curAnimID)
	{
	case ANIM_CELEBRATE1:
		if (chance < 70)
			SetAnim(self, ANIM_CELEBRATE1);
		else if (chance < 80)
			SetAnim(self, ANIM_CELEBRATE5_TRANS);
		else
			SetAnim(self, ANIM_CELEBRATE2);
		break;

	case ANIM_CELEBRATE2:
		if (chance < 5)
			SetAnim(self, ANIM_CELEBRATE4_TRANS);
		else if (chance < 10)
			SetAnim(self, ANIM_CELEBRATE3_TRANS);
		else if (chance < 50)
			SetAnim(self, ANIM_CELEBRATE1);
		else if (chance < 80)
			SetAnim(self, ANIM_CELEBRATE5_TRANS);
		else
			SetAnim(self, ANIM_CELEBRATE2);
		break;

	case ANIM_CELEBRATE3_TRANS:
		SetAnim(self, ANIM_CELEBRATE3);
		break;

	case ANIM_CELEBRATE4_TRANS:
		SetAnim(self, ANIM_CELEBRATE4);
		break;

	case ANIM_CELEBRATE5_TRANS:
		SetAnim(self, ANIM_CELEBRATE5);
		break;

	case ANIM_CELEBRATE3:
		if (chance < 50)
			SetAnim(self, ANIM_CELEBRATE4);
		else if (chance < 60)
			SetAnim(self, ANIM_CELEBRATE2);
		else
			SetAnim(self, ANIM_CELEBRATE3);
		break;

	case ANIM_CELEBRATE4:
		if (chance < 50)
			SetAnim(self, ANIM_CELEBRATE3);
		else if (chance < 60)
			SetAnim(self, ANIM_CELEBRATE2);
		else
			SetAnim(self, ANIM_CELEBRATE4);
		break;

	case ANIM_CELEBRATE5:
		if (chance < 90)
			SetAnim(self, ANIM_CELEBRATE5);
		else if (chance < 95)
			SetAnim(self, ANIM_CELEBRATE1);
		else
			SetAnim(self, ANIM_CELEBRATE2);
		break;
	}
}

//Spawn the dust and debris, based on what anim the ogle is in
void ogle_pick_dust(edict_t *self)
{
	vec3_t	dustPos, vf, vr, vu;
	byte	type = 0;

	AngleVectors(self->s.angles, vf, vr, vu);
	VectorCopy(self->s.origin, dustPos);
	
	switch (self->curAnimID)
	{
	case ANIM_WORK3:
		VectorMA(dustPos, 38, vf, dustPos);
		VectorMA(dustPos, 6,  vr, dustPos);
		VectorMA(dustPos, -4,  vu, dustPos);
		VectorScale(vf, -1, vf);
		break;
		
	case ANIM_WORK5:
		VectorMA(dustPos, 42, vf, dustPos);
		VectorMA(dustPos, 2,  vr, dustPos);
		VectorMA(dustPos, -24,  vu, dustPos);
		VectorScale(vu, 1, vf);
		type = CEF_FLAG6;
		break;
	
	case ANIM_WORK4:
		VectorMA(dustPos, 42, vf, dustPos);
		VectorMA(dustPos, 4,  vr, dustPos);
		VectorMA(dustPos, 8,  vu, dustPos);
		VectorScale(vf, -1, vf);
		break;

	default:
		VectorMA(dustPos, 32, vf, dustPos);
		VectorMA(dustPos, 4,  vr, dustPos);
		VectorMA(dustPos, 22,  vu, dustPos);
		VectorScale(vf, -1, vf);
		break;
	}

	//Random chance to create a spark
	if (irand(0,20) < 1)
		gi.CreateEffect(NULL, FX_SPARKS, CEF_FLAG6, dustPos, "d", vu);

	gi.CreateEffect(NULL, FX_OGLE_HITPUFF, type, dustPos, "v", vf);

	//Check for the chisel and hammer
	if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
	{
		if (irand(0,1))
			gi.sound (self, CHAN_WEAPON, sounds[SND_SPIKE1], 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_WEAPON, sounds[SND_SPIKE2], 1, ATTN_IDLE, 0);
	}
	else if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
	{
		if (irand(0,1))
			gi.sound (self, CHAN_WEAPON, sounds[SND_PICK1], 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_WEAPON, sounds[SND_PICK2], 1, ATTN_IDLE, 0);
	}
	else
	{
		if (irand(0,1))
			gi.sound (self, CHAN_WEAPON, sounds[SND_HAMMER1], 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_WEAPON, sounds[SND_HAMMER2], 1, ATTN_IDLE, 0);
	}
}

/*
==========================================================

	Ogle Message functions

==========================================================
*/

void ogle_pain (edict_t *self, G_Message_t *msg)
{
	edict_t		*targ, *attacker;
	int			chance, damage, temp;
	qboolean	force_pain;

	chance = irand(0,100);
	
	ParseMsgParms(msg, "eeiii", &targ, &attacker, &force_pain, &damage, &temp);

//	if (attacker->client)
//	{
		self->mood_think = ogle_mood_think;

		/*
		if (chance < 95)
		{
			self->monsterinfo.aiflags |= AI_COWARD;
			self->ai_mood = AI_MOOD_FLEE;
			
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			return;
		}
		*/
		if(attacker && !infront(self, attacker))
			SetAnim(self, ANIM_PAIN3);
		else if (chance < 33)
			SetAnim(self, ANIM_PAIN1);
		else if (chance < 66)
			SetAnim(self, ANIM_PAIN2);
		else
			SetAnim(self, ANIM_PAIN3);
//	}

	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PAIN1, SND_PAIN2)], 1, ATTN_NORM, 0);
}

void ogle_dismember(edict_t *self, int damage, int HitLocation)
{
	int				throw_nodes = 0;
	vec3_t			gore_spot, right;
	qboolean dismember_ok = false;

	if(HitLocation & hl_MeleeHit)
	{
		dismember_ok = true;
		HitLocation &= ~hl_MeleeHit;
	}

	if(HitLocation<1)
		return;

	if(HitLocation>hl_Max)
		return;

	VectorCopy(vec3_origin,gore_spot);

	switch(HitLocation)
	{
	case hl_Head:
			self->s.fmnodeinfo[MESH__TORSO].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__TORSO].skin = self->s.skinnum+1;
		break;

	case hl_TorsoFront:
	case hl_TorsoBack:
			self->s.fmnodeinfo[MESH__TORSO].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__TORSO].skin = self->s.skinnum+1;
		break;

	case hl_ArmUpperLeft:
		if(self->s.fmnodeinfo[MESH__LUPARM].flags & FMNI_NO_DRAW)
			break;
		
		if (dismember_ok)
		{
			AngleVectors(self->s.angles,NULL,right,NULL);
			gore_spot[2]+=self->maxs[2]*0.3;
			VectorMA(gore_spot,-8,right,gore_spot);
			
			throw_nodes |= BPN_LUPARM;
			self->s.fmnodeinfo[MESH__LUPARM].flags |= FMNI_NO_DRAW;

			if (!(self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_L4ARM;
				self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_NO_DRAW;
			}

			if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_NAIL;
				self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
			}

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
		}
		else
		{
			self->s.fmnodeinfo[MESH__LUPARM].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__LUPARM].skin = self->s.skinnum+1;
		}

		break;

	case hl_ArmLowerLeft:
		if(self->s.fmnodeinfo[MESH__L4ARM].flags & FMNI_NO_DRAW)
			break;
		
		if (dismember_ok)
		{
			AngleVectors(self->s.angles,NULL,right,NULL);
			gore_spot[2]+=self->maxs[2]*0.3;
			VectorMA(gore_spot,-8,right,gore_spot);
			
			throw_nodes |= BPN_L4ARM;
			self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_NO_DRAW;

			if (!(self->s.fmnodeinfo[MESH__NAIL].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_NAIL;
				self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
			}

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);
		}
		else
		{
			self->s.fmnodeinfo[MESH__L4ARM].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__L4ARM].skin = self->s.skinnum+1;
		}

		break;

	case hl_ArmUpperRight:
		if(self->s.fmnodeinfo[MESH__RUPARM].flags & FMNI_NO_DRAW)
			break;
		
		if (dismember_ok)
		{
			AngleVectors(self->s.angles,NULL,right,NULL);
			gore_spot[2]+=self->maxs[2]*0.3;
			VectorMA(gore_spot,8,right,gore_spot);
			
			throw_nodes |= BPN_RUPARM;
			self->s.fmnodeinfo[MESH__RUPARM].flags |= FMNI_NO_DRAW;

			if (!(self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_R4ARM;
				self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_NO_DRAW;
			}

			if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_HAMMER;
				self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			}

			if (!(self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_HANDLE;
				self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			}

			if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_PICK;
				self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
			}

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

			self->monsterinfo.aiflags |= AI_NO_MELEE;
			self->monsterinfo.aiflags |= AI_COWARD;
		}
		else
		{
			self->s.fmnodeinfo[MESH__RUPARM].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__RUPARM].skin = self->s.skinnum+1;
		}

		break;

	case hl_ArmLowerRight:
		if(self->s.fmnodeinfo[MESH__R4ARM].flags & FMNI_NO_DRAW)
			break;
		
		if (dismember_ok)
		{
			AngleVectors(self->s.angles,NULL,right,NULL);
			gore_spot[2]+=self->maxs[2]*0.3;
			VectorMA(gore_spot,8,right,gore_spot);
			
			throw_nodes |= BPN_R4ARM;
			self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_NO_DRAW;

			if (!(self->s.fmnodeinfo[MESH__HAMMER].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_HAMMER;
				self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			}

			if (!(self->s.fmnodeinfo[MESH__HANDLE].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_HANDLE;
				self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			}

			if (!(self->s.fmnodeinfo[MESH__PICK].flags & FMNI_NO_DRAW))
			{
				throw_nodes |= BPN_PICK;
				self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
			}

			ThrowBodyPart(self, &gore_spot, throw_nodes, damage, 0);

			self->monsterinfo.aiflags |= AI_NO_MELEE;
			self->monsterinfo.aiflags |= AI_COWARD;
		}
		else
		{
			self->s.fmnodeinfo[MESH__R4ARM].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__R4ARM].skin = self->s.skinnum+1;
		}

		break;

	case hl_LegUpperLeft:
	case hl_LegLowerLeft:
			self->s.fmnodeinfo[MESH__LLEG].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__LLEG].skin = self->s.skinnum+1;
		break;

	case hl_LegUpperRight:
	case hl_LegLowerRight:
			self->s.fmnodeinfo[MESH__RLEG].flags |= FMNI_USE_SKIN;			
			self->s.fmnodeinfo[MESH__RLEG].skin = self->s.skinnum+1;
		break;
	}
}

void ogle_death_pain(edict_t *self, G_Message_t *msg)
{
	if(self->health <= -80) //gib death
	{
		BecomeDebris(self);
		return;
	}
	else if(msg)
		MG_parse_dismember_msg(self, msg);
}

void ogle_death(edict_t *self, G_Message_t *msg)
{
	edict_t	*targ, *inflictor, *attacker;
	float	damage;
	vec3_t	dVel, vf, yf;

	ParseMsgParms(msg, "eeei", &targ, &inflictor, &attacker, &damage);

	M_StartDeath(self, ANIM_DEATH1);
	
	ogle_cast_off_tools_of_oppression ( self );

	if (self->health < -80)
	{
		return;
	}
	else if (self->health < -10)
	{
		SetAnim(self, ANIM_DEATH2);
		
		VectorCopy(targ->velocity, vf);
		VectorNormalize(vf);

		VectorScale(vf, -1, yf);

		self->elasticity = 1.2;
		self->friction = 0.8;

		VectorScale(vf, 300, dVel);
		dVel[2] = irand(200,250);

		VectorCopy(dVel, self->velocity);
//		self->groundentity = NULL;
	}
	else
	{
		SetAnim(self, ANIM_DEATH1);
	}

	gi.sound (self, CHAN_BODY, sounds[SND_DEATH], 1, ATTN_NORM, 0);

}

//Get to work!
void ogle_work1(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_WORK1);
}

qboolean ogle_findtarget (edict_t *self)
{
	edict_t		*found = NULL;

	//take down weak overlords
	while(found = findradius(found, self->s.origin, 1024))
	{
		if(found->classID == CID_SERAPH_OVERLORD && found->health > 0 && (found->health<SERAPH_HEALTH/2 || found->ai_mood == AI_MOOD_FLEE))
		{
			self->enemy = found;
			FoundTarget(self, false);
			return true;
		}
	}

	/*//Used to go after other stuff and break it...
	found = NULL;
	//ok, search for utensils of their oppression
	while(found = findradius(found, self->s.origin, 512))
	{
		if(found->classID == CID_OBJECT)
		{
			if(found->takedamage && found->health > 0)
			{
				if(!strcmp(found->classname, "obj_minecart1")||
					!strcmp(found->classname, "obj_minecart2")||
					!strcmp(found->classname, "obj_minecart3")||
					!strcmp(found->classname, "obj_pick")||
					!strcmp(found->classname, "obj_gascan")||
					!strcmp(found->classname, "obj_barrel_metal")||
					!strcmp(found->classname, "obj_metalchunk1")||
					!strcmp(found->classname, "obj_metalchunk2")||
					!strcmp(found->classname, "obj_metalchunk3")||
					!strcmp(found->classname, "obj_pipe1")||
					!strcmp(found->classname, "obj_pipe2")||
					!strcmp(found->classname, "obj_pushcart")||
					!strcmp(found->classname, "obj_shovel")||
					!strcmp(found->classname, "obj_wheelbarrow")||
					!strcmp(found->classname, "obj_wheelbarrowdamaged"))
				{
					if(irand(0, 1))
					{
						if(visible(self, found))
						{
							self->enemy = found;
							FoundTarget(self, false);
							return true;
						}
					}
				}
			}
		}
	}*/
	
	found = NULL;
	//help out other ogles
	while(found = findradius(found, self->s.origin, 1024))
	{
		if(found->classID == CID_OGLE && found->health > 0 && found != self)
		{
			if(found->enemy)
			{
				if(found->enemy->health > 0)
				{
					{
						if(found->enemy->client)
							found->enemy = NULL;
						else
						{
							self->enemy = found->enemy;
							FoundTarget(self, false);
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}
//Classic melee function (fear the ogles)
void ogle_melee( edict_t *self, G_Message_t *msg )
{
	qboolean	ret;
	int chance = irand(0,4);

	if (!self->enemy)
		return;

	if(self->enemy->client)
		self->enemy = NULL;

	if (self->enemy->health <= 0)
	{
		ret = ogle_findtarget(self);

		if (!ret)
		{
			self->enemy = self->goalentity = NULL;
			self->ai_mood = AI_MOOD_PURSUE;

			ogle_cast_off_tools_of_oppression( self );

			switch( chance )
			{
			case 0:	SetAnim(self, ANIM_CELEBRATE1);	break;
			case 1:	SetAnim(self, ANIM_CELEBRATE2);	break;
			case 2:	SetAnim(self, ANIM_CELEBRATE3);	break;
			case 3:	SetAnim(self, ANIM_CELEBRATE4);	break;
			case 4:	SetAnim(self, ANIM_CELEBRATE5);	break;
			}

			return;
		}
	}

	SetAnim(self, ANIM_ATTACK1);
}

//do a little dance.. make a little.. oh, sorry
void ogle_stand1(edict_t *self, G_Message_t *msg)
{
	if(self->monsterinfo.awake)
	{
		int	change = irand(0,4);

		switch( change )
		{
		case 0:	SetAnim(self, ANIM_CELEBRATE1);	break;
		case 1:	SetAnim(self, ANIM_CELEBRATE2);	break;
		case 2:	SetAnim(self, ANIM_CELEBRATE3);	break;
		case 3:	SetAnim(self, ANIM_CELEBRATE4);	break;
		case 4:	SetAnim(self, ANIM_CELEBRATE5);	break;
		}
	}
}

//Classic run-attack function, who thought mortal combat could be so cute?
void ogle_run1(edict_t *self, G_Message_t *msg)
{
	trace_t		trace;
	vec3_t		start, end, mins;
	float		len;
	int			change = irand(0,4);
	qboolean	ret;

	if(self->enemy && self->enemy->client)
		self->enemy = NULL;

	if (self->enemy && self->enemy->health <= 0)
	{
		ret = ogle_findtarget(self);

		if (!ret)
		{
			self->enemy = self->goalentity = NULL;
			self->ai_mood = AI_MOOD_PURSUE;
			
			ogle_cast_off_tools_of_oppression( self );
	
			switch( change )
			{
			case 0:	SetAnim(self, ANIM_CELEBRATE1);	break;
			case 1:	SetAnim(self, ANIM_CELEBRATE2);	break;
			case 2:	SetAnim(self, ANIM_CELEBRATE3);	break;
			case 3:	SetAnim(self, ANIM_CELEBRATE4);	break;
			case 4:	SetAnim(self, ANIM_CELEBRATE5);	break;
			}
		}
		
		return;
	}
	else if (self->enemy)
	{
		len = M_DistanceToTarget(self, self->enemy);

		if (len < 40)	// close enough to swing, not necessarily hit						
		{
			SetAnim(self, ANIM_ATTACK1);		
		}
		else if (len < 100)	// close enough to swing, not necessarily hit						
		{
			VectorCopy(self->s.origin, start);
			VectorCopy(self->enemy->s.origin, end);
			start[2]+=self->viewheight;
			end[2]+=self->enemy->viewheight;

			VectorCopy(self->mins, mins);
			mins[2]+=self->maxs[0]/2;//because this guys's mins are 0

			gi.trace(start, mins, self->maxs, end, self, MASK_MONSTERSOLID,&trace);
			
			if(trace.ent==self->enemy)
			{
				if (irand(0,1))
					SetAnim(self, ANIM_ATTACK2);
				else
					SetAnim(self, ANIM_ATTACK3);
			}
			else
			{
				switch (change)
				{
				case 0:	SetAnim(self, ANIM_CHARGE1);	break;
				case 1:	SetAnim(self, ANIM_CHARGE2);	break;
				case 2:	SetAnim(self, ANIM_CHARGE3);	break;
				case 3:	SetAnim(self, ANIM_CHARGE4);	break;
				case 4:	SetAnim(self, ANIM_CHARGE5);	break;
				}
			}
		}
		else		
		{
			switch (self->curAnimID)
			{
			case ANIM_CHARGE1:
			case ANIM_CHARGE2:
			case ANIM_CHARGE3:
			case ANIM_CHARGE4:
			case ANIM_CHARGE5:
				break;
			
			default:
				switch (change)
				{
				case 0:	SetAnim(self, ANIM_CHARGE1);	break;
				case 1:	SetAnim(self, ANIM_CHARGE2);	break;
				case 2:	SetAnim(self, ANIM_CHARGE3);	break;
				case 3:	SetAnim(self, ANIM_CHARGE4);	break;
				case 4:	SetAnim(self, ANIM_CHARGE5);	break;
				}
				break;
			}
		}
	
		return;
	}
	
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void ogle_check_leadsong (edict_t *self)
{
	edict_t *ogle = NULL;

	while((ogle = G_Find(ogle, FOFS(target), self->target)) != NULL)
	{
		if(ogle->monsterinfo.ogleflags & OF_SONG_LEADER)
			return;
	}
	
	self->monsterinfo.ogleflags |= OF_SONG_LEADER;
}

void ogle_start_push (edict_t *self, edict_t *other, edict_t *activator)
{
	if(!irand(0, 2))
		SetAnim(self, ANIM_PUSH1);
	else if(irand(0, 1))
		SetAnim(self, ANIM_PUSH2);
	else
		SetAnim(self, ANIM_PUSH3);
}

qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);
void pitch_roll_for_slope (edict_t *forwhom, vec3_t *slope);
void ogle_push (edict_t *self, float dist)
{
	edict_t	*found = NULL;
	float	yaw;
	vec3_t	move, forward, endpos;
	trace_t	trace;
	qboolean	done = false;

	if(found = G_Find(found, FOFS(targetname), self->target))
	{
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(self->s.origin, 64, forward, endpos);
		gi.trace(self->s.origin, vec3_origin, vec3_origin, endpos, self, MASK_MONSTERSOLID, &trace);
		if(trace.ent && trace.ent == found)
		{
			yaw = self->s.angles[YAW]*M_PI*2 / 360;
			
			move[0] = cos(yaw)*dist;
			move[1] = sin(yaw)*dist;
			move[2] = 0;
			if(SV_movestep(found, move, true))
			{
				pitch_roll_for_slope(found, NULL);
				M_walkmove(self, self->s.angles[YAW], dist);
				return;
			}
			done = true;
		}
	}

	if(!done)
		SetAnim(self, ANIM_REST4_TRANS);
	else
	{
		gi.sound (self, CHAN_VOICE, sounds[SND_WIPE_BROW], 1, ATTN_IDLE, 0);
		SetAnim(self, ANIM_REST1_WIPE);
	}
	self->ai_mood = AI_MOOD_REST;
	self->mood_think = ogle_mood_think;
}

/*
==========================================================

	Ogle Spawn functions

==========================================================
*/

void OgleStaticsInit()
{
	classStatics[CID_OGLE].msgReceivers[MSG_STAND]		= ogle_stand1;
	classStatics[CID_OGLE].msgReceivers[MSG_RUN]		= ogle_run1;
	classStatics[CID_OGLE].msgReceivers[MSG_MELEE]		= ogle_melee;
	classStatics[CID_OGLE].msgReceivers[MSG_DISMEMBER]  = MG_parse_dismember_msg;
	classStatics[CID_OGLE].msgReceivers[MSG_DEATH]		= ogle_death;
	classStatics[CID_OGLE].msgReceivers[MSG_PAIN]		= ogle_pain;
	classStatics[CID_OGLE].msgReceivers[MSG_DEATH_PAIN]		= ogle_death_pain;

	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION1] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION2] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION3] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION4] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION5] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION6] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION7] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION8] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION9] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION10] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION11] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION12] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION13] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION14] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ACTION15] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ATTACK1] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ATTACK2] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_ATTACK3] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_DEATH1] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_DEATH2] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_GIB1] = ai_c_gib;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE1] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE2] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE3] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE4] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE5] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_IDLE6] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_PAIN1] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_PAIN2] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_PAIN3] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_THINKAGAIN] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS1] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS2] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS3] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS4] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS5] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_TRANS6] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_WALK1] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_WALK2] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_WALK3] = ogle_c_anims;
	classStatics[CID_OGLE].msgReceivers[MSG_C_WALK4] = ogle_c_anims;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/ogle/tris.fm");
	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	sounds[SND_PICK1]		=	gi.soundindex("monsters/ogle/pick1.wav");	
	sounds[SND_PICK2]		=	gi.soundindex("monsters/ogle/pick2.wav");	
	sounds[SND_SPIKE1]		=	gi.soundindex("monsters/ogle/spike1.wav");	
	sounds[SND_SPIKE2]		=	gi.soundindex("monsters/ogle/spike2.wav");	
	sounds[SND_HAMMER1]		=	gi.soundindex("monsters/ogle/hammer1.wav");	
	sounds[SND_HAMMER2]		=	gi.soundindex("monsters/ogle/hammer2.wav");	
	sounds[SND_PICK_FLESH]	=	gi.soundindex("monsters/ogle/pickflesh.wav");	
	sounds[SND_HAMMER_FLESH]=	gi.soundindex("monsters/ogle/hammerflesh.wav");	
	sounds[SND_WIPE_BROW]	=	gi.soundindex("monsters/ogle/wipebrow.wav");	
	sounds[SND_ENRAGE1]		=	gi.soundindex("monsters/ogle/enrage1.wav");	
	sounds[SND_ENRAGE2]		=	gi.soundindex("monsters/ogle/enrage2.wav");	
	sounds[SND_DEATH]		=	gi.soundindex("monsters/ogle/death.wav");	
	sounds[SND_CHEER1]		=	gi.soundindex("monsters/ogle/cheer1.wav");	
	sounds[SND_CHEER2]		=	gi.soundindex("monsters/ogle/cheer2.wav");	
	sounds[SND_CHEER3]		=	gi.soundindex("monsters/ogle/cheer3.wav");	

	sounds[SND_PAIN1]		=	gi.soundindex("monsters/ogle/oglemoan1.wav");
	sounds[SND_PAIN2]		=	gi.soundindex("monsters/ogle/oglemoan2.wav");
	
	//Singing
	sounds[SND_CHORUS1]		=	gi.soundindex("monsters/ogle/chorus1.wav");	
	sounds[SND_CHORUS2]		=	gi.soundindex("monsters/ogle/chorus3.wav");	
	sounds[SND_CHORUS3]		=	gi.soundindex("monsters/ogle/chorus5.wav");	
	sounds[SND_SOLO1]		=	gi.soundindex("monsters/ogle/solo2.wav");	
	sounds[SND_SOLO2]		=	gi.soundindex("monsters/ogle/solo4.wav");	

	classStatics[CID_OGLE].resInfo = &resInfo;
}

/*QUAKED monster_ogle(1 .5 0) (-16 -16 -24) (16 16 16) pushing pick_up pick_down chisel_up chisel_down hammer_up hammer_down singing CINEMATIC

The little, disgruntled Ogle

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

mintel - monster intelligence- this basically tells a monster how many buoys away an enemy has to be for it to give up.

melee_range - How close the player has to be, maximum, for the monster to go into melee.  If this is zero, the monster will never melee.  If it is negative, the monster will try to keep this distance from the player.  If the monster has a backup, he'll use it if too clode, otherwise, a negative value here means the monster will just stop running at the player at this distance.
	Examples:
		melee_range = 60 - monster will start swinging it player is closer than 60
		melee_range = 0 - monster will never do a mele attack
		melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close

missile_range - Maximum distance the player can be from the monster to be allowed to use it's ranged attack.

min_missile_range - Minimum distance the player can be from the monster to be allowed to use it's ranged attack.

bypass_missile_chance - Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot.  This, in effect, will make the monster come in more often than hang back and fire.  A percentage (0 = always fire/never close in, 100 = never fire/always close in).- must be whole number

jump_chance - every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time)- must be whole number

wakeup_distance - How far (max) the player can be away from the monster before it wakes up.  This just means that if the monster can see the player, at what distance should the monster actually notice him and go for him.

DEFAULTS:
mintel					= 16
melee_range				= 48
missile_range			= 0
min_missile_range		= 0
bypass_missile_chance	= 0
jump_chance				= 10
wakeup_distance			= 1024

NOTE: A value of zero will result in defaults, if you actually want zero as the value, use -1
*/
void SP_monster_ogle(edict_t *self)
{
	qboolean	skip_inits = false;
	edict_t		*found = NULL;
	int chance;

	if ((deathmatch->value == 1) && !((int)sv_cheats->value & self_spawn))
	{
		G_FreeEdict (self);
		return;
	}

	self->classID = CID_OGLE;

	self->monsterinfo.ogleflags = self->spawnflags;
	self->spawnflags = 0;//don't want incorrect handling due to weird ogle spawnflags!

	if (!monster_start(self))
		return;				// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->monsterinfo.alert = NULL;//can't be woken up
	self->monsterinfo.dismember = ogle_dismember;
	
	if (!self->health)
		self->health = OGLE_HEALTH;

	//Apply to the end result (whether designer set or not)
	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = OGLE_MASS;
	self->yaw_speed = 16;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	

	self->materialtype = MAT_FLESH;

	self->s.modelindex = classStatics[CID_OGLE].resInfo->modelIndex;
	self->s.skinnum=0;

	if (self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	self->monsterinfo.otherenemyname = "monster_rat";	

	MG_InitMoods(self);

	self->monsterinfo.aiflags |= AI_NO_ALERT;

	self->mood_think = ogle_mood_think;

	self->use = ogle_use;
	
	chance = irand(0,4);

	if (!self->monsterinfo.ogleflags)
	{
		switch (chance)
		{
		case 0:	self->monsterinfo.ogleflags |= OF_PICK_UP;
				break;
		case 1:	self->monsterinfo.ogleflags |= OF_PICK_DOWN;
				break;
		case 2:	self->monsterinfo.ogleflags |= OF_HAMMER_UP;
				break;
		case 3:	self->monsterinfo.ogleflags |= OF_HAMMER_DOWN;
				break;
		case 4:	self->monsterinfo.ogleflags |= OF_CHISEL_UP;
				break;
		}
	}
	
	self->monsterinfo.attack_finished = level.time + irand(10,100);	

	if (self->monsterinfo.ogleflags & OF_PUSHING)
	{
		//if (self->monsterinfo.ogleflags & OF_CINEMATIC)
		if(self->targetname && self->target)
		{
			if(found = G_Find(NULL, FOFS(targetname), self->target))
				pitch_roll_for_slope(found, NULL);

			skip_inits = true;

			self->mood_think = NULL;
			SetAnim(self, ANIM_REST4);
			self->use = ogle_start_push;

			self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		}
		else
		{	// We gotta do SOMETHING if there is no target, otherwise the monster will puke.
#ifdef _DEVEL
			gi.dprintf("Ogle at (%s) set to push with no target.\n",
					vtos(self->s.origin));
#endif
			SetAnim(self, ANIM_WORK3);
			self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
		}	
	}
	else if (self->monsterinfo.ogleflags & OF_PICK_UP)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK4);
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_PICK_DOWN)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
		SetAnim(self, ANIM_WORK5);
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_CHISEL_UP)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
		{
			if (irand(0,1))
				SetAnim(self, ANIM_WORK1);
			else
				SetAnim(self, ANIM_WORK2);
		}

		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_HAMMER_UP)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK4);
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else if (self->monsterinfo.ogleflags & OF_HAMMER_DOWN)
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK5);
		self->s.fmnodeinfo[MESH__NAIL].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}
	else
	{
		if (self->monsterinfo.ogleflags & OF_CINEMATIC)
			SetAnim(self, ANIM_C_IDLE1);
		else
			SetAnim(self, ANIM_WORK3);
		self->s.fmnodeinfo[MESH__PICK].flags |= FMNI_NO_DRAW;
	}

	if (self->monsterinfo.ogleflags & OF_CINEMATIC)
	{
		self->svflags|=SVF_FLOAT;
		self->monsterinfo.c_mode = 1;
	}

	self->svflags |= SVF_NO_AUTOTARGET;

	if(!skip_inits)
	{
		//Find out who our overlord is
		self->think = ogle_init_overlord;
		self->nextthink = level.time + 0.1;

		if(singing_ogles->value)
		{
			if(!(self->monsterinfo.ogleflags & OF_SONG_LEADER))
				ogle_check_leadsong(self);
		}

		if (self->monsterinfo.ogleflags & OF_SONG_LEADER)
			self->noise_index = 0;
	}
	else
	{
		self->think = walkmonster_start_go;
		self->nextthink = level.time + 0.1;
	}
}

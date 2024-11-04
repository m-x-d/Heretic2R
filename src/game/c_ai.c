//==============================================================================
//
// c_ai.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "g_local.h"
#include "g_DefaultMessageHandler.h"

#pragma hdrstop("g_local.pch")
// PRECOMPILED HEADER ABOVE
// WARNING:  DO NOT CHANGE THE ABOVE HEADERS OR THE PRECOMPILED STUFF WILL BREAK!  
// ADD ANY ADDITIONAL FILES BELOW
#include "fx.h"
#include "g_monster.h"
#include "vector.h"
#include "g_misc.h"
#include "c_corvus1_anim.h"
#include "p_main.h"



trace_t MG_MoveStep (edict_t *self, vec3_t move, qboolean relink);

#define	NUM_C_SOUNDS	2

static	char *c_wavname[NUM_C_SOUNDS] =
{
"",
"Cinematics/Dranor/19-316.wav",
};



void ai_c_wav (edict_t *self,float wav_num)
{
	gi.sound (self, CHAN_WEAPON, gi.soundindex(c_wavname[(int)wav_num]), 1, ATTN_NORM, 0);
}


void ai_c_readmessage(edict_t *self, G_Message_t *msg)
{
	int turning;
	int repeat;

	ParseMsgParms(msg, "iiige", &self->monsterinfo.c_dist,&turning,&repeat,
		&self->monsterinfo.c_callback,&self->monsterinfo.c_ent);

	self->monsterinfo.c_repeat = repeat;
	self->ideal_yaw = anglemod(self->s.angles[YAW] + turning);
}



// This is called at the end of each anim cycle
void ai_c_cycleend (edict_t *self)
{
	// A movement action that still has a distance to walk
	if ((self->monsterinfo.c_anim_flag & C_ANIM_MOVE) &&  ((self->monsterinfo.c_dist) || (self->s.angles[YAW] != self->ideal_yaw)))	 
		return;

	// A repeating action that still has to repeat
	if ((self->monsterinfo.c_anim_flag & C_ANIM_REPEAT) && (self->monsterinfo.c_repeat))
	{
		--self->monsterinfo.c_repeat;
		if (self->monsterinfo.c_repeat)
			return;
	}

	if (self->monsterinfo.c_anim_flag & C_ANIM_DONE)
	{
		self->nextthink = -1;
		self->think = NULL;

		if (self->monsterinfo.c_callback)	// Was a callback specified?
			self->monsterinfo.c_callback(self);

		return;
	}

	// This anim is all done
	if (self->monsterinfo.c_callback)	// Was a callback specified?
		self->monsterinfo.c_callback(self);
	else								// Well then just sit there if you aren't already
	{
		if (!(self->monsterinfo.c_anim_flag & C_ANIM_IDLE))	// 
			QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);
	} 
}


void ai_c_move (edict_t *self,float forward,float right,float up)
{
	vec3_t move;
	float yaw;
	float dist;

	dist = forward;

  	M_ChangeYaw(self);

	if (dist == 0)	//	Just standing there
	{
		return;
	}

	// Is the distance desired to move in the next frame past what it should be?
	if (Q_fabs(dist) > abs(self->monsterinfo.c_dist))
	{
		dist = self->monsterinfo.c_dist;
	}


	yaw = self->s.angles[YAW]*M_PI*2 / 360;
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	MG_MoveStep(self, move, true);

//	if (dist <0)
//		self->monsterinfo.c_dist += dist;
//	else
		self->monsterinfo.c_dist -= dist;

	// If this cycle gets stopped by finishing a distance, then kill it.
	if ((!self->monsterinfo.c_dist) && (self->monsterinfo.c_anim_flag & C_ANIM_MOVE))
		ai_c_cycleend(self);
}

void c_swapplayer(edict_t *Self,edict_t *Cinematic)
{
	int i;

	if (!Self->client)	// What are you trying to do?  Exchange a non-player for Corvus?
		return;

	if (Cinematic==NULL)
	{
//		gi.dprintf("Trying to swap Corvus for a non-existent cinematic version\n");
		return;
	}

	P_PlayerUpdateModelAttributes(&Self->client->playerinfo);

//	Cinematic->s.skinnum = Self->client->playerinfo.skinnum;
	// Add in plague level for the skin, since the cinematic players use six stock skins.
	Cinematic->s.skinnum = Self->client->playerinfo.skinnum + (Self->client->playerinfo.plaguelevel * DAMAGE_NUM_LEVELS);

	for (i=0;i<NUM_MESH_NODES;++i)
	{
		Cinematic->s.fmnodeinfo[i].flags =  Self->s.fmnodeinfo[i].flags;
		Cinematic->s.fmnodeinfo[i].skin = Self->s.fmnodeinfo[i].skin;
	}

	// Open up hands
	if (Cinematic->s.fmnodeinfo[MESH__RHANDHI].flags & FMNI_NO_DRAW)
		Cinematic->s.fmnodeinfo[MESH__RHANDHI].flags &= ~FMNI_NO_DRAW;

	if (Cinematic->s.fmnodeinfo[MESH__LHANDHI].flags & FMNI_NO_DRAW)
		Cinematic->s.fmnodeinfo[MESH__LHANDHI].flags &= ~FMNI_NO_DRAW;

	// If bow is active put it on his back
	if (!(Cinematic->s.fmnodeinfo[MESH__BOWACTV].flags & FMNI_NO_DRAW))
		Cinematic->s.fmnodeinfo[MESH__BOFF].flags &= ~FMNI_NO_DRAW;

	// If staff is active put it on his hip
	if (!(Cinematic->s.fmnodeinfo[MESH__STAFACTV].flags & FMNI_NO_DRAW))
		Cinematic->s.fmnodeinfo[MESH__STOFF].flags &= ~FMNI_NO_DRAW;


	// Get rid of all weapons in the hands
	Cinematic->s.fmnodeinfo[MESH__BLADSTF].flags |= FMNI_NO_DRAW;
	Cinematic->s.fmnodeinfo[MESH__HELSTF].flags |= FMNI_NO_DRAW;
	Cinematic->s.fmnodeinfo[MESH__BOWACTV].flags |= FMNI_NO_DRAW;
	Cinematic->s.fmnodeinfo[MESH__STAFACTV].flags |= FMNI_NO_DRAW;
}

#define ENT_INVISIBLE 1

vec3_t	c_mins = {-16, -16, -34};
vec3_t	c_maxs = {16, 16, 25};

void c_corvus_init(edict_t *self,int classId)
{
	self->classID = classId;
	self->s.modelindex = classStatics[classId].resInfo->modelIndex;

	if (!monster_start(self))		// Failed initialization
		return;
		
	self->msgHandler = DefaultMsgHandler;
	self->think = walkmonster_start_go;

	if (!self->health)
	{
		self->health = 30;
	}

	self->mass = 300;
	self->yaw_speed = 20;
	VectorClear(self->knockbackvel);
	
	VectorCopy (c_mins, self->mins);
	VectorCopy (c_maxs, self->maxs);
	VectorCopy (c_mins, self->intentMins);
	VectorCopy (c_maxs, self->intentMaxs);
	self->viewheight = self->maxs[2]*0.8;
	

	if (!self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = 1;
	}

	self->materialtype = MAT_FLESH;
	self->clipmask = MASK_MONSTERSOLID;
	self->count = self->s.modelindex;
	self->takedamage = DAMAGE_NO;

	if (self->spawnflags & ENT_INVISIBLE)
	{
		self->s.modelindex = 0;
		self->solid = SOLID_NOT;
		self->movetype = PHYSICSTYPE_NONE;
	}
	else
	{
		self->solid=SOLID_BBOX;
		self->movetype = PHYSICSTYPE_STEP;
	}

	//set up my mood function
	MG_InitMoods(self);
	VectorClear(self->knockbackvel);

	self->monsterinfo.c_mode = 1;
}


void c_character_init(edict_t *self,int classId)
{
	self->classID = classId;
	self->s.modelindex = classStatics[classId].resInfo->modelIndex;

	if (!monster_start(self))		
		return;						// Failed initialization
		
	self->msgHandler = DefaultMsgHandler;
	self->think = walkmonster_start_go;

	self->viewheight = self->maxs[2]*0.8;

	if (!self->health)
	{
		self->health = 30;
	}

	self->mass = 300;
	self->yaw_speed = 20;

	VectorClear(self->knockbackvel);
	
	if (!self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = 1;
	}

	self->count = self->s.modelindex;
	self->clipmask = MASK_MONSTERSOLID;
	self->materialtype = MAT_FLESH;
	self->takedamage = DAMAGE_NO;

	if (self->spawnflags & ENT_INVISIBLE)
	{
		self->s.modelindex = 0;
		self->solid = SOLID_NOT;
		self->movetype = PHYSICSTYPE_NONE;
	}
	else
	{
		self->solid=SOLID_BBOX;
		self->movetype = PHYSICSTYPE_STEP;
	}

	BboxYawAndScale(self);

	//set up my mood function
	MG_InitMoods(self);

	self->monsterinfo.c_mode = 1;
	QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige",0,0,0,NULL,NULL);

}

void ai_c_gib(edict_t *self, G_Message_t *msg)
{
	gi.sound(self, CHAN_BODY, gi.soundindex("monsters/plagueElf/gib2.wav"), 1, ATTN_NORM, 0);
	self->think = BecomeDebris;
	self->nextthink = level.time + 0.1;
}

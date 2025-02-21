//
// c_ai.c
//
// Copyright 1998 Raven Software
//

#include "c_ai.h" //mxd
#include "c_corvus1_anim.h"
#include "g_DefaultMessageHandler.h"
#include "g_misc.h"
#include "g_monster.h"
#include "mg_ai.h" //mxd
#include "FX.h"
#include "Vector.h"
#include "g_local.h"

void ai_c_readmessage(edict_t* self, G_Message_t* msg)
{
	int turning;
	int repeat;
	ParseMsgParms(msg, "iiige", &self->monsterinfo.c_dist, &turning, &repeat, &self->monsterinfo.c_callback, &self->monsterinfo.c_ent);

	self->monsterinfo.c_repeat = repeat;
	self->ideal_yaw = anglemod(self->s.angles[YAW] + (float)turning);
}

// This is called at the end of each anim cycle.
void ai_c_cycleend(edict_t* self)
{
	// A movement action that still has a distance to walk.
	if ((self->monsterinfo.c_anim_flag & C_ANIM_MOVE) && (self->monsterinfo.c_dist > 0 || self->s.angles[YAW] != self->ideal_yaw))
		return;

	// A repeating action that still has to repeat.
	if ((self->monsterinfo.c_anim_flag & C_ANIM_REPEAT) && self->monsterinfo.c_repeat > 0)
	{
		self->monsterinfo.c_repeat--;
		if (self->monsterinfo.c_repeat > 0)
			return;
	}

	if (self->monsterinfo.c_anim_flag & C_ANIM_DONE)
	{
		self->nextthink = -1;
		self->think = NULL;
	}

	// This anim is all done.
	if (self->monsterinfo.c_callback != NULL) // Was a callback specified?
	{
		self->monsterinfo.c_callback(self);
		return;
	}

	// Well then just sit there if you aren't already.
	if (!(self->monsterinfo.c_anim_flag & C_ANIM_IDLE))
		QPostMessage(self, MSG_C_IDLE1, PRI_DIRECTIVE, "iiige", 0, 0, 0, NULL, NULL);
}

void ai_c_move(edict_t* self, const float forward, float right, float up)
{
	M_ChangeYaw(self);

	if (forward == 0.0f) // Just standing there.
		return;

	// Is the distance desired to move in the next frame past what it should be?
	float dist = forward;
	if (Q_fabs(dist) > fabsf((float)self->monsterinfo.c_dist))
		dist = (float)self->monsterinfo.c_dist;

	const float yaw = self->s.angles[YAW] * ANGLE_180 * 2.0f / 360.0f;
	vec3_t move = { cosf(yaw) * dist, sinf(yaw) * dist, 0.0f };
	MG_MoveStep(self, move, true);

	self->monsterinfo.c_dist -= (int)dist;

	// If this cycle gets stopped by finishing a distance, then kill it.
	if (self->monsterinfo.c_dist == 0 && (self->monsterinfo.c_anim_flag & C_ANIM_MOVE))
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

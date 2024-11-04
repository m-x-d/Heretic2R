//==============================================================================
//
// m_bee.
//
// Heretic II
// Copyright 1998 Raven Software
//
//==============================================================================

#include "g_local.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "Random.h"
#include "vector.h"
#include "fx.h"
#include "g_HitLocation.h"
#include "g_misc.h"
#include "m_stats.h"

static ClassResourceInfo_t resInfo;

typedef enum SoundID_e
{
	SND_BUZZ1,
	SND_BUZZ2,
	SND_STING,
	SND_GIB,
	NUM_SOUNDS
} SoundID_t;

static int sounds[NUM_SOUNDS];
/*
==========================================================

	Bee Spawn functions

==========================================================
*/

void BeeStaticsInit()
{
//	classStatics[CID_BEE].msgReceivers[MSG_STAND]		= bee_stand1;
/*	classStatics[CID_BEE].msgReceivers[MSG_RUN]		= ogle_run1;
	classStatics[CID_BEE].msgReceivers[MSG_MELEE]		= ogle_melee;
	classStatics[CID_BEE].msgReceivers[MSG_DISMEMBER]  = ogle_dismember;
	classStatics[CID_BEE].msgReceivers[MSG_DEATH]		= ogle_death;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations; */
	resInfo.modelIndex = gi.modelindex("models/monsters/bee/tris.fm");

	sounds[SND_BUZZ1] = gi.soundindex ("monsters/bee/buzz1.wav");	
	sounds[SND_BUZZ2] = gi.soundindex ("monsters/bee/buzz2.wav");	
	sounds[SND_STING] = gi.soundindex ("monsters/bee/sting.wav");	
	sounds[SND_GIB] = gi.soundindex ("monsters/bee/gib.wav");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

}

/*QUAKED monster_bee (1 .5 0) (-16 -16 -24) (16 16 16) 
The bee
*/
void SP_monster_bee(edict_t *self)
{

	self->s.modelindex = gi.modelindex("models/monsters/bee/tris.fm");

	VectorSet(self->mins, -2, -2, -25);
	VectorSet(self->maxs, 2, 2, 25);

	ObjectInit(self,40,40,MAT_WOOD,SOLID_BBOX);

	return;

//	walkmonster_start(self);
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_BEE;

	if (!self->health)
		self->health = BEE_HEALTH;

	self->mass = BEE_MASS;
	self->yaw_speed = 32;

//	self->movetype = PHYSICSTYPE_STEP;
	self->solid=SOLID_BBOX;

	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, 16);

	self->materialtype = MAT_INSECT;

	self->s.modelindex = classStatics[CID_BEE].resInfo->modelIndex;
	self->s.skinnum=0;
/*
	if (self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}
*/
	self->monsterinfo.otherenemyname = "monster_rat";	

//	AI_SpawnGuide(self);

//	self->use = ogle_use;
	
}
//
// m_mother.c
//
// Copyright 1998 Raven Software
//

#include "m_mother.h"
#include "m_mother_shared.h"
#include "m_mother_anim.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"

#pragma region ========================== Tcheckrik Mother Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&mother_move_pain,
	&mother_move_stand,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

void mother_growl(edict_t* self)
{
	if (irand(0, 2) != 0)
		gi.sound(self, CHAN_BODY, sounds[irand(SND_GROWL1, SND_GROWL2)], 1.0f, ATTN_NORM, 0.0f);
}

static void MotherPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'mother_pain' in original logic.
{
	int temp;
	int damage;
	qboolean force_pain;
	ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + 1.0f;
		SetAnim(self, ANIM_PAIN);
	}

	gi.sound(self, CHAN_BODY, sounds[SND_PAIN], 1.0f, ATTN_NORM, 0.0f);
}

/*-------------------------------------------------------------------------
-------------------------------------------------------------------------*/
void mother_stand(edict_t *self, G_Message_t *msg)
{	
	SetAnim(self, ANIM_STAND);	

	return;
}

void mother_pause (edict_t *self)
{
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}


void mother_gib(edict_t *self, G_Message_t *msg)
{
	gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
	self->think = BecomeDebris;
	self->nextthink = level.time + 0.1;
}

/*-------------------------------------------------------------------------
	MotherStaticsInit
-------------------------------------------------------------------------*/
void MotherStaticsInit(void)
{
	static ClassResourceInfo_t resInfo;

	classStatics[CID_MOTHER].msgReceivers[MSG_STAND] = mother_stand;
	classStatics[CID_MOTHER].msgReceivers[MSG_PAIN] = MotherPainMsgHandler;
	classStatics[CID_MOTHER].msgReceivers[MSG_DEATH] = mother_gib;

	sounds[SND_GROWL1]=gi.soundindex("monsters/insect/growlf1.wav");	
	sounds[SND_GROWL2] = gi.soundindex ("monsters/insect/growlf2.wav");	
	sounds[SND_PAIN] = gi.soundindex("monsters/insect/painf.wav");	
	sounds[SND_GIB] = gi.soundindex("monsters/insect/gib.wav");

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	
	resInfo.modelIndex = gi.modelindex("models/monsters/mother/tris.fm");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_MOTHER].resInfo = &resInfo;
}

/*QUAKED monster_tcheckrik_mothers (1 .5 0) (-40 -40 -75) (40 40 75) 
Momma egg layer

"pain_target" - monsters will fire this target the first time it gets hurt (only once)
*/
void SP_monster_tcheckrik_mothers (edict_t *self)
{
	if (!M_Start(self))
		return;					// Failed initialization
		
	self->msgHandler = DefaultMsgHandler;
	self->think = M_WalkmonsterStartGo;

	if (!self->health)
	{
		self->health = PLAGUEELF_HEALTH;
	}

	self->mass = 2000;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STATIC;
	VectorClear(self->knockbackvel);
	
	self->solid=SOLID_BBOX;

	VectorSet (self->mins, -40, -40, -75);
	VectorSet (self->maxs,  40,  40,  75);	
	self->viewheight = self->maxs[2]*0.8;
	
	self->s.modelindex = classStatics[CID_MOTHER].resInfo->modelIndex;

	if (!self->s.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	self->materialtype = MAT_INSECT;

	self->svflags |= SVF_WAIT_NOTSOLID;

}


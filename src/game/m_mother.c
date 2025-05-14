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

#pragma region ========================== Message handlers ==========================

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

static void MotherStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'mother_stand' in original logic.
{
	SetAnim(self, ANIM_STAND);
}

static void MotherDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'mother_gib' in original logic.
{
	gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);

	self->think = BecomeDebris;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void mother_growl(edict_t* self)
{
	if (irand(0, 2) != 0)
		gi.sound(self, CHAN_BODY, sounds[irand(SND_GROWL1, SND_GROWL2)], 1.0f, ATTN_NORM, 0.0f);
}

void mother_pause(edict_t* self)
{
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

#pragma endregion

void MotherStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_MOTHER].msgReceivers[MSG_STAND] = MotherStandMsgHandler;
	classStatics[CID_MOTHER].msgReceivers[MSG_PAIN] = MotherPainMsgHandler;
	classStatics[CID_MOTHER].msgReceivers[MSG_DEATH] = MotherDeathMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;

	res_info.modelIndex = gi.modelindex("models/monsters/mother/tris.fm");

	sounds[SND_GROWL1] = gi.soundindex("monsters/insect/growlf1.wav");
	sounds[SND_GROWL2] = gi.soundindex("monsters/insect/growlf2.wav");
	sounds[SND_PAIN] = gi.soundindex("monsters/insect/painf.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/insect/gib.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_MOTHER].resInfo = &res_info;
}

// QUAKED monster_tcheckrik_mothers (1 .5 0) (-40 -40 -75) (40 40 75)
// Momma egg layer.
// pain_target - Monsters will fire this target the first time it gets hurt (only once).
void SP_monster_tcheckrik_mothers(edict_t* self)
{
	if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
		return; // Failed initialization.

	self->msgHandler = DefaultMsgHandler;

	if (self->health == 0)
		self->health = PLAGUEELF_HEALTH; //TODO: Add separate define? //TODO: Also call MonsterHealth()?

	self->mass = 2000;
	self->yaw_speed = 20.0f;

	self->movetype = PHYSICSTYPE_STATIC;
	VectorClear(self->knockbackvel);

	self->solid = SOLID_BBOX;
	self->materialtype = MAT_INSECT;
	self->svflags |= SVF_WAIT_NOTSOLID;

	VectorSet(self->mins, -40.0f, -40.0f, -75.0f); //TODO: init via STDMinsForClass?
	VectorSet(self->maxs, 40.0f, 40.0f, 75.0f); //TODO: init via STDMaxsForClass?
	self->viewheight = (int)(self->maxs[2] * 0.8f);

	self->s.modelindex = (byte)classStatics[CID_MOTHER].resInfo->modelIndex;

	if (self->s.scale == 0.0f)
	{
		self->s.scale = MODEL_SCALE;
		self->monsterinfo.scale = self->s.scale;
	}
}
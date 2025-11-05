//
// m_chicken.c
//
// Copyright 1998 Raven Software
//

#include "m_chicken.h"
#include "m_chicken_anim.h"
#include "m_chicken_shared.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_Physics.h"
#include "m_stats.h"
#include "mg_guide.h" //mxd
#include "spl_morph.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#pragma region ========================== Chicken base info ==========================

static const animmove_t* animations[NUM_ANIMS] =
{
	&chicken_move_stand1,
	&chicken_move_walk,
	&chicken_move_run,
	&chicken_move_cluck,
	&chicken_move_attack,
	&chicken_move_eat,
	&chicken_move_jump,
};

static int sounds[NUM_SOUNDS];

#pragma endregion

#pragma region ========================== Utility functions ==========================

// Fade the original monster back in again.
static void MorphOriginalIn(edict_t* self)
{
	self->s.color.a += MORPH_TELE_FADE;
	self->nextthink = level.time + FRAMETIME;

	if (--self->morph_timer == 0)
		self->think = self->oldthink;
}

// Fade out the existing model till its gone.
static void MorphChickenOut(edict_t* self)
{
	self->s.color.a -= MORPH_TELE_FADE;
	self->nextthink = level.time + FRAMETIME;

	if (self->morph_timer < (int)level.time)
	{
		// Create the original bad guy.
		edict_t* new_ent = G_Spawn();

		new_ent->classname = self->morph_classname;
		VectorCopy(self->s.origin, new_ent->s.origin);
		VectorCopy(self->s.angles, new_ent->s.angles);
		new_ent->enemy = self->enemy;

		ED_CallSpawn(new_ent);

		new_ent->s.color.c = 0xffffff;
		new_ent->s.frame = (short)self->morph_animation_frame; //mxd. Restore animation frame (stored in MonsterMorphFadeOut()).
		new_ent->oldthink = new_ent->think;
		new_ent->think = MorphOriginalIn;
		new_ent->nextthink = level.time + FRAMETIME; //mxd. Not set in original logic (expected monster spawn function to set it?).
		new_ent->morph_timer = MORPH_TELE_TIME;
		new_ent->target = self->target;

		// Make physics expand the bounding box for us, so the player doesn't get stuck inside the new bad guys box.

		// Store the old mins max's
		VectorCopy(new_ent->mins, new_ent->intentMins);
		VectorCopy(new_ent->maxs, new_ent->intentMaxs);

		VectorCopy(self->mins, new_ent->mins);
		VectorCopy(self->maxs, new_ent->maxs);

		new_ent->physicsFlags |= PF_RESIZE;

		// Do the teleport sound and effect.
		gi.sound(new_ent, CHAN_WEAPON, gi.soundindex("weapons/teleport.wav"), 1.0f, ATTN_NORM, 0.0f);
		gi.CreateEffect(&new_ent->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, NULL, "");

		G_FreeEdict(self);
	}
}

#pragma endregion

#pragma region ========================== Message handlers ==========================

// Decide which standing animations to use.
static void ChickenStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'chicken_stand' in original logic.
{
	SetAnim(self, ANIM_STAND1);
}

// Choose a walk animation to use.
static void ChickenWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'chicken_walk' in original logic.
{
	SetAnim(self, ANIM_WALK);
}

// Choose a run animation to use.
static void ChickenRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'chicken_run' in original logic.
{
	SetAnim(self, ANIM_RUN);
}

// Chicken attack - peck us to death.
static void ChickenAttackMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'chicken_attack' in original logic.
{
	SetAnim(self, ANIM_ATTACK);
}

static void ChickenDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'chicken_death' in original logic.
{
	self->msgHandler = DeadMsgHandler;

	gi.sound(self, CHAN_BODY, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
	self->dead_state = DEAD_DEAD;

	BecomeDebris(self);
	gi.CreateEffect(&self->s, FX_CHICKEN_EXPLODE, CEF_OWNERS_ORIGIN, NULL, "");
}

static void ChickenCluckMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'chicken_cluck' in original logic.
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_CLUCK1, SND_CLUCK2)], 1.0f, ATTN_NORM, 0.0f);
	SetAnim(self, ANIM_CLUCK);
}

static void ChickenEatMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'chicken_eat' in original logic.
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_PECK1, SND_PECK2)], 1.0f, ATTN_NORM, 0.0f);
	SetAnim(self, ANIM_EAT);
}

static void ChickenJumpMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'chicken_jump' in original logic.
{
	gi.sound(self, CHAN_VOICE, sounds[irand(SND_JUMP1, SND_JUMP3)], 1.0f, ATTN_NORM, 0.0f);
	SetAnim(self, ANIM_JUMP);
}

#pragma endregion

#pragma region ========================== Action functions ==========================

// Check if its time to return to our original shape.
void chicken_check_unmorph(edict_t* self)
{
	// Are we done yet?
	if (self->time > level.time)
		return;

	// Make that pretty effect around us.
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN, NULL, "");

	// Deal with the existing chicken.
	self->think = MorphChickenOut;
	self->nextthink = level.time + FRAMETIME;
	self->touch = NULL;
	self->morph_timer = MORPH_TELE_TIME;

	VectorClear(self->velocity);
}

// In Soviet Russia, chicken bites YOU!
void chicken_bite(edict_t* self)
{
	// In case we try pecking at someone that's not there.
	if (self->enemy == NULL)
		return;

	vec3_t diff;
	VectorSubtract(self->s.origin, self->enemy->s.origin, diff);

	// Determine if we've actually bitten the player, or just missed.
	if (VectorLength(diff) <= self->maxs[0] + self->enemy->maxs[0] + 24.0f)	// A hit.
	{
		vec3_t point;
		const vec3_t offset = { 20.0f, 0.0f, 5.0f };
		VectorGetOffsetOrigin(offset, self->s.origin, self->s.angles[YAW], point);

		T_Damage(self->enemy, self, self, NULL, point, vec3_origin, 1, 0, 0, MOD_DIED);
		gi.sound(self, CHAN_VOICE, sounds[irand(SND_BITE1, SND_BITE2)], 1.0f, ATTN_NORM, 0.0f);
	}
}

void chicken_pause(edict_t* self)
{
	self->mood_think(self);

	if (self->ai_mood == AI_MOOD_NORMAL)
	{
		if (FindTarget(self))
		{
			vec3_t diff;
			VectorSubtract(self->s.origin, self->enemy->s.origin, diff);

			if (VectorLength(diff) > 60.0f || (self->monsterinfo.aiflags & AI_FLEE)) // Far enough to run after.
				G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			else // Close enough to attack.
				G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		}

		return;
	}

	// If we can attack, then we do that, no question.
	if (self->ai_mood == AI_MOOD_ATTACK)
	{
		G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		return;
	}

	// Decide if we will do a random action.
	const int random_action = irand(0, 10);

	// A chance to cluck.
	if (random_action == 0)
	{
		G_PostMessage(self, MSG_WATCH, PRI_DIRECTIVE, NULL);
		return;
	}

	// A chance to peck.
	if (random_action == 1)
	{
		G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
		return;
	}

	// Otherwise run or track the player target.
	switch (self->ai_mood)
	{
		case AI_MOOD_PURSUE:
			G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_NAVIGATE:
			G_PostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_STAND:
			G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
			break;

		case AI_MOOD_JUMP:
			VectorCopy(self->movedir, self->velocity);
			VectorNormalize(self->movedir);
			SetAnim(self, ANIM_JUMP);
			break;

		case AI_MOOD_EAT:
			G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
			break;

		default:
			gi.dprintf("Unhandled chicken mood %d!\n", self->ai_mood);
			break;
	}
}

// Chicken eats again, possibly. Can happen after clucking too.
void chicken_eat_again(edict_t* self)
{
	// One in three chance we will peck again :) //TODO: 2 in 3 chances, actually. Should change?
	if (irand(0, 2) != 0)
		G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
	else
		chicken_pause(self);
}

void chicken_sound(edict_t* self, float channel, float sound_index, float attenuation)
{
	gi.sound(self, (int)channel, sounds[(int)sound_index], 1.0f, attenuation, 0.0f);
}

#pragma endregion

void ChickenStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

	classStatics[CID_CHICKEN].msgReceivers[MSG_STAND] = ChickenStandMsgHandler;
	classStatics[CID_CHICKEN].msgReceivers[MSG_WALK] = ChickenWalkMsgHandler;
	classStatics[CID_CHICKEN].msgReceivers[MSG_RUN] = ChickenRunMsgHandler;
	classStatics[CID_CHICKEN].msgReceivers[MSG_MELEE] = ChickenAttackMsgHandler;
	classStatics[CID_CHICKEN].msgReceivers[MSG_DEATH] = ChickenDeathMsgHandler;
	classStatics[CID_CHICKEN].msgReceivers[MSG_WATCH] = ChickenCluckMsgHandler;
	classStatics[CID_CHICKEN].msgReceivers[MSG_EAT] = ChickenEatMsgHandler;
	classStatics[CID_CHICKEN].msgReceivers[MSG_JUMP] = ChickenJumpMsgHandler;

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/chicken2/tris.fm");

	// For the cluck animation.
	sounds[SND_CLUCK1] = gi.soundindex("monsters/chicken/cluck1.wav");
	sounds[SND_CLUCK2] = gi.soundindex("monsters/chicken/cluck2.wav");

	// For getting hit - even though right now, it dies immediately - they want this changed.
	//sounds[SND_PAIN1] = gi.soundindex("monsters/chicken/pain1.wav"); //mxd. Unused.
	//sounds[SND_PAIN2] = gi.soundindex("monsters/chicken/pain2.wav"); //mxd. Unused.

	// For dying - we only ever get gibbed, so no other sound is required.
	sounds[SND_DIE] = gi.soundindex("monsters/chicken/die.wav");

	// For biting the player.
	sounds[SND_BITE1] = gi.soundindex("monsters/chicken/bite1.wav");
	sounds[SND_BITE2] = gi.soundindex("monsters/chicken/bite2.wav");

	// For pecking the ground.
	sounds[SND_PECK1] = gi.soundindex("monsters/chicken/peck1.wav");
	sounds[SND_PECK2] = gi.soundindex("monsters/chicken/peck2.wav");

	// And lastly, I thought it might be cool to have some cries for when the chicken jumps.
	sounds[SND_JUMP1] = gi.soundindex("monsters/chicken/jump1.wav");
	sounds[SND_JUMP2] = gi.soundindex("monsters/chicken/jump2.wav");
	sounds[SND_JUMP3] = gi.soundindex("monsters/chicken/jump3.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_CHICKEN].resInfo = &res_info;
}

// QUAKED monster_chicken (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP EATING
// The chicken.
// wakeup_target	- Monsters will fire this target the first time it wakes up (only once).
// pain_target		- Monsters will fire this target the first time it gets hurt (only once).
void SP_monster_chicken(edict_t* self)
{
	// Generic monster initialization.
	if (!M_WalkmonsterStart(self)) //mxd. M_Start -> M_WalkmonsterStart.
		return; // Failed initialization.

	self->msgHandler = DefaultMsgHandler;
	self->materialtype = MAT_FLESH;

	self->health = CHICKEN_HEALTH;
	self->mass = CHICKEN_MASS;
	self->yaw_speed = 20.0f;

	self->movetype = PHYSICSTYPE_STEP;
	self->solid = SOLID_BBOX;
	VectorClear(self->knockbackvel);

	VectorSet(self->mins, -12.0f, -12.0f, -16.0f);
	VectorSet(self->maxs,  12.0f,  12.0f,  16.0f);

	self->s.modelindex = (byte)classStatics[CID_CHICKEN].resInfo->modelIndex;
	self->s.skinnum = 0;
	self->monsterinfo.scale = MODEL_SCALE;
	self->monsterinfo.otherenemyname = "obj_barrel"; //TODO: why?

	MG_InitMoods(self);
	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	gi.linkentity(self);
}
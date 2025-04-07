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
static ClassResourceInfo_t res_info;

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

		new_ent->classname = self->map;
		VectorCopy(self->s.origin, new_ent->s.origin);
		VectorCopy(self->s.angles, new_ent->s.angles);
		new_ent->enemy = self->enemy;

		ED_CallSpawn(new_ent);

		new_ent->s.color.c = 0xffffff;
		new_ent->oldthink = new_ent->think;
		new_ent->think = MorphOriginalIn; //TODO: should also set new_ent->nextthink?
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
	self->deadflag = DEAD_DEAD;

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
void chicken_check(edict_t* self) //TODO: rename to chicken_check_unmorph.
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

#pragma endregion

void ChickenStaticsInit(void)
{
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

	//for the cluck animation
	sounds[SND_CLUCK1]= gi.soundindex ("monsters/chicken/cluck1.wav");	
	sounds[SND_CLUCK2]= gi.soundindex ("monsters/chicken/cluck2.wav");	

	//for getting hit - even though right now, it dies immediately - they want this changed
	sounds[SND_PAIN1]= gi.soundindex ("monsters/chicken/pain1.wav");	
	sounds[SND_PAIN2]= gi.soundindex ("monsters/chicken/pain2.wav");	

	//for dying - we only ever get gibbed, so no other sound is required
	sounds[SND_DIE]= gi.soundindex ("monsters/chicken/die.wav");	

	//for biting the player
	sounds[SND_BITE1]= gi.soundindex ("monsters/chicken/bite1.wav");	
	sounds[SND_BITE2]= gi.soundindex ("monsters/chicken/bite2.wav");	

	//for pecking the ground
	sounds[SND_PECK1]= gi.soundindex ("monsters/chicken/peck1.wav");	
	sounds[SND_PECK2]= gi.soundindex ("monsters/chicken/peck2.wav");	

	//and lastly, I thought it might be cool to have some cries for when the chicken jumps
	sounds[SND_JUMP1]= gi.soundindex ("monsters/chicken/jump1.wav");	
	sounds[SND_JUMP2]= gi.soundindex ("monsters/chicken/jump2.wav");	
	sounds[SND_JUMP3]= gi.soundindex ("monsters/chicken/jump3.wav");	

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_CHICKEN].resInfo = &res_info;
}

/*QUAKED monster_chicken (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP EATING

The chicken 

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/
void SP_monster_chicken (edict_t *self)
{
	// Generic Monster Initialization
	if (!M_Start(self))		// Failed initialization
		return;

	self->msgHandler = DefaultMsgHandler;
	self->think = M_WalkmonsterStartGo;
	self->materialtype = MAT_FLESH;

	self->health = CHICKEN_HEALTH;
	self->mass = CHICKEN_MASS;
	self->yaw_speed = 20;

	self->movetype = PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	VectorSet(self->mins,-12,-12,-16);
	VectorSet(self->maxs,12,12,16);

	self->s.modelindex = classStatics[CID_CHICKEN].resInfo->modelIndex;

	self->s.skinnum = 0;
	self->monsterinfo.scale = MODEL_SCALE;

	self->monsterinfo.otherenemyname = "obj_barrel";

	//Spawn off the guide
//	self->guide = AI_SpawnGuide(self);
	MG_InitMoods(self);

 	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);

	gi.linkentity(self); 

}

/*-------------------------------------------------------------------------
	chicken_bites you
-------------------------------------------------------------------------*/
void chicken_bite (edict_t *self)
{
	vec3_t	v, off, dir, org;
	float	len;

	// incase we try pecking at someone thats not there.
	if (!self->enemy)
		return;

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);

	// determine if we've actually bitten the player, or just missed
	if (len <= (self->maxs[0] + self->enemy->maxs[0] + 24)  )	// A hit
	{	
		VectorSet(off, 20.0, 0.0, 5.0);
		VectorGetOffsetOrigin(off, self->s.origin, self->s.angles[YAW], org);
		// this is not apparently used for anything ?
		VectorClear(dir);
		// cause us some damage.
	 	T_Damage (self->enemy, self, self, dir, org, vec3_origin, 1, 0, 0,MOD_DIED);
		
		if(!irand(0,1))
			gi.sound(self, CHAN_VOICE, sounds[SND_BITE1], 1, ATTN_NORM, 0);
		else
			gi.sound(self, CHAN_VOICE, sounds[SND_BITE2], 1, ATTN_NORM, 0);
	}
}

/*-------------------------------------------------------------------------
	chicken_pause
-------------------------------------------------------------------------*/
void chicken_pause (edict_t *self)
{
	vec3_t	v;
	float	len;
	int		random_action;

	self->mood_think(self);

	if (self->ai_mood == AI_MOOD_NORMAL)
	{
		FindTarget(self);

		if(self->enemy)
		{
			VectorSubtract (self->s.origin, self->enemy->s.origin, v);
			len = VectorLength (v);

			if ((len > 60) || (self->monsterinfo.aiflags & AI_FLEE))  // Far enough to run after
			{
				QPostMessage(self, MSG_RUN,PRI_DIRECTIVE, NULL);
			}
			else	// Close enough to Attack 
			{
				QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
			}
		}
	}
	else
	{
		// decide if we will do a random action
		random_action = (irand(0,10));
		// ok, if we can attack, then we do that, no question
		if (self->ai_mood == AI_MOOD_ATTACK)
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		// else, one chance in a remote time, we will just go "cluck"
		else if (!random_action)
			// make us cluck
			QPostMessage(self, MSG_WATCH, PRI_DIRECTIVE, NULL);
		else
		{
			random_action--;
			if (!random_action)
				// make us peck
				QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
			// otherwise run or track the player target
			else
			{
				switch (self->ai_mood)
				{
				case AI_MOOD_PURSUE:
					QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
					break;
				case AI_MOOD_NAVIGATE:
					QPostMessage(self, MSG_WALK, PRI_DIRECTIVE, NULL);
					break;
				case AI_MOOD_STAND:
					QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
					break;
				case AI_MOOD_JUMP:
					VectorCopy(self->movedir, self->velocity);
					VectorNormalize(self->movedir);
					SetAnim(self, ANIM_JUMP);
					break;
				case AI_MOOD_EAT:
					QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
					break;

				default :
#ifdef _DEVEL
					gi.dprintf("Chicken Unusable mood %d!\n", self->ai_mood);
#endif
					break;
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------
	chicken_eat again, possibly
-------------------------------------------------------------------------*/
void chicken_eat_again (edict_t *self)
{
	// a one in three chance we will peck again :) 
	if (irand(0,2))
	 		QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
	else
		chicken_pause(self);
}

void chickensqueal (edict_t *self)
{
	if(!irand(0, 1))
		gi.sound (self, CHAN_WEAPON, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_WEAPON, sounds[SND_PAIN2], 1, ATTN_NORM, 0);
}

void ChickenGlide ( playerinfo_t *playerinfo )
{
}

void chickenSound (edict_t *self, float channel, float sndindex, float atten)
{
	gi.sound(self, channel, sounds[(int)(sndindex)], 1, atten, 0);
}

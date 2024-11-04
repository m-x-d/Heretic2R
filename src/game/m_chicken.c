//==============================================================================
//
// m_chicken.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//
//	AI :
//
//	STAND1		: Looking straight ahead
//
//==============================================================================

#include "g_local.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "fx.h"
#include "random.h"
#include "buoy.h"
#include "vector.h"
#include "g_Physics.h"
#include "g_teleport.h"
#include "g_Skeletons.h"
#include "p_anims.h"

#include "m_chicken.h"
#include "m_chicken_anim.h"
#include "m_stats.h"

extern void BecomeDebris(edict_t *self);
extern void ED_CallSpawn (edict_t *ent);

/*----------------------------------------------------------------------
  Chicken Base Info
-----------------------------------------------------------------------*/

static animmove_t *animations[NUM_ANIMS] =
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

static ClassResourceInfo_t resInfo;

void ChickenStaticsInit()
{
	classStatics[CID_CHICKEN].msgReceivers[MSG_STAND] = chicken_stand;
	classStatics[CID_CHICKEN].msgReceivers[MSG_WALK] = chicken_walk;
	classStatics[CID_CHICKEN].msgReceivers[MSG_RUN] = chicken_run;
	classStatics[CID_CHICKEN].msgReceivers[MSG_MELEE] = chicken_attack;
	classStatics[CID_CHICKEN].msgReceivers[MSG_DEATH] = chicken_death;
	classStatics[CID_CHICKEN].msgReceivers[MSG_WATCH] = chicken_cluck;
	classStatics[CID_CHICKEN].msgReceivers[MSG_EAT] = chicken_eat;
	classStatics[CID_CHICKEN].msgReceivers[MSG_JUMP] = chicken_jump;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/chicken2/tris.fm");

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

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_CHICKEN].resInfo = &resInfo;
}

/*QUAKED monster_chicken (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP EATING

The chicken 

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/
void SP_monster_chicken (edict_t *self)
{
	// Generic Monster Initialization
	if (!monster_start(self))		// Failed initialization
		return;

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_CHICKEN;
	self->think = walkmonster_start_go;
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

void chicken_death(edict_t *self, G_Message_t *msg)
{
	self->msgHandler = DeadMsgHandler;

	// if we have a use, fire it off.
//	monster_death_use (self);

	gi.sound (self, CHAN_BODY, sounds[SND_DIE], 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	BecomeDebris(self);
	gi.CreateEffect(&self->s, FX_CHICKEN_EXPLODE, CEF_OWNERS_ORIGIN, NULL, "" ); 

}

// fade the original monster back in again
void MorphOriginalIn(edict_t *self)
{
	self->s.color.a += MORPH_TELE_FADE;
	self->nextthink = level.time + FRAMETIME;
	if (!(--self->morph_timer))
	{
		self->think = self->oldthink;
	}
}

//Fade out the existing model till its gone
void MorphChickenOut(edict_t *self)
{
	edict_t	*newent;

	self->s.color.a -= MORPH_TELE_FADE;
	self->nextthink = level.time + FRAMETIME;
	if (self->morph_timer < level.time)
	{
		// create the original bad guy
		newent = G_Spawn();
		newent->classname = self->map;
		VectorCopy(self->s.origin, newent->s.origin);
		VectorCopy(self->s.angles, newent->s.angles);
		newent->enemy = self->enemy;
		ED_CallSpawn(newent);
		newent->s.color.c = 0xffffff;
		newent->oldthink = newent->think;
		newent->think = MorphOriginalIn;
		newent->morph_timer = MORPH_TELE_TIME;
		newent->target = self->target;

		// make physics expand the bounding box for us, so the player doesn't get stuck inside the new bad guys box.

		// store the old mins max's

		VectorCopy(newent->mins,newent->intentMins);
		VectorCopy(newent->maxs,newent->intentMaxs);

		VectorCopy(self->mins,newent->mins);
		VectorCopy(self->maxs,newent->maxs);

		newent->physicsFlags |= PF_RESIZE;

		// do the teleport sound
		gi.sound(newent,CHAN_WEAPON,gi.soundindex("weapons/teleport.wav"),1,ATTN_NORM,0);
		// do the teleport effect
		gi.CreateEffect(&newent->s, FX_PLAYER_TELEPORT_IN, CEF_OWNERS_ORIGIN, NULL, "" ); 
		G_FreeEdict(self);
	}
}


/*-------------------------------------------------------------------------
	chicken checks to see if we are still a chicken, or is it time to return to 
	our original shape ?
-------------------------------------------------------------------------*/
void chicken_check (edict_t *self)
{

	// are we done yet ?
	if (self->time > level.time)
		return;

	//	make that pretty effect around us
	gi.CreateEffect(&self->s, FX_PLAYER_TELEPORT_OUT, CEF_OWNERS_ORIGIN, NULL, "" ); 

	// deal with the existing chicken
	self->think = MorphChickenOut;
	self->nextthink = level.time + FRAMETIME;
	self->touch = NULL;
	self->morph_timer = MORPH_TELE_TIME;
	VectorClear(self->velocity);

	// wipe out the guide, so we don't do anything else
//	G_FreeEdict(self->guide);

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

//----------------------------------------------------------------------
//  Chicken attack - peck us to death
//----------------------------------------------------------------------
void chicken_attack(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_ATTACK);
}

//----------------------------------------------------------------------
//  Chicken jump 
//----------------------------------------------------------------------
void chicken_jump(edict_t *self, G_Message_t *msg)
{
	if(!irand(0,2))
		gi.sound(self, CHAN_VOICE, sounds[SND_JUMP1], 1, ATTN_NORM, 0);
	else if (!irand(0, 1))
		gi.sound(self, CHAN_VOICE, sounds[SND_JUMP2], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sounds[SND_JUMP3], 1, ATTN_NORM, 0);
	SetAnim(self, ANIM_JUMP);
}

//----------------------------------------------------------------------
//  Chicken Cluck 
//----------------------------------------------------------------------
void chicken_eat(edict_t *self, G_Message_t *msg)
{
	if (!irand(0, 1))
		gi.sound(self, CHAN_VOICE, sounds[SND_PECK1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sounds[SND_PECK2], 1, ATTN_NORM, 0);
	SetAnim(self, ANIM_EAT);
}

//----------------------------------------------------------------------
//  Chicken Cluck 
//----------------------------------------------------------------------
void chicken_cluck(edict_t *self, G_Message_t *msg)
{
	if (!irand(0, 1))
		gi.sound(self, CHAN_VOICE, sounds[SND_CLUCK1], 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sounds[SND_CLUCK2], 1, ATTN_NORM, 0);

	SetAnim(self, ANIM_CLUCK);
}

//----------------------------------------------------------------------
//  Chicken Run - choose a run to use
//----------------------------------------------------------------------
void chicken_run(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_RUN);
}

//----------------------------------------------------------------------
//  Chicken Walk - choose a walk to use
//----------------------------------------------------------------------
void chicken_walk(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_WALK);
}

//----------------------------------------------------------------------
//  Chicken Stand -decide which standing animations to use
//----------------------------------------------------------------------
void chicken_stand(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_STAND1);
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

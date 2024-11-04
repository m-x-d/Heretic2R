//==============================================================================
//
// m_rat.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//
//	AI :
//
//	STAND1		: Looking straight ahead
//	STAND2		: Rising up to sit on his haunches
//	STAND3		: Sitting on his haunches looking straight ahead
//	STAND4		: On haunches, looking left
//	STAND5		: On haunches, looking right
//	STAND6		: On haunches, scratching left
//	STAND7		: On haunches, scratching right
//	STAND8		: On haunches, dropping to a stand
//
//	WATCH1		: Hiss while on all fours
//	WATCH2		: Stand up and hiss then go back to all fours
//
//	WALK1       : a normal straight line
//
//	MELEE1      : Attack at feet 
//	MELEE2      : Attack in air
//
//	RUN1        : chasing an enemy straight ahead
//	RUN2        : chasing an enemy and turning left
//	RUN3        : chasing an enemy and turning right
//
//	EATING1		: Bite low
//	EATING2		: Bite low and tear up
//	EATING3		: Bite low and pull back a little
//
//	PAIN1		: backup and runaway
//
//	DIE1		: Big death, dbig flip over
//	DIE2		: Little death flipping over
//
//
//==============================================================================

#include "g_local.h"
#include "Utilities.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "fx.h"
#include "random.h"
#include "vector.h"

#include "m_rat.h"
#include "m_rat_anim.h"
#include "g_misc.h"
#include "m_stats.h"

#include "g_monster.h"

void ai_runaway (edict_t *self, float dist);

/*----------------------------------------------------------------------
  Rat Base Info
-----------------------------------------------------------------------*/

static animmove_t *animations[NUM_ANIMS] =
{
	&rat_move_eat1,
	&rat_move_eat2,
	&rat_move_eat3,
	&rat_move_stand1,
	&rat_move_stand2,
	&rat_move_stand3,
	&rat_move_stand4,
	&rat_move_stand5,
	&rat_move_stand6,
	&rat_move_stand7,
	&rat_move_stand8,
	&rat_move_watch1,
	&rat_move_watch2,
	&rat_move_walk1,
	&rat_move_run1,
	&rat_move_run2,
	&rat_move_run3,
	&rat_move_melee1,
	&rat_move_melee2,
	&rat_move_melee3,
	&rat_move_pain1,
	&rat_move_death1,
	&rat_move_death2,
};

static int sounds[NUM_SOUNDS];

static ClassResourceInfo_t resInfo;

void rat_dead_pain (edict_t *self, G_Message_t *msg)
{
	if (self->health <= -80)
	{
		gi.sound (self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
		BecomeDebris(self);
	}
	return;
}

//----------------------------------------------------------------------
//  Rat Pain - choose a pain to use
//----------------------------------------------------------------------
void rat_pain(edict_t *self, G_Message_t *msg)
{
	rat_pain_init(self);

	SetAnim(self, ANIM_PAIN1);

	return;
}

//----------------------------------------------------------------------
//  Rat Death - choose deatha or deathb or gib.
//----------------------------------------------------------------------
void rat_death(edict_t *self, G_Message_t *msg)
{
	if(self->monsterinfo.aiflags&AI_DONT_THINK)
	{
		// Big enough death to be thrown back
		if (irand(0,10) < 5)
			SetAnim(self, ANIM_DIE2);
		else 
			SetAnim(self, ANIM_DIE1);
		return;
	}

	M_StartDeath(self, 0);

	if (self->health <= -40 + irand(0, 20))
	{
		gi.sound (self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
		BecomeDebris(self);
	}
	else
	{
		if(!strcmp(self->classname, "monster_rat_giant"))
			self->s.skinnum = 1;

		// Big enough death to be thrown back
		if (self->health <= -20)
		{
			SetAnim(self, ANIM_DIE1);
		}
		else 
		{
			SetAnim(self, ANIM_DIE2);
		}
	}
}

//----------------------------------------------------------------------
//  Rat Run - choose a run to use
//----------------------------------------------------------------------
void rat_run(edict_t *self, G_Message_t *msg)
{
	float	delta, len;
	vec3_t	vec;

	if (M_ValidTarget(self, self->enemy))
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
		self->ideal_yaw = vectoyaw(vec);
		delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);

		if(self->monsterinfo.attack_finished < level.time)
		{
			len = VectorLength(vec);
			
			if (len < 150 && len > 50)
			{
				if (delta < 10)
				{
					SetAnim(self, ANIM_MELEE2);
					return;
				}
			}
		}

		// Look right
		if (delta > 25 && delta <= 180)
		{
			SetAnim(self, ANIM_RUN3);
			return;
		}
		else if (delta > 180 && delta < 335)	// Look left
		{
			SetAnim(self, ANIM_RUN2);
			return;
		}
		else
		{
			SetAnim(self, ANIM_RUN1);
			return;
		}

		return;
	}

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}


//----------------------------------------------------------------------
//  Rat Walk - choose a walk to use
//----------------------------------------------------------------------
void rat_walk(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, ANIM_WALK1);
}

//----------------------------------------------------------------------
//  Rat Melee - choose a melee to use
//----------------------------------------------------------------------
void rat_melee(edict_t *self, G_Message_t *msg)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if (irand(0,1) < 1)
		{
			SetAnim(self, ANIM_MELEE1);
		}
		else
		{
			SetAnim(self, ANIM_MELEE3);
		}

		return;
	}

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

//----------------------------------------------------------------------
//  Rat Watch - which watch animation to use? 
//----------------------------------------------------------------------
void rat_watch(edict_t *self, G_Message_t *msg)
{
	if (irand(0, 1))
	{
		SetAnim(self, ANIM_WATCH1);
	}
	else 
	{
		SetAnim(self, ANIM_WATCH2);
	}
}

//----------------------------------------------------------------------
//  Rat Stand -decide which standing animations to use
//----------------------------------------------------------------------
void rat_stand(edict_t *self, G_Message_t *msg)
{
	int chance;

	chance = irand(0, 100);

	// On the ground
	switch(self->curAnimID)
	{
	case ANIM_STAND1:
		if (chance < 95)
			SetAnim(self, ANIM_STAND1);
		else
			SetAnim(self, ANIM_STAND2);

		break;
	case ANIM_STAND2:
		if(chance < 75)
			SetAnim(self, ANIM_STAND3);
		else
			SetAnim(self, ANIM_STAND4 + irand(0, 4));
		break;
	case ANIM_STAND4:
	case ANIM_STAND5:
	case ANIM_STAND6:
	case ANIM_STAND7:
			SetAnim(self, ANIM_STAND8);
			break;
	case ANIM_STAND8:
		SetAnim(self, ANIM_STAND1);
		break;
	default:
		if(chance < 75)
			SetAnim(self, ANIM_STAND3);
		else
			SetAnim(self, ANIM_STAND4 + irand(0, 4));
		break;
	}
}

//----------------------------------------------------------------------
//  Rat Eat - decide which eating animations to use
//----------------------------------------------------------------------
void rat_eat(edict_t *self, G_Message_t *msg)
{
	int chance;

	chance = irand(0, 10);
	if (chance < 4)
	{
		SetAnim(self, ANIM_EATING1);
	}
	else if (chance < 8)
	{
		SetAnim(self, ANIM_EATING3);
	}
	else 
	{
		SetAnim(self, ANIM_EATING2);
	}
}

/*----------------------------------------------------------------------

  ACTION FUNCTIONS FOR THE MONSTER

-----------------------------------------------------------------------*/

void ratchew (edict_t *self)
{
	int chance;

	chance = irand(0, 100);

	if (chance > 50 && chance < 65)
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_CHEW1], 1, ATTN_IDLE, 0);
	}
	else if (chance < 85)
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_CHEW2], 1, ATTN_IDLE, 0);
	}
	else
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_CHEW3], 1, ATTN_IDLE, 0);
	}
}

void ratchatter (edict_t *self)
{
	int chance;

	chance = irand(0, 20);

	if (chance < 1)
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_CHATTER1], 1, ATTN_IDLE, 0);
	}
	else if (chance < 2)
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_CHATTER2], 1, ATTN_IDLE, 0);
	}
	else if (chance < 3)
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_CHATTER3], 1, ATTN_IDLE, 0);
	}
}

void ratswallow (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sounds[SND_SWALLOW], 1, ATTN_IDLE, 0);
}

void rathiss (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sounds[SND_HISS], 1, ATTN_NORM, 0);
}

void ratscratch (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sounds[SND_SCRATCH], 1, ATTN_IDLE, 0);
}

void ratdeathsqueal (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sounds[SND_DIE], 1, ATTN_NORM, 0);
}

void ratsqueal (edict_t *self)
{
	if (irand(0, 1))
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_PAIN1], 1, ATTN_NORM, 0);
	}
	else
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_PAIN2], 1, ATTN_NORM, 0);
	}
}


void ratbite (edict_t *self)
{
	float	len;
	vec3_t	dir, endpos, normal, forward, startpos;
	trace_t	trace;

	if(!self->enemy)
		return;

	self->monsterinfo.attack_finished = level.time + 3 - skill->value + flrand(0.5, 1);

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorCopy(self->s.origin, startpos);
	VectorMA(startpos, self->maxs[0] * 0.5, forward, startpos);
	startpos[2] += self->viewheight;

	VectorCopy(self->enemy->s.origin, endpos);
	
	endpos[0]+=flrand(-4, 4);
	endpos[1]+=flrand(-4, 4);
	endpos[2]+=flrand(-4, 4);

	VectorSubtract(endpos, startpos, dir);
	len = VectorNormalize(dir);
		
	if (len < (self->maxs[0] + self->enemy->maxs[0] + 45))	// A hit
	{
		gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_MONSTERSOLID,&trace);

		if (trace.ent->takedamage)	// A hit
		{
			gi.sound (self, CHAN_WEAPON, sounds[SND_BITEHIT1], 1, ATTN_NORM, 0);
			VectorMA(trace.endpos, flrand(0, 8), dir, endpos);
			VectorScale(dir, -1, normal);
			// do 1 point.
			if(self->s.scale > 1.5)
				T_Damage (trace.ent, self, self, dir, endpos, normal, RAT_DMG_BITE * irand(2, 4), 0, DAMAGE_AVOID_ARMOR,MOD_DIED);
			else
				T_Damage (trace.ent, self, self, dir, endpos, normal, RAT_DMG_BITE, 0, DAMAGE_AVOID_ARMOR,MOD_DIED);
			return;
		}
	}
// A misssss
	if (irand(0, 1))
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_BITEMISS1], 1, ATTN_NORM, 0);
	}
	else
	{
		gi.sound (self, CHAN_WEAPON, sounds[SND_BITEMISS2], 1, ATTN_NORM, 0);
	}
}

void rat_pain_init(edict_t *self)
{
	if (self->s.scale < 2.0 && irand(0,100) < 50)
	{
		self->monsterinfo.aiflags |= AI_FLEE;				// Run away
		self->monsterinfo.flee_finished = level.time + flrand(3.0, 7.0);
	}
}

void rat_runorder(edict_t *self)
{
	QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}

void rat_standorder(edict_t *self)
{
	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void rat_pause (edict_t *self)
{
	float	len;

	if (M_ValidTarget(self, self->enemy))
	{
		len = M_DistanceToTarget(self, self->enemy);

		// Far enough to run after
		if ((len > 60) || (self->monsterinfo.aiflags & AI_FLEE))
		{
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		}
		else	// Close enough to Attack 
		{
			QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
		}
		
		return;
	}

	QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void ratjump(edict_t *self)
{
	vec3_t vf;

	AngleVectors(self->s.angles, vf, NULL, NULL);

	VectorMA(self->velocity, 125, vf, self->velocity);
	self->velocity[2] += 225;
}

/*----------------------------------------------------------------------
  Rat WatchOrder - order the rat to watch
-----------------------------------------------------------------------*/
void rat_watchorder(edict_t *self)
{
	QPostMessage(self, MSG_WATCH, PRI_DIRECTIVE, NULL);
}

/*----------------------------------------------------------------------
  Rat EatOrder - order the rat to choose an eat animation
-----------------------------------------------------------------------*/
void rat_eatorder(edict_t *self)
{
	QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
}

void rat_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->enemy = activator;
	FoundTarget(self, 1);
}

void rat_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//M_Touch is overridden because the player can just step over rats

	vec3_t	pos1, pos2, dir;
	float	zdiff;

	if ((other->svflags & SVF_MONSTER) || other->client)
	{
		VectorCopy(other->s.origin, pos1);
		pos1[2] += other->mins[2];

		VectorCopy(ent->s.origin, pos2);
		pos2[2] += ent->maxs[2];

		zdiff = pos1[2] - pos2[2];

		// On top
		if (zdiff >= 0 )
		{
			//Squish the rat a bit
			T_Damage (ent, other, other, dir, pos2, vec3_origin, flrand(4,6), 0, DAMAGE_AVOID_ARMOR,MOD_DIED);
		}
	/*	else	//Kick um
		{
			//Squish the rat a bit
			T_Damage (ent, other, other, dir, pos2, vec3_origin, flrand(0,1), 0, DAMAGE_AVOID_ARMOR);

			VectorSubtract(ent->s.origin, ent->enemy->s.origin, dir);
			dir[2] = 0;
			VectorNormalize(dir);
			VectorScale(dir, 64, dir);
			
			dir[2] = 150;
			VectorCopy(dir, ent->velocity);
		}*/
	}
}

#define MAX_RAT_ATTACK	2
#define MAX_RAT_IGNORE_DIST 150

void rat_ai_stand(edict_t *self, float dist)
{
	float MG_FaceGoal (edict_t *self, qboolean doturn);

	if (M_ValidTarget(self, self->enemy))
	{
		//Find the number of rats around us (-1 denotes it hasn't check previously)
		if (self->monsterinfo.supporters == -1)
			self->monsterinfo.supporters = M_FindSupport( self, RAT_GROUP_RANGE );

		//We've got an enemy, now see if we have enough support to attack it
		if (self->monsterinfo.supporters >= MAX_RAT_ATTACK)
		{
			FoundTarget(self, true);
		}
		else
		{
			//Is he close enough to scare us away?
			if (M_DistanceToTarget(self, self->enemy) < MAX_RAT_IGNORE_DIST)
			{
				if (self->s.scale < 2.0 || irand(0,1))
				{
					//Just attack him
					FoundTarget(self, true);
				}
				else
				{
					//Run away
					self->monsterinfo.aiflags |= AI_FLEE;
					self->monsterinfo.flee_finished = level.time + 10;
				}
			}
			else
			{
				//Not close enough to bother us right now, but watch this enemy
				MG_FaceGoal(self, true);
				self->enemy = NULL;
			}
		}
	}
}

void rat_ai_eat(edict_t *self, float dist)
{
	if (M_ValidTarget(self, self->enemy))
	{
		if (M_DistanceToTarget(self, self->enemy) < MAX_RAT_IGNORE_DIST)
		{
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		}
		else
		{
			QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
		}
	}
}

qboolean EqualAngle(float angle1, float angle2, float leniency);
void rat_ai_run (edict_t *self, float dist)
{
	vec3_t		vec;

	if (!self->enemy)
		return;
	
	if (self->monsterinfo.aiflags & AI_FLEE||self->monsterinfo.aiflags & AI_COWARD)
	{
		if(!self->count)
			self->count = 180;
		VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
		self->ideal_yaw = vectoyaw(vec);
		self->ideal_yaw = anglemod(self->ideal_yaw + self->count);
		M_ChangeYaw(self);
		if(!M_walkmove(self, self->s.angles[YAW], dist) && EqualAngle(self->s.angles[YAW], self->ideal_yaw, 5))
			self->count = flrand(60, 300);
	}
	else
	{
		ai_run(self, dist);
	}
}

void RatStaticsInit()
{
	classStatics[CID_RAT].msgReceivers[MSG_STAND] = rat_stand;
	classStatics[CID_RAT].msgReceivers[MSG_WALK] = rat_walk;
	classStatics[CID_RAT].msgReceivers[MSG_RUN] = rat_run;
	classStatics[CID_RAT].msgReceivers[MSG_EAT] = rat_eat;
	classStatics[CID_RAT].msgReceivers[MSG_MELEE] = rat_melee;
	classStatics[CID_RAT].msgReceivers[MSG_WATCH] = rat_watch;
	classStatics[CID_RAT].msgReceivers[MSG_PAIN] = rat_pain;
	classStatics[CID_RAT].msgReceivers[MSG_DEATH] = rat_death;
	classStatics[CID_RAT].msgReceivers[MSG_JUMP] = M_jump;
	classStatics[CID_RAT].msgReceivers[MSG_DEATH_PAIN] = rat_dead_pain;

	resInfo.numAnims = NUM_ANIMS;
	resInfo.animations = animations;
	resInfo.modelIndex = gi.modelindex("models/monsters/rat/tris.fm");

	sounds[SND_BITEHIT1] = gi.soundindex ("monsters/rat/meleehit1.wav");	
	sounds[SND_BITEMISS1] = gi.soundindex ("monsters/rat/meleemiss1.wav");	
	sounds[SND_BITEMISS2] = gi.soundindex ("monsters/rat/meleemiss2.wav");	
	sounds[SND_HISS] = gi.soundindex ("monsters/rat/hiss.wav");	
	sounds[SND_SCRATCH] = gi.soundindex ("monsters/rat/scratch.wav");	
	sounds[SND_PAIN1] = gi.soundindex ("monsters/rat/pain1.wav");	
	sounds[SND_PAIN2] = gi.soundindex ("monsters/rat/pain2.wav");	

	sounds[SND_CHATTER1] = gi.soundindex ("monsters/rat/chatter1.wav");	
	sounds[SND_CHATTER2] = gi.soundindex ("monsters/rat/chatter2.wav");	
	sounds[SND_CHATTER3] = gi.soundindex ("monsters/rat/chatter3.wav");	

	sounds[SND_CHEW1] = gi.soundindex ("monsters/rat/chew1.wav");	
	sounds[SND_CHEW2] = gi.soundindex ("monsters/rat/chew2.wav");	
	sounds[SND_CHEW3] = gi.soundindex ("monsters/rat/chew3.wav");	

	sounds[SND_SWALLOW] = gi.soundindex ("monsters/rat/swallow.wav");	

	sounds[SND_DIE] = gi.soundindex ("monsters/rat/death1.wav");	
	sounds[SND_GIB] = gi.soundindex ("monsters/rat/gib.wav");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_RAT].resInfo = &resInfo;
}

/*QUAKED monster_rat (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP EATING 8 16 32 64 FIXED(na) WANDER(na) MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

The rat 

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

EATING - Chomp chomp... chewie chomp

COWARD - Runs away

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/

void SP_monster_rat (edict_t *self)
{
	// Generic Monster Initialization
	if (!monster_start(self))		// Failed initialization
		return;	

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_RAT;
	self->think = walkmonster_start_go;
	self->materialtype = MAT_FLESH;

	if (!self->health)
		self->health = RAT_HEALTH;

	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = RAT_MASS;
	self->yaw_speed = 20;

	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	self->movetype=PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);	
	
	//Init this to -1 because we can't check for supporters in this function
	self->monsterinfo.supporters = -1;

	self->s.modelindex = classStatics[CID_RAT].resInfo->modelIndex;

	self->s.skinnum = 0;

	if (self->monsterinfo.scale)
	{
		self->s.scale = self->monsterinfo.scale = MODEL_SCALE;
	}

	self->viewheight = self->maxs[2] * 0.5 * self->s.scale;

	self->use = rat_use;
	self->touch = rat_touch;

	if (self->spawnflags & MSF_EATING)
	{
		//self->monsterinfo.aiflags |= AI_EATING;
		QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
		if(!self->wakeup_distance)
			self->wakeup_distance = 300;
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}


/*QUAKED monster_rat_giant (1 .5 0) (-16 -16 0) (16 16 32) AMBUSH ASLEEP EATING 8 16 32 64 FIXED(na) WANDER(na) MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4

A giant rat witha second skin and a bit tougher

AMBUSH - Will not be woken up by other monsters or shots from player

ASLEEP - will not appear until triggered

EATING - Chomp chomp... chewie chomp  (wakeup_distance will default to 100)

COWARD - Runs away

"wakeup_target" - monsters will fire this target the first time it wakes up (only once)

"pain_target" - monsters will fire this target the first time it gets hurt (only once)

*/
void SP_monster_rat_giant (edict_t *self)
{
	// Generic Monster Initialization
	if (!walkmonster_start(self))		// Failed initialization
		return;

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_RAT;
	self->materialtype = MAT_FLESH;

	if (!self->health)
		self->health = RAT_HEALTH * 4;

	self->max_health = self->health = MonsterHealth(self->health);

	self->mass = RAT_MASS * 2;
	self->yaw_speed = 20;

	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	self->movetype=PHYSICSTYPE_STEP;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	//this will be scaled up later
	VectorSet(self->mins, -8, -8, 0);
	VectorSet(self->maxs, 8, 8, 16);

	self->s.modelindex = gi.modelindex("models/monsters/rat/superduperat/tris.fm");

	self->s.skinnum = 0;

	if (self->s.scale < 2.0)
	{
		self->s.scale = self->monsterinfo.scale = 2.0;
	}
	else
		self->monsterinfo.scale = self->s.scale;

	//Init this to -1 because we can't check for supporters in this function
	self->monsterinfo.supporters = -1;

	self->viewheight = self->maxs[2] * 0.5 * self->s.scale;

	self->use = rat_use;
	self->touch = rat_touch;

	if (self->spawnflags & MSF_EATING)
	{
		self->monsterinfo.aiflags |= AI_EATING;
		QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
		if(!self->wakeup_distance)
			self->wakeup_distance = 300;
	}
	else
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

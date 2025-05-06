//
// m_rat.c
//
// Copyright 1998 Raven Software
//

#include "m_rat.h"
#include "m_rat_shared.h"
#include "m_rat_anim.h"
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "m_stats.h"
#include "mg_ai.h" //mxd
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_monster.h"
#include "g_local.h"

#define RAT_MIN_ATTACK_SUPPORTERS	2 //mxd. Named 'MAX_RAT_ATTACK' in original logic.
#define RAT_IGNORE_DISTANCE			150.0f //mxd. Named 'MAX_RAT_IGNORE_DIST' in original logic.

#pragma region ========================== Rat Base Info ==========================

static const animmove_t* animations[NUM_ANIMS] =
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

#pragma endregion

static void RatDeathPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_dead_pain' in original logic.
{
	if (self->health <= -80) // Gib death.
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);
		BecomeDebris(self);
	}
}

static void RatPainMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_pain' in original logic.
{
	rat_pain_init(self);
	SetAnim(self, ANIM_PAIN1);
}

static void RatDeathMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_death' in original logic.
{
	if (self->monsterinfo.aiflags & AI_DONT_THINK)
	{
		// Big enough death to be thrown back.
		SetAnim(self, irand(ANIM_DIE1, ANIM_DIE2));
		return;
	}

	M_StartDeath(self, 0);

	if (self->health <= -40 + irand(0, 20))
	{
		gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1.0f, ATTN_NORM, 0.0f);
		BecomeDebris(self);

		return;
	}

	if (strcmp(self->classname, "monster_rat_giant") == 0)
		self->s.skinnum = 1;

	// Big enough death to be thrown back?
	SetAnim(self, ((self->health <= -20) ? ANIM_DIE1 : ANIM_DIE2));
}

static void RatRunMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_run' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);
	self->ideal_yaw = VectorYaw(diff);

	const float delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);

	if (self->monsterinfo.attack_finished < level.time)
	{
		const float dist = VectorLength(diff);

		if (dist > 50.0f && dist < 150.0f && delta < 10.0f)
		{
			SetAnim(self, ANIM_MELEE2);
			return;
		}
	}

	if (delta > 25.0f && delta <= 180.0f) // Run to the right.
		SetAnim(self, ANIM_RUN3);
	else if (delta > 180.0f && delta < 335.0f)	// Run to the left.
		SetAnim(self, ANIM_RUN2);
	else // Run forwards.
		SetAnim(self, ANIM_RUN1);
}

static void RatWalkMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_walk' in original logic.
{
	SetAnim(self, ANIM_WALK1);
}

static void RatMeleeMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_melee' in original logic.
{
	if (!M_ValidTarget(self, self->enemy))
	{
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
		return;
	}

	SetAnim(self, (irand(0, 1) == 0 ? ANIM_MELEE1 : ANIM_MELEE3));
}

static void RatWatchMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_watch' in original logic.
{
	SetAnim(self, irand(ANIM_WATCH1, ANIM_WATCH2));
}

static void RatStandMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_stand' in original logic.
{
	const int chance = irand(0, 100);

	// On the ground.
	switch (self->curAnimID)
	{
		case ANIM_STAND1:
			SetAnim(self, (chance < 95 ? ANIM_STAND1 : ANIM_STAND2));
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
			if (chance < 75)
				SetAnim(self, ANIM_STAND3);
			else
				SetAnim(self, irand(ANIM_STAND4, ANIM_STAND8));
			break;
	}
}

static void RatEatMsgHandler(edict_t* self, G_Message_t* msg) //mxd. Named 'rat_eat' in original logic.
{
	const int chance = irand(0, 10);

	if (chance < 4)
		SetAnim(self, ANIM_EATING1);
	else if (chance < 8)
		SetAnim(self, ANIM_EATING3);
	else
		SetAnim(self, ANIM_EATING2);
}

void ratchew(edict_t* self) //TODO: rename to rat_chew.
{
	const int chance = irand(0, 100);

	if (chance > 50 && chance < 65)
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHEW1], 1.0f, ATTN_IDLE, 0.0f);
	else if (chance < 85)
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHEW2], 1.0f, ATTN_IDLE, 0.0f);
	else
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHEW3], 1.0f, ATTN_IDLE, 0.0f);
}

void ratchatter(edict_t* self) //TODO: rename to rat_chatter.
{
	if (irand(0, 20) < 3)
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_CHATTER1, SND_CHATTER3)], 1.0f, ATTN_IDLE, 0.0f);
}

void ratswallow(edict_t* self) //TODO: rename to rat_swallow.
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_SWALLOW], 1.0f, ATTN_IDLE, 0.0f);
}

void rathiss(edict_t* self) //TODO: rename to rat_hiss.
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_HISS], 1.0f, ATTN_NORM, 0.0f);
}

void ratscratch(edict_t* self) //TODO: rename to rat_scratch.
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_SCRATCH], 1.0f, ATTN_IDLE, 0.0f);
}

void ratdeathsqueal(edict_t* self) //TODO: rename to rat_death_squeal.
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
}

void ratsqueal(edict_t* self) //TODO: rename to rat_squeal.
{
	gi.sound(self, CHAN_WEAPON, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
}

void ratbite(edict_t* self) //TODO: rename to rat_bite.
{
	if (self->enemy == NULL)
		return;

	self->monsterinfo.attack_finished = level.time + 3.0f - skill->value + flrand(0.5f, 1.0f);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t start_pos;
	VectorCopy(self->s.origin, start_pos);
	VectorMA(start_pos, self->maxs[0] * 0.5f, forward, start_pos);
	start_pos[2] += (float)self->viewheight;

	vec3_t end_pos;
	VectorCopy(self->enemy->s.origin, end_pos);
	VectorRandomCopy(end_pos, end_pos, 4.0f);

	vec3_t dir;
	VectorSubtract(end_pos, start_pos, dir);
	const float dist = VectorNormalize(dir);

	if (dist < self->maxs[0] + self->enemy->maxs[0] + 45.0f) // A hit.
	{
		trace_t	trace;
		gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, self, MASK_MONSTERSOLID, &trace);

		if (trace.ent->takedamage != DAMAGE_NO)
		{
			gi.sound(self, CHAN_WEAPON, sounds[SND_BITEHIT1], 1.0f, ATTN_NORM, 0.0f);

			VectorMA(trace.endpos, flrand(0.0f, 8.0f), dir, end_pos);

			vec3_t normal;
			VectorScale(dir, -1.0f, normal);

			// Do 1 point.
			const int damage = RAT_DMG_BITE * (self->s.scale > 1.5f ? irand(2, 4) : 1);
			T_Damage(trace.ent, self, self, dir, end_pos, normal, damage, 0, DAMAGE_AVOID_ARMOR, MOD_DIED);

			return;
		}
	}

	// A miss.
	gi.sound(self, CHAN_WEAPON, sounds[irand(SND_BITEMISS1, SND_BITEMISS2)], 1.0f, ATTN_NORM, 0.0f);
}

void rat_pain_init(edict_t* self)
{
	if (self->s.scale < 2.0f && irand(0, 100) < 50)
	{
		self->monsterinfo.aiflags |= AI_FLEE; // Run away.
		self->monsterinfo.flee_finished = level.time + flrand(3.0f, 7.0f);
	}
}

void rat_runorder(edict_t* self) //TODO: rename to rat_run_order.
{
	QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}

void rat_standorder(edict_t* self) //TODO: rename to rat_stand_order.
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
	AI_FoundTarget(self, 1);
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

void rat_ai_stand(edict_t *self, float dist)
{
	if (M_ValidTarget(self, self->enemy))
	{
		//Find the number of rats around us (-1 denotes it hasn't check previously)
		if (self->monsterinfo.supporters == -1)
			self->monsterinfo.supporters = M_FindSupport( self, RAT_GROUP_RANGE );

		//We've got an enemy, now see if we have enough support to attack it
		if (self->monsterinfo.supporters >= RAT_MIN_ATTACK_SUPPORTERS)
		{
			AI_FoundTarget(self, true);
		}
		else
		{
			//Is he close enough to scare us away?
			if (M_DistanceToTarget(self, self->enemy) < RAT_IGNORE_DISTANCE)
			{
				if (self->s.scale < 2.0 || irand(0,1))
				{
					//Just attack him
					AI_FoundTarget(self, true);
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
		if (M_DistanceToTarget(self, self->enemy) < RAT_IGNORE_DISTANCE)
		{
			QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
		}
		else
		{
			QPostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
		}
	}
}

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
		self->ideal_yaw = VectorYaw(vec);
		self->ideal_yaw = anglemod(self->ideal_yaw + self->count);
		M_ChangeYaw(self);
		if(!M_walkmove(self, self->s.angles[YAW], dist) && AnglesEqual(self->s.angles[YAW], self->ideal_yaw, 5))
			self->count = flrand(60, 300);
	}
	else
	{
		MG_AI_Run(self, dist);
	}
}

void RatStaticsInit(void)
{
	static ClassResourceInfo_t resInfo;

	classStatics[CID_RAT].msgReceivers[MSG_STAND] = RatStandMsgHandler;
	classStatics[CID_RAT].msgReceivers[MSG_WALK] = RatWalkMsgHandler;
	classStatics[CID_RAT].msgReceivers[MSG_RUN] = RatRunMsgHandler;
	classStatics[CID_RAT].msgReceivers[MSG_EAT] = RatEatMsgHandler;
	classStatics[CID_RAT].msgReceivers[MSG_MELEE] = RatMeleeMsgHandler;
	classStatics[CID_RAT].msgReceivers[MSG_WATCH] = RatWatchMsgHandler;
	classStatics[CID_RAT].msgReceivers[MSG_PAIN] = RatPainMsgHandler;
	classStatics[CID_RAT].msgReceivers[MSG_DEATH] = RatDeathMsgHandler;
	classStatics[CID_RAT].msgReceivers[MSG_JUMP] = M_jump;
	classStatics[CID_RAT].msgReceivers[MSG_DEATH_PAIN] = RatDeathPainMsgHandler;

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
	if (!M_Start(self))		// Failed initialization
		return;	

	self->msgHandler = DefaultMsgHandler;
	self->think = M_WalkmonsterStartGo;
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
	if (!M_WalkmonsterStart(self))		// Failed initialization
		return;

	self->msgHandler = DefaultMsgHandler;
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

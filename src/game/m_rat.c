//
// m_rat.c
//
// Copyright 1998 Raven Software
//

#include "m_rat.h"
#include "m_rat_shared.h"
#include "m_rat_anim.h"
#include "m_rat_moves.h"
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

#pragma region ========================== Message handlers ==========================

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
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
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
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
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

#pragma endregion

#pragma region ========================== Edict callbacks ===========================

void RatUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'rat_use' in original logic.
{
	self->enemy = activator;
	AI_FoundTarget(self, true);
}

void RatTouch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'rat_touch' in original logic.
{
	// M_Touch is overridden because the player can just step over rats.
	if (!(other->svflags & SVF_MONSTER) && other->client == NULL)
		return;

	const vec3_t other_pos = VEC3_INITA(other->s.origin, 0.0f, 0.0f, other->mins[2]);
	const vec3_t self_pos = VEC3_INITA(ent->s.origin, 0.0f, 0.0f, ent->maxs[2]);

	// When on top, squish the rat a bit. //TODO: always kill rat (at least on easy and medium skill)? Gib when player jumped on rat? Do this only for regular rats (not giant)?
	if (other_pos[2] - self_pos[2] >= 0.0f)
		T_Damage(ent, other, other, vec3_down, self_pos, vec3_origin, irand(4, 6), 0, DAMAGE_AVOID_ARMOR, MOD_DIED); //mxd. flrand() in original logic.
}

#pragma endregion

#pragma region ========================== Action functions ==========================

void rat_chew(edict_t* self) //mxd. Named 'ratchew' in original logic.
{
	const int chance = irand(0, 100);

	if (chance > 50 && chance < 65)
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHEW1], 1.0f, ATTN_IDLE, 0.0f);
	else if (chance < 85)
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHEW2], 1.0f, ATTN_IDLE, 0.0f);
	else
		gi.sound(self, CHAN_WEAPON, sounds[SND_CHEW3], 1.0f, ATTN_IDLE, 0.0f);
}

void rat_chatter(edict_t* self) //mxd. Named 'ratchatter' in original logic.
{
	if (irand(0, 20) < 3)
		gi.sound(self, CHAN_WEAPON, sounds[irand(SND_CHATTER1, SND_CHATTER3)], 1.0f, ATTN_IDLE, 0.0f);
}

void rat_swallow(edict_t* self) //mxd. Named 'ratswallow' in original logic.
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_SWALLOW], 1.0f, ATTN_IDLE, 0.0f);
}

void rat_hiss(edict_t* self) //mxd. Named 'rathiss' in original logic.
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_HISS], 1.0f, ATTN_NORM, 0.0f);
}

void rat_scratch(edict_t* self) //mxd. Named 'ratscratch' in original logic.
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_SCRATCH], 1.0f, ATTN_IDLE, 0.0f);
}

void rat_death_squeal(edict_t* self) //mxd. Named 'ratdeathsqueal' in original logic.
{
	gi.sound(self, CHAN_WEAPON, sounds[SND_DIE], 1.0f, ATTN_NORM, 0.0f);
}

void rat_squeal(edict_t* self) //mxd. Named 'ratsqueal' in original logic.
{
	gi.sound(self, CHAN_WEAPON, sounds[irand(SND_PAIN1, SND_PAIN2)], 1.0f, ATTN_NORM, 0.0f);
}

void rat_bite(edict_t* self) //mxd. Named 'ratsqueal' in original logic.
{
	if (self->enemy == NULL)
		return;

	self->monsterinfo.attack_finished = level.time + 3.0f - skill->value + flrand(0.5f, 1.0f);

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t start_pos = VEC3_INIT(self->s.origin);
	VectorMA(start_pos, self->maxs[0] * 0.5f, forward, start_pos);
	start_pos[2] += (float)self->viewheight;

	vec3_t end_pos = VEC3_INIT(self->enemy->s.origin);
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

void rat_run_order(edict_t* self) //mxd. Named 'rat_runorder' in original logic.
{
	G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
}

void rat_stand_order(edict_t* self) //mxd. Named 'rat_standorder' in original logic.
{
	G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}

void rat_eat_order(edict_t* self) //mxd. Named 'rat_eatorder' in original logic.
{
	G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
}

void rat_pause(edict_t* self)
{
	if (!M_ValidTarget(self, self->enemy))
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	else if (M_DistanceToTarget(self, self->enemy) > 60.0f || (self->monsterinfo.aiflags & AI_FLEE)) // Far enough to run after.
		G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	else // Close enough to attack.
		G_PostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
}

void rat_jump(edict_t* self) //mxd. Named 'ratjump' in original logic.
{
	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	VectorMA(self->velocity, 125.0f, forward, self->velocity);
	self->velocity[2] += 225.0f;
}

void rat_ai_stand(edict_t* self, float distance)
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	// Find the number of rats around us (-1 denotes it hasn't check previously).
	if (self->monsterinfo.supporters == -1)
		self->monsterinfo.supporters = M_FindSupport(self, RAT_GROUP_RANGE);

	// We've got an enemy, now see if we have enough support to attack it.
	if (self->monsterinfo.supporters >= RAT_MIN_ATTACK_SUPPORTERS)
	{
		AI_FoundTarget(self, true);
		return;
	}

	// Is he close enough to scare us away?
	if (M_DistanceToTarget(self, self->enemy) < RAT_IGNORE_DISTANCE)
	{
		if (self->s.scale < 2.0f || irand(0, 1) == 1)
		{
			// Just attack him.
			AI_FoundTarget(self, true);
		}
		else
		{
			// Run away.
			self->monsterinfo.aiflags |= AI_FLEE;
			self->monsterinfo.flee_finished = level.time + 10.0f;
		}
	}
	else
	{
		// Not close enough to bother us right now, but watch this enemy.
		MG_FaceGoal(self, true);
		self->enemy = NULL;
	}
}

void rat_ai_eat(edict_t* self, float distance)
{
	if (!M_ValidTarget(self, self->enemy))
		return;

	if (M_DistanceToTarget(self, self->enemy) < RAT_IGNORE_DISTANCE)
		G_PostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);
	else
		G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);
}

void rat_ai_run(edict_t* self, float distance)
{
	if (self->enemy == NULL)
		return;

	if (self->monsterinfo.aiflags & (AI_FLEE | AI_COWARD))
	{
		if (self->rat_flee_angle == 0.0f)
			self->rat_flee_angle = 180.0f; //TODO: randomize this a bit?

		vec3_t diff;
		VectorSubtract(self->enemy->s.origin, self->s.origin, diff);

		self->ideal_yaw = anglemod(VectorYaw(diff) + self->rat_flee_angle);
		M_ChangeYaw(self);

		if (!M_walkmove(self, self->s.angles[YAW], distance) && AnglesEqual(self->s.angles[YAW], self->ideal_yaw, 5.0f))
			self->rat_flee_angle = flrand(60.0f, 300.0f);
	}
	else
	{
		MG_AI_Run(self, distance);
	}
}

#pragma endregion

void RatStaticsInit(void)
{
	static ClassResourceInfo_t res_info; //mxd. Made local static.

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

	res_info.numAnims = NUM_ANIMS;
	res_info.animations = animations;
	res_info.modelIndex = gi.modelindex("models/monsters/rat/tris.fm");

	sounds[SND_BITEHIT1] = gi.soundindex("monsters/rat/meleehit1.wav");
	sounds[SND_BITEMISS1] = gi.soundindex("monsters/rat/meleemiss1.wav");
	sounds[SND_BITEMISS2] = gi.soundindex("monsters/rat/meleemiss2.wav");
	sounds[SND_HISS] = gi.soundindex("monsters/rat/hiss.wav");
	sounds[SND_SCRATCH] = gi.soundindex("monsters/rat/scratch.wav");
	sounds[SND_PAIN1] = gi.soundindex("monsters/rat/pain1.wav");
	sounds[SND_PAIN2] = gi.soundindex("monsters/rat/pain2.wav");

	sounds[SND_CHATTER1] = gi.soundindex("monsters/rat/chatter1.wav");
	sounds[SND_CHATTER2] = gi.soundindex("monsters/rat/chatter2.wav");
	sounds[SND_CHATTER3] = gi.soundindex("monsters/rat/chatter3.wav");

	sounds[SND_CHEW1] = gi.soundindex("monsters/rat/chew1.wav");
	sounds[SND_CHEW2] = gi.soundindex("monsters/rat/chew2.wav");
	sounds[SND_CHEW3] = gi.soundindex("monsters/rat/chew3.wav");

	sounds[SND_SWALLOW] = gi.soundindex("monsters/rat/swallow.wav");

	sounds[SND_DIE] = gi.soundindex("monsters/rat/death1.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/rat/gib.wav");

	res_info.numSounds = NUM_SOUNDS;
	res_info.sounds = sounds;

	classStatics[CID_RAT].resInfo = &res_info;
}

// QUAKED monster_rat (1 .5 0) (-16 -16 -0) (16 16 32) AMBUSH ASLEEP EATING 8 16 32 64 128 256 MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// The rat.

// Spawnflags:
// AMBUSH - Will not be woken up by other monsters or shots from player.
// ASLEEP - will not appear until triggered.
// EATING - Chomp chomp... chewie chomp.
// COWARD - Runs away.

// Variables:
// wakeup_target	- Monsters will fire this target the first time it wakes up (only once).
// pain_target		- Monsters will fire this target the first time it gets hurt (only once).
void SP_monster_rat(edict_t* self)
{
	// Generic Monster Initialization.
	if (!M_WalkmonsterStart(self)) // Failed initialization.
		return;

	if (self->health == 0)
		self->health = RAT_HEALTH;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->solid = SOLID_BBOX;
	self->mass = RAT_MASS;
	self->movetype = PHYSICSTYPE_STEP;
	self->materialtype = MAT_FLESH;
	self->yaw_speed = 20.0f;

	self->s.modelindex = (byte)classStatics[CID_RAT].resInfo->modelIndex;
	self->s.skinnum = 0;

	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	VectorClear(self->knockbackvel);

	VectorCopy(STDMinsForClass[self->classID], self->mins);
	VectorCopy(STDMaxsForClass[self->classID], self->maxs);

	if (self->s.scale == 0.0f) //mxd. 'if (self->monsterinfo.scale)' in original logic.
	{
		self->s.scale = MODEL_SCALE;
		self->monsterinfo.scale = self->s.scale;
	}

	self->viewheight = (int)(self->maxs[2] * 0.5f * self->s.scale);

	// Init this to -1 because we can't check for supporters in this function.
	self->monsterinfo.supporters = -1;

	self->msgHandler = DefaultMsgHandler;
	self->use = RatUse;
	self->touch = RatTouch;

	if (self->spawnflags & MSF_EATING)
	{
		G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);

		if (self->wakeup_distance == 0.0f)
			self->wakeup_distance = 300.0f;
	}
	else
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

// QUAKED monster_rat_giant (1 .5 0) (-16 -16 0) (16 16 32) AMBUSH ASLEEP EATING 8 16 32 64 128 256 MELEE_LEAD STALK COWARD EXTRA1 EXTRA2 EXTRA3 EXTRA4
// A giant rat with a second skin and a bit tougher.

// Spawnflags:
// AMBUSH - Will not be woken up by other monsters or shots from player.
// ASLEEP - will not appear until triggered.
// EATING - Chomp chomp... chewie chomp.
// COWARD - Runs away.

// Variables:
// wakeup_target	- Monsters will fire this target the first time it wakes up (only once).
// pain_target		- Monsters will fire this target the first time it gets hurt (only once).
void SP_monster_rat_giant(edict_t* self)
{
	// Generic Monster Initialization.
	if (!M_WalkmonsterStart(self)) // Failed initialization.
		return;

	if (self->health == 0)
		self->health = RAT_HEALTH * 4;

	self->health = MonsterHealth(self->health);
	self->max_health = self->health;

	self->solid = SOLID_BBOX;
	self->mass = RAT_MASS * 2;
	self->movetype = PHYSICSTYPE_STEP;
	self->materialtype = MAT_FLESH;
	self->yaw_speed = 20.0f;

	self->s.modelindex = (byte)gi.modelindex("models/monsters/rat/superduperat/tris.fm");
	self->s.skinnum = 0;

	self->ai_mood_flags |= AI_MOOD_FLAG_PREDICT;
	VectorClear(self->knockbackvel);

	// This will be scaled up later.
	VectorSet(self->mins, -8.0f, -8.0f, 0.0f);
	VectorSet(self->maxs,  8.0f,  8.0f, 16.0f);

	self->s.scale = max(2.0f, self->s.scale);
	self->monsterinfo.scale = self->s.scale;

	self->viewheight = (int)(self->maxs[2] * 0.5f * self->s.scale);

	// Init this to -1 because we can't check for supporters in this function.
	self->monsterinfo.supporters = -1;

	self->msgHandler = DefaultMsgHandler;
	self->use = RatUse;
	self->touch = RatTouch;

	if (self->spawnflags & MSF_EATING)
	{
		self->monsterinfo.aiflags |= AI_EATING; //TODO: not set in SP_monster_rat(). Which is correct?..
		G_PostMessage(self, MSG_EAT, PRI_DIRECTIVE, NULL);

		if (self->wakeup_distance == 0.0f)
			self->wakeup_distance = 300.0f;
	}
	else
	{
		G_PostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}
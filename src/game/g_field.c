//
// g_field.c
//
// Copyright 1998 Raven Software
//

#include "g_field.h" //mxd
#include "g_combat.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_misc.h" //mxd
#include "g_trigger.h" //mxd
#include "mg_guide.h" //mxd
#include "m_stats.h"
#include "p_main.h"
#include "p_teleport.h"
#include "Vector.h"
#include "g_local.h"

static void InitField(edict_t* self)
{
	if (Vec3NotZero(self->s.angles))
		G_SetMovedir(self->s.angles, self->movedir);

	self->solid = SOLID_TRIGGER;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags = SVF_NOCLIENT;

	gi.setmodel(self, self->model);
	gi.linkentity(self);
}

#pragma region ========================== trigger_fogdensity ==========================

void FogDensityTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'FogDensity_touch' in original logic.
{
	// Only players can know about fog density changes
	if (other->client != NULL)
		other->client->ps.fog_density = ((self->target != NULL) ? (float)strtod(self->target, NULL) : 0.0f);
}

// QUAKED trigger_fogdensity (.5 .5 .5) ?
// Sets the value of r_fog_density and the fog color. //TODO: doesn't set the fog color!
// Variables:
// target	- fog density (.01 - .0001)
// color	- red green blue values (0 0 0), range of 0.0 - 1.0.
void SP_trigger_fogdensity(edict_t* self)
{
	InitField(self);

	self->touch = FogDensityTouch;
	self->solid = SOLID_TRIGGER;
}

#pragma endregion

#pragma region ========================== trigger_push ==========================

#define SF_FORCE_ONCE	1

void TriggerPushTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'push_touch' in original logic.
{
	if (other->health > 0)
	{
		if (other->client != NULL) // A player?
		{
			// Don't take falling damage immediately from this.
			VectorCopy(other->velocity, other->client->playerinfo.oldvelocity);
			other->client->playerinfo.flags |= PLAYER_FLAG_USE_ENT_POS;
			other->groundentity = NULL;
		}

		vec3_t forward;
		vec3_t up;
		AngleVectors(self->s.angles, forward, NULL, up);

		VectorMA(other->velocity, self->speed, forward, other->velocity);
		VectorMA(other->velocity, self->speed, up, other->velocity);
	}

	G_UseTargets(self, self);

	if (self->spawnflags & SF_FORCE_ONCE)
		G_FreeEdict(self);
}

void TriggerPushActivated(edict_t* self, edict_t* activator) //mxd. Named 'push_touch_trigger' in original logic.
{
	TriggerPushTouch(self, activator, NULL, NULL);
}

static void TriggerPushDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'TrigPush_Deactivate' in original logic.
{
	self->solid = SOLID_NOT;
	self->touch = NULL;
}

static void TriggerPushActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'TrigPush_Activate' in original logic.
{
	self->solid = SOLID_TRIGGER;
	self->touch = TriggerPushTouch;

	gi.linkentity(self);
}

void TriggerPushStaticsInit(void)
{
	classStatics[CID_TRIG_PUSH].msgReceivers[G_MSG_SUSPEND] = TriggerPushDeactivate;
	classStatics[CID_TRIG_PUSH].msgReceivers[G_MSG_UNSUSPEND] = TriggerPushActivate;
}

// QUAKED trigger_push (.5 .5 .5) ? FORCE_ONCE
// Pushes the player.

// Spawnflags:
// FORCE_ONCE - Pushes once and then goes away.

// Variables:
// speed	- How fast the player is pushed (default 500).
// angle	- The angle to push the player along the X,Y axis.
// zangle	- The up direction to push the player (0 is straight up, 180 is straight down).
void SP_trigger_push(edict_t* self)
{
	TriggerInit(self);

	self->solid = SOLID_TRIGGER;
	self->s.angles[2] = st.zangle;

	if (self->speed == 0.0f)
		self->speed = 500;

	// Can't really use the normal trigger setup, because it doesn't update velocity often enough.
	self->touch = TriggerPushTouch;
	self->TriggerActivated = TriggerPushActivated;
}

#pragma endregion

#pragma region ========================== trigger_damage ==========================

#define SF_START_OFF		1 //mxd
#define SF_TOGGLE			2 //mxd
#define SF_SILENT			4 //mxd
#define SF_NO_PROTECTION	8 //mxd
#define SF_SLOW				16 //mxd

void TriggerDamageUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'DamageField_Use' in original logic.
{
	self->solid = ((self->solid == SOLID_NOT) ? SOLID_TRIGGER : SOLID_NOT);

	if (!(self->spawnflags & SF_TOGGLE))
		self->use = NULL;
}

void TriggerDamageTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'DamageField_Touch' in original logic.
{
	if (other->takedamage == DAMAGE_NO || self->timestamp > level.time)
		return;

	self->timestamp = level.time + ((self->spawnflags & SF_SLOW) ? 1.0f : FRAMETIME);

	if (!(self->spawnflags & SF_SILENT) && (level.framenum % 10) == 0)
		gi.sound(other, CHAN_AUTO, self->noise_index, 1.0f, ATTN_NORM, 0.0f);

	const int dmg_flags = ((self->spawnflags & SF_NO_PROTECTION) ? DAMAGE_NO_PROTECTION : 0);
	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dmg_flags | DAMAGE_SPELL | DAMAGE_AVOID_ARMOR, MOD_DIED);

	G_UseTargets(self, self);
}

static void TriggerDamageDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'TrigDamage_Deactivate' in original logic.
{
	self->solid = SOLID_NOT;
	self->use = NULL;
}

static void TriggerDamageActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'TrigDamage_Activate' in original logic.
{
	self->solid = SOLID_TRIGGER;
	self->use = TriggerDamageUse;

	gi.linkentity(self);
}

void TriggerDamageStaticsInit(void)
{
	classStatics[CID_TRIG_DAMAGE].msgReceivers[G_MSG_SUSPEND] = TriggerDamageDeactivate;
	classStatics[CID_TRIG_DAMAGE].msgReceivers[G_MSG_UNSUSPEND] = TriggerDamageActivate;
}

// QUAKED trigger_damage (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
// Any entity that touches this will be damaged. Does dmg points of damage each server frame.

// Spawnflags:
// SILENT			- Suppresses playing the sound.
// SLOW				- Changes the damage rate to once per second.
// NO_PROTECTION	- NOTHING stops the damage.

// Variables:
// dmg - default 5 (whole numbers only).
void SP_trigger_damage(edict_t* self)
{
	if (DEATHMATCH && self->dmg > 100)
	{
		self->spawnflags = DEATHMATCH_RANDOM;
		self->classID = CID_TELEPORTER;
		SP_misc_teleporter(self);

		return;
	}

	InitField(self);

	self->movetype = PHYSICSTYPE_NONE;
	self->msgHandler = DefaultMsgHandler;
	self->touch = TriggerDamageTouch;
	self->solid = ((self->spawnflags & SF_START_OFF) ? SOLID_NOT : SOLID_TRIGGER);

	if (self->dmg == 0)
		self->dmg = 5;

	if (self->spawnflags & SF_TOGGLE)
		self->use = TriggerDamageUse;

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== trigger_gravity ==========================

void TriggerGravityTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'GravityField_Touch' in original logic.
{
	other->gravity = self->gravity;
	G_UseTargets(self, self);
}

// QUAKED trigger_gravity (.5 .5 .5) ?
// Changes the Touching entities gravity.
// Variables:
// gravity - Target gravity. 1.0 is standard gravity for the level.
void SP_trigger_gravity(edict_t* self)
{
	if (st.gravity == NULL)
	{
		gi.dprintf("trigger_gravity without gravity set at %s\n", vtos(self->s.origin));
		G_FreeEdict(self);
	}
	else
	{
		InitField(self);
		self->gravity = (float)(Q_atoi(st.gravity));
		self->touch = TriggerGravityTouch;
	}
}

#pragma endregion

#pragma region ========================== trigger_monsterjump ==========================

void TriggerMonsterJumpTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'MonsterJumpField_Touch' in original logic.
{
	if ((other->flags & (FL_FLY | FL_SWIM)) || (other->svflags & SVF_DEADMONSTER) || !(other->svflags & SVF_MONSTER))
		return;

	VectorMA(other->velocity, self->speed, self->movedir, other->velocity);
	other->velocity[2] += self->accel;

	other->groundentity = NULL;
}

// QUAKED trigger_monsterjump (.5 .5 .5) ?
// Walking monsters that touch this will jump in the direction of the trigger's angle.
// Variables:
// speed	- The speed thrown forward (default 200).
// height	- The speed thrown upwards (default 200).
void SP_trigger_monsterjump(edict_t* self)
{
	if (self->s.angles[YAW] == 0.0f)
		self->s.angles[YAW] = 360.0f;

	InitField(self);

	if (self->speed == 0.0f)
		self->speed = 200.0f;

	self->accel = (st.height == 0 ? 200.0f : (float)st.height);
	self->touch = TriggerMonsterJumpTouch;
}

#pragma endregion

#pragma region ========================== trigger_goto_buoy ==========================

#define SF_BUOY_TOUCH			1
#define SF_BUOY_IGNORE_ENEMY	2
#define SF_BUOY_TELEPORT_SAFE	4
#define SF_BUOY_TELEPORT_UNSAFE	8
#define SF_BUOY_FIXED			16
#define SF_BUOY_STAND			32
#define SF_BUOY_WANDER			64

static void TriggerGotoBuoyExecute(const edict_t* self, edict_t* monster, edict_t* activator) //mxd. Named 'trigger_goto_buoy_execute' in original logic.
{
	const buoy_t* found_buoy = NULL;

	for (int i = 0; i < level.active_buoys; i++)
	{
		found_buoy = &level.buoy_list[i];

		if (found_buoy->targetname != NULL && strcmp(found_buoy->targetname, self->pathtarget) == 0)
			break;
	}

	if (found_buoy == NULL)
	{
		vec3_t org;
		VectorMA(self->mins, 0.5f, self->maxs, org);
		gi.dprintf("trigger_goto_buoy at %s can't find it's pathtargeted buoy %s\n", vtos(org), self->pathtarget);

		return;
	}

	if (self->spawnflags & SF_BUOY_TELEPORT_SAFE)
	{
		MG_MonsterAttemptTeleport(monster, found_buoy->origin, false);
		return;
	}

	if (self->spawnflags & SF_BUOY_TELEPORT_UNSAFE)
	{
		MG_MonsterAttemptTeleport(monster, found_buoy->origin, true);
		return;
	}

	if (self->spawnflags & SF_BUOY_IGNORE_ENEMY) // Make him ignore enemy until gets to dest buoy.
		monster->ai_mood_flags |= AI_MOOD_FLAG_IGNORE_ENEMY;

	monster->spawnflags &= ~MSF_FIXED;
	monster->ai_mood_flags |= AI_MOOD_FLAG_FORCED_BUOY;
	monster->forced_buoy = found_buoy->id;
	monster->ai_mood = AI_MOOD_NAVIGATE;

	if (monster->enemy == NULL)
		monster->enemy = activator;

	MG_RemoveBuoyEffects(monster);
	MG_MakeConnection(monster, NULL, false);

	if (self->spawnflags & SF_BUOY_FIXED)
		monster->ai_mood_flags |= AI_MOOD_FLAG_GOTO_FIXED;

	if (self->spawnflags & SF_BUOY_STAND)
		monster->ai_mood_flags |= AI_MOOD_FLAG_GOTO_STAND;

	if (self->spawnflags & SF_BUOY_WANDER)
		monster->ai_mood_flags |= AI_MOOD_FLAG_GOTO_WANDER;

	// Make him check mood NOW and get going! Don't wait for current anim to finish!
	if (classStatics[monster->classID].msgReceivers[MSG_CHECK_MOOD] != NULL)
	{
		G_PostMessage(monster, MSG_CHECK_MOOD, PRI_DIRECTIVE, "i", monster->ai_mood);
	}
	else
	{
		// No MSG_CHECK_MOOD message handler, just send a run and let him wait, I guess!
		monster->mood_nextthink = 0;
		G_PostMessage(monster, MSG_RUN, PRI_DIRECTIVE, NULL);
	}
}

void TriggerGotoBuoyTouchThink(edict_t* self) //mxd. Named 'trigger_goto_buoy_touch_go' in original logic.
{
	if (self->enemy != NULL && self->enemy->health > 0 && (self->enemy->svflags & SVF_MONSTER) && (self->enemy->monsterinfo.aiflags & AI_USING_BUOYS))
		TriggerGotoBuoyExecute(self, self->enemy, self->activator);
}

void TriggerGotoBuoyTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'trigger_goto_buoy_touch' in original logic.
{
	if (level.time < self->air_finished || other->health <= 0 || !(other->svflags & SVF_MONSTER) || !(other->monsterinfo.aiflags & AI_USING_BUOYS))
		return;

	self->activator = other->enemy;

	if (self->delay > 0.0f)
	{
		self->enemy = other;
		self->think = TriggerGotoBuoyTouchThink;
		self->nextthink = level.time + self->delay;

		return;
	}

	TriggerGotoBuoyExecute(self, other, self->activator);

	if (self->wait == -1.0f)
	{
		self->touch = NULL;
		self->use = NULL;
	}
	else
	{
		self->air_finished = level.time + self->wait;
	}
}

void TriggerGotoBuoyUseThink(edict_t* self) //mxd. Named 'trigger_goto_buoy_use_go' in original logic.
{
	edict_t* monster = NULL;
	monster = G_Find(monster, FOFS(targetname), self->target);

	if (monster != NULL && monster->health > 0 && (monster->svflags & SVF_MONSTER) && (monster->monsterinfo.aiflags & AI_USING_BUOYS))
		TriggerGotoBuoyExecute(self, monster, self->activator);
	else
		gi.dprintf("ERROR: trigger_goto_buoy can't find it's target monster %s\n", self->pathtarget);
}

void TriggerGotoBuoyUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'trigger_goto_buoy_use' in original logic.
{
	if (level.time < self->air_finished)
		return;

	self->activator = activator;

	if (self->delay > 0.0f)
	{
		self->think = TriggerGotoBuoyUseThink;
		self->nextthink = level.time + self->delay;

		return;
	}

	TriggerGotoBuoyUseThink(self);
	self->air_finished = level.time + self->wait; //TODO: add dedicated edict_t property instead of hijacking 'air_finished'?
}

// QUAKED trigger_goto_buoy (.5 .5 .5) ? BUOY_TOUCH BUOY_IGNORE_ENEMY BUOY_TELEPORT_SAFE BUOY_TELEPORT_UNSAFE BUOY_FIXED BUOY_STAND BUOY_WANDER
// A monster touching this trigger will find the buoy with the "pathtarget" targetname and head for it if it can.
// This is NOT a touch trigger for a player, only monsters should ever touch it and only if the TOUCH spawnflag is on.
// To have a player touch-trigger it, have the player touch a normal trigger that fires this trigger.
// Otherwise, acts like a normal trigger.

// Spawnflags:
// BUOY_TOUCH			- Should be able to be touch-activated by monsters. NOTE: This will try to force the entity touching the trigger
//						  to it's buoy - should NOT be intended to be touched by anything but monsters.
// BUOY_IGNORE_ENEMY	- Monster will ignore his enemy until he gets to his target buoy (or until attacked or woken up some other way,
//						  working on preventing this if desired).
// BUOY_TELEPORT_SAFE	- Make monster teleport to target buoy only if there is nothing there and the player cannot see the monster and/or destination buoy.
// BUOY_TELEPORT_UNSAFE	- Same as BUOY_TELEPORT_SAFE, but ignores whether or not the player can see the monster and/or desination buoy.
//						  If you wish to make an assassin teleport to a buoy, use BUOY_TELEPORT_UNSAFE since he doesn't need to hide the teleport from the player.
// BUOY_FIXED			- Upon arriving at the target buoy, the monster will become fixed and wait for an enemy (will not move from that spot no matter what).
// BUOY_STAND			- Upon arriving at the target buoy, the monster will forget any enemy it has and simply stand around there until it sees another enemy.
// BUOY_WANDER			- Upon arriving at the target buoy, the monster will forget any enemy it has and begin to wander around that buoy's vicinity.

// Variables:
// pathtarget	- Targetname of buoy monster should head to.
// wait			- How long to wait between firings.
// delay		- How long to wait after being activated to actually try to send the monster away.
void SP_trigger_goto_buoy(edict_t* self)
{
	InitField(self);

	if (self->pathtarget == NULL)
	{
		gi.dprintf("trigger_goto_buoy with no pathtarget!\n");
		G_FreeEdict(self);

		return;
	}

	if (self->spawnflags & SF_BUOY_TOUCH)
		self->touch = TriggerGotoBuoyTouch;

	if (self->targetname != NULL)
	{
		if (self->target == NULL)
			gi.dprintf("targeted trigger_goto_buoy with no monster target!\n");

		self->use = TriggerGotoBuoyUse;
	}
}

#pragma endregion
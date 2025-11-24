//
// g_func_Door.c -- Originally part of g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Door.h"
#include "g_combat.h"
#include "g_debris.h"
#include "g_DefaultMessageHandler.h"
#include "g_func_Utility.h"
#include "EffectFlags.h"
#include "Vector.h"

#pragma region ========================== func_door, func_door_rotating, func_water ==========================

#define SF_DOOR_ROTATING_X_AXIS		64
#define SF_DOOR_ROTATING_Y_AXIS		128
#define DOOR_MOVE_LOOP				(-2) //mxd

static void FuncDoorUseAreaportals(const edict_t* self, const qboolean open) //mxd. Named 'door_use_areaportals' in original logic.
{
	if (self->target == NULL)
		return;

	edict_t* target = NULL;
	while ((target = G_Find(target, FOFS(targetname), self->target)) != NULL)
		if (Q_stricmp(target->classname, "func_areaportal") == 0)
			gi.SetAreaPortalState(target->style, open);
}

void FuncDoorHitTop(edict_t* self) //mxd. Named 'door_hit_top' in original logic.
{
	FuncPlayMoveEndSound(self); //mxd
	self->moveinfo.state = STATE_TOP;

	if (self->spawnflags & SF_DOOR_TOGGLE)
		return;

	if (self->moveinfo.wait >= 0.0f)
	{
		self->think = FuncDoorGoDown;
		self->nextthink = level.time + self->moveinfo.wait;
	}
	else if ((int)self->moveinfo.wait == DOOR_MOVE_LOOP) // Endless cycle.
	{
		self->think = FuncDoorGoDown;
		self->nextthink = level.time + FRAMETIME; // Next frame is soon enough to fire this off.
	}
}

void FuncDoorHitBottom(edict_t* self) //mxd. Named 'door_hit_bottom' in original logic.
{
	FuncPlayMoveEndSound(self); //mxd
	self->moveinfo.state = STATE_BOTTOM;

	if ((int)self->moveinfo.wait == DOOR_MOVE_LOOP) // Endless cycle.
		FuncDoorGoUp(self, NULL);
	else
		FuncDoorUseAreaportals(self, false);
}

void FuncDoorGoDown(edict_t* self) //mxd. Named 'door_go_down' in original logic.
{
	FuncPlayMoveStartSound(self); //mxd

	if (self->max_health > 0)
	{
		self->takedamage = DAMAGE_YES;
		self->health = self->max_health;
	}

	self->moveinfo.state = STATE_DOWN;

	if (strcmp(self->classname, "func_door") == 0)
		MoveCalc(self, self->moveinfo.start_origin, FuncDoorHitBottom);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
		AngleMoveCalc(self, FuncDoorHitBottom);
}

static void FuncDoorGoUp(edict_t* self, edict_t* activator) //mxd. Named 'door_go_up' in original logic.
{
	if (self->moveinfo.state == STATE_UP)
		return; // Already going up.

	if (self->moveinfo.state == STATE_TOP)
	{
		// Reset top wait time.
		if (self->moveinfo.wait >= 0.0f)
			self->nextthink = level.time + self->moveinfo.wait;
		else if ((int)self->moveinfo.wait == DOOR_MOVE_LOOP) // Endless cycle.
			self->nextthink = level.time + FRAMETIME; //mxd. Add FRAMETIME (mostly for consistency's sake).

		return;
	}

	FuncPlayMoveStartSound(self); //mxd
	self->moveinfo.state = STATE_UP;

	if (strcmp(self->classname, "func_door") == 0)
		MoveCalc(self, self->moveinfo.end_origin, FuncDoorHitTop);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
		AngleMoveCalc(self, FuncDoorHitTop);

	G_UseTargets(self, activator);
	FuncDoorUseAreaportals(self, true);
}

// Checks to see if a rotating door will get in activator's way when it opens.
static qboolean FuncDoorSmartSideCheck(const edict_t* self, const edict_t* activator) //mxd. Named 'smart_door_side_check' in original logic.
{
	if (activator == NULL)
		return false;

	// Make a plane containing the origins of the origin brush, the door, and a point which is the sum of movedir
	// (slightly rearranged (x, z, y)) and one of the others.

	vec3_t door_points[3];
	VectorCopy(self->s.origin, door_points[0]); // Origin brush origin.
	VectorAdd(self->s.origin, self->mins, door_points[1]);
	VectorMA(door_points[1], 0.5f, self->size, door_points[1]); // Door center.

	door_points[2][0] = self->s.origin[0] + self->movedir[2];
	door_points[2][1] = self->s.origin[1] + self->movedir[0];
	door_points[2][2] = self->s.origin[2] + self->movedir[1]; // Third point.

	vec3_t in_plane[2];
	VectorSubtract(door_points[1], door_points[0], in_plane[0]);
	VectorSubtract(door_points[2], door_points[0], in_plane[1]);

	vec3_t normal;
	CrossProduct(in_plane[0], in_plane[1], normal);

	vec3_t to_player;
	VectorSubtract(activator->s.origin, door_points[1], to_player);

	return DotProduct(normal, to_player) < 0.0f;
}

void FuncDoorUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'smart_door_side_check' in original logic.
{
	if (Vec3IsZero(self->avelocity) && strcmp(self->classname, "func_door_rotating") == 0 && (self->spawnflags & SF_DOOR_SWINGAWAY))
	{
		if (FuncDoorSmartSideCheck(self, activator))
		{
			VectorNegate(self->movedir, self->movedir);
			VectorNegate(self->moveinfo.end_angles, self->moveinfo.end_angles);
		}
	}

	if (self->flags & FL_TEAMSLAVE)
		return;

	// Trigger all paired doors.
	if ((self->spawnflags & SF_DOOR_TOGGLE) && (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP))
	{
		for (edict_t* ent = self; ent != NULL; ent = ent->teamchain)
		{
			ent->message = NULL;
			ent->isBlocking = NULL;
			FuncDoorGoDown(ent);
		}
	}
	else
	{
		for (edict_t* ent = self; ent != NULL; ent = ent->teamchain)
		{
			ent->message = NULL;
			ent->isBlocking = NULL;
			FuncDoorGoUp(ent, activator);
		}
	}
}

void FuncDoorTriggerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'Touch_DoorTrigger' in original logic.
{
	if (other->health <= 0 || (!(other->svflags & SVF_MONSTER) && other->client == NULL))
		return;

	if ((self->owner->spawnflags & SF_DOOR_NOMONSTER) && (other->svflags & SVF_MONSTER))
		return;

	if (level.time < self->touch_debounce_time)
		return;

	self->touch_debounce_time = level.time + 1.0f;
	FuncDoorUse(self->owner, other, other);
}

void FuncDoorCalcMoveSpeedThink(edict_t* self) //mxd. Named 'Think_CalcMoveSpeed' in original logic.
{
	if (self->flags & FL_TEAMSLAVE)
	{
		self->think = NULL;
		return; // Only the team master does this.
	}

	// Find the smallest distance any member of the team will be moving.
	float min_dist = fabsf(self->moveinfo.distance);

	for (const edict_t* ent = self->teamchain; ent != NULL; ent = ent->teamchain)
	{
		const float dist = fabsf(ent->moveinfo.distance);
		min_dist = min(dist, min_dist);
	}

	const float time = min_dist / self->moveinfo.speed;

	// Adjust speeds so they will all complete at the same time.
	for (edict_t* ent = self; ent != NULL; ent = ent->teamchain)
	{
		const float new_speed = fabsf(ent->moveinfo.distance) / time;
		const float ratio = new_speed / ent->moveinfo.speed;

		if (ent->moveinfo.accel == ent->moveinfo.speed)
			ent->moveinfo.accel = new_speed;
		else
			ent->moveinfo.accel *= ratio;

		if (ent->moveinfo.decel == ent->moveinfo.speed)
			ent->moveinfo.decel = new_speed;
		else
			ent->moveinfo.decel *= ratio;

		ent->moveinfo.speed = new_speed;
	}

	gi.linkentity(self);
	self->think = NULL;
}

void FuncDoorSpawnDoorTriggerThink(edict_t* self) //mxd. Named 'Think_SpawnDoorTrigger' in original logic.
{
	self->think = NULL;

	if (self->flags & FL_TEAMSLAVE)
		return; // Only the team leader spawns a trigger.

	vec3_t mins;
	vec3_t maxs;
	VectorCopy(self->absmin, mins);
	VectorCopy(self->absmax, maxs);

	for (const edict_t* ent = self->teamchain; ent != NULL; ent = ent->teamchain)
	{
		AddPointToBounds(ent->absmin, mins, maxs);
		AddPointToBounds(ent->absmax, mins, maxs);
	}

	// Expand on XY axis.
	for (int i = 0; i < 2; i++)
	{
		mins[i] -= 60.0f;
		maxs[i] += 60.0f;
	}

	edict_t* trigger = G_Spawn();

	VectorCopy(mins, trigger->mins);
	VectorCopy(maxs, trigger->maxs);
	trigger->owner = self;
	trigger->solid = SOLID_TRIGGER;
	trigger->movetype = PHYSICSTYPE_NONE;
	trigger->touch = FuncDoorTriggerTouch;

	if (self->spawnflags & SF_DOOR_START_OPEN)
		FuncDoorUseAreaportals(self, true);

	FuncDoorCalcMoveSpeedThink(self);

	gi.linkentity(trigger);
}

void FuncDoorBlocked(edict_t* self, edict_t* other) //mxd. Named 'door_blocked' in original logic.
{
	if ((other->svflags & SVF_MONSTER) && other->client == NULL && !(other->svflags & SVF_BOSS))
	{
		// Give it a chance to go away on it's own terms (like gibs).
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 3000, 1, DAMAGE_AVOID_ARMOR, MOD_CRUSH);

		// If it's still there, nuke it.
		if (other->health > 0)
			BecomeDebris(other);

		return;
	}

	if (self->spawnflags & SF_DOOR_CRUSHER)
	{
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg * 10, 1, 0, MOD_CRUSH);
		return;
	}

	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	// If a door has a negative wait, it would never come back if blocked, (unless -2) so let it just squash the object to death real fast.
	if (self->moveinfo.wait >= 0.0f || (int)self->moveinfo.wait == DOOR_MOVE_LOOP)
	{
		if (self->moveinfo.state == STATE_DOWN)
		{
			for (edict_t* ent = self->teammaster; ent != NULL; ent = ent->teamchain)
				FuncDoorGoUp(ent, ent->activator);
		}
		else
		{
			for (edict_t* ent = self->teammaster; ent != NULL; ent = ent->teamchain)
				FuncDoorGoDown(ent);
		}
	}
}

void FuncDoorDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point) //mxd. Named 'door_killed' in original logic.
{
	for (edict_t* ent = self->teammaster; ent != NULL; ent = ent->teamchain)
	{
		ent->health = ent->max_health;
		ent->takedamage = DAMAGE_NO;
	}

	FuncDoorUse(self->teammaster, attacker, attacker);
}

void FuncDoorTouch(edict_t* self, trace_t* trace) //mxd. Named 'door_killed' in original logic.
{
	const edict_t* other = trace->ent;

	if (other->client != NULL && level.time >= self->touch_debounce_time)
	{
		self->touch_debounce_time = level.time + 5.0f;
		gi.levelmsg_centerprintf(other, (short)Q_atoi(self->message));
	}
}

// QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER NOMONSTER ANIMATED TOGGLE ANIMATED_FAST

// Spawnflags:
// START_OPEN	- The door to moves to its destination when spawned, and operate in reverse.
//				  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
// NOMONSTER	- Monsters will not trigger this door.
// TOGGLE		- Wait in both the start and end states for a trigger event.

// Variables:
// message		- Is printed when the door is touched if it is a trigger door and it hasn't been fired yet.
// angle		- Determines the opening direction.
// targetname	- If set, no touch field will be spawned and a remote button or trigger field activates the door.
// health		- If set, door must be shot open.
// height		- If set, tells how far up door opens.
// speed		- Movement speed (default 100).
// wait			- Wait before returning (default 3, -1 = never return,-2 = never stop cycle).
// lip			- Lip remaining at end of move (default 8).
// dmg			- Damage to inflict when blocked (default 2).
// sounds:
//		0)	Silent.
//		1)	Generic door.
//		2)	Heavy stone door.
//		3)  For swing arm on palace level.
//		4)  For stone bridge in palace level.
//		5)  Small/medium wood door swinging.
//		6)  Large/huge wood door swinging.
//		7)  Medium sized stone/wood door sliding.
//		8)  Large stone/wood sliding door or portcullis.
//		9)  Average metal door swinging.
//		10) Fast sliding doors.
//		11) Hive, Metal, Multipaneled sliding.
//		12) Huge stone door swinging.
//		13) Medium/large elevator.
//		14) Crane (warehouse).
//		15) Hammer-like pump in oglemine1.
//		16) Sliding metal table in cloudlabs.
//		17) Lab table which rotates up to ceiling - cloublabs.
//		18) Piston sound.
//		19) Short, sharp metal clang.
//		20) Something going under water.
//		21) The bam sound.
void SP_func_door(edict_t* self)
{
	FuncDoorSetSounds(self);
	G_SetMovedir(self->s.angles, self->movedir);

	self->msgHandler = DefaultMsgHandler;

	self->movetype = PHYSICSTYPE_PUSH;
	self->solid = SOLID_BSP;
	self->blocked = FuncDoorBlocked;
	self->use = FuncDoorUse;

	gi.setmodel(self, self->model);
	gi.linkentity(self);

	if (self->speed == 0.0f)
		self->speed = 100.0f;

	if (self->accel == 0.0f)
		self->accel = self->speed;

	if (self->decel == 0.0f)
		self->decel = self->speed;

	if (self->wait == 0.0f)
		self->wait = 3.0f;

	if (st.lip == 0)
		st.lip = 8;

	if (self->dmg == 0)
		self->dmg = 2;

	// Calculate second position.
	VectorCopy(self->s.origin, self->pos1);

	vec3_t abs_movedir;
	VectorAbs(self->movedir, abs_movedir);

	if (st.height == 0)
		self->moveinfo.distance = DotProduct(abs_movedir, self->size) - (float)st.lip;
	else
		self->moveinfo.distance = abs_movedir[0] * self->size[0] + abs_movedir[1] * self->size[1] + abs_movedir[2] * (float)st.height;

	VectorMA(self->pos1, self->moveinfo.distance, self->movedir, self->pos2);

	// If it starts open, switch the positions.
	if (self->spawnflags & SF_DOOR_START_OPEN)
	{
		VectorCopy(self->pos2, self->s.origin);
		VectorCopy(self->pos1, self->pos2);
		VectorCopy(self->s.origin, self->pos1);
	}

	self->moveinfo.state = STATE_BOTTOM;

	if (self->health > 0)
	{
		self->takedamage = DAMAGE_YES;
		self->die = FuncDoorDie;
		self->max_health = self->health;
	}
	else if (self->targetname != NULL && self->message != NULL)
	{
		gi.soundindex("misc/talk.wav");
		self->isBlocking = FuncDoorTouch;
	}

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->accel;
	self->moveinfo.decel = self->decel;
	self->moveinfo.wait = self->wait;
	VectorCopy(self->pos1, self->moveinfo.start_origin);
	VectorCopy(self->s.angles, self->moveinfo.start_angles);
	VectorCopy(self->pos2, self->moveinfo.end_origin);
	VectorCopy(self->s.angles, self->moveinfo.end_angles);

	VectorSubtract(self->maxs, self->mins, self->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, self->s.bmodel_origin);
	VectorAdd(self->mins, self->s.bmodel_origin, self->s.bmodel_origin);

	if (self->spawnflags & SF_DOOR_ANIMATED)
		self->s.effects |= EF_ANIM_ALL;

	if (self->spawnflags & SF_DOOR_ANIMATED_FAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	// To simplify logic elsewhere, make non-teamed doors into a team of one.
	if (self->team == NULL)
		self->teammaster = self;

	self->nextthink = level.time + FRAMETIME;

	if (self->health > 0 || self->targetname != NULL)
		self->think = FuncDoorCalcMoveSpeedThink;
	else
		self->think = FuncDoorSpawnDoorTriggerThink;
}

// QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS SWINGAWAY
// You need to have an origin brush as part of this entity. The center of that brush will be the point around which it is rotated.
// It will rotate around the Z axis by default. You can check either the X_AXIS or Y_AXIS box to change that.

// Spawnflags:
// START_OPEN	- The door to moves to its destination when spawned, and operate in reverse.
//				  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
// REVERSE		- Will cause the door to rotate in the opposite direction.
// NOMONSTER	- Monsters will not trigger this door.
// TOGGLE		- Wait in both the start and end states for a trigger event.
// SWINGAWAY	- Door will always swing away from the activator.

// Variables:
// distance		- How many degrees the door will be rotated.
// message		- Is printed when the door is touched if it is a trigger door and it hasn't been fired yet.
// angle		- Determines the opening direction.
// targetname	- If set, no touch field will be spawned and a remote button or trigger field activates the door.
// health		- If set, door must be shot open.
// speed		- Movement speed (default 100).
// wait			- Wait before returning (default 3, -1 = never return,-2 = never stop cycle).
// dmg			- Damage to inflict when blocked (default 2).
// sounds:
//		0)	Silent.
//		1)	Generic door.
//		2)	Heavy stone door.
//		3)  For swing arm on palace level.
//		4)  For stone bridge in palace level.
//		5)  Small/medium wood door swinging.
//		6)  Large/huge wood door swinging.
//		7)  Medium sized stone/wood door sliding.
//		8)  Large stone/wood sliding door or portcullis.
//		9)  Average metal door swinging.
//		10) Fast sliding doors.
//		11) Hive, Metal, Multipaneled sliding.
//		12) Huge stone door swinging.
//		13) Medium/large elevator.
//		14) Crane (warehouse).
//		15) Hammer-like pump in oglemine1.
//		16) Sliding metal table in cloudlabs.
//		17) Lab table which rotates up to ceiling - cloublabs.
//		18) Piston sound.
//		19) Short, sharp metal clang.
//		20) Something going under water.
//		21) The bam sound.
void SP_func_door_rotating(edict_t* ent)
{
	VectorClear(ent->s.angles);

	// Set the axis of rotation.
	VectorClear(ent->movedir);

	if (ent->spawnflags & SF_DOOR_ROTATING_X_AXIS)
		ent->movedir[2] = 1.0f;
	else if (ent->spawnflags & SF_DOOR_ROTATING_Y_AXIS)
		ent->movedir[0] = 1.0f;
	else // Z_AXIS
		ent->movedir[1] = 1.0f;

	// Check for reverse rotation.
	if (ent->spawnflags & SF_DOOR_REVERSE)
		VectorNegate(ent->movedir, ent->movedir);

	if (st.distance == 0)
	{
		gi.dprintf("%s at %s with no distance set\n", ent->classname, vtos(ent->s.origin));
		st.distance = 90;
	}

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	VectorCopy(ent->s.angles, ent->pos1);
	VectorMA(ent->s.angles, (float)st.distance, ent->movedir, ent->pos2);
	ent->moveinfo.distance = (float)st.distance;

	ent->movetype = PHYSICSTYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->blocked = FuncDoorBlocked;
	ent->use = FuncDoorUse;

	if (ent->speed == 0.0f)
		ent->speed = 100;

	if (ent->accel == 0.0f)
		ent->accel = ent->speed;

	if (ent->decel == 0.0f)
		ent->decel = ent->speed;

	if (ent->wait == 0.0f)
		ent->wait = 3.0f;

	if (ent->dmg == 0)
		ent->dmg = 2;

	FuncDoorSetSounds(ent);

	// If it starts open, switch the positions.
	if (ent->spawnflags & SF_DOOR_START_OPEN)
	{
		VectorCopy(ent->pos2, ent->s.angles);
		VectorCopy(ent->pos1, ent->pos2);
		VectorCopy(ent->s.angles, ent->pos1);
		VectorNegate(ent->movedir, ent->movedir);
	}

	if (ent->health > 0)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = FuncDoorDie;
		ent->max_health = ent->health;
	}

	if (ent->targetname != NULL && ent->message != NULL)
	{
		gi.soundindex("misc/talk.wav");
		ent->isBlocking = FuncDoorTouch;
	}

	ent->moveinfo.state = STATE_BOTTOM;
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy(ent->s.origin, ent->moveinfo.start_origin);
	VectorCopy(ent->pos1, ent->moveinfo.start_angles);
	VectorCopy(ent->s.origin, ent->moveinfo.end_origin);
	VectorCopy(ent->pos2, ent->moveinfo.end_angles);

	VectorSubtract(ent->maxs, ent->mins, ent->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, ent->s.bmodel_origin);
	VectorAdd(ent->mins, ent->s.bmodel_origin, ent->s.bmodel_origin);

	if (ent->spawnflags & SF_DOOR_ANIMATED)
		ent->s.effects |= EF_ANIM_ALL;

	if (ent->health > 0 || ent->targetname != NULL)
		ent->think = FuncDoorCalcMoveSpeedThink;
	else
		ent->think = FuncDoorSpawnDoorTriggerThink;

	ent->nextthink = level.time + FRAMETIME;
}

// QUAKED func_water (0 .5 .8) ? START_OPEN
// func_water is a moveable water brush. It must be targeted to operate. Use a non-water texture at your own risk.

// Spawnflags:
// START_OPEN - Causes the water to move to its destination when spawned and operate in reverse.

// Variables:
// angle	- Determines the opening direction (up or down only)
// speed	- Movement speed (default 25).
// wait		- Wait before returning (default -1, -1 = TOGGLE).
// lip		- Lip remaining at end of move (default 0).
// sounds: (yes, these need to be changed)
//		0)	No sound.
//		1)	Water.
//		2)	Lava.
void SP_func_water(edict_t* self)
{
	G_SetMovedir(self->s.angles, self->movedir);

	self->movetype = PHYSICSTYPE_PUSH;
	self->solid = SOLID_BSP;
	gi.setmodel(self, self->model);
	gi.linkentity(self);

	// Calculate second position.
	VectorCopy(self->s.origin, self->pos1);

	vec3_t abs_movedir;
	VectorAbs(self->movedir, abs_movedir);

	self->moveinfo.distance = DotProduct(abs_movedir, self->size) - (float)st.lip;
	VectorMA(self->pos1, self->moveinfo.distance, self->movedir, self->pos2);

	// If it starts open, switch the positions.
	if (self->spawnflags & SF_DOOR_START_OPEN)
	{
		VectorCopy(self->pos2, self->s.origin);
		VectorCopy(self->pos1, self->pos2);
		VectorCopy(self->s.origin, self->pos1);
	}

	VectorCopy(self->pos1, self->moveinfo.start_origin);
	VectorCopy(self->s.angles, self->moveinfo.start_angles);
	VectorCopy(self->pos2, self->moveinfo.end_origin);
	VectorCopy(self->s.angles, self->moveinfo.end_angles);

	VectorSubtract(self->maxs, self->mins, self->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, self->s.bmodel_origin);
	VectorAdd(self->mins, self->s.bmodel_origin, self->s.bmodel_origin);

	self->moveinfo.state = STATE_BOTTOM;

	if (self->speed == 0.0f)
		self->speed = 25.0f;

	self->moveinfo.accel = self->speed;
	self->moveinfo.decel = self->speed;
	self->moveinfo.speed = self->speed;

	if (self->wait == 0.0f)
		self->wait = -1.0f;

	self->moveinfo.wait = self->wait;

	self->use = FuncDoorUse;

	if (self->wait == -1.0f)
		self->spawnflags |= SF_DOOR_TOGGLE;

	self->classname = "func_door";
}

#pragma endregion

#pragma region ========================== func_door_secret ==========================

#define SF_SECRET_ALWAYS_SHOOT	1
#define SF_SECRET_1ST_LEFT		2
#define SF_SECRET_1ST_DOWN		4

void FuncDoorSecretUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'door_secret_use' in original logic.
{
	// Make sure we're not already moving.
	if (!VectorCompare(self->s.origin, vec3_origin))
		return;

	if (self->moveinfo.sound_start > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, self->pos1, FuncDoorSecretMove1);
	FuncDoorUseAreaportals(self, true);
}

void FuncDoorSecretMove1(edict_t* self) //mxd. Named 'door_secret_move1' in original logic.
{
	self->nextthink = level.time + 1.0f;
	self->think = FuncDoorSecretMove2;
}

void FuncDoorSecretMove2(edict_t* self) //mxd. Named 'door_secret_move2' in original logic.
{
	if (self->moveinfo.sound_middle > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, self->pos2, FuncDoorSecretMove3);
}

void FuncDoorSecretMove3(edict_t* self) //mxd. Named 'door_secret_move3' in original logic.
{
	if (self->moveinfo.sound_end > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_end, 1.0f, ATTN_IDLE, 0.0f);

	if (self->wait != -1.0f)
	{
		self->nextthink = level.time + self->wait;
		self->think = FuncDoorSecretMove4;
	}
}

void FuncDoorSecretMove4(edict_t* self) //mxd. Named 'door_secret_move4' in original logic.
{
	if (self->moveinfo.sound_middle > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, self->pos1, FuncDoorSecretMove5);
}

void FuncDoorSecretMove5(edict_t* self) //mxd. Named 'door_secret_move5' in original logic.
{
	if (self->moveinfo.sound_end > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_end, 1.0f, ATTN_IDLE, 0.0f);

	self->nextthink = level.time + 1.0f;
	self->think = FuncDoorSecretMove6;
}

void FuncDoorSecretMove6(edict_t* self) //mxd. Named 'door_secret_move6' in original logic.
{
	if (self->moveinfo.sound_start > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, vec3_origin, FuncDoorSecretDone);
}

void FuncDoorSecretDone(edict_t* self) //mxd. Named 'door_secret_done' in original logic.
{
	if (self->targetname == NULL || (self->spawnflags & SF_SECRET_ALWAYS_SHOOT))
	{
		self->health = 0;
		self->takedamage = DAMAGE_YES;
	}

	FuncDoorUseAreaportals(self, false);
}

void FuncDoorSecretBlocked(edict_t* self, edict_t* other) //mxd. Named 'door_secret_blocked' in original logic.
{
	if ((other->svflags & SVF_MONSTER) && other->client == NULL && !(other->svflags & SVF_BOSS))
	{
		// Give it a chance to go away on it's own terms (like gibs).
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 3000, 1, DAMAGE_AVOID_ARMOR, MOD_CRUSH);

		// If it's still there, nuke it.
		if (other->health > 0)
			BecomeDebris(other);

		return;
	}

	if (level.time >= self->touch_debounce_time)
	{
		self->touch_debounce_time = level.time + 0.5f;
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
	}
}

void FuncDoorSecretDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point) //mxd. Named 'door_secret_die' in original logic.
{
	self->takedamage = DAMAGE_NO;
	FuncDoorSecretUse(self, attacker, attacker);
}

// QUAKED func_door_secret (0 .5 .8) ? ALWAYS_SHOOT 1ST_LEFT 1ST_DOWN
// A secret door. Slide back and then to the side.

// Spawnflags:
// ALWAYS_SHOOT	- Door is shooteable even if targeted.
// 1ST_LEFT		- 1-st move is left of arrow.
// 1ST_DOWN		- 1-st move is down from arrow.

// Variables:
// angle	- Determines the direction.
// dmg		- Damage to inflict when blocked (default 2).
// wait		- How long to hold in the open position (default 5, -1 means hold).
// sounds:
//		0)	Silent.
//		1)	Generic door.
//		2)	Heavy stone door.
//		3)  For swing arm on palace level.
//		4)  For stone bridge in palace level.
//		5)  Small/medium wood door swinging.
//		6)  Large/huge wood door swinging.
//		7)  Medium sized stone/wood door sliding.
//		8)  Large stone/wood sliding door or portcullis.
//		9)  Average metal door swinging.
//		10) Fast sliding doors.
//		11) Hive, Metal, Multipaneled sliding.
//		12) Huge stone door swinging.
//		13) Medium/large elevator.
//		14) Crane (warehouse).
//		15) Hammer-like pump in oglemine1.
//		16) Sliding metal table in cloudlabs.
//		17) Lab table which rotates up to ceiling - cloublabs.
//		18) Piston sound.
//		19) Short, sharp metal clang.
//		20) Something going under water.
//		21) The bam sound.
void SP_func_door_secret(edict_t* ent)
{
	FuncDoorSetSounds(ent);

	ent->movetype = PHYSICSTYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	ent->blocked = FuncDoorSecretBlocked;
	ent->use = FuncDoorSecretUse;

	if (ent->targetname == NULL || (ent->spawnflags & SF_SECRET_ALWAYS_SHOOT))
	{
		ent->health = 0;
		ent->takedamage = DAMAGE_YES;
		ent->die = FuncDoorSecretDie;
	}

	if (ent->dmg == 0)
		ent->dmg = 2;

	if (ent->wait == 0.0f)
		ent->wait = 5.0f;

	ent->moveinfo.accel = 50.0f;
	ent->moveinfo.decel = 50.0f;
	ent->moveinfo.speed = 50.0f;

	// Calculate positions.
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	AngleVectors(ent->s.angles, forward, right, up);
	VectorClear(ent->s.angles);

	float width;
	if (ent->spawnflags & SF_SECRET_1ST_DOWN)
		width = fabsf(DotProduct(up, ent->size));
	else
		width = fabsf(DotProduct(right, ent->size));

	float length = fabsf(DotProduct(forward, ent->size));

	if (ent->spawnflags & SF_SECRET_1ST_DOWN)
	{
		VectorMA(ent->s.origin, -width, up, ent->pos1);
	}
	else
	{
		const float side = ((ent->spawnflags & SF_SECRET_1ST_LEFT) ? -1.0f : 1.0f);
		VectorMA(ent->s.origin, side * width, right, ent->pos1);
	}

	if (st.lip > 0)
		length -= (float)st.lip;

	VectorMA(ent->pos1, length, forward, ent->pos2);

	if (ent->health > 0)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = FuncDoorDie;
		ent->max_health = ent->health;
	}
	else if (ent->targetname != NULL && ent->message != NULL)
	{
		gi.soundindex("misc/talk.wav");
		ent->isBlocking = FuncDoorTouch;
	}

	ent->classname = "func_door";
}

#pragma endregion

#pragma region ========================== func_door support logic ==========================

void FuncDoorSetSounds(edict_t* ent) //mxd. Named 'door_sounds' in original logic.
{
	switch ((DoorSoundID_t)ent->sounds)
	{
		case DS_GENERIC:
			ent->moveinfo.sound_start = gi.soundindex("doors/gendoorstart.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = gi.soundindex("doors/gendoorstop.wav");
			break;

		case DS_HEAVYSTONE:
			ent->moveinfo.sound_start = gi.soundindex("doors/stonestart.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/stoneloop.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/stoneend.wav");
			break;

		case DS_SWINGARM:
			ent->moveinfo.sound_start = gi.soundindex("doors/bigcreak.wav");
			break;

		case DS_SWINGBRIDGE:
			ent->moveinfo.sound_start = gi.soundindex("doors/stoneloop.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = gi.soundindex("doors/stoneend.wav");
			break;

		case DS_MEDIUMWOOD:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk2.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/creak4.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/doorclose1.wav");
			break;

		case DS_HUGEWOOD:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk1.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/creak2.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/doorshut1.wav");
			break;

		case DS_MEDIUMSTONE:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk7.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/stndoor.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/thud7.wav");
			break;

		case DS_LARGESTONE:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk6.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/stoneloop.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/thud3.wav");
			break;

		case DS_MEDIUMMETAL:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk3.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/metal1.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/thud2.wav");
			break;

		case DS_FASTSLIDING:
			ent->moveinfo.sound_start = gi.soundindex("doors/fastdoor.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;

		case DS_METALSLIDING:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk5.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = gi.soundindex("doors/thud2.wav");
			break;

		case DS_HUGESTONE:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk5.wav");
			ent->moveinfo.sound_middle = gi.soundindex("objects/creak2a.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/thud4.wav");
			break;

		case DS_HUGEELEVATOR:
			ent->moveinfo.sound_start = gi.soundindex("doors/elevatorstart.wav");
			ent->moveinfo.sound_middle = gi.soundindex("doors/elevatormove.wav");
			ent->moveinfo.sound_end = gi.soundindex("doors/elevatorstop.wav");
			break;

		case DS_CRANEWAREHOUSE:
			ent->moveinfo.sound_start = gi.soundindex("doors/kchunk6.wav");
			ent->moveinfo.sound_middle = gi.soundindex("objects/winch2.wav");
			ent->moveinfo.sound_end = gi.soundindex("objects/cratedown.wav");
			break;

		case DS_HAMMERPUMP:
			ent->moveinfo.sound_start = gi.soundindex("objects/oilpump.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;

		case DS_METALTABLE:
			ent->moveinfo.sound_start = gi.soundindex("objects/slabslide.wav");
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;

		case DS_LABTABLE:
			ent->moveinfo.sound_start = gi.soundindex("objects/globebottomstart.wav");
			ent->moveinfo.sound_end = gi.soundindex("objects/globebottomend.wav");
			break;

		case DS_PISTON:
			ent->moveinfo.sound_start = gi.soundindex("objects/piston.wav");
			break;

		case DS_CLANG:
			ent->moveinfo.sound_start = gi.soundindex("objects/klang.wav");
			break;

		case DS_UNDERWATER:
			ent->moveinfo.sound_start = gi.soundindex("objects/submerge.wav");
			break;

		case DS_BAM:
			ent->moveinfo.sound_start = gi.soundindex("objects/bam1.wav");
			break;

		case DS_NONE:
		default:
			ent->moveinfo.sound_start = 0;
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;
	}
}

static void FuncDoorDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'FuncDoor_Deactivate' in original logic.
{
	VectorClear(self->velocity);
	VectorClear(self->avelocity);
}

static void FuncDoorActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'FuncDoor_Activate' in original logic.
{
	self->use(self, NULL, NULL);
	gi.linkentity(self);
}

void FuncDoorStaticsInit(void)
{
	classStatics[CID_FUNC_DOOR].msgReceivers[G_MSG_SUSPEND] = FuncDoorDeactivate;
	classStatics[CID_FUNC_DOOR].msgReceivers[G_MSG_UNSUSPEND] = FuncDoorActivate;
}

#pragma endregion
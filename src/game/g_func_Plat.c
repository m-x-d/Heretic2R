//
// g_func_Plat.c -- Originally part of g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Plat.h"
#include "g_combat.h"
#include "g_debris.h"
#include "g_func_Door.h"
#include "g_func_Utility.h"
#include "Vector.h"

// PLATS
// Movement options:

// linear
// smooth start, hard stop
// smooth start, smooth stop

// start
// end
// acceleration
// speed
// deceleration
// begin sound
// end sound
// target fired when reaching end
// wait at end

// Object characteristics that use move segments:

// PHYSICSTYPE_PUSH, or PHYSICSTYPE_STOP
// action when touched
// action when blocked
// action when used -- disabled?
// auto trigger spawning

#define SF_PLAT_LOW_TRIGGER	1 //mxd

void FuncPlatHitTop(edict_t* ent) //mxd. Named 'plat_hit_top' in original logic.
{
	FuncPlayMoveEndSound(ent); //mxd

	ent->moveinfo.state = STATE_TOP;
	ent->think = FuncPlatGoDown;
	ent->nextthink = level.time + 3.0f;
}

void FuncPlatHitBottom(edict_t* ent) //mxd. Named 'plat_hit_bottom' in original logic.
{
	FuncPlayMoveEndSound(ent); //mxd
	ent->moveinfo.state = STATE_BOTTOM;
}

void FuncPlatGoDown(edict_t* ent) //mxd. Named 'plat_go_down' in original logic.
{
	FuncPlayMoveStartSound(ent); //mxd
	ent->moveinfo.state = STATE_DOWN;
	MoveCalc(ent, ent->moveinfo.end_origin, FuncPlatHitBottom);
}

static void FuncPlatGoUp(edict_t* ent) //mxd. Named 'plat_go_up' in original logic.
{
	FuncPlayMoveStartSound(ent); //mxd
	ent->moveinfo.state = STATE_UP;
	MoveCalc(ent, ent->moveinfo.start_origin, FuncPlatHitTop);
}

void FuncPlatBlocked(edict_t* self, edict_t* other) //mxd. Named 'plat_blocked' in original logic.
{
	//TODO: invalid logic? Checks for both presence and absence of SVF_MONSTER flag! Last check is '!(other->svflags & SVF_BOSS)' in FuncDoorBlocked().
	if ((other->svflags & SVF_MONSTER) && other->client == NULL && !(other->svflags & SVF_MONSTER))
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

	if (self->moveinfo.state == STATE_UP)
		FuncPlatGoDown(self);
	else if (self->moveinfo.state == STATE_DOWN)
		FuncPlatGoUp(self);
}

void FuncPlatUse(edict_t* ent, edict_t* other, edict_t* activator) //mxd. Named 'Use_Plat' in original logic.
{
	if (ent->think == NULL) // Already down otherwise.
		FuncPlatGoDown(ent);
}

void FuncPlatCenterTouch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'Touch_Plat_Center' in original logic.
{
	if (other->client == NULL || other->health <= 0)
		return;

	ent = ent->enemy; // Now point at the plat, not the trigger.

	if (ent->moveinfo.state == STATE_BOTTOM)
		FuncPlatGoUp(ent);
	else if (ent->moveinfo.state == STATE_TOP)
		ent->nextthink = level.time + 1.0f; // The player is still on the plat, so delay going down.
}

static void FuncPlatSpawnInsideTrigger(edict_t* ent) //mxd. Named 'plat_spawn_inside_trigger' in original logic.
{
	// Middle trigger.
	edict_t* trigger = G_Spawn();

	trigger->touch = FuncPlatCenterTouch;
	trigger->movetype = PHYSICSTYPE_NONE;
	trigger->solid = SOLID_TRIGGER;
	trigger->enemy = ent;

	vec3_t t_maxs;
	t_maxs[0] = ent->maxs[0] - 25.0f;
	t_maxs[1] = ent->maxs[1] - 25.0f;
	t_maxs[2] = ent->maxs[2] + 8.0f;

	vec3_t t_mins;
	t_mins[0] = ent->mins[0] + 25.0f;
	t_mins[1] = ent->mins[1] + 25.0f;
	t_mins[2] = t_maxs[2] - (ent->pos1[2] - ent->pos2[2] + (float)st.lip);

	if (ent->spawnflags & SF_PLAT_LOW_TRIGGER)
		t_maxs[2] = t_mins[2] + 8.0f;

	for (int i = 0; i < 2; i++)
	{
		if (t_maxs[i] - t_mins[i] <= 0.0f)
		{
			t_mins[i] = (ent->mins[i] + ent->maxs[i]) * 0.5f;
			t_maxs[i] = t_mins[i] + 1.0f;
		}
	}

	VectorCopy(t_mins, trigger->mins);
	VectorCopy(t_maxs, trigger->maxs);

	gi.linkentity(trigger);
}

// QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
// Plats are always drawn in the extended position, so they will light correctly.
// If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is triggered,
// when it will lower and become a normal plat.

// Spawnflags:
// PLAT_LOW_TRIGGER - When set, platform trigger height is 8.

// Variables:
// speed	- overrides default 200.
// accel	- overrides default 500.
// lip		- overrides default 8 pixel lip.
// height	- is set, that will determine the amount the plat moves, instead of being implicitly determined by the model's height.
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
void SP_func_plat(edict_t* ent)
{
	VectorClear(ent->s.angles);

	ent->solid = SOLID_BSP;
	ent->movetype = PHYSICSTYPE_PUSH;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->blocked = FuncPlatBlocked;
	ent->use = FuncPlatUse;

	if (ent->speed == 0.0f)
		ent->speed = 20.0f;
	else
		ent->speed *= 0.1f;

	if (ent->accel == 0.0f)
		ent->accel = 5.0f;
	else
		ent->accel *= 0.1f;

	if (ent->decel == 0.0f)
		ent->decel = 5.0f;
	else
		ent->decel *= 0.1f;

	if (ent->dmg == 0)
		ent->dmg = 2;

	if (st.lip == 0)
		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	VectorCopy(ent->s.origin, ent->pos1);
	VectorCopy(ent->s.origin, ent->pos2);

	if (st.height > 0)
		ent->pos2[2] -= (float)st.height;
	else
		ent->pos2[2] -= ent->maxs[2] - ent->mins[2] - (float)st.lip;

	if (ent->targetname != NULL)
	{
		ent->moveinfo.state = STATE_UP;
	}
	else
	{
		ent->moveinfo.state = STATE_BOTTOM;
		VectorCopy(ent->pos2, ent->s.origin);
	}

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;

	VectorCopy(ent->pos1, ent->moveinfo.start_origin);
	VectorCopy(ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy(ent->pos2, ent->moveinfo.end_origin);
	VectorCopy(ent->s.angles, ent->moveinfo.end_angles);

	VectorSubtract(ent->maxs, ent->mins, ent->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, ent->s.bmodel_origin);
	VectorAdd(ent->mins, ent->s.bmodel_origin, ent->s.bmodel_origin);

	FuncDoorSetSounds(ent);

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	FuncPlatSpawnInsideTrigger(ent); // The "start moving" trigger.
}
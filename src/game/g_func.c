//
// g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Utility.h" //mxd
#include "g_func_Door.h" //mxd
#include "g_combat.h" //mxd
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "FX.h"
#include "Vector.h"
#include "g_local.h"

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

#pragma region ========================== func_plat ==========================

#define SF_PLAT_LOW_TRIGGER	1 //mxd

static void FuncPlatGoDown(edict_t* ent); //TODO: move to g_funcs.h

static void FuncPlatHitTop(edict_t* ent) //mxd. Named 'plat_hit_top' in original logic.
{
	FuncPlayMoveEndSound(ent); //mxd

	ent->moveinfo.state = STATE_TOP;
	ent->think = FuncPlatGoDown;
	ent->nextthink = level.time + 3.0f;
}

static void FuncPlatHitBottom(edict_t* ent) //mxd. Named 'plat_hit_bottom' in original logic.
{
	FuncPlayMoveEndSound(ent); //mxd
	ent->moveinfo.state = STATE_BOTTOM;
}

static void FuncPlatGoDown(edict_t* ent) //mxd. Named 'plat_go_down' in original logic.
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

static void FuncPlatBlocked(edict_t* self, edict_t* other) //mxd. Named 'plat_blocked' in original logic.
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

static void FuncPlatUse(edict_t* ent, edict_t* other, edict_t* activator) //mxd. Named 'Use_Plat' in original logic.
{
	if (ent->think == NULL) // Already down otherwise.
		FuncPlatGoDown(ent);
}

static void FuncPlatCenterTouch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'Touch_Plat_Center' in original logic.
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

static void FuncRotateDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'FuncRotate_Deactivate' in original logic.
{
	VectorClear(self->velocity);
	VectorClear(self->avelocity);
}

static void FuncRotateActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'FuncRotate_Activate' in original logic.
{
	self->use(self, NULL, NULL);
	gi.linkentity(self);
}

void FuncRotateStaticsInit(void)
{
	classStatics[CID_FUNC_ROTATE].msgReceivers[G_MSG_SUSPEND] = FuncRotateDeactivate;
	classStatics[CID_FUNC_ROTATE].msgReceivers[G_MSG_UNSUSPEND] = FuncRotateActivate;
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

#pragma endregion

#pragma region ========================== func_rotating ==========================

#define SF_START_ON			1 //mxd
#define SF_REVERSE			2 //mxd
#define SF_X_AXIS			4 //mxd
#define SF_Y_AXIS			8 //mxd
#define SF_TOUCH_PAIN		16 //mxd
#define SF_STOP				32 //mxd
#define SF_ANIMATED			64 //mxd
#define SF_ANIMATED_FAST	128 //mxd
#define SF_CRUSHER			256 //mxd

static void FuncRotatingSetSounds(edict_t* ent) //mxd. Named 'rotate_sounds' in original logic.
{
	switch (ent->sounds)
	{
		case 1: ent->moveinfo.sound_middle = gi.soundindex("doors/stoneloop.wav"); break;
		case 2: ent->moveinfo.sound_middle = gi.soundindex("objects/hugewheel.wav"); break;
		case 3: ent->moveinfo.sound_middle = gi.soundindex("objects/pizzawheel.wav"); break;
		case 4: ent->moveinfo.sound_middle = gi.soundindex("objects/spankers.wav"); break;

		default:
			ent->moveinfo.sound_start = 0;
			ent->moveinfo.sound_middle = 0;
			ent->moveinfo.sound_end = 0;
			break;
	}
}

static void FuncRotatingBlocked(edict_t* self, edict_t* other) //mxd. Named 'rotating_blocked' in original logic.
{
	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

static void FuncRotatingTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'rotating_touch' in original logic.
{
	if (Vec3NotZero(self->avelocity))
		FuncRotatingBlocked(self, other);
}

static void FuncRotatingUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'rotating_use' in original logic.
{
	if (Vec3NotZero(self->avelocity))
	{
		self->s.sound = 0;
		VectorClear(self->avelocity);
		self->touch = NULL;
	}
	else
	{
		self->s.sound = (byte)self->moveinfo.sound_middle;
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_IDLE;
		VectorScale(self->movedir, self->speed, self->avelocity);

		if (self->spawnflags & SF_TOUCH_PAIN)
			self->touch = FuncRotatingTouch;
	}
}

// QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST CRUSHER
// You need to have an origin brush as part of this entity. The center of that brush will be the point around which it is rotated.
// It will rotate around the Z axis by default. You can check either the X_AXIS or Y_AXIS box to change that.

// Spawnflags:
// REVERSE	- Will cause the it to rotate in the opposite direction.
// STOP		- Mean it will stop moving instead of pushing entities.

// Variables:
// speed	- Determines how fast it moves (default 100).
// dmg		- Damage to inflict when blocked (default 2).
// sounds:
//		0 - Silent.
//		1 - Generic rotate.
//		2 - Huge wheel ogles push in cloudlabs.
//		3 - Rock crusher which turns at end of conveyor on ogle2.
//		4 - 'Spanking' paddles on gauntlet.
void SP_func_rotating(edict_t* ent)
{
	ent->classID = CID_FUNC_ROTATE;
	ent->msgHandler = DefaultMsgHandler;

	ent->solid = SOLID_BSP;
	ent->movetype = ((ent->spawnflags & SF_STOP) ? PHYSICSTYPE_STOP : PHYSICSTYPE_PUSH);

	FuncRotatingSetSounds(ent);

	// Set the axis of rotation.
	VectorClear(ent->movedir);

	if (ent->spawnflags & SF_X_AXIS)
		ent->movedir[2] = 1.0f;
	else if (ent->spawnflags & SF_Y_AXIS)
		ent->movedir[0] = 1.0f;
	else // Z_AXIS
		ent->movedir[1] = 1.0f;

	// Check for reverse rotation.
	if (ent->spawnflags & SF_REVERSE)
		VectorNegate(ent->movedir, ent->movedir);

	if (ent->speed == 0.0f)
		ent->speed = 100.0f;

	if (ent->dmg == 0)
		ent->dmg = 2;

	ent->use = FuncRotatingUse;

	if (ent->dmg > 0)
		ent->blocked = FuncRotatingBlocked;

	if (ent->spawnflags & SF_START_ON)
		ent->use(ent, NULL, NULL);

	if (ent->spawnflags & SF_ANIMATED)
		ent->s.effects |= EF_ANIM_ALL;

	if (ent->spawnflags & SF_ANIMATED_FAST)
		ent->s.effects |= EF_ANIM_ALLFAST;

	//TODO: neither SF_CRUSHER nor SF_DOOR_CRUSHER flags seem to be used by func_rotating logic...
	if (ent->spawnflags & SF_CRUSHER) // Because of a mixup in flags.
		ent->spawnflags |= SF_DOOR_CRUSHER;

	VectorSubtract(ent->maxs, ent->mins, ent->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, ent->s.bmodel_origin);
	VectorAdd(ent->mins, ent->s.bmodel_origin, ent->s.bmodel_origin);

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== func_button ==========================

#define SF_TOUCH	1 //mxd

static void FuncButtonMove(edict_t* self); //TODO: move to header.

static void FuncButtonOnDeathMessage(edict_t* self, G_Message_t* msg) //mxd. Named 'button_killed2' in original logic.
{
	self->activator = self->enemy;
	FuncButtonMove(self);
	self->health = self->max_health;
}

void ButtonStaticsInit(void) //TODO: rename to FuncButtonStaticsInit.
{
	classStatics[CID_BUTTON].msgReceivers[MSG_DEATH] = FuncButtonOnDeathMessage;
}

static void FuncButtonDone(edict_t* self) //mxd. Named 'button_done' in original logic.
{
	self->moveinfo.state = STATE_BOTTOM;
	self->s.frame = 0;
}

static void FuncButtonReturn(edict_t* self) //mxd. Named 'button_return' in original logic.
{
	self->moveinfo.state = STATE_DOWN;
	MoveCalc(self, self->moveinfo.start_origin, FuncButtonDone);
	self->s.frame = 0;

	if (self->health > 0)
		self->takedamage = DAMAGE_YES;
}

static void FuncButtonWait(edict_t* self) //mxd. Named 'button_wait' in original logic.
{
	self->moveinfo.state = STATE_TOP;
	G_UseTargets(self, self->activator);
	self->s.frame = 1;

	if (self->moveinfo.wait >= 0)
	{
		self->nextthink = level.time + self->moveinfo.wait;
		self->think = FuncButtonReturn;
	}
}

static void FuncButtonMove(edict_t* self) //mxd. Named 'button_fire' in original logic.
{
	if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP)
		return;

	if (self->moveinfo.sound_start > 0 && !(self->flags & FL_TEAMSLAVE))
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

	self->moveinfo.state = STATE_UP;
	MoveCalc(self, self->moveinfo.end_origin, FuncButtonWait);
}

static void FuncButtonUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'button_use' in original logic.
{
	self->activator = activator;
	FuncButtonMove(self);
}

static void FuncButtonTouch(edict_t* self, trace_t* trace) //mxd. Named 'button_touch' in original logic.
{
	edict_t* other = trace->ent;

	if (other->client != NULL && other->health > 0)
	{
		self->activator = other;
		FuncButtonMove(self);
	}
}

static void FuncButtonSetSounds(edict_t* self) //mxd. Named 'button_sounds' in original logic.
{
	switch (self->sounds)
	{
		case 1: self->moveinfo.sound_start = gi.soundindex("doors/basicbutton.wav"); break;
		case 2: self->moveinfo.sound_start = gi.soundindex("doors/clankybutton.wav"); break;
		case 3: self->moveinfo.sound_start = gi.soundindex("doors/steambutton.wav"); break;
		default: break;
	}
}

// QUAKED func_button (0 .5 .8) ? TOUCH
// When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets,
// waits "wait" time, then returns to it's original position where it can be triggered again.

// Spawnflags:
// TOUCH - Player can touch button to set it off.

// Variables:
// angle	- Determines the opening direction.
// target	- All entities with a matching targetname will be used.
// speed	- Override the default 40 speed.
// wait		- Override the default 1 second wait (-1 = never return).
// lip		- Override the default 4 pixel lip remaining at end of move.
// health	- If set, the button must be killed instead of touched.
// sounds:
//		0) Silent.
//		1) Basic Button.
//		2) Clanky Button.
//		3) Steam Button.
void SP_func_button(edict_t* ent)
{
	G_SetMovedir(ent->s.angles, ent->movedir);
	ent->movetype = PHYSICSTYPE_STOP;
	ent->solid = SOLID_BSP;
	ent->takedamage = DAMAGE_NO;

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	FuncButtonSetSounds(ent);

	if (ent->speed == 0.0f)
		ent->speed = 40.0f;

	if (ent->accel == 0.0f)
		ent->accel = ent->speed;

	if (ent->decel == 0.0f)
		ent->decel = ent->speed;

	if (ent->wait == 0.0f)
		ent->wait = 3.0f;

	if (st.lip == 0)
		st.lip = 4;

	VectorCopy(ent->s.origin, ent->pos1);

	vec3_t abs_movedir;
	VectorAbs(ent->movedir, abs_movedir);

	const float dist = DotProduct(abs_movedir, ent->size) - (float)st.lip;
	VectorMA(ent->pos1, dist, ent->movedir, ent->pos2);

	ent->use = FuncButtonUse;

	if (ent->health > 0)
	{
		ent->max_health = ent->health;
		ent->takedamage = DAMAGE_YES;
	}

	if (ent->targetname == NULL || (ent->spawnflags & SF_TOUCH))
		ent->isBlocking = FuncButtonTouch;

	ent->moveinfo.state = STATE_BOTTOM;
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;

	VectorCopy(ent->pos1, ent->moveinfo.start_origin);
	VectorCopy(ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy(ent->pos2, ent->moveinfo.end_origin);
	VectorCopy(ent->s.angles, ent->moveinfo.end_angles);

	ent->msgHandler = DefaultMsgHandler;
}

#pragma endregion
//
// g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Utility.h" //mxd
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

#define SF_DOOR_START_OPEN	1
#define SF_DOOR_REVERSE		2
#define SF_DOOR_CRUSHER		4
#define SF_DOOR_NOMONSTER	8
#define SF_DOOR_TOGGLE		32
#define SF_DOOR_X_AXIS		64
#define SF_DOOR_Y_AXIS		128
#define SF_DOOR_SWINGAWAY	8192

#pragma region ========================== func_plat ==========================

#define SF_PLAT_LOW_TRIGGER	1 //mxd

static void FuncPlatPlayMoveStartSound(edict_t* ent) //mxd. Added to reduce code duplication.
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start > 0)
			gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

		ent->s.sound = (byte)ent->moveinfo.sound_middle;
		ent->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_IDLE;
	}
}

static void FuncPlatPlayMoveEndSound(edict_t* ent) //mxd. Added to reduce code duplication.
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end > 0)
			gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_end, 1.0f, ATTN_IDLE, 0.0f);

		ent->s.sound = 0;
	}
}

static void FuncPlatGoDown(edict_t* ent); //TODO: move to g_funcs.h

static void FuncPlatHitTop(edict_t* ent) //mxd. Named 'plat_hit_top' in original logic.
{
	FuncPlatPlayMoveEndSound(ent); //mxd

	ent->moveinfo.state = STATE_TOP;
	ent->think = FuncPlatGoDown;
	ent->nextthink = level.time + 3.0f;
}

static void FuncPlatHitBottom(edict_t* ent) //mxd. Named 'plat_hit_bottom' in original logic.
{
	FuncPlatPlayMoveEndSound(ent); //mxd
	ent->moveinfo.state = STATE_BOTTOM;
}

static void FuncPlatGoDown(edict_t* ent) //mxd. Named 'plat_go_down' in original logic.
{
	FuncPlatPlayMoveStartSound(ent); //mxd
	ent->moveinfo.state = STATE_DOWN;
	MoveCalc(ent, ent->moveinfo.end_origin, FuncPlatHitBottom);
}

static void FuncPlatGoUp(edict_t* ent) //mxd. Named 'plat_go_up' in original logic.
{
	FuncPlatPlayMoveStartSound(ent); //mxd
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

void FuncDoorStaticsInit(void)
{
	classStatics[CID_FUNC_DOOR].msgReceivers[G_MSG_SUSPEND] = FuncRotateDeactivate;
	classStatics[CID_FUNC_DOOR].msgReceivers[G_MSG_UNSUSPEND] = FuncRotateActivate;
}

static void FuncDoorSetSounds(edict_t* ent); //TODO: move to g_funcs.h

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

#pragma region ========================== func_door, func_door_rotating, func_water ==========================

#define DOOR_MOVE_LOOP	(-2.0f) //mxd

static void FuncDoorUseAreaportals(const edict_t* self, const qboolean open) //mxd. Named 'door_use_areaportals' in original logic.
{
	if (self->target == NULL)
		return;

	edict_t* target = NULL;
	while ((target = G_Find(target, FOFS(targetname), self->target)) != NULL)
		if (Q_stricmp(target->classname, "func_areaportal") == 0)
			gi.SetAreaPortalState(target->style, open);
}

static void FuncDoorGoDown(edict_t* self); //TODO: move to header.

static void FuncDoorHitTop(edict_t* self) //mxd. Named 'door_hit_top' in original logic.
{
	FuncPlatPlayMoveEndSound(self); //mxd
	self->moveinfo.state = STATE_TOP;

	if (self->spawnflags & SF_DOOR_TOGGLE)
		return;

	if (self->moveinfo.wait >= 0.0f)
	{
		self->think = FuncDoorGoDown;
		self->nextthink = level.time + self->moveinfo.wait;
	}
	else if (self->moveinfo.wait == DOOR_MOVE_LOOP)
	{
		self->think = FuncDoorGoDown;
		self->nextthink = level.time + FRAMETIME; // Next frame is soon enough to fire this off.
	}
}

static void FuncDoorGoUp(edict_t* self, edict_t* activator); //TODO: move to header.

static void FuncDoorHitBottom(edict_t* self) //mxd. Named 'door_hit_bottom' in original logic.
{
	FuncPlatPlayMoveEndSound(self); //mxd
	self->moveinfo.state = STATE_BOTTOM;

	if (self->moveinfo.wait == DOOR_MOVE_LOOP) // Endless cycle.
		FuncDoorGoUp(self, NULL);
	else
		FuncDoorUseAreaportals(self, false);
}

static void FuncDoorGoDown(edict_t* self) //mxd. Named 'door_go_down' in original logic.
{
	FuncPlatPlayMoveStartSound(self); //mxd

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
		if (self->moveinfo.wait >= 0)
			self->nextthink = level.time + self->moveinfo.wait;
		else if (self->moveinfo.wait == DOOR_MOVE_LOOP)
			self->nextthink = level.time;

		return;
	}

	FuncPlatPlayMoveStartSound(self); //mxd
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

static void FuncDoorUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'smart_door_side_check' in original logic.
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

static void FuncDoorTriggerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'Touch_DoorTrigger' in original logic.
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

static void FuncDoorCalcMoveSpeedThink(edict_t* self) //mxd. Named 'Think_CalcMoveSpeed' in original logic.
{
	if (self->flags & FL_TEAMSLAVE)
	{
		self->think = NULL;
		return; // Only the team master does this.
	}

	// Find the smallest distance any member of the team will be moving.
	float min_dist = Q_fabs(self->moveinfo.distance);

	for (const edict_t* ent = self->teamchain; ent != NULL; ent = ent->teamchain)
	{
		const float dist = Q_fabs(ent->moveinfo.distance);
		min_dist = min(dist, min_dist);
	}

	const float time = min_dist / self->moveinfo.speed;

	// Adjust speeds so they will all complete at the same time.
	for (edict_t* ent = self; ent != NULL; ent = ent->teamchain)
	{
		const float new_speed = Q_fabs(ent->moveinfo.distance) / time;
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

static void FuncDoorSpawnDoorTriggerThink(edict_t* self) //mxd. Named 'Think_SpawnDoorTrigger' in original logic.
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

static void FuncDoorBlocked(edict_t* self, edict_t* other) //mxd. Named 'door_blocked' in original logic.
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
	if (self->moveinfo.wait >= 0.0f || self->moveinfo.wait == DOOR_MOVE_LOOP)
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

static int FuncDoorKilled(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point) //mxd. Named 'door_killed' in original logic.
{
	for (edict_t* ent = self->teammaster; ent != NULL; ent = ent->teamchain)
	{
		ent->health = ent->max_health;
		ent->takedamage = DAMAGE_NO;
	}

	FuncDoorUse(self->teammaster, attacker, attacker);
	return 0;
}

static void FuncDoorTouch(edict_t* self, trace_t* trace) //mxd. Named 'door_killed' in original logic.
{
	const edict_t* other = trace->ent;

	if (other->client != NULL && level.time >= self->touch_debounce_time)
	{
		self->touch_debounce_time = level.time + 5.0f;
		gi.levelmsg_centerprintf(other, (short)Q_atoi(self->message));
	}
}

static void FuncDoorSetSounds(edict_t* ent) //mxd. Named 'door_sounds' in original logic.
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

	self->classID = CID_FUNC_DOOR;
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
		self->die = FuncDoorKilled;
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

	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALL;

	if (self->spawnflags & 64)
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

	if (ent->spawnflags & SF_DOOR_X_AXIS)
		ent->movedir[2] = 1.0f;
	else if (ent->spawnflags & SF_DOOR_Y_AXIS)
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
		ent->die = FuncDoorKilled;
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

	if (ent->spawnflags & 16)
		ent->s.effects |= EF_ANIM_ALL;

	ent->nextthink = level.time + FRAMETIME;

	if (ent->health || ent->targetname)
		ent->think = FuncDoorCalcMoveSpeedThink;
	else
		ent->think = FuncDoorSpawnDoorTriggerThink;
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

#pragma region ========================== func_train ==========================

#define SF_TRAIN_START_ON		1
#define SF_TRAIN_TOGGLE			2
#define SF_TRAIN_BLOCK_STOPS	4
#define SF_TRAIN_HAS_ORIGIN		8 //mxd
#define SF_TRAIN_NO_CLIP		16 //mxd

static void FuncTrainNext(edict_t* self); //TODO: move to header.

static void FuncTrainAnim(edict_t* self) //mxd. Named 'train_anim' in original logic.
{
	if (self->s.frame == 0 && self->moveinfo.sound_middle > 0) // Start sound if there is one.
		gi.sound(self, CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_NORM, 0.0f);

	if (self->s.frame + 1 < self->count)
	{
		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
		self->think = FuncTrainAnim;
	}
	else
	{
		FuncTrainNext(self);
	}
}

static void FuncTrainAnimBackwards(edict_t* self) //mxd. Named 'train_animbackwards' in original logic.
{
	if (self->s.frame + 1 == self->count && self->moveinfo.sound_middle > 0) // Start sound if there is one.
		gi.sound(self, CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_NORM, 0.0f);

	if (self->s.frame > 0)
	{
		self->s.frame--;
		self->nextthink = level.time + FRAMETIME;
		self->think = FuncTrainAnimBackwards;
	}
	else
	{
		FuncTrainNext(self);
	}
}

static void FuncTrainBlocked(edict_t* self, edict_t* other) //mxd. Named 'train_blocked' in original logic.
{
	if ((other->svflags & SVF_MONSTER) && other->client == NULL && !(other->svflags & SVF_BOSS))
	{
		// Give it a chance to go away on it's own terms (like gibs).
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 3000, 1, DAMAGE_AVOID_ARMOR, MOD_CRUSH);

		// If it's still there, nuke it.
		if (other->health > 0)
			BecomeDebris(other);
	}
	else if (self->dmg > 0 && level.time >= self->touch_debounce_time)
	{
		self->touch_debounce_time = level.time + 0.5f;
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
	}
}

static void FuncTrainWait(edict_t* self) //mxd. Named 'train_wait' in original logic.
{
	if (self->target_ent->moveinfo.sound_middle > 0)
		gi.sound(self->target_ent, CHAN_VOICE, self->target_ent->moveinfo.sound_middle, 1.0f, ATTN_NORM, 0.0f);

	if (self->target_ent->pathtarget != NULL)
	{
		edict_t* ent = self->target_ent;
		char* save_target = ent->target;
		ent->target = ent->pathtarget;

		G_UseTargets(ent, self->activator);
		ent->target = save_target;

		// Make sure we didn't get killed by a killtarget.
		if (!self->inuse)
			return;
	}

	if (self->moveinfo.wait != 0.0f)
	{
		if (self->moveinfo.wait > 0.0f)
		{
			self->nextthink = level.time + self->moveinfo.wait;
			self->think = FuncTrainNext;
		}
		else if (self->moveinfo.wait == -3.0f)
		{
			BecomeDebris(self);
			return;
		}
		else if (self->moveinfo.wait == -4.0f)	// Make model animate.
		{
			if (self->s.frame + 1 < self->count)
				FuncTrainAnim(self);
			else
				FuncTrainAnimBackwards(self);
		}
		else if (self->spawnflags & SF_TRAIN_TOGGLE) // && wait < 0
		{
			FuncTrainNext(self);
			self->spawnflags &= ~SF_TRAIN_START_ON;
			VectorClear(self->velocity);
			self->nextthink = 0.0f;
		}

		FuncPlatPlayMoveEndSound(self); //mxd
	}
	else
	{
		FuncTrainNext(self);
	}
}

static void FuncTrainNext(edict_t* self) //mxd. Named 'train_next' in original logic.
{
	for (int i = 0; i < 2; i++)
	{
		if (self->target == NULL)
			return;

		edict_t* ent = G_PickTarget(self->target);

		if (ent == NULL)
		{
			gi.dprintf("train_next: bad target %s\n", self->target);
			return;
		}

		self->target = ent->target;

		// Check for SF_TELEPORT path_corner spawnflag.
		if (ent->spawnflags & 1)
		{
			if (i > 0)
			{
				gi.dprintf("connected teleport path_corners, see %s at %s\n", ent->classname, vtos(ent->s.origin));
				return;
			}

			VectorSubtract(ent->s.origin, self->mins, self->s.origin);
			VectorCopy(self->s.origin, self->s.old_origin);
			gi.linkentity(self);

			continue;
		}

		self->moveinfo.wait = ent->wait;
		self->target_ent = ent;

		FuncPlatPlayMoveStartSound(ent); //mxd

		vec3_t dest;
		if (self->spawnflags & SF_TRAIN_HAS_ORIGIN)
			VectorCopy(ent->s.origin, dest);
		else
			VectorSubtract(ent->s.origin, self->mins, dest);

		self->moveinfo.state = STATE_TOP;

		VectorCopy(self->s.origin, self->moveinfo.start_origin);
		VectorCopy(dest, self->moveinfo.end_origin);

		if (ent->speed > 0.0f)
		{
			self->moveinfo.speed = self->speed = ent->speed;
			self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
		}

		MoveCalc(self, dest, FuncTrainWait);

		self->spawnflags |= SF_TRAIN_START_ON;

		VectorCopy(self->moveinfo.end_angles, self->s.angles); // Snap the train to the last ending angle.
		FuncTrainAngleMoveCalc(self, ent, dest); // Recalculate new angles.

		return;
	}
}

void FuncTrainResume(edict_t* self) //mxd. Named 'train_next' in original logic. //TODO: add to header.
{
	const edict_t* ent = self->target_ent;

	vec3_t dest;
	VectorSubtract(ent->s.origin, self->mins, dest);

	self->moveinfo.state = STATE_TOP;
	VectorCopy(self->s.origin, self->moveinfo.start_origin);
	VectorCopy(dest, self->moveinfo.end_origin);

	MoveCalc(self, dest, FuncTrainWait);
	self->spawnflags |= SF_TRAIN_START_ON;
}

static void FuncTrainFind(edict_t* self) //mxd. Named 'func_train_find' in original logic.
{
	if (self->target == NULL)
	{
		gi.dprintf("train_find: no target\n");
		self->think = NULL;

		return;
	}

	const edict_t* ent = G_PickTarget(self->target);

	if (ent == NULL)
	{
		gi.dprintf("train_find: target %s not found\n", self->target);
		self->think = NULL;

		return;
	}

	self->target = ent->target;

	if (Vec3NotZero(self->s.origin))
		VectorCopy(ent->s.origin, self->s.origin);
	else
		VectorSubtract(ent->s.origin, self->mins, self->s.origin);

	gi.linkentity(self);

	// If not triggered, start immediately.
	if (self->targetname == NULL)
		self->spawnflags |= SF_TRAIN_START_ON;

	if (self->spawnflags & SF_TRAIN_START_ON)
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = FuncTrainNext;
		self->activator = self;
	}
	else
	{
		self->think = NULL;
	}
}

static void FuncTrainUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'train_use' in original logic.
{
	self->activator = activator;

	if (Vec3NotZero(self->velocity))
		return;

	if (self->spawnflags & SF_TRAIN_START_ON)
	{
		if (!(self->spawnflags & SF_TRAIN_TOGGLE))
			return;

		self->spawnflags &= ~SF_TRAIN_START_ON;
		VectorClear(self->velocity);
		self->nextthink = 0.0f;
	}
	else if (self->target_ent != NULL)
	{
		FuncTrainResume(self);
	}
	else
	{
		FuncTrainNext(self);
	}
}

// QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS HASORIGIN NO_CLIP PUSHPULL
// Trains are moving platforms that players can ride. The targets origin specifies the min point of the train at each corner.
// The train spawns at the first target it is pointing at. If the train is the target of a button or trigger, it will not begin moving until activated.
// This means if it has a targetname it won't move unless triggered.

// Spawnflags:
// HASORIGIN	- Makes train move from an origin brush rather than the lower left point of the train.
// NO_CLIP		- Train will not block anything.

// Variables:
// speed	- default 100.
// dmg		- default 2.
// noise	- Looping file to play when the train is in motion.
//			- objects/piston.wav for large steam pistons in ogle2 and cloudlabs.
//			- objects/winch2.wav for wooden ore hauler going across river.
// rotate	- Speed train should rotate at.
// wait		- -1: Stop and don't move again until triggered.
//			  -3: Stop and explode.
//			  -4: Go through animations (only if a model)
// file		- Specifies the train is a model. This is the exact directory of the model (example: models/objects/broom/tris.fm).
// count	- Number of frames in animation (only if a model)
// materialtype:
//			0 = MAT_WOOD
//			1 = MAT_GREYSTONE (default)
//			2 = MAT_CLOTH
//			3 = MAT_METAL
//			9 = MAT_BROWNSTONE
//			10 = MAT_NONE - just makes smoke.
void SP_func_train(edict_t* self)
{
	self->movetype = PHYSICSTYPE_PUSH;
	self->solid = ((self->spawnflags & SF_TRAIN_NO_CLIP) ? SOLID_NOT : SOLID_BBOX);

	if (self->spawnflags & SF_TRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else if (self->dmg == 0)
		self->dmg = 100;

	if (st.file != NULL)
	{
		self->s.modelindex = (byte)gi.modelindex(st.file);
		VectorCopy(self->s.angles, self->moveinfo.end_angles);
	}
	else
	{
		VectorClear(self->s.angles);
		gi.setmodel(self, self->model);
	}

	if (st.noise != NULL)
		self->moveinfo.sound_middle = gi.soundindex(st.noise);

	if (self->speed == 0.0f)
		self->speed = 100.0f;

	if (self->materialtype == 0) //TODO: MAT_STONE (0) can't be set...
		self->materialtype = MAT_GREYSTONE;

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->speed;
	self->moveinfo.decel = self->speed;

	self->blocked = FuncTrainBlocked;
	self->use = FuncTrainUse;

	VectorClear(self->movedir);

	if (st.rotate != 0)
		VectorScale(self->movedir, (float)st.rotate, self->avelocity);
	else
		VectorClear(self->avelocity);

	vec3_t space;
	VectorSubtract(self->maxs, self->mins, space);
	const float space_cube = space[0] * space[1] * space[2];
	self->mass = (int)(space_cube / 64.0f);

	VectorSubtract(self->maxs, self->mins, self->s.bmodel_origin);
	Vec3ScaleAssign(0.5f, self->s.bmodel_origin);
	VectorAdd(self->mins, self->s.bmodel_origin, self->s.bmodel_origin);

	gi.linkentity(self);

	if (self->target != NULL)
	{
		// Start trains on the second frame, to make sure their targets have had a chance to spawn.
		self->nextthink = level.time + FRAMETIME;
		self->think = FuncTrainFind;
	}
	else
	{
		gi.dprintf("func_train without a target at %s\n", vtos(self->absmin));
	}
}

#pragma endregion

#pragma region ========================== func_door_secret ==========================

#define SF_SECRET_ALWAYS_SHOOT	1
#define SF_SECRET_1ST_LEFT		2
#define SF_SECRET_1ST_DOWN		4

static void FuncDoorSecretMove1(edict_t* self);
static void FuncDoorSecretMove2(edict_t* self);
static void FuncDoorSecretMove3(edict_t* self);
static void FuncDoorSecretMove4(edict_t* self);
static void FuncDoorSecretMove5(edict_t* self);
static void FuncDoorSecretMove6(edict_t* self);
static void FuncDoorSecretDone(edict_t* self);

static void FuncDoorSecretUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'door_secret_use' in original logic.
{
	// Make sure we're not already moving.
	if (!VectorCompare(self->s.origin, vec3_origin))
		return;

	if (self->moveinfo.sound_start > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, self->pos1, FuncDoorSecretMove1);
	FuncDoorUseAreaportals(self, true);
}

static void FuncDoorSecretMove1(edict_t* self) //mxd. Named 'door_secret_move1' in original logic.
{
	self->nextthink = level.time + 1.0f;
	self->think = FuncDoorSecretMove2;
}

static void FuncDoorSecretMove2(edict_t* self) //mxd. Named 'door_secret_move2' in original logic.
{
	if (self->moveinfo.sound_middle > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, self->pos2, FuncDoorSecretMove3);
}

static void FuncDoorSecretMove3(edict_t* self) //mxd. Named 'door_secret_move3' in original logic.
{
	if (self->moveinfo.sound_end > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_end, 1.0f, ATTN_IDLE, 0.0f);

	if (self->wait != -1.0f)
	{
		self->nextthink = level.time + self->wait;
		self->think = FuncDoorSecretMove4;
	}
}

static void FuncDoorSecretMove4(edict_t* self) //mxd. Named 'door_secret_move4' in original logic.
{
	if (self->moveinfo.sound_middle > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_middle, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, self->pos1, FuncDoorSecretMove5);
}

static void FuncDoorSecretMove5(edict_t* self) //mxd. Named 'door_secret_move5' in original logic.
{
	if (self->moveinfo.sound_end > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_end, 1.0f, ATTN_IDLE, 0.0f);

	self->nextthink = level.time + 1.0f;
	self->think = FuncDoorSecretMove6;
}

static void FuncDoorSecretMove6(edict_t* self) //mxd. Named 'door_secret_move6' in original logic.
{
	if (self->moveinfo.sound_start > 0)
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1.0f, ATTN_IDLE, 0.0f);

	MoveCalc(self, vec3_origin, FuncDoorSecretDone);
}

static void FuncDoorSecretDone(edict_t* self) //mxd. Named 'door_secret_done' in original logic.
{
	if (self->targetname == NULL || (self->spawnflags & SF_SECRET_ALWAYS_SHOOT))
	{
		self->health = 0;
		self->takedamage = DAMAGE_YES;
	}

	FuncDoorUseAreaportals(self, false);
}

static void FuncDoorSecretBlocked(edict_t* self, edict_t* other) //mxd. Named 'door_secret_blocked' in original logic.
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

static int FuncDoorSecretDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point) //mxd. Named 'door_secret_die' in original logic.
{
	self->takedamage = DAMAGE_NO;
	FuncDoorSecretUse(self, attacker, attacker);

	return 0;
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
		width = Q_fabs(DotProduct(up, ent->size));
	else
		width = Q_fabs(DotProduct(right, ent->size));

	float length = Q_fabs(DotProduct(forward, ent->size));

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
		ent->die = FuncDoorKilled;
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
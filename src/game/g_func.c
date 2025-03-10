//
// g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Utility.h" //mxd
#include "g_func_Door.h" //mxd
#include "g_combat.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "FX.h"
#include "Vector.h"
#include "g_local.h"

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
//
// g_func_Rotating.c -- Originally part of g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Rotating.h"
#include "EffectFlags.h"
#include "g_combat.h"
#include "g_DefaultMessageHandler.h"
#include "g_func_Door.h"
#include "Vector.h"

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

void FuncRotatingBlocked(edict_t* self, edict_t* other) //mxd. Named 'rotating_blocked' in original logic.
{
	T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void FuncRotatingTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'rotating_touch' in original logic.
{
	if (Vec3NotZero(self->avelocity))
		FuncRotatingBlocked(self, other);
}

void FuncRotatingUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'rotating_use' in original logic.
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
		VectorInverse(ent->movedir);

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
	Vec3AddAssign(ent->mins, ent->s.bmodel_origin);

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== func_rotating support logic ==========================

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

#pragma endregion
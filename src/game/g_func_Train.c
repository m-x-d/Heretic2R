//
// g_func_Train.c -- Originally part of g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_Train.h"
#include "g_func_Utility.h"
#include "g_combat.h"
#include "g_debris.h"
#include "FX.h"
#include "Vector.h"

#define SF_TRAIN_START_ON		1
#define SF_TRAIN_TOGGLE			2
#define SF_TRAIN_BLOCK_STOPS	4
#define SF_TRAIN_HAS_ORIGIN		8 //mxd
#define SF_TRAIN_NO_CLIP		16 //mxd

void FuncTrainAnim(edict_t* self) //mxd. Named 'train_anim' in original logic.
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

void FuncTrainAnimBackwards(edict_t* self) //mxd. Named 'train_animbackwards' in original logic.
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

void FuncTrainBlocked(edict_t* self, edict_t* other) //mxd. Named 'train_blocked' in original logic.
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

void FuncTrainWait(edict_t* self) //mxd. Named 'train_wait' in original logic.
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
			self->nextthink = THINK_NEVER; //mxd. '0' in original logic. Changed for consistency sake.
		}

		FuncPlayMoveEndSound(self); //mxd
	}
	else
	{
		FuncTrainNext(self);
	}
}

void FuncTrainNext(edict_t* self) //mxd. Named 'train_next' in original logic.
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

		FuncPlayMoveStartSound(ent); //mxd

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

void FuncTrainResume(edict_t* self) //mxd. Named 'train_next' in original logic.
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

void FuncTrainFind(edict_t* self) //mxd. Named 'func_train_find' in original logic.
{
	if (self->target == NULL)
	{
		gi.dprintf("FuncTrainFind: no target\n");
		self->think = NULL;

		return;
	}

	const edict_t* ent = G_PickTarget(self->target);

	if (ent == NULL)
	{
		gi.dprintf("FuncTrainFind: target %s not found\n", self->target);
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

void FuncTrainUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'train_use' in original logic.
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
		self->nextthink = THINK_NEVER; //mxd. '0' in original logic. Changed for consistency sake.
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

	if (self->spawnflags & SF_TRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else if (self->dmg == 0)
		self->dmg = 100;

	if (st.file != NULL)
	{
		self->solid = ((self->spawnflags & SF_TRAIN_NO_CLIP) ? SOLID_NOT : SOLID_BBOX);
		self->s.modelindex = (byte)gi.modelindex(st.file);
		VectorCopy(self->s.angles, self->moveinfo.end_angles);
	}
	else
	{
		self->solid = ((self->spawnflags & SF_TRAIN_NO_CLIP) ? SOLID_NOT : SOLID_BSP);
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
	Vec3AddAssign(self->mins, self->s.bmodel_origin);

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
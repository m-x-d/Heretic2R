//
// g_morcalavin_barrier.c -- Part of m_morcalavin.c in original logic.
//
// Copyright 2025 mxd.
//

#include "g_morcalavin_barrier.h"
#include "g_combat.h"
#include "p_main.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

static void MorcalavinBarrierThink(edict_t* self) //mxd. Named 'morcalavin_barrier_think' in original logic.
{
	// If we haven't found an owner yet, find one.
	if (self->owner == NULL)
	{
		edict_t* owner = G_Find(NULL, FOFS(classname), "monster_morcalavin");

		if (owner != NULL)
		{
			self->owner = owner;
			owner->morcalavin_barrier = self;
		}
		else
		{
			//mxd. Print warning.
			gi.dprintf("Warning: obj_morcalavin_barrier without monster_morcalavin at %s!\n", vtos(self->s.origin));
			self->think = NULL;
			self->nextthink = THINK_NEVER; //mxd. Use define.

			return;
		}
	}

	if (self->monsterinfo.attack_finished > level.time)
	{
		self->morcalavin_barrier_enabled = false;
		self->svflags |= SVF_NOCLIENT;
	}
	else
	{
		self->morcalavin_barrier_enabled = true;
		self->svflags &= ~SVF_NOCLIENT;
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void MorcalavinBarrierTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'morcalavin_barrier_touch' in original logic.
{
	if (strcmp(other->classname, "player") != 0 || !self->morcalavin_barrier_enabled) //TODO: check other->client instead?
		return;

	vec3_t vel;
	VectorSubtract(self->s.origin, other->s.origin, vel);
	VectorNormalize(vel);

	VectorScale(vel, -1.0f, vel);
	VectorScale(vel, 512.0f, other->velocity);

	other->velocity[2] = 128.0f;
	other->client->playerinfo.flags |= PLAYER_FLAG_USE_ENT_POS;

	// NOTENOTE: We should always have an owner. But this is for safety.
	edict_t* attacker = (self->owner != NULL ? self->owner : self);
	T_Damage(other, self, attacker, vel, other->s.origin, vel, irand(5, 10), 250, DAMAGE_AVOID_ARMOR, MOD_DIED);

	if (self->delay < level.time)
	{
		gi.CreateEffect(NULL, FX_WEAPON_STAFF_STRIKE, 0, other->s.origin, "db", vel, 2);
		self->delay = level.time + 0.5f;
	}
}

static void MorcalavinBarrierUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'morcalavin_barrier_use' in original logic.
{
	self->svflags &= ~SVF_NOCLIENT; // Become visible again.
	self->use = NULL; // Never do this again.

	// Start blocking.
	self->think = MorcalavinBarrierThink;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

// QUAKED obj_morcalavin_barrier (1 .5 0)
// The magical barrier that prevents the player from entering the tome area and defeating Morcalavin.
void SP_obj_morcalavin_barrier(edict_t* self)
{
	gi.setmodel(self, self->model);

	self->solid = SOLID_TRIGGER;
	self->movetype = PHYSICSTYPE_NONE;

	self->s.color.c = 0xffffffff;
	self->morcalavin_barrier_enabled = true;
	self->health = 1; //TODO: unused?

	self->touch = MorcalavinBarrierTouch;
	self->use = MorcalavinBarrierUse;

	gi.linkentity(self);

	// Be invisible until used.
	self->svflags |= SVF_NOCLIENT;
}
//
// spl_magicmissile.c
//
// Copyright 1998 Raven Software
//

#include "spl_magicmissile.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "Decals.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define MISSILE_RADIUS	2.0f //mxd. ARROW_RADIUS in original version.

static void MagicMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// Has the target got reflection turned on?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);

		// Scale speed down.
		Vec3ScaleAssign(MAGICMISSILE_SPEED / 2.0f, self->velocity);
		MagicMissileReflect(self, other, self->velocity);

		return;
	}

	if (other == self->owner || strcmp(self->classname, other->classname) == 0) // Don't collide with owner or other magic missiles.
		return;

	vec3_t scorch_origin;
	VectorCopy(self->s.origin, scorch_origin);

	// Calculate the position for the explosion entity.
	vec3_t origin;
	VectorMA(self->s.origin, -0.02f, self->velocity, origin);

	AlertMonsters(self, self->owner, 1.0f, false);

	if (other->takedamage != DAMAGE_NO)
	{
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, self->dmg, DAMAGE_SPELL, MOD_MMISSILE);
	}
	else
	{
		// Back off the origin for the damage a bit.
		// We are a point and this will help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -8.0f, self->movedir, self->s.origin);
	}

	// Do some blast damage when in deathmatch (too wimpy without it).
	if (DEATHMATCH)
	{
		T_DamageRadius(self, self->owner, self->owner, MAGICMISSILE_RADIUS, MAGICMISSILE_DAMAGE_RAD,
			MAGICMISSILE_DAMAGE_RAD * 0.25f, DAMAGE_SPELL | DAMAGE_EXTRA_KNOCKBACK, MOD_MMISSILE);
	}

	// Attempt to apply a scorchmark decal to the thing I hit.
	int make_scorch = 0;
	if (IsDecalApplicable(other, self->s.origin, surface, plane, NULL))
		make_scorch = CEF_FLAG6;

	gi.CreateEffect(&self->s, FX_WEAPON_MAGICMISSILEEXPLODE, make_scorch | CEF_OWNERS_ORIGIN, self->s.origin, "d", self->movedir);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/MagicMissileHit.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?

	G_SetToFree(self);
}

static void MagicMissileThink(edict_t* self)
{
	// Prevent any further transmission of this entity to clients.
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
}

// Create guts of magic missile
static void CreateMagicMissile(edict_t* missile) //mxd. Named 'create_magic' in original version.
{
	missile->s.effects = (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);
	missile->movetype = MOVETYPE_FLYMISSILE;
	missile->classname = "Spell_MagicMissile";
	missile->solid = SOLID_BBOX;
	missile->clipmask = MASK_SHOT;

	VectorSet(missile->mins, -MISSILE_RADIUS, -MISSILE_RADIUS, -MISSILE_RADIUS);
	VectorSet(missile->maxs,  MISSILE_RADIUS,  MISSILE_RADIUS,  MISSILE_RADIUS);

	missile->dmg = irand(MAGICMISSILE_DAMAGE_MIN, MAGICMISSILE_DAMAGE_MAX); // 30 - 40
	if (DEATHMATCH)
		missile->dmg /= 2; // 15 - 20

	missile->touch = MagicMissileTouch;
	missile->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

edict_t* MagicMissileReflect(edict_t* self, edict_t* other, vec3_t vel)
{
	// Create a new missile to replace the old one - this is necessary because physics will do nasty things
	// with the existing one, since we hit something. Hence, we create a new one totally.
	edict_t* missile = G_Spawn();

	// Copy everything across.
	CreateMagicMissile(missile);
	VectorCopy(self->s.origin, missile->s.origin);
	VectorCopy(vel, missile->velocity);
	VectorNormalize2(vel, missile->movedir);
	AnglesFromDir(missile->movedir, missile->s.angles);
	missile->owner = other;
	missile->think = MagicMissileThink;
	missile->health = self->health;
	missile->enemy = self->owner;
	missile->flags |= (self->flags & FL_NO_KNOCKBACK);
	missile->reflect_debounce_time = self->reflect_debounce_time - 1; // So it doesn't infinitely reflect in one frame somehow.
	missile->reflected_time = self->reflected_time;

	G_LinkMissile(missile);

	// Create new trails for the new missile.
	const short s_yaw = ANGLE2SHORT(missile->s.angles[YAW]);
	const short s_pitch = ANGLE2SHORT(missile->s.angles[PITCH]);
	gi.CreateEffect(&missile->s, FX_WEAPON_MAGICMISSILE, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "ss", s_yaw, s_pitch);

	// Kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point.
	gi.CreateEffect(&missile->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", missile->velocity);

	return missile;
}

void SpellCastMagicMissile(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles, const vec3_t aim_dir)
{
	// Spawn the magic-missile.
	edict_t* missile = G_Spawn();

	VectorNormalize2(aim_dir, missile->movedir);
	VectorAdd(start_pos, aim_dir, missile->s.origin);

	CreateMagicMissile(missile);
	missile->owner = caster;
	missile->reflect_debounce_time = MAX_REFLECT;

	G_LinkMissile(missile);

	trace_t trace;
	gi.trace(caster->s.origin, missile->mins, missile->maxs, missile->s.origin, caster, MASK_PLAYERSOLID, &trace);

	if (trace.startsolid)
	{
		MagicMissileTouch(missile, trace.ent, &trace.plane, trace.surface);
		return;
	}

	// Handle auto-targeting by looking for the nearest monster that:
	// a) Lies in a 30 degree degree horizontal, 180 degree vertical cone from my facing.
	// b) Lies within 0 to 1000 meters of myself.
	// c) Is visible (i.e. LOS exists from the missile to myself).
	missile->enemy = FindNearestVisibleActorInFrustum(missile, aim_angles, 0.0f, 1000.0f, ANGLE_30, ANGLE_180, false, missile->s.origin);

	if (missile->enemy != NULL)
	{
		vec3_t temp;
		VectorCopy(missile->s.origin, temp);
		VectorSubtract(missile->enemy->s.origin, temp, temp);

		for (int i = 0; i < 3; i++)
			temp[i] += (missile->enemy->mins[i] + missile->enemy->maxs[i]) / 2.0f;

		VectorNormalize(temp);
		vectoangles(temp, missile->s.angles);

		// The pitch is flipped in these?
		missile->s.angles[PITCH] *= -1.0f;
		VectorScale(temp, MAGICMISSILE_SPEED, missile->velocity);
	}
	else
	{
		VectorScale(aim_dir, MAGICMISSILE_SPEED, missile->velocity);
		VectorCopy(aim_angles, missile->s.angles);
	}

	const short s_yaw = ANGLE2SHORT(missile->s.angles[YAW]);
	const short s_pitch = ANGLE2SHORT(missile->s.angles[PITCH]);
	gi.CreateEffect(&missile->s, FX_WEAPON_MAGICMISSILE, CEF_OWNERS_ORIGIN, NULL, "ss", s_yaw, s_pitch);

	missile->think = MagicMissileThink;
	missile->nextthink = level.time + FRAMETIME; //mxd. Use define.
}
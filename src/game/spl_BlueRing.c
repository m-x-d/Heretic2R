//
// spl_bluering.c
//
// Copyright 1998 Raven Software
//

#include "spl_BlueRing.h" //mxd
#include "g_cmds.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "m_assassin.h" //mxd
#include "m_gkrokon.h" //mxd
#include "m_imp.h" //mxd
#include "m_mssithra.h" //mxd
#include "m_tcheckrik_spells.h"
#include "spl_flyingfist.h"
#include "spl_HellStaff.h"
#include "spl_magicmissile.h"
#include "spl_meteorbarrier.h"
#include "spl_morph.h"
#include "spl_Phoenix.h"
#include "spl_RedRain.h"
#include "spl_sphereofannihlation.h"
#include "FX.h"
#include "Vector.h"
#include "g_local.h"

// Since findradius is not specific enough for our needs.
// This, for one, will seek out player maceballs, arrows, and meteors.
edict_t* FindRingRadius(edict_t* from, const vec3_t org, const float rad, const edict_t* ring_ent) //mxd. Named 'findringradius' in original logic.
{
	static float max_sq;
	static vec3_t min;
	static vec3_t max;

	if (from == NULL)
	{
		max_sq = rad * rad;
		VectorCopy(org, min);
		VectorCopy(org, max);

		for (int i = 0; i < 3; i++)
		{
			min[i] -= rad;
			max[i] += rad;
		}
	}

	while ((from = FindInBounds(from, min, max)) != NULL)
	{
		if (from->reflected_time > level.time || from == ring_ent->owner)
			continue;

		if (fabsf(from->s.origin[2] - ring_ent->s.origin[2]) > 50.0f) // This is a RING, not a sphere. Cap the vert at 50].
			continue;

		if (from->client != NULL)
		{
			// Don't let these affect coop friends.
			if (COOP && !(DMFLAGS & DF_HURT_FRIENDS))
				continue;

			// Don't target team members in team deathmatch if they are on the same team and friendly fire is not enabled.
			if (DEATHMATCH && !(DMFLAGS & DF_HURT_FRIENDS) && (DMFLAGS & (DF_MODELTEAMS | DF_SKINTEAMS)))
				continue;
		}

		vec3_t dist;
		for (int i = 0; i < 3; i++)
			dist[i] = org[i] - (from->s.origin[i] + (from->mins[i] + from->maxs[i]) * 0.5f);

		if (VectorLengthSquared(dist) > max_sq)
			continue;

		// If we've already reflected this, don't do it again.  
		// We DO have to wait for after the radius check, however, because the shot might get closer over the next half second.
		from->reflected_time = level.time + 0.6f;

		return from;
	}

	return NULL;
}

void RingThink(edict_t* self)
{
#define RING_THINKS	4 // This is 0.4 seconds.

	// Kill the ring eventually.
	if (self->count-- < 1)
	{
		G_SetToFree(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	// Since find radius is not specific enough for our needs, here is.
	edict_t* ent = NULL;
	while ((ent = FindRingRadius(ent, self->s.origin, RING_EFFECT_RADIUS * 0.25f * (float)(RING_THINKS - self->count), self)) != NULL)
	{
		// If ent has mass, push it away.
		if (ent->mass > 0)
		{
			vec3_t vel;
			VectorSubtract(ent->s.origin, self->s.origin, vel);

			float scale = (RING_EFFECT_RADIUS - VectorLength(vel)) * (RING_KNOCKBACK_SCALE / RING_EFFECT_RADIUS)
				* sqrtf(RING_MASS_FACTOR / (float)ent->mass) + RING_KNOCKBACK_BASE;

			VectorNormalize(vel);

			// For players, force them up more and faster.
			if (ent->client != NULL)
			{
				vel[2] = 0.5f; //TODO: below block is never executed. Supposed to be 'vel[2] += 0.5f'?

				if (vel[2] > 0.0f && vel[2] < 0.5f)
				{
					scale *= 2.0f;
					VectorNormalize(vel);
				}
			}

			// Vel is just passing the direction of the knockback.
			G_PostMessage(ent, MSG_REPULSE, PRI_DIRECTIVE, "fff", vel[0], vel[1], vel[2] + 30.0f); //TODO: never handled!

			if (ent->takedamage != DAMAGE_NO)
			{
				vec3_t hit_loc;
				VectorMA(ent->s.origin, -ent->maxs[0], vel, hit_loc);

				const int knockback = ((ent->movetype != PHYSICSTYPE_NONE) ? (int)scale : 0); //mxd
				T_Damage(ent, ent, self, vel, hit_loc, vec3_origin, 4, knockback, DAMAGE_RADIUS | DAMAGE_SPELL, MOD_ROR);
			}

			continue;
		}

		// If it's a projectile, reflect it.
		qboolean hit = false;
		edict_t* (*reflect)(edict_t*, edict_t*, const vec3_t) = NULL;

		if (strcmp(ent->classname, "Spell_RedRainArrow") == 0) //TODO: convert to defines? Or add/use ent->classID?
		{
			reflect = RedRainMissileReflect;
		}
		else if (strcmp(ent->classname, "Spell_PhoenixArrow") == 0)
		{
			reflect = PhoenixMissileReflect;
		}
		else if (strcmp(ent->classname, "Spell_MeteorBarrier") == 0)
		{
			reflect = MeteorBarrierReflect;
		}
		else if (strcmp(ent->classname, "Spell_SphereOfAnnihilation") == 0)
		{
			reflect = SphereReflect;
		}
		else if (strcmp(ent->classname, "Spell_Hellbolt") == 0)
		{
			reflect = HellboltReflect;
		}
		else if (strcmp(ent->classname, "Spell_MorphArrow") == 0)
		{
			reflect = MorphReflect;
		}
		else if (strcmp(ent->classname, "Spell_MagicMissile") == 0)
		{
			reflect = MagicMissileReflect;
		}
		else if (strcmp(ent->classname, "Spell_FlyingFist") == 0)
		{
			reflect = FlyingFistReflect;
		}
		else if (strcmp(ent->classname, "Assassin_Dagger") == 0)
		{
			reflect = AssassinDaggerReflect;
		}
		else if (strcmp(ent->classname, "Gkrokon_Spoo") == 0)
		{
			reflect = GkrokonSpooReflect;
		}
		else if (strcmp(ent->classname, "imp fireball") == 0) //TODO: unused.
		{
			reflect = ImpFireballReflect;
		}
		else if (strcmp(ent->classname, "mssithra_Arrow") == 0)
		{
			reflect = MssithraArrowReflect;
		}
		else if (strcmp(ent->classname, "Spell_SpearProj") == 0)
		{
			reflect = SpearProjReflect;
		}
		else if (strcmp(ent->classname, "Spell_Maceball") == 0 && ent->owner != self->owner) // Don't reflect your own projectiles.
		{
			hit = true;

			// Give the self credit for stuff killed with it, or worse yet, set the originator as the enemy.
			ent->enemy = ent->owner;
			ent->owner = self->owner;

			// Do a nasty looking blast at the impact point.
			gi.CreateEffect(&ent->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", ent->velocity);

			//TODO: actually reflect when projectile is not our own?..
		}

		if (reflect != NULL)
		{
			if (ent->owner != self && ent->reflect_debounce_time > 0)
				hit = true;
			else
				reflect = NULL;
		}

		if (hit)
		{
			vec3_t vel;
			VectorSubtract(self->s.origin, ent->s.origin, vel);
			VectorNormalize(vel);

			// The dot product is the velocity towards the self (normally negative), let's reverse it.
			float scale = DotProduct(vel, ent->velocity);

			// If heading towards the self, reverse that portion of the velocity.
			if (scale > 0.0f)
				scale *= -2.0f;

			VectorMA(ent->velocity, scale, vel, vel);

			if (reflect != NULL)
			{
				if (Vec3IsZero(vel)) // Reflect needs a non-zero vel. If zeroed, throw it straight up.
					VectorSet(vel, 0.0f, 0.0f, 200.0f);

				edict_t* new_ent = reflect(ent, self->owner, vel);
				vectoangles(new_ent->velocity, new_ent->s.angles);
			}
		}

	}
}

// Formula for knockback:
// 1 to 0 (center to outside) * KNOCKBACK_SCALE + KNOCKBACK_BASE.
// This total is multiplied by (MASS_FACTOR / mass) (less if mass > 200, more if < 200).
void SpellCastBlueRing(edict_t* caster)
{
	// Create the actual effect entity.
	edict_t* ring = G_Spawn();

	ring->owner = caster;
	ring->solid = SOLID_NOT;
	ring->svflags |= SVF_NOCLIENT;
	ring->movetype = PHYSICSTYPE_NONE;
	ring->classname = "Spell_Ring";
	ring->count = RING_THINKS;
	ring->timestamp = level.time;
	VectorCopy(caster->s.origin, ring->s.origin);

	ring->think = RingThink;
	ring->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(ring);

	// Fire off a special effect.
	gi.CreateEffect(&caster->s, FX_SPELL_BLUERING, CEF_OWNERS_ORIGIN, NULL, "");

	//mxd. Play the reflect sound (unused in original logic).
	gi.sound(caster, CHAN_VOICE, gi.soundindex("weapons/reflect.wav"), 1.0f, ATTN_NORM, 0.0f);
}
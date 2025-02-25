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

		if (fabsf(from->s.origin[2] - ring_ent->s.origin[2]) > 50.0f) // This is a RING, not a sphere. Cap the vert at 40].
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

void RingThink(edict_t *self)
{
#define RING_THINKS	4		// This is a .4 seconds

	int			hit;
	edict_t		*ent = NULL, *newent;
	vec3_t		vel, hitloc;
	vec_t		scale;
	edict_t* (*reflect)(edict_t*, edict_t*, vec3_t);

	// kill the ring eventually
	self->nextthink = level.time + 0.1;
	if (self->count <= 0)
	{
		G_SetToFree(self);
		return;
	}
	self->count--;

	// Since find radius is not specific enough for our needs, here is 
	while(ent = FindRingRadius(ent, self->s.origin, RING_EFFECT_RADIUS*0.25*(RING_THINKS-self->count), self))
	{
		hit = false;
		reflect = NULL;
		if (ent->mass)
		{
			VectorSubtract(ent->s.origin, self->s.origin, vel);
			scale = (RING_EFFECT_RADIUS - VectorLength(vel)) 
						* (RING_KNOCKBACK_SCALE/RING_EFFECT_RADIUS) 
						* sqrt(RING_MASS_FACTOR / ent->mass)
						+ RING_KNOCKBACK_BASE;
			VectorNormalize(vel);
			if (ent->client)
			{	// For players, force them up more and faster.
				vel[2] = 0.5;
				if (vel[2] < 0.5 && vel[2] > 0.0)
				{
					scale *= 2.0;
					VectorNormalize(vel);
				}
			}
			// Vel is just passing the direction of the knockback.
			QPostMessage(ent, MSG_REPULSE, PRI_DIRECTIVE, "fff", vel[0], vel[1], vel[2] + 30.0);
			if (ent->takedamage)
			{
				VectorMA(ent->s.origin, -ent->maxs[0], vel, hitloc);
				if (ent->movetype != PHYSICSTYPE_NONE)
					T_Damage (ent, ent, self, vel, hitloc, vec3_origin, 4, (int)scale, DAMAGE_RADIUS | DAMAGE_SPELL,MOD_ROR);
				else
					T_Damage (ent, ent, self, vel, hitloc, vec3_origin, 4, 0, DAMAGE_RADIUS | DAMAGE_SPELL,MOD_ROR);
			}
		}
		else if (strcmp(ent->classname, "Spell_RedRainArrow") == 0)
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
			reflect = AssassinArrowReflect;
		}
		else if (strcmp(ent->classname, "Gkrokon_Spoo") == 0)
		{
			reflect = GkrokonSpooReflect;
		}
		else if (strcmp(ent->classname, "imp fireball") == 0)
		{
			reflect = ImpFireballReflect;
		}
		else if (strcmp(ent->classname, "mssithra_Arrow") == 0)
		{
			reflect = MssithraAlphaArrowReflect;
		}
		else if (strcmp(ent->classname, "Spell_SpearProj") == 0)
		{
			reflect = SpearProjReflect;
		}
		else if (strcmp(ent->classname, "Spell_Maceball") == 0)
		{
			if (ent->owner != self->owner)
			{	// Don't screw up your own projectiles.

				hit = true;
				// Give the self credit for stuff killed with it, or worse yet, set the originator as the enemy.
				ent->enemy = ent->owner;
				ent->owner = self->owner;

				// Do a nasty looking blast at the impact point
				gi.CreateEffect(&ent->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", ent->velocity);
			}
		}

		if (reflect)
		{
			if (ent->owner != self && ent->reflect_debounce_time)
			{
				hit = true;
			}
			else
			{
				reflect = NULL;
			}
		}

		if (hit)
		{
			VectorSubtract(self->s.origin, ent->s.origin, vel);
			VectorNormalize(vel);
			// The dot product is the velocity towards the self (normally negative), let's reverse it.
			scale = DotProduct(vel, ent->velocity);
			if (scale > 0)	// If heading towards the self, reverse that portion of the velocity
				VectorMA(ent->velocity, -2.0*scale, vel, vel);
			else	// Jes' double the speed away
				VectorMA(ent->velocity, scale, vel, vel);
			if(reflect)
			{
				if (Vec3IsZero(vel))	// Reflect needs a non-zero vel.  If zeroed, throw it straight up.
					VectorSet(vel, 0, 0, 200.0);
				newent = reflect(ent, self->owner, vel);
				vectoangles(newent->velocity, newent->s.angles);
			}
		}
	}
}


// Formula for knockback:	1 to 0 (center to outside) * KNOCKBACK_SCALE + KNOCKBACK_BASE
//							This total is multiplied by (MASS_FACTOR / mass).  (If mass > 200, less, if < 200, more)
void SpellCastBlueRing(edict_t *Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t AimDir, float value)
{
	edict_t		*newent;
				  
	// create the actual effect entity
	newent = G_Spawn();
	newent->owner = Caster;
	newent->solid = SOLID_NOT;
	newent->svflags |= SVF_NOCLIENT;
	newent->movetype = PHYSICSTYPE_NONE;
	newent->classname = "Spell_Ring";
	newent->nextthink = level.time + 0.1;
	newent->think = RingThink;
	newent->count = RING_THINKS;
	newent->timestamp = level.time;
	VectorCopy(Caster->s.origin, newent->s.origin);
	gi.linkentity(newent); 

	// fire off a special effect.
	gi.CreateEffect(&Caster->s, FX_SPELL_BLUERING, CEF_OWNERS_ORIGIN, 0, "");
}

// end

//
// spl_bluering.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "random.h"
#include "vector.h"
#include "g_playstats.h"
#include "p_actions.h"


#define RING_THINKS	4		// This is a .4 seconds


extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);
extern edict_t *PhoenixMissileReflect(edict_t *missile, edict_t *other, vec3_t vel);
extern edict_t *RedRainMissileReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *MeteorBarrierReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *SphereReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *HellboltReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *MorphReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *MagicMissileReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *FlyingFistReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *AssassinArrowReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *GkrokonSpooReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *ImpFireballReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *MssithraAlphaArrowReflect(edict_t *self, edict_t *other, vec3_t vel);
extern edict_t *SpearProjReflect(edict_t *self, edict_t *other, vec3_t vel);



// Since findradius is not specific enough for our needs
// This, for one, will seek out player maceballs, arrows, and meteors.
edict_t *findringradius (edict_t *from, vec3_t org, float rad, edict_t *ringent)
{
	static float max2;
	static vec3_t min;
	static vec3_t max;
	vec3_t	eorg;
	int		j;
	float elen;

	if (!from)
	{
		max2=rad*rad;
		VectorCopy(org,min);
		VectorCopy(org,max);
		for (j=0 ; j<3 ; j++)
		{
			min[j]-=rad;
			max[j]+=rad;
		}
	}

	while (1)
	{
		from=findinbounds(from,min,max);
		if (!from)
			return 0;
		if ((from->reflected_time > level.time) || (from == ringent->owner))
			continue;
		if ((fabs(from->s.origin[2] - ringent->s.origin[2])) > 50.0)		// This is a RING, not a sphere.  Cap the vert at 40].
			continue;

		// don't let these affect coop friends
		if (from->client && coop->value && !((int)dmflags->value & DF_HURT_FRIENDS))
			continue;

		// don't target team members in team deathmatching, if they are on the same team, and friendly fire is not enabled.
		if ((from->client && (int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)) && !((int)dmflags->value & DF_HURT_FRIENDS) && deathmatch->value)
		{
			if (OnSameTeam(from, ringent->owner))
				continue;
		}


		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		elen = DotProduct(eorg,eorg);

		if (elen > max2)
			continue;

		// if we've already reflected this, don't do it again.  
		// We DO have to wait for after the radius check, however, because the shot might get closer over the next half second.
		from->reflected_time = level.time + 0.6;

		return from;
	} 
}


void RingThink(edict_t *self)
{
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
	while(ent = findringradius(ent, self->s.origin, RING_EFFECT_RADIUS*0.25*(RING_THINKS-self->count), self))
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

//
// spl_meteorbarrier.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "Angles.h"
#include "Utilities.h"
#include "random.h"
#include "vector.h"
#include "decals.h"
#include "g_playstats.h"

#define METEOR_BARRIER_DIE_EXPLODE			1
#define METEOR_BARRIER_DIE_EXPLODEIMPACT	2

#define	METEOR_RADIUS	4.0
#define	METEOR_HUNT_SPEED	600.0

static void MeteorBarrierDie(edict_t *self, int Flags);
static void MeteorBarrierSearchInitThink(edict_t *self);
static void MeteorBarrierSearchThink(edict_t *self);
static void MeteorBarrierHuntThink(edict_t *self);
static void MeteorBarrierTouch(edict_t *self, trace_t *trace);
extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);

void create_meteor(edict_t *Meteor);
vec3_t	BBMin = {-5.0,-5.0,-5.0};
vec3_t	BBMax = {5.0,5.0,5.0};

// ************************************************************************************************
// MeteorBarrierDie
// ----------------
// ************************************************************************************************

static void MeteorBarrierDie(edict_t *self, int Flags)
{
	vec3_t	ExplodeDir;
	int		flag = 0;

	// If required, create an explode client effect and make an explosion noise.
	VectorScale(self->movedir, -1.0, ExplodeDir);

	if(Flags & METEOR_BARRIER_DIE_EXPLODE)
	{
		if(Flags & METEOR_BARRIER_DIE_EXPLODEIMPACT)
		{
			flag = CEF_FLAG6;
		}
		gi.CreateEffect(NULL,
						FX_SPELL_METEORBARRIEREXPLODE,
						CEF_BROADCAST|flag,
						self->s.origin,
						"d",
						ExplodeDir);

		gi.sound(self, CHAN_BODY, gi.soundindex("weapons/MeteorBarrierImpact.wav"), 2, ATTN_NORM, 0);
	}

	// remove the persistant effect from the persistant effect list
	if (self->PersistantCFX)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_METEOR);
		gi.RemoveEffects(&self->owner->s, FX_SPELL_METEORBARRIER+self->health);
		self->PersistantCFX = 0;
	}

	// remove the players pointer to this now in-active meteor
	self->owner->client->Meteors[self->health] = NULL;

	// now we've been cast, remove us from the count of meteors the caster owns, and turn off his looping sound if need be
	self->owner->client->playerinfo.meteor_count &= ~(1<<self->health);
	if (!self->owner->client->playerinfo.meteor_count)
		self->owner->s.sound = 0;

	// And finally remove myself (with assoc cfx)
	G_SetToFree(self);
}

static void Kill_Meteor(edict_t *self)
{
	MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE);
}

// ************************************************************************************************
// MeteorBarrierHuntThink
// ----------------------
// ************************************************************************************************

static void MeteorBarrierHuntThink(edict_t *self)
{
	vec3_t	Heading, Dest;
	float	dist;
	trace_t	tr;
	
//	self->svflags &= ~SVF_ALWAYS_SEND;
	if (deathmatch->value == 0 || self->accel == 0.0)			// These don't home in on an enemy in deathmatch... too powerful.
	{
		if((self->enemy->health > 0) && (self->targetname && !strcmp(self->enemy->classname, self->targetname)) && self->enemy->inuse)
		{
			VectorCopy(self->enemy->s.origin, Dest);

			Dest[0] += (self->enemy->mins[0] + self->enemy->maxs[0]) * 0.5;
			Dest[1] += (self->enemy->mins[1] + self->enemy->maxs[1]) * 0.5;
			Dest[2] += (self->enemy->mins[2] + self->enemy->maxs[2]) * 0.5;

			VectorSubtract(Dest, self->s.origin, Heading);
			VectorScale(Heading, 10, self->velocity);
			dist = VectorLength(Heading);
			// are we now in the center of something ?
			if (dist < 5)
			{
				gi.trace(self->s.origin, self->mins, self->maxs, self->s.origin, self, MASK_MONSTERSOLID, &tr);
				MeteorBarrierTouch(self,&tr);
				return;
			}

			VectorNormalize(Heading);
			VectorCopy(Heading, self->movedir);
			// don't let us over shoot the enemy
			if (dist > (METEOR_HUNT_SPEED * 0.1))
				VectorScale(self->movedir, METEOR_HUNT_SPEED, self->velocity);
			
			
			self->nextthink = level.time + 0.1;
			self->accel = 1.0;		// Signal that we have already gotten a target speed.
		}
		else
		{
			// My enemy has died so I die too.
			MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE);
		}
	}
	else
	{
		// be sure it dies within 2 minutes.
		self->nextthink = level.time + 2.0;
		self->think = Kill_Meteor;
	}

}

// ************************************************************************************************
// MeteorBarrierHuntThink
// ----------------------
// ************************************************************************************************

static void MeteorBarrierBounceThink(edict_t *self)
{
	self->random += 20;			// Lifetime
	if((self->enemy->health > 0) && (self->random < (5000 + (self->health * 200.0))))
	{
		self->nextthink = level.time + 0.1;
	}
	else
	{
		// My enemy has died so I die too.
		MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE);
	}
}



// ************************************************************************************************
// MeteorBarrierReflect
// ------------------
// ************************************************************************************************

edict_t *MeteorBarrierReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t		*Meteor;

	Meteor = G_Spawn();
	VectorCopy(self->s.origin, Meteor->s.origin);
	create_meteor(Meteor);
	VectorCopy(vel, Meteor->velocity);
	Meteor->owner = self->owner;
	Meteor->enemy = self->enemy;
	Meteor->health = self->health;
	Meteor->count = self->count;
	Meteor->random = self->random;							// Lifetime count
	Meteor->solid = self->solid;
	VectorCopy(BBMin,Meteor->mins);
	VectorCopy(BBMax,Meteor->maxs);
	Meteor->movetype = PHYSICSTYPE_FLY;
	Meteor->think = MeteorBarrierBounceThink;
	Meteor->nextthink = level.time+0.1;
	Meteor->reflect_debounce_time = self->reflect_debounce_time -1;
	Meteor->reflected_time=self->reflected_time;
	Meteor->s.effects |= EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;
	Meteor->svflags = SVF_ALWAYS_SEND;
	gi.linkentity(Meteor); 

	// remove the persistant effect from the persistant effect list
	if (self->PersistantCFX)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_METEOR);
		gi.RemoveEffects(&self->owner->s, FX_SPELL_METEORBARRIER+self->health);
		self->PersistantCFX = 0;
	}

	// replace this new meteor in the owners meteor list
	Meteor->owner->client->Meteors[Meteor->health] = Meteor;

	// create a client effect for this new meteor
	gi.CreateEffect(&Meteor->s, FX_SPELL_METEORBARRIER_TRAVEL, CEF_BROADCAST|CEF_OWNERS_ORIGIN, NULL, "");

	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(&Meteor->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", Meteor->velocity);

	return(Meteor);
}



// ************************************************************************************************
// MeteorBarrierTouch
// ------------------
// ************************************************************************************************

static void MeteorBarrierTouch(edict_t *self, trace_t *trace)
{
	edict_t		*Other;
	csurface_t	*Surface;
	cplane_t   	*Plane;

	Other = trace->ent;
	Surface = trace->surface;
	Plane = &trace->plane;

	// has the target got reflection turned on ?
	if (self->reflect_debounce_time)
	{
		if(EntReflecting(Other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			Vec3ScaleAssign(METEOR_HUNT_SPEED/2,self->velocity);
			MeteorBarrierReflect(self, Other, self->velocity);

			return;
		}
	}

	if(Surface && (Surface->flags & SURF_SKY))
	{
		MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE);
		return;
	}

	AlertMonsters (self, self->owner, 1, false);
	if(Other->takedamage)
	{
		T_Damage(Other, self, self->owner, self->movedir, self->s.origin, Plane->normal, self->dmg, 0, DAMAGE_SPELL,MOD_METEORS);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will help fix hitting
		// the base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -8.0, self->movedir, self->s.origin);
	}
	MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE | METEOR_BARRIER_DIE_EXPLODEIMPACT);
}

// ************************************************************************************************
// SpellCastMeteorBarrier
// ----------------------
// ************************************************************************************************

// Make meteors orbit player

static void MeteorBarrierSearchThink(edict_t *self)
{
	edict_t *NewTarg = NULL;
	int		DoneSearching = 0;
	trace_t	tr;
	
	// Only check for a target every so often as this reduces CPU requirements AND it looks much
	// cooler.
	// (using self->owner->enemy as the target would be much quicker...but not 360 degrees)

	if(!irand(0, METEOR_SEARCH_CHANCE))
	{
		NewTarg = FindSpellTargetInRadius(self, METEOR_SEARCH_RADIUS, self->s.origin, BBMin, BBMax);

		// we found something to shoot at, lets go get it
		if(NewTarg)
		{
			self->enemy = NewTarg;
			self->solid = SOLID_BBOX;
			VectorCopy(BBMin,self->mins);
			VectorCopy(BBMax,self->maxs);
			self->accel = 0.0;
			self->think = MeteorBarrierHuntThink;
			self->movetype = PHYSICSTYPE_FLY;
			self->nextthink = level.time + 0.1;
			self->svflags = SVF_ALWAYS_SEND;
			self->s.effects |= EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;
			self->targetname = self->enemy->classname;
			self->alert_time = 0;
	
			// did we start up inside someone ? - check and see
			gi.trace(self->s.origin, self->mins, self->maxs, self->s.origin, self, MASK_MONSTERSOLID, &tr);
			if(tr.startsolid)
			{
				MeteorBarrierTouch(self,&tr);
				return;
			}

			gi.sound(self,CHAN_BODY,gi.soundindex("weapons/MeteorBarrierSeek.wav"),1,ATTN_NORM,0);
			gi.CreateEffect(&self->s, FX_SPELL_METEORBARRIER_TRAVEL, CEF_BROADCAST|CEF_OWNERS_ORIGIN, NULL, "");

			// remove the persistant effect from the persistant effect list
			if (self->PersistantCFX)
			{
				gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_METEOR);
				gi.RemoveEffects(&self->owner->s, FX_SPELL_METEORBARRIER+self->health);
				self->PersistantCFX = 0;
			}

			// now we've been cast, remove us from the count of meteors the caster owns, and turn off his looping sound if need be
			self->owner->client->playerinfo.meteor_count &= ~(1<<self->health);
			if (!self->owner->client->playerinfo.meteor_count)
				self->owner->s.sound = 0;
			return;
		}
	}

	self->random += 20;			// Lifetime

	if((self->owner->health > 0) && (self->random < (5000 + (self->health * 200.0))))
	{
		float Angle;

		Angle = ((level.time * 150.0) + (90.0 * self->health)) * ANGLE_TO_RAD;
		VectorCopy(self->owner->s.origin, self->s.origin);
		self->s.origin[0] += cos(Angle) * 30.0;
		self->s.origin[1] += sin(Angle) * 30.0;
		self->s.origin[2] += cos(Angle / (M_PI / 5)) * 10.0;
		self->nextthink = level.time + 0.1;
	}
	else
	{	
		// My lifetime has expired so I die.
		MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE);
	}
}

// Move the meteors out to radius

static void MeteorBarrierSearchInitThink(edict_t *self)
{
	float	Angle;

	if(self->owner->health > 0)
	{
		Angle = ((level.time * 150.0) + (90.0 * self->health)) * ANGLE_TO_RAD;
		VectorCopy(self->owner->s.origin, self->s.origin);
		self->s.origin[0] += cos(Angle) * 30.0 * (self->count / 5.0);
		self->s.origin[1] += sin(Angle) * 30.0 * (self->count / 5.0);
		self->s.origin[2] += cos(Angle / (M_PI / 5)) * 10.0;

		if(self->count++ > 5)
		{
			self->random = self->health * 90.0;
			self->think = MeteorBarrierSearchThink;
		}
		self->nextthink = level.time + 0.1;
	}
	else
	{	
		// My caster died so I die too.
		MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE);
	}
}

void create_meteor(edict_t *Meteor)
{
   	Meteor->movetype = PHYSICSTYPE_NOCLIP;
   	Meteor->classname = "Spell_MeteorBarrier";
   	Meteor->isBlocked = MeteorBarrierTouch;
   	Meteor->isBlocking = MeteorBarrierTouch;
   	Meteor->dmg = irand(METEOR_DAMAGE_MIN, METEOR_DAMAGE_MAX);
	if (deathmatch->value)
		Meteor->dmg *= 0.5;		// These badasses do half damage in deathmatch.
   	Meteor->clipmask = MASK_SHOT;
   	VectorSet(Meteor->mins, -METEOR_RADIUS, -METEOR_RADIUS, -METEOR_RADIUS);
   	VectorSet(Meteor->maxs, METEOR_RADIUS, METEOR_RADIUS, METEOR_RADIUS);
   	Meteor->nextthink = level.time+0.1;
	Meteor->takedamage = DAMAGE_NO;
	// no gravity
	Meteor->gravity = 0;
}

// Spawn the meteors

void SpellCastMeteorBarrier(edict_t *Caster,vec3_t StartPos,vec3_t AimAngles,vec3_t AimDir,float Value)
{
	int		I, cast;
	edict_t	*Meteor;

	// Now create up to 4 spinning meteors.

	cast = false;
	for(I = 0; I < 4; I++)
	{		
		// If my caster is a player, then make sure they only have one instance of me active, then
		if(Caster->client)
		{
			if(Caster->client->Meteors[I])
				continue;
		}

		// enough mana to do this ?
		if (Caster->client->playerinfo.pers.inventory.Items[Caster->client->playerinfo.def_ammo_index] < Caster->client->playerinfo.pers.defence->quantity)
			break;

		// decrement our mana
		if (!deathmatch->value || (deathmatch->value && !((int)dmflags->value & DF_INFINITE_MANA)))
			Caster->client->playerinfo.pers.inventory.Items[Caster->client->playerinfo.def_ammo_index] -= Caster->client->playerinfo.pers.defence->quantity;

		cast = true;
		Meteor = G_Spawn();
		Meteor->svflags |= SVF_NOCLIENT;

		if(Caster->client)
		{
			Caster->client->Meteors[I] = Meteor;
		}

		VectorCopy(StartPos, Meteor->s.origin);
		create_meteor(Meteor);
		Meteor->reflect_debounce_time = MAX_REFLECT;
		Meteor->health = I;
		Meteor->think = MeteorBarrierSearchInitThink;
	   	Meteor->count = 0;
		Meteor->random = 0;							// Lifetime count
		Meteor->solid = SOLID_NOT;
		Meteor->owner = Caster;
		Caster->client->playerinfo.meteor_count |= 1<<I;				// determine how many meteors are still attached to the player

		gi.linkentity(Meteor);

		Meteor->PersistantCFX = gi.CreatePersistantEffect(&Caster->s, FX_SPELL_METEORBARRIER+I, CEF_BROADCAST|CEF_OWNERS_ORIGIN|(I<<5), NULL, "" );

	}
	if(cast)
	{
		gi.sound(Caster,CHAN_WEAPON,gi.soundindex("weapons/MeteorBarrierCast.wav"),1,ATTN_NORM,0);
	 	Caster->s.sound = gi.soundindex("weapons/MeteorBarrierAmbient.wav");
	 	Caster->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
	}
}
// end
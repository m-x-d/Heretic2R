//
// spl_meteorbarrier.c
//
// Copyright 1998 Raven Software
//

#include "spl_meteorbarrier.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define METEOR_BARRIER_DIE_EXPLODE			1
#define METEOR_BARRIER_DIE_EXPLODEIMPACT	2

#define	METEOR_RADIUS		4.0f
#define	METEOR_HUNT_SPEED	600.0f

static vec3_t bb_min = { -5.0f, -5.0f, -5.0f };
static vec3_t bb_max = {  5.0f,  5.0f,  5.0f };

void create_meteor(edict_t *Meteor);

static void MeteorBarrierDie(edict_t* self, const int flags)
{
	// If required, create an explode client effect and make an explosion noise.
	if (flags & METEOR_BARRIER_DIE_EXPLODE)
	{
		vec3_t explode_dir;
		VectorScale(self->movedir, -1.0f, explode_dir);

		const int fx_flags = ((flags & METEOR_BARRIER_DIE_EXPLODEIMPACT) ? CEF_FLAG6 : 0);
		gi.CreateEffect(NULL, FX_SPELL_METEORBARRIEREXPLODE, fx_flags | CEF_BROADCAST, self->s.origin, "d", explode_dir);
		gi.sound(self, CHAN_BODY, gi.soundindex("weapons/MeteorBarrierImpact.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?
	}

	// Remove the persistent effect from the persistent effects list.
	if (self->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_METEOR);
		gi.RemoveEffects(&self->owner->s, FX_SPELL_METEORBARRIER + self->health);
		self->PersistantCFX = 0;
	}

	// Remove the players pointer to this now in-active meteor.
	self->owner->client->Meteors[self->health] = NULL;

	// Now we've been cast, remove us from the count of meteors the caster owns, and turn off his looping sound if need be.
	self->owner->client->playerinfo.meteor_count &= ~(1 << self->health);

	if (self->owner->client->playerinfo.meteor_count == 0)
		self->owner->s.sound = 0;

	// And finally remove myself (with associated cfx).
	G_SetToFree(self);
}

static void Kill_Meteor(edict_t* self)
{
	MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE);
}

static void MeteorBarrierOnBlocked(edict_t* self, trace_t* trace) //mxd. Named 'MeteorBarrierTouch' in original version.
{
	edict_t* other = trace->ent;
	
	// Has the target got reflection turned on?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(METEOR_HUNT_SPEED / 2.0f, self->velocity);
		MeteorBarrierReflect(self, other, self->velocity);

		return;
	}

	if (trace->surface != NULL && (trace->surface->flags & SURF_SKY))
	{
		Kill_Meteor(self);
		return;
	}

	AlertMonsters(self, self->owner, 1.0f, false);

	if (other->takedamage != DAMAGE_NO)
	{
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, trace->plane.normal, self->dmg, 0, DAMAGE_SPELL, MOD_METEORS);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will help fix hitting
		// the base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -8.0f, self->movedir, self->s.origin);
	}

	MeteorBarrierDie(self, METEOR_BARRIER_DIE_EXPLODE | METEOR_BARRIER_DIE_EXPLODEIMPACT);
}

static void MeteorBarrierHuntThink(edict_t* self)
{
	// Don't home in on an enemy in deathmatch... too powerful.
	if (DEATHMATCH && self->accel > 0.0f)
	{
		// Be sure it dies within 2 minutes.
		self->nextthink = level.time + 2.0f;
		self->think = Kill_Meteor;

		return;
	}

	if (self->enemy->inuse && self->enemy->health > 0 && self->targetname != NULL && strcmp(self->enemy->classname, self->targetname) == 0)
	{
		vec3_t dest;
		VectorCopy(self->enemy->s.origin, dest);

		for (int i = 0; i < 3; i++)
			dest[i] += (self->enemy->mins[i] + self->enemy->maxs[i]) * 0.5f;

		vec3_t heading;
		VectorSubtract(dest, self->s.origin, heading);
		VectorScale(heading, 10.0f, self->velocity);

		const float dist = VectorLength(heading);

		// Are we now in the center of something?
		if (dist < 5.0f)
		{
			trace_t	tr;
			gi.trace(self->s.origin, self->mins, self->maxs, self->s.origin, self, MASK_MONSTERSOLID, &tr);
			MeteorBarrierOnBlocked(self, &tr);

			return;
		}

		VectorNormalize(heading);
		VectorCopy(heading, self->movedir);

		// Don't let us over shoot the enemy.
		if (dist > METEOR_HUNT_SPEED * 0.1f)
			VectorScale(self->movedir, METEOR_HUNT_SPEED, self->velocity);

		self->nextthink = level.time + 0.1f;
		self->accel = 1.0f; // Signal that we have already gotten a target speed.
	}
	else
	{
		Kill_Meteor(self); // My enemy has died so I die too.
	}
}

static void MeteorBarrierBounceThink(edict_t* self)
{
	self->random += 20.0f; // Lifetime.

	if (self->enemy->health > 0 && self->random < 5000.0f + (float)self->health * 200.0f)
		self->nextthink = level.time + 0.1f;
	else
		Kill_Meteor(self); // My enemy has died so I die too.
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
	VectorCopy(bb_min,Meteor->mins);
	VectorCopy(bb_max,Meteor->maxs);
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
		NewTarg = FindSpellTargetInRadius(self, METEOR_SEARCH_RADIUS, self->s.origin, bb_min, bb_max);

		// we found something to shoot at, lets go get it
		if(NewTarg)
		{
			self->enemy = NewTarg;
			self->solid = SOLID_BBOX;
			VectorCopy(bb_min,self->mins);
			VectorCopy(bb_max,self->maxs);
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
				MeteorBarrierOnBlocked(self,&tr);
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
   	Meteor->isBlocked = MeteorBarrierOnBlocked;
   	Meteor->isBlocking = MeteorBarrierOnBlocked;
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
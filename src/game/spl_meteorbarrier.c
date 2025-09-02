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

		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
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
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	else
		Kill_Meteor(self); // My enemy has died so I die too.
}

static void CreateMeteor(edict_t* meteor) //mxd. Named 'create_meteor' in original version.
{
	meteor->movetype = PHYSICSTYPE_NOCLIP;
	meteor->gravity = 0.0f; // No gravity.
	meteor->classname = "Spell_MeteorBarrier";
	meteor->takedamage = DAMAGE_NO;
	meteor->clipmask = MASK_SHOT;

	meteor->dmg = irand(METEOR_DAMAGE_MIN, METEOR_DAMAGE_MAX);
	if (DEATHMATCH)
		meteor->dmg /= 2; // These badasses do half damage in deathmatch.

	VectorSet(meteor->mins, -METEOR_RADIUS, -METEOR_RADIUS, -METEOR_RADIUS);
	VectorSet(meteor->maxs, METEOR_RADIUS, METEOR_RADIUS, METEOR_RADIUS);

	meteor->isBlocked = MeteorBarrierOnBlocked;
	meteor->isBlocking = MeteorBarrierOnBlocked;
	meteor->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

edict_t* MeteorBarrierReflect(edict_t* self, edict_t* other, vec3_t vel)
{
	// Create a new missile to replace the old one - this is necessary because physics will do nasty things
	// with the existing one, since we hit something. Hence, we create a new one totally.
	edict_t* meteor = G_Spawn();

	// Copy everything across.
	CreateMeteor(meteor);
	VectorCopy(self->s.origin, meteor->s.origin);
	VectorCopy(vel, meteor->velocity);
	meteor->owner = self->owner;
	meteor->enemy = self->enemy;
	meteor->health = self->health;
	meteor->count = self->count;
	meteor->random = self->random; // Lifetime count.
	meteor->solid = self->solid;
	VectorCopy(bb_min, meteor->mins);
	VectorCopy(bb_max, meteor->maxs);
	meteor->movetype = PHYSICSTYPE_FLY;
	meteor->reflect_debounce_time = self->reflect_debounce_time - 1;
	meteor->reflected_time = self->reflected_time;
	meteor->s.effects |= (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);
	meteor->svflags = SVF_ALWAYS_SEND;

	meteor->think = MeteorBarrierBounceThink;
	meteor->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(meteor);

	// Remove the persistent effect from the persistent effects list.
	if (self->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_METEOR);
		gi.RemoveEffects(&self->owner->s, FX_SPELL_METEORBARRIER + self->health);
		self->PersistantCFX = 0;
	}

	// Replace this new meteor in the owners meteor list.
	meteor->owner->client->Meteors[meteor->health] = meteor;

	// Create a client effect for this new meteor.
	gi.CreateEffect(&meteor->s, FX_SPELL_METEORBARRIER_TRAVEL, CEF_BROADCAST | CEF_OWNERS_ORIGIN, NULL, "");

	// Kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(&meteor->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", meteor->velocity);

	return meteor;
}

static qboolean FindNewTarget(edict_t* self) //mxd. Split out of MeteorBarrierSearchThink().
{
	edict_t* target = FindSpellTargetInRadius(self, METEOR_SEARCH_RADIUS, self->s.origin, bb_min, bb_max);

	if (target == NULL)
		return false;

	// We found something to shoot at, lets go get it.
	self->enemy = target;
	self->solid = SOLID_BBOX;
	self->movetype = PHYSICSTYPE_FLY;
	VectorCopy(bb_min, self->mins);
	VectorCopy(bb_max, self->maxs);
	self->accel = 0.0f;
	self->svflags = SVF_ALWAYS_SEND;
	self->s.effects |= EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS;
	self->targetname = self->enemy->classname;
	self->alert_time = 0.0f;

	self->think = MeteorBarrierHuntThink;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.

	// Did we start up inside someone?
	trace_t	tr;
	gi.trace(self->s.origin, self->mins, self->maxs, self->s.origin, self, MASK_MONSTERSOLID, &tr);

	if (tr.startsolid)
	{
		MeteorBarrierOnBlocked(self, &tr);
		return true;
	}

	gi.sound(self, CHAN_BODY, gi.soundindex("weapons/MeteorBarrierSeek.wav"), 1.0f, ATTN_NORM, 0.0f);
	gi.CreateEffect(&self->s, FX_SPELL_METEORBARRIER_TRAVEL, CEF_BROADCAST | CEF_OWNERS_ORIGIN, NULL, "");

	// Remove the persistent effect from the persistent effects list.
	if (self->PersistantCFX > 0)
	{
		gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_METEOR);
		gi.RemoveEffects(&self->owner->s, FX_SPELL_METEORBARRIER + self->health);
		self->PersistantCFX = 0;
	}

	// Now we've been cast, remove us from the count of meteors the caster owns, and turn off his looping sound if need be.
	self->owner->client->playerinfo.meteor_count &= ~(1 << self->health);

	if (self->owner->client->playerinfo.meteor_count == 0)
		self->owner->s.sound = 0;

	return true;
}

// Make meteors orbit player.
static void MeteorBarrierSearchThink(edict_t* self)
{
	// Only check for a target every so often as this reduces CPU requirements AND it looks much cooler.
	// (using self->owner->enemy as the target would be much quicker...but not 360 degrees).
	if (irand(0, METEOR_SEARCH_CHANCE) == 0 && FindNewTarget(self))
		return;

	self->random += 20.0f; // Lifetime

	if (self->owner->health > 0 && self->random < 5000.0f + (float)self->health * 200.0f)
	{
		VectorCopy(self->owner->s.origin, self->s.origin);

		const float angle = ((level.time * 150.0f) + ((float)self->health * 90.0f)) * ANGLE_TO_RAD;
		self->s.origin[0] += cosf(angle) * 30.0f;
		self->s.origin[1] += sinf(angle) * 30.0f;
		self->s.origin[2] += cosf(angle / (ANGLE_180 / 5.0f)) * 10.0f;

		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
	else
	{
		Kill_Meteor(self); // My lifetime has expired so I die.
	}
}

// Move the meteors out to radius.
static void MeteorBarrierSearchInitThink(edict_t* self)
{
	if (self->owner->health > 0)
	{
		VectorCopy(self->owner->s.origin, self->s.origin);

		const float angle = ((level.time * 150.0f) + ((float)self->health * 90.0f)) * ANGLE_TO_RAD;
		self->s.origin[0] += cosf(angle) * 30.0f * ((float)self->count / 5.0f);
		self->s.origin[1] += sinf(angle) * 30.0f * ((float)self->count / 5.0f);
		self->s.origin[2] += cosf(angle / (ANGLE_180 / 5.0f)) * 10.0f;

		if (self->count++ > 5)
		{
			self->random = (float)self->health * 90.0f;
			self->think = MeteorBarrierSearchThink;
		}

		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
	else
	{
		Kill_Meteor(self); // My caster died so I die too.
	}
}

// Spawn the meteors.
void SpellCastMeteorBarrier(edict_t* caster, const vec3_t start_pos)
{
	assert(caster->client != NULL); //mxd. caster->client NULL checks were done inconsistently in original logic. Replace with assert for now...

	qboolean do_cast_sound = false;
	playerinfo_t* info = &caster->client->playerinfo; //mxd

	for (int i = 0; i < 4; i++)
	{
		// If my caster is a player, make sure they only have one instance of me active.
		if (caster->client->Meteors[i] != NULL)
			continue;

		// Enough mana to do this?
		if (info->pers.inventory.Items[info->def_ammo_index] < info->pers.defence->quantity)
			break;

		// Decrement our mana?
		if (!DEATHMATCH || !(DMFLAGS & DF_INFINITE_MANA))
			info->pers.inventory.Items[info->def_ammo_index] -= info->pers.defence->quantity;

		edict_t* meteor = G_Spawn();

		CreateMeteor(meteor);
		meteor->svflags |= SVF_NOCLIENT;
		VectorCopy(start_pos, meteor->s.origin);
		meteor->reflect_debounce_time = MAX_REFLECT;
		meteor->health = i;
		meteor->think = MeteorBarrierSearchInitThink;
		meteor->count = 0;
		meteor->random = 0.0f; // Lifetime count
		meteor->solid = SOLID_NOT;
		meteor->owner = caster;

		gi.linkentity(meteor);

		meteor->PersistantCFX = gi.CreatePersistantEffect(&caster->s, FX_SPELL_METEORBARRIER + i, CEF_BROADCAST | CEF_OWNERS_ORIGIN | (i << 5), NULL, "");

		// Store on client.
		info->meteor_count |= (1 << i); // Determine how many meteors are still attached to the player.
		caster->client->Meteors[i] = meteor;

		// Play spell cast sound.
		do_cast_sound = true;
	}

	if (do_cast_sound)
	{
		gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/MeteorBarrierCast.wav"), 1.0f, ATTN_NORM, 0.0f);

		caster->s.sound = (byte)gi.soundindex("weapons/MeteorBarrierAmbient.wav");
		caster->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
	}
}
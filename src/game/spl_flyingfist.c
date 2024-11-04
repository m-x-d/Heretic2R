//
// spl_flyingfist.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "vector.h"
#include "random.h"
#include "decals.h"
#include "Utilities.h"
#include "g_playstats.h"

#define FIST_RADIUS				2.0

extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);
static void FlyingFistThink(edict_t *self);
static void FlyingFistTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface);


// ************************************************************************************************
// FlyingFistThink
// ************************************************************************************************

static void FlyingFistFizzleThink(edict_t *self)
{
	// Don't fizzle in deathmatch, or if powered up
	if (!(deathmatch->value) && self->health==0)
	{
		if ( (self->dmg - 2) > FIREBALL_MIN_FIZZLE_DAMAGE )
		{
			self->dmg -= 2;
		}
		self->nextthink = level.time + 0.1;
	}
}



// ****************************************
// Creation functions
// ****************************************
void FlyingFistInitThink(edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;
	self->think = FlyingFistFizzleThink;
	FlyingFistFizzleThink(self);
}

void CreateFlyingFist(edict_t *flyingfist)
{
	flyingfist->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	flyingfist->svflags |= SVF_ALWAYS_SEND;
	flyingfist->movetype = MOVETYPE_FLYMISSILE;
	flyingfist->s.scale = 1.0;

	flyingfist->touch = FlyingFistTouch;
	flyingfist->think = FlyingFistInitThink;
	flyingfist->classname = "Spell_FlyingFist";
	flyingfist->nextthink = level.time + 0.1;
	VectorSet(flyingfist->mins, -FIST_RADIUS, -FIST_RADIUS, -FIST_RADIUS);
	VectorSet(flyingfist->maxs, FIST_RADIUS, FIST_RADIUS, FIST_RADIUS);

	flyingfist->solid = SOLID_BBOX;
	flyingfist->clipmask = MASK_SHOT;
}




edict_t *FlyingFistReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*flyingfist;

	// create a new missile to replace the old one - this is necessary cos physics will do nasty things
	// with the existing one,since we hit something. Hence, we create a new one totally.
	flyingfist = G_Spawn();

	// copy everything across
	VectorCopy(self->s.origin, flyingfist->s.origin);
	CreateFlyingFist(flyingfist);
	VectorCopy(vel, flyingfist->velocity);
	VectorNormalize2(vel, flyingfist->movedir);
	AnglesFromDir(flyingfist->movedir, flyingfist->s.angles);
	flyingfist->owner = other;
	flyingfist->health = self->health;
	flyingfist->enemy = self->owner;
	flyingfist->flags |= (self->flags & FL_NO_KNOCKBACK);
	flyingfist->reflect_debounce_time = self->reflect_debounce_time -1; //so it doesn't infinitely reflect in one frame somehow
	flyingfist->reflected_time=self->reflected_time;
	G_LinkMissile(flyingfist); 

	// create new trails for the new missile
	gi.CreateEffect(&flyingfist->s, FX_WEAPON_FLYINGFIST, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL,
					"t", flyingfist->velocity);

	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(&flyingfist->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", flyingfist->velocity);

	return(flyingfist);
}



// ************************************************************************************************
// FlyingFistTouch
// ************************************************************************************************

static void FlyingFistTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	int			damage;
	vec3_t		planedir;
	qboolean	powerup, wimpy;
	int			flags;

	if(other == self->owner)
	{
		return;
	}
	if(surface && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	if (self->health)
	{	// Powered up meteor
		powerup=true;
		flags = CEF_FLAG7;
	}
	else
	{	// Unpowered fireball
		powerup=false;
		flags = 0;
	}

	if (self->flags & FL_NO_KNOCKBACK)
	{	// Wimpy out-of-ammo weapon.
		wimpy=true;
		flags |= CEF_FLAG8;
	}
	else
	{
		wimpy=false;
	}

	// has the target got reflection turned on ?
	if (self->reflect_debounce_time)
	{
		if(EntReflecting(other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			// scale speed down
			Vec3ScaleAssign(FLYING_FIST_SPEED/2, self->velocity);
			FlyingFistReflect(self, other, self->velocity);

			return;
		}
	}

	AlertMonsters (self, self->owner, 1, false);
	if(other->takedamage)
	{
		if(powerup)		// Check for powered up meteor
		{
			if(deathmatch->value)
				damage = irand(FIREBALL_DAMAGE_MIN_POWER/2, FIREBALL_DAMAGE_MAX_POWER/2);
			else
				damage = irand(FIREBALL_DAMAGE_MIN_POWER, FIREBALL_DAMAGE_MAX_POWER);
			if (wimpy)
			{	// Wimpy shots do half damage.
				damage /= 2;
				T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal,
						damage, damage, DAMAGE_SPELL,MOD_FIREBALL);									// No blast damage, just direct.
			}
			else
			{
				T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal,
						damage>>1, damage, DAMAGE_SPELL|DAMAGE_EXTRA_KNOCKBACK,MOD_FIREBALL);		// Half goes directly to target, blast does rest.
				T_DamageRadius(self, self->owner, self->owner, FIREBALL_RADIUS, 
						FIREBALL_DAMAGE_MAX_POWER, FIREBALL_DAMAGE_MIN_POWER, DAMAGE_SPELL,MOD_FIREBALL);
			}
		}
		else
		{
			if(deathmatch->value)
				damage = irand(FIREBALL_DAMAGE_MIN/2, FIREBALL_DAMAGE_MAX/2);
			else
				damage = irand(FIREBALL_DAMAGE_MIN, FIREBALL_DAMAGE_MAX);
			if (wimpy)	// Wimpy (no mana) shots do half damage
				damage /= 2;
			T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, damage, damage, DAMAGE_SPELL,MOD_FIREBALL);
		}
	}
	else
	{
		VectorMA(self->s.origin, -8.0, self->movedir, self->s.origin);
	}

	// Attempt to apply a scorchmark decal to the thing I hit.
	if(IsDecalApplicable(self, other, self->s.origin, surface, plane, planedir))
	{
		flags |= CEF_FLAG6;
	}

	gi.CreateEffect(NULL, FX_WEAPON_FLYINGFISTEXPLODE, flags, self->s.origin, "d", self->movedir);

	G_SetToFree(self);
}


// ************************************************************************************************
// SpellCastFlyingFist
// ************************************************************************************************


void SpellCastFlyingFist(edict_t *caster, vec3_t startpos, vec3_t aimangles, vec3_t aimdir, float value)
{
	edict_t		*flyingfist;
	trace_t		trace;
	int			flags;
	qboolean	powerup, wimpy;
	vec3_t		forward, endpos;
	playerinfo_t	*playerinfo;

	// Spawn the flying-fist (fireball)
	flyingfist = G_Spawn();

	playerinfo = &caster->client->playerinfo;

	if (playerinfo->pers.inventory.Items[playerinfo->weap_ammo_index] < playerinfo->pers.weapon->quantity)
	{
		wimpy=true;
		flags = CEF_FLAG8;
	}
	else
	{
		wimpy=false;
		flags = 0;
	}

	if (playerinfo->powerup_timer > level.time)
	{	// Powered up flying fist.  Make it a meteor!
		powerup = true;
		flags |= CEF_FLAG7;
		flyingfist->health = 1;
		if (wimpy)	// Play it quiet
			gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/FireballPowerCast.wav"), 0.5, ATTN_NORM, 0);
		else		// Play it loud
			gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/FireballPowerCast.wav"), 1.0, ATTN_NORM, 0);
	}
	else
	{	// Not powered up
		powerup = false;
		if (wimpy)	// Play special wimpy sound
			gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/FireballNoMana.wav"), 1.0, ATTN_NORM, 0);
		else		// Normal fireball sound
			gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/FlyingFistCast.wav"), 1.0, ATTN_NORM, 0);
	}

	CreateFlyingFist(flyingfist);
	flyingfist->reflect_debounce_time = MAX_REFLECT;
	VectorCopy(startpos, flyingfist->s.origin);	

	if (wimpy)
		flyingfist->flags |= FL_NO_KNOCKBACK;	// Just using the no knockback flag to indicate a wussy weapon.

	//Check ahead first to see if it's going to hit anything at this angle
	AngleVectors(aimangles, forward, NULL, NULL);
	VectorMA(flyingfist->s.origin, FLYING_FIST_SPEED, forward, endpos);
	gi.trace(startpos, vec3_origin, vec3_origin, endpos, caster, MASK_MONSTERSOLID,&trace);
	if(trace.ent && ok_to_autotarget(caster, trace.ent))
	{//already going to hit a valid target at this angle- so don't autotarget
		VectorScale(forward, FLYING_FIST_SPEED, flyingfist->velocity);
	}
	else
	{//autotarget current enemy
		GetAimVelocity(caster->enemy, flyingfist->s.origin, FLYING_FIST_SPEED, aimangles, flyingfist->velocity);
	}
	flyingfist->owner = caster;
	flyingfist->enemy = caster->enemy;
	// Remember velocity in case we have to reverse it
	VectorNormalize2(flyingfist->velocity, flyingfist->movedir);

	G_LinkMissile(flyingfist); 

	// Make sure we don`t start in a solid
	gi.trace(caster->s.origin, vec3_origin, vec3_origin, flyingfist->s.origin, caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid || trace.fraction < 1.0)
	{
		VectorCopy(trace.endpos, flyingfist->s.origin);
		FlyingFistTouch(flyingfist, trace.ent, &trace.plane, trace.surface);
		return;
	}
	// Spawn effect after it has been determined it has not started in wall
	// This is so it won`t try to remove it before it exists
	gi.CreateEffect(&flyingfist->s, FX_WEAPON_FLYINGFIST, CEF_OWNERS_ORIGIN | flags, NULL,
					"t", flyingfist->velocity);
}


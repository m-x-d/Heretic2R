//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "vector.h"
#include "angles.h"
#include "matrix.h"
#include "g_volume_effect.h"
#include "Utilities.h"
#include "g_ClassStatics.h"
#include "g_Physics.h"
#include "g_volume_effect.h"
#include "g_playstats.h"


#define FIREWALL_DOT_MIN	0.25

// ****************************************************************************
// FireBlast
// Unpowered
// ****************************************************************************

void FireBlastBlocked(edict_t *self, trace_t *trace);
void FireBlastStartThink(edict_t *self);


// ****************************************************************************
// SpellCastBlast
// ****************************************************************************

edict_t *CreateFireBlast(vec3_t startpos, vec3_t angles, edict_t *owner, int health, float timestamp)
{
	edict_t *wall;
	short	angle_yaw, angle_pitch;

	wall = G_Spawn();

   	VectorSet(wall->mins, -FIREBLAST_PROJ_RADIUS, -FIREBLAST_PROJ_RADIUS, -FIREBLAST_PROJ_RADIUS);
   	VectorSet(wall->maxs, FIREBLAST_PROJ_RADIUS, FIREBLAST_PROJ_RADIUS, FIREBLAST_PROJ_RADIUS);

	VectorCopy(startpos, wall->s.origin);
	VectorCopy(angles, wall->s.angles);
	AngleVectors(angles, wall->movedir, NULL, NULL);
	VectorScale(wall->movedir, FIREBLAST_SPEED, wall->velocity);

	wall->mass = 250;
	wall->elasticity = ELASTICITY_NONE;
	wall->friction = 0;
	wall->gravity = 0;

	wall->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	wall->svflags |= SVF_ALWAYS_SEND|SVF_DO_NO_IMPACT_DMG;
	wall->movetype = PHYSICSTYPE_FLY;
	wall->isBlocked = FireBlastBlocked;

	wall->classname = "Spell_FireBlast";
	wall->solid = SOLID_BBOX;
	wall->clipmask = MASK_DRIP;
	wall->owner = owner;
	wall->think = FireBlastStartThink;
	wall->nextthink = level.time + 0.1;
	wall->dmg_radius = FIREBLAST_RADIUS;
	wall->dmg = FIREBLAST_DAMAGE;

	wall->health = health;					// Can bounce 3 times

	wall->fire_timestamp = timestamp;		// This marks the wall with a more-or-less unique value so the wall 
											//		doesn't damage twice.

	gi.linkentity(wall);

	angle_yaw = (short)(angles[YAW]*(65536.0/360.0));
	angle_pitch = (short)(angles[PITCH]*(65536.0/360.0));
	gi.CreateEffect(&wall->s, FX_WEAPON_FIREBURST, CEF_OWNERS_ORIGIN, NULL, "ss", angle_yaw, angle_pitch);

	return wall;
}


// This called when missile touches anything (world or edict)
void FireBlastBlocked(edict_t *self, trace_t *trace)
{
	edict_t *newwall;
	float	dot, speed, factor;
	vec3_t	surfvect, surfvel, testpos, newang;
	trace_t	newtrace;

	assert(trace);

	// If we haven't damaged what we are hitting yet, damage it now.  Mainly for the Trial Beast.
	if (trace->ent && trace->ent->takedamage && self->fire_timestamp > trace->ent->fire_timestamp)
	{
		// if we have reflection on, then no damage
		if(!EntReflecting(trace->ent, true, true))
		{
			if(trace->ent != self->owner)		// No damage to casting player
			{
				T_Damage(trace->ent, self, self->owner, self->movedir, self->s.origin, vec3_origin, 
						self->dmg, self->dmg, DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_FIREWALL); 
				gi.CreateEffect(&(trace->ent->s), FX_FLAREUP, CEF_OWNERS_ORIGIN, NULL, "");

				gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/FirewallDamage.wav"),1,ATTN_NORM,0);

				trace->ent->fire_timestamp = self->fire_timestamp;		// Mark so the fire doesn't damage an ent twice.
			}
		}
	}

	if (self->health > 0 && !(trace->contents & CONTENTS_WATER) && 
			(trace->plane.normal[2] > FIREWALL_DOT_MIN || trace->plane.normal[2] < -FIREWALL_DOT_MIN))
	dot = DotProduct(self->movedir, trace->plane.normal);
	speed = VectorLength(self->velocity);
	if (dot < 0 && dot > -0.67)	// slide on all but the most extreme angles.
	{
		VectorMA(self->movedir, -dot, trace->plane.normal, surfvel);	// Vel then holds the velocity negated by the impact.
		factor = VectorNormalize2(surfvel, surfvect);					// Yes, there is the tiniest chance this could be a zero vect, 
		if (factor > 0)
		{
			VectorMA(self->s.origin, 16.0, surfvect, testpos);				// test distance
			
			gi.trace(self->s.origin, self->mins, self->maxs, testpos, self, MASK_SHOT, &newtrace);
			if (newtrace.fraction > 0.99)
			{	// If this is successful, then we can make another fireblast moving in the new direction.
				vectoangles(surfvect, newang);
				newwall = CreateFireBlast(self->s.origin, newang, self->owner, self->health-1, level.time);
			}
		}
	}

	// Well, whatever happened, free the current blast.
	VectorSet(self->velocity, 0.0, 0.0, 0.0);

	self->s.effects |= EF_ALTCLIENTFX;		// Indicate to the wall that it's time to die
	G_SetToFree(self);
}


// ****************************************************************************
// FireBlast think
// ****************************************************************************

void FireBlastThink(edict_t *self)
{	// Check in the area and try to damage anything in the immediate area
	edict_t *ent=NULL;
	vec3_t min, max;

	// Set up the checking volume
	VectorSet(min, -self->dmg_radius, -self->dmg_radius, -FIREBLAST_VRADIUS);
	VectorAdd(self->s.origin, min, min);
	VectorSet(max, self->dmg_radius, self->dmg_radius, FIREBLAST_VRADIUS);
	VectorAdd(self->s.origin, max, max);

	// find all the entities in the volume
	while(ent = findinbounds(ent, min, max))
	{					
//		if ((!(ent->svflags & SVF_MONSTER) && !(ent->client && deathmatch->value)) || (ent->svflags & SVF_DEADMONSTER))
		if(!ent->takedamage)
		{	// Anything that takes damage now.
			continue;
		}

		if (self->fire_timestamp <= ent->fire_timestamp)
			continue;

		// if we have reflection on, then no damage
		if(EntReflecting(ent, true, true))
			continue;

		if(ent != self->owner)		// No damage to casting player
		{
			T_Damage(ent, self, self->owner, self->movedir, self->s.origin, vec3_origin, 
					self->dmg, self->dmg, DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_FIREWALL); 
			gi.CreateEffect(&ent->s, FX_FLAREUP, CEF_OWNERS_ORIGIN, NULL, "");

			gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/FirewallDamage.wav"),1,ATTN_NORM,0);

			ent->fire_timestamp = self->fire_timestamp;		// Mark so the fire doesn't damage an ent twice.
		}
	}

	self->nextthink = level.time + 0.1;
	self->dmg_radius += FIREBLAST_DRADIUS*0.1;
	self->dmg -= 3;
	if (self->dmg < FIREBLAST_DAMAGE_MIN)
		self->dmg = FIREBLAST_DAMAGE_MIN;
}


void FireBlastStartThink(edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;			// Allow transmission to client

	FireBlastThink(self);
	self->think = FireBlastThink;
	self->nextthink = level.time + 0.1;
}


void CastFireBlast(edict_t *caster, vec3_t startpos, vec3_t aimangles)
{
	edict_t	*wall;
	vec3_t	newpos, fwd;	
	trace_t trace;

	wall = G_Spawn();

	AngleVectors(aimangles, fwd, NULL, NULL);
	VectorMA(startpos, FIREBLAST_RADIUS*0.5, fwd, newpos);
	wall = CreateFireBlast(newpos, aimangles, caster, 3, level.time);		// Bounce 3 times

	// Check to see if this is a legit spawn.
	gi.trace(caster->s.origin, wall->mins, wall->maxs, wall->s.origin, caster, MASK_SOLID, &trace);
	if (trace.startsolid || trace.fraction < .99)
	{
		if (trace.startsolid)
			VectorCopy(caster->s.origin, wall->s.origin);
		else
			VectorCopy(trace.endpos, wall->s.origin);

		FireBlastBlocked(wall, &trace);
		return;
	}

	FireBlastThink(wall);
}






// ****************************************************************************
// FireWall
// Powered up
// ****************************************************************************

void WallMissileBlocked(edict_t *self, trace_t *trace);
void WallMissileStartThink(edict_t *self);

edict_t *CreateFireWall(vec3_t startpos, vec3_t angles, edict_t *owner, int health, float timestamp, float sidespeed)
{
	edict_t *wall;
	vec3_t	right;
	short	angle_yaw, angle_pitch;
	int flags=0;

	wall = G_Spawn();

   	VectorSet(wall->mins, -FIREWAVE_PROJ_RADIUS, -FIREWAVE_PROJ_RADIUS, -FIREWAVE_PROJ_RADIUS);
   	VectorSet(wall->maxs, FIREWAVE_PROJ_RADIUS, FIREWAVE_PROJ_RADIUS, FIREWAVE_PROJ_RADIUS);

	VectorCopy(startpos, wall->s.origin);
	VectorCopy(angles, wall->s.angles);
	AngleVectors(angles, wall->movedir, right, NULL);

	if (deathmatch->value)				
	{
		flags |= CEF_FLAG8;
		VectorScale(wall->movedir, FIREWAVE_DM_SPEED, wall->velocity);		// Goes faster in deathmatch
	}
	else
	{
		VectorScale(wall->movedir, FIREWAVE_SPEED, wall->velocity);
	}
	VectorMA(wall->velocity, sidespeed, right, wall->velocity);

	if (sidespeed < 0)
		flags |= CEF_FLAG6;
	else if (sidespeed > 0)
		flags |= CEF_FLAG7;

	wall->mass = 250;
	wall->elasticity = ELASTICITY_NONE;
	wall->friction = 0;
	wall->gravity = 0;

	wall->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	wall->svflags |= SVF_ALWAYS_SEND;
	wall->movetype = PHYSICSTYPE_FLY;
	wall->isBlocked = WallMissileBlocked;

	wall->classname = "Spell_FireWall";
	wall->solid = SOLID_BBOX;
	wall->clipmask = MASK_DRIP;
	wall->owner = owner;
	wall->think = WallMissileStartThink;
	wall->nextthink = level.time + 0.1;
	wall->dmg = FIREWAVE_DAMAGE;
	wall->dmg_radius = FIREWAVE_RADIUS;

	wall->health = health;					// Can bounce 3 times

	wall->fire_timestamp = timestamp;		// Mark the wall so it can't damage something twice.

	gi.linkentity(wall);

	angle_yaw = (short)(angles[YAW]*(65536.0/360.0));
	angle_pitch = (short)(angles[PITCH]*(65536.0/360.0));
	gi.CreateEffect(&wall->s, FX_WEAPON_FIREWAVE, CEF_OWNERS_ORIGIN | flags, startpos, "ss", angle_yaw, angle_pitch);

	return wall;
}


// ****************************************************************************
// WallMissile touch
// ****************************************************************************

void WallMissileWormThink(edict_t *self)
{
	T_DamageRadius(self, self->owner, self->owner, 64.0, 
			FIREWAVE_WORM_DAMAGE, FIREWAVE_WORM_DAMAGE, DAMAGE_FIRE, MOD_FIREWALL);

	G_SetToFree(self);
}


#define FIREWORM_LIFETIME		1.0


// This called when missile touches anything (world or edict)
void WallMissileBlocked(edict_t *self, trace_t *trace)
{
	edict_t *newwall;
	float	dot, speed, factor;
	vec3_t	surfvect, surfvel, testpos, newang;
	trace_t	newtrace;
	edict_t *worm;

	assert(trace);

	// If we haven't damaged what we are hitting yet, damage it now.  Mainly for the Trial Beast.
	if (trace->ent && trace->ent->takedamage && self->fire_timestamp > trace->ent->fire_timestamp)
	{
		// if we have reflection on, then no damage
		if(!EntReflecting(trace->ent, true, true))
		{
			if(trace->ent != self->owner)		// No damage to casting player
			{
				T_Damage(trace->ent, self, self->owner, self->movedir, self->s.origin, vec3_origin, 
						self->dmg, self->dmg, DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_FIREWALL); 
				gi.CreateEffect(&(trace->ent->s), FX_FLAREUP, CEF_OWNERS_ORIGIN, NULL, "");

				trace->ent->fire_timestamp = self->fire_timestamp;

				gi.CreateEffect(NULL, FX_WEAPON_FIREWAVEWORM, 0, trace->ent->s.origin, "t", self->movedir);
				
				worm = G_Spawn();
				VectorCopy(trace->ent->s.origin, worm->s.origin);
				worm->think = WallMissileWormThink;
				worm->nextthink = level.time + FIREWORM_LIFETIME;
				worm->solid = SOLID_NOT;
				worm->clipmask = MASK_DRIP;
				worm->owner = self->owner;
				gi.linkentity(worm);

				gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/FirewallDamage.wav"),1,ATTN_NORM,0);
			}
		}
	}

	if (self->health > 0 && !(trace->contents & CONTENTS_WATER) && 
			(trace->plane.normal[2] > FIREWALL_DOT_MIN || trace->plane.normal[2] < -FIREWALL_DOT_MIN))
	dot = DotProduct(self->movedir, trace->plane.normal);
	speed = VectorLength(self->velocity);
	if (dot < 0 && dot > -0.67)	// slide on all but the most extreme angles.
	{
		VectorMA(self->movedir, -dot, trace->plane.normal, surfvel);	// Vel then holds the velocity negated by the impact.
		factor = VectorNormalize2(surfvel, surfvect);					// Yes, there is the tiniest chance this could be a zero vect, 
		if (factor > 0)
		{
			VectorMA(self->s.origin, 16.0, surfvect, testpos);				// test distance
			
			gi.trace(self->s.origin, self->mins, self->maxs, testpos, self, MASK_SOLID, &newtrace);
			if (newtrace.fraction > 0.99)
			{	// If this is successful, then we can make another fireblast moving in the new direction.
				vectoangles(surfvect, newang);
				newwall = CreateFireWall(self->s.origin, newang, self->owner, self->health-1, level.time, 0);
			}
		}
	}

	// Well, whatever happened, free the current blast.
	VectorSet(self->velocity, 0.0, 0.0, 0.0);

	self->s.effects |= EF_ALTCLIENTFX;		// Indicate to the wall that it's time to die
	G_SetToFree(self);
}

// ****************************************************************************
// WallMissile think
// ****************************************************************************

void WallMissileThink(edict_t *self)
{	// Check in the area and try to damage anything in the immediate area
	edict_t *ent=NULL, *worm;
	vec3_t	min, max;

	// Set up the checking volume
	VectorSet(min, -self->dmg_radius, -self->dmg_radius, -FIREWAVE_DOWN);
	VectorAdd(self->s.origin, min, min);
	VectorSet(max, self->dmg_radius, self->dmg_radius, FIREWAVE_UP);
	VectorAdd(self->s.origin, max, max);

	// find all the entities in the volume
	while(ent = findinbounds(ent, min, max))
	{					
//		if ((!(ent->svflags & SVF_MONSTER) && !(ent->client && deathmatch->value)) || (ent->svflags & SVF_DEADMONSTER))
		if(!ent->takedamage)
		{	// Anything that takes damage now.
			continue;
		}

		if (ent->fire_timestamp >= self->fire_timestamp)
			continue;

		// if we have reflection on, then no damage
		if(EntReflecting(ent, true, true))
			continue;

		if(ent != self->owner)		// No damage to casting player
		{
			T_Damage(ent, self, self->owner, self->movedir, self->s.origin, vec3_origin, 
					self->dmg, self->dmg, DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_FIREWALL); 
			gi.CreateEffect(&ent->s, FX_FLAREUP, CEF_OWNERS_ORIGIN, NULL, "");

			ent->fire_timestamp = self->fire_timestamp;

			gi.CreateEffect(NULL, FX_WEAPON_FIREWAVEWORM, 0, ent->s.origin, "t", self->movedir);
			
			worm = G_Spawn();
			VectorCopy(ent->s.origin, worm->s.origin);
			worm->think = WallMissileWormThink;
			worm->nextthink = level.time + FIREWORM_LIFETIME;
			worm->solid = SOLID_NOT;
			worm->clipmask = MASK_DRIP;
			worm->owner = self->owner;
			gi.linkentity(worm);

			gi.sound(self,CHAN_WEAPON,gi.soundindex("weapons/FirewallDamage.wav"),1,ATTN_NORM,0);
		}
	}

	self->nextthink = level.time + 0.1;
	self->dmg_radius += .1*FIREWAVE_DRADIUS;
	self->dmg -= 3;
	if (self->dmg < FIREWAVE_DAMAGE_MIN)
		self->dmg = FIREWAVE_DAMAGE_MIN;
}


void WallMissileStartThink(edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;			// Allow transmission to client

	WallMissileThink(self);
	self->think = WallMissileThink;
	self->nextthink = level.time + 0.1;
}


void CastFireWall(edict_t *caster, vec3_t startpos, vec3_t aimangles)
{	// Big wall is powered up
	edict_t	*wall;
	vec3_t	fwd, right, spawnpos;	
	trace_t	trace;

	AngleVectors(aimangles, fwd, right, NULL);

	// Spawn wall to left
	VectorMA(startpos, -FIREWAVE_RADIUS, right, spawnpos);
	wall = CreateFireWall(spawnpos, aimangles, caster, 3, level.time, -FIREWAVE_DRADIUS);

	// Check to see if this is a legit spawn.
	gi.trace(caster->s.origin, wall->mins, wall->maxs, wall->s.origin, caster, MASK_SOLID, &trace);
	if (trace.startsolid || trace.fraction < .99)
	{
		if (trace.startsolid)
			VectorCopy(caster->s.origin, wall->s.origin);
		else
			VectorCopy(trace.endpos, wall->s.origin);

		WallMissileBlocked(wall, &trace);
		goto rightwall;
	}

	WallMissileThink(wall);

rightwall:
	// Spawn wall to right
	VectorMA(startpos, FIREWAVE_RADIUS, right, spawnpos);
	wall = CreateFireWall(spawnpos, aimangles, caster, 3, level.time, FIREWAVE_DRADIUS);

	// Check to see if this is a legit spawn.
	gi.trace(caster->s.origin, wall->mins, wall->maxs, wall->s.origin, caster, MASK_SOLID, &trace);
	if (trace.startsolid || trace.fraction < .99)
	{
		if (trace.startsolid)
			VectorCopy(caster->s.origin, wall->s.origin);
		else
			VectorCopy(trace.endpos, wall->s.origin);

		WallMissileBlocked(wall, &trace);
		return;
	}

	WallMissileThink(wall);
}




// ****************************************************************************
// The Firewall spell is cast.
// ****************************************************************************


void SpellCastWall(edict_t *caster, vec3_t startpos, vec3_t aimangles, vec3_t unused, float value)
{
	vec3_t	castpos;

	VectorCopy(startpos, castpos);
	
	if (caster->client->playerinfo.powerup_timer <= level.time)
	{	// Not powered up.
		castpos[2] += 16;	// Aim higher than powered up version.
		CastFireBlast(caster, castpos, aimangles);
		gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/FirewallCast.wav"), 1, ATTN_NORM, 0);
	}
	else
	{	// Powered up, cast big wall o' doom.
		CastFireWall(caster, castpos, aimangles);
		gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/FirewallPowerCast.wav"), 1, ATTN_NORM, 0);
	}
}

// end

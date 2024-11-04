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
#include "g_playstats.h"
#include "decals.h"

#define ARROW_RADIUS			4.0F
extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);

void create_phoenix(edict_t *phoenix);


// ****************************************************************************
// PhoenixMissile Reflect
// ****************************************************************************

edict_t *PhoenixMissileReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t *phoenix;

	// create a new missile to replace the old one - this is necessary cos physics will do nasty shit
	// with the existing one,since we hit something. Hence, we create a new one totally.
	phoenix = G_Spawn();
	VectorCopy(self->s.origin, phoenix->s.origin);
	create_phoenix(phoenix);
	phoenix->owner = other;
	phoenix->enemy = self->owner;
	phoenix->health = self->health;
	phoenix->reflect_debounce_time = self->reflect_debounce_time - 1;
	phoenix->reflected_time=self->reflected_time;

	VectorCopy(vel, phoenix->velocity);
	G_LinkMissile(phoenix);
	gi.CreateEffect(&phoenix->s, FX_WEAPON_PHOENIXMISSILE, CEF_OWNERS_ORIGIN | (phoenix->health << 5) | CEF_FLAG8, NULL, "t", phoenix->velocity);
	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// travel sound on the weapon itself - the one on the original arrow will be deleted when the object is removed.
	phoenix->s.sound = gi.soundindex("weapons/PhoenixTravel.wav");
	phoenix->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
	// but just to be safe..
	self->s.sound = 0;

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(&phoenix->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", phoenix->velocity);

	return(phoenix);
}


// ****************************************************************************
// PhoenixMissile touch
// ****************************************************************************
void PhoenixMissileTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	int			makeScorch;
	vec3_t		planedir;
			    
	// did we hit the sky ? 
	if(surface && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// are we reflecting ?
	if (self->reflect_debounce_time)
	{
		if(EntReflecting(other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			Vec3ScaleAssign(PHOENIX_ARROW_SPEED/2,self->velocity);
			PhoenixMissileReflect(self, other, self->velocity);
			return;
		}
	}

	// kill the travel sound
	gi.sound(self, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1, ATTN_NORM, 0);

	// I'm gonna set the enemy to what I hit...  The DAMAGE_ENEMY_MAX assures that what I hit takes full damage.
	// I do this rather than do direct damage AND radius because that tends to do double damage to what is hit...
	if (other && other->takedamage)
		self->enemy = other;
	else
		self->enemy = NULL;
	if(self->health == 1)
	{	// must be powered up version - storing in health is not so good, though
		// Powered up Phoenix will NOT damage the shooter.
		T_DamageRadius(self, self->owner, NULL, PHOENIX_EXPLODE_RADIUS_POWER, 
						PHOENIX_EXPLODE_DAMAGE_POWER, PHOENIX_EXPLODE_DAMAGE_POWER>>4, 
						DAMAGE_NORMAL|DAMAGE_FIRE|DAMAGE_EXTRA_KNOCKBACK|DAMAGE_POWERPHOENIX|DAMAGE_ENEMY_MAX|DAMAGE_PHOENIX,
						MOD_P_PHOENIX_SPL);
	}
	else
	{
		T_DamageRadius(self, self->owner, NULL, PHOENIX_EXPLODE_RADIUS, 
						PHOENIX_EXPLODE_DAMAGE, PHOENIX_EXPLODE_DAMAGE>>4, 
						DAMAGE_NORMAL|DAMAGE_FIRE|DAMAGE_EXTRA_KNOCKBACK|DAMAGE_ENEMY_MAX|DAMAGE_PHOENIX,
						MOD_PHOENIX_SPL);
	}
	
	AlertMonsters (self, self->owner, 3, false);

	// Attempt to apply a scorchmark decal to the thing I hit.
	makeScorch = 0;
	if(IsDecalApplicable(self, other, self->s.origin, surface, plane, planedir))
	{
		makeScorch = CEF_FLAG8;
	}

	VectorNormalize2(self->velocity,self->movedir);
	// Start the explosion
	if (plane->normal)
		gi.CreateEffect(&self->s, FX_WEAPON_PHOENIXEXPLODE, CEF_BROADCAST |(self->health << 5)| makeScorch, self->s.origin, "td", plane->normal, self->movedir);
	else
		gi.CreateEffect(&self->s, FX_WEAPON_PHOENIXEXPLODE, CEF_BROADCAST |(self->health << 5)| makeScorch, self->s.origin, "td", self->movedir, self->movedir);

	VectorClear(self->velocity);
   // Turn off the client effect
	gi.RemoveEffects(&self->s, FX_WEAPON_PHOENIXMISSILE);

	G_SetToFree(self);
}

// ****************************************************************************
// PhoenixMissile think
// ****************************************************************************

void PhoenixMissileThink(edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
}

// create the guts of a phoenix arrow
void create_phoenix(edict_t *phoenix)
{
	phoenix->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	phoenix->svflags |= SVF_ALWAYS_SEND;
	phoenix->movetype = MOVETYPE_FLYMISSILE;

	phoenix->touch = PhoenixMissileTouch;
	phoenix->think = PhoenixMissileThink;
	phoenix->classname = "Spell_PhoenixArrow";
	phoenix->nextthink = level.time + 0.1;
	VectorSet(phoenix->mins, -ARROW_RADIUS, -ARROW_RADIUS, -ARROW_RADIUS);
	VectorSet(phoenix->maxs, ARROW_RADIUS, ARROW_RADIUS, ARROW_RADIUS);

	phoenix->solid = SOLID_BBOX;
	phoenix->clipmask = MASK_SHOT;

}


// ****************************************************************************
// SpellCastPhoenix
// ****************************************************************************

void SpellCastPhoenix(edict_t *Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t unused, float value)
{
	edict_t		*phoenix;
	trace_t		trace;
	vec3_t		forward, endpos;


	phoenix = G_Spawn();

	VectorCopy(StartPos, phoenix->s.origin);
	//Check ahead first to see if it's going to hit anything at this angle
	AngleVectors(AimAngles, forward, NULL, NULL);
	VectorMA(StartPos, PHOENIX_ARROW_SPEED, forward, endpos);
	gi.trace(StartPos, vec3_origin, vec3_origin, endpos, Caster, MASK_MONSTERSOLID,&trace);
	if(trace.ent && ok_to_autotarget(Caster, trace.ent))
	{//already going to hit a valid target at this angle- so don't autotarget
		VectorScale(forward, PHOENIX_ARROW_SPEED, phoenix->velocity);
	}
	else
	{//autotarget current enemy
		GetAimVelocity(Caster->enemy, phoenix->s.origin, PHOENIX_ARROW_SPEED, AimAngles, phoenix->velocity);
	}
	VectorCopy(AimAngles, phoenix->s.angles);
	create_phoenix(phoenix);
 	phoenix->reflect_debounce_time = MAX_REFLECT;

	phoenix->owner = Caster;
	G_LinkMissile(phoenix);

	// travel sound on the weapon itself
	phoenix->s.sound = gi.soundindex("weapons/PhoenixTravel.wav");
	phoenix->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;

	if (Caster->client->playerinfo.powerup_timer > level.time)
	{
		// We still want the sound on the player.
		gi.sound(Caster, CHAN_WEAPON, gi.soundindex("weapons/PhoenixPowerFire.wav"), 1, ATTN_NORM, 0);
 		phoenix->health = 1;
	}
	else
	{
		// We still want the sound on the player.
		gi.sound(Caster, CHAN_WEAPON, gi.soundindex("weapons/PhoenixFire.wav"), 1, ATTN_NORM, 0);
 		phoenix->health = 0;
	}

	// remove the bow ready sound
	Caster->s.sound = 0;

	// Trace from the player's origin because then if we hit a wall, the effect won't be inside it...
	gi.trace(Caster->s.origin, phoenix->mins, phoenix->maxs, phoenix->s.origin, Caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid || trace.fraction < .99)
	{
		if (trace.startsolid)
			VectorCopy(Caster->s.origin, phoenix->s.origin);
		else
			VectorCopy(trace.endpos, phoenix->s.origin);
		PhoenixMissileTouch(phoenix, trace.ent, &trace.plane, trace.surface);
		return;
	}
	gi.CreateEffect(&phoenix->s, FX_WEAPON_PHOENIXMISSILE, CEF_OWNERS_ORIGIN|(phoenix->health<<5), NULL, "t", phoenix->velocity);
}

// end

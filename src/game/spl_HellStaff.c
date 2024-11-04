//
// spl_hellstaff.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "fx.h"
#include "Angles.h"
#include "Utilities.h"
#include "vector.h"
#include "random.h"
#include "decals.h"
#include "p_main.h"
#include "g_playstats.h"
#include "m_beast.h"

extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);
static void HellboltThink(edict_t *Self);
static void HellboltTouch(edict_t *Self,edict_t *Other,cplane_t *Plane,csurface_t *Surface);

// Radius of zero seems to prevent collision between bolts

#define HELLBOLT_RADIUS			0.0

// guts of creating a hell bolt
void create_hellbolt(edict_t *hellbolt)
{

	hellbolt->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	hellbolt->svflags |= SVF_ALWAYS_SEND;
	hellbolt->movetype = MOVETYPE_FLYMISSILE;

	VectorSet(hellbolt->mins, -HELLBOLT_RADIUS, -HELLBOLT_RADIUS, -HELLBOLT_RADIUS);
	VectorSet(hellbolt->maxs, HELLBOLT_RADIUS, HELLBOLT_RADIUS, HELLBOLT_RADIUS);

	hellbolt->solid = SOLID_BBOX;
	hellbolt->clipmask = MASK_SHOT;
	hellbolt->touch = HellboltTouch;
	hellbolt->think = HellboltThink;
	hellbolt->dmg = irand(HELLBOLT_DAMAGE_MIN, HELLBOLT_DAMAGE_MAX);
	hellbolt->classname = "Spell_Hellbolt";
	hellbolt->nextthink = level.time + 0.1;
}

edict_t *HellboltReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t	*hellbolt;

	hellbolt = G_Spawn();

   	VectorCopy(self->s.origin, hellbolt->s.origin);
   	create_hellbolt(hellbolt);
	VectorCopy(vel, hellbolt->velocity);
   	hellbolt->owner = other;
   	hellbolt->reflect_debounce_time = self->reflect_debounce_time -1;
	hellbolt->reflected_time=self->reflected_time;
	VectorNormalize2(vel, hellbolt->movedir);
	vectoangles(hellbolt->movedir, hellbolt->s.angles);
   	G_LinkMissile(hellbolt); 
   	gi.CreateEffect(&hellbolt->s, FX_WEAPON_HELLBOLT, CEF_OWNERS_ORIGIN|CEF_FLAG6, NULL, "t", hellbolt->velocity);

   	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
   	G_SetToFree(self);

   	// Do a nasty looking blast at the impact point
   	// only do this rarely, since we hit alot.
   	if (!(irand(0,10)))
   		gi.CreateEffect(&hellbolt->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", hellbolt->velocity);

	return(hellbolt);
}

// ****************************************************************************
// HellboltTouch
// ****************************************************************************

static void HellboltTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	byte	makeScorch = 0;

	// did we hit the sky ? 
	if(surface && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// did we hit someone where reflection is functional ?
	if (self->reflect_debounce_time)
	{
		if(EntReflecting(other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			Vec3ScaleAssign(HELLBOLT_SPEED/2,self->velocity);
			HellboltReflect(self, other, self->velocity);

			return;
		}
	}

   	VectorNormalize2(self->velocity, self->movedir);

	if(other->takedamage)
	{
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, self->dmg*2, 
					DAMAGE_SPELL, MOD_HELLSTAFF);
	}
	else
	{
		// Back off the origin for the damage a bit. We are a point and this will
		// help fix hitting base of a stair and not hurting a guy on next step up.
		AlertMonsters (self, self->owner, 1, false);
		VectorMA(self->s.origin, -8.0, self->movedir, self->s.origin);
	}

	makeScorch = 0;
	if(IsDecalApplicable(self, other, self->s.origin, surface, plane, NULL))
	{
		makeScorch = CEF_FLAG6;
	}
	gi.CreateEffect(&self->s, FX_WEAPON_HELLBOLTEXPLODE, CEF_OWNERS_ORIGIN | makeScorch, NULL, "d", self->movedir);
	G_SetToFree(self);
}

// ****************************************************************************
// HellboltThink
// ****************************************************************************

static void HellboltThink(edict_t *Self)
{
	// Prevent any further transmission of this entity to clients.
	Self->svflags |= SVF_NOCLIENT;
	Self->think = NULL;
}




// ****************************************************************************
// SpellCastHellbolt
// ****************************************************************************

#define HELLSTAFF_LASER_MAX_TARGETS	8
void SpellCastHellstaff(edict_t *caster, vec3_t loc, vec3_t aimangles, vec3_t unused)
{
	edict_t	*hellbolt, *tracebuddy;
	trace_t trace;
	vec3_t	endpos, startpos, forward, vect;
	int		numHit = 0;	// can hit no more than 8 guys...
	vec3_t	min={-4, -4, -4};
	vec3_t	max={ 4,  4,  4};
	byte	blen;
	float	laser_dist;
	vec3_t		planedir;

	assert(caster->client);
	if (caster->client->playerinfo.powerup_timer > level.time)
	{	// Powered up version of this weapon-- a laser.
		// We must trace from the player's centerpoint to the casting location to assure we don't hit anything.before the laser starts
		tracebuddy = caster;
		VectorCopy(loc, startpos);
		gi.trace(caster->s.origin, min, max, startpos, caster, MASK_SHOT,&trace);
		if(level.fighting_beast)
		{
			edict_t *ent;
			
			if(ent = check_hit_beast(caster->s.origin, trace.endpos))
				trace.ent = ent;
		}
		if ((trace.fraction > .99) || !(trace.contents & MASK_SOLID))
		{	// It's okay to continue with the shot.  If not, we should skip right to the impact.
			// Now then, if we hit something on the way to the start location, damage him NOW!
			if (trace.ent && trace.ent->takedamage)
			{
				// did we hit something that reflects ?
  				if(EntReflecting(trace.ent, true, true))
  				{
  					// reflect it off into space - powerless now, so it won't hurt anyone it hits

  					// draw line to this point
  					VectorSubtract(trace.endpos, startpos, vect);
  					blen = (byte)(VectorLength(vect)/8.0);

  					gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER, 0, startpos, "tb", 
  						forward, blen);
  					// re-constitute aimangle
  					aimangles[1] += flrand(160,200);
  					aimangles[0] += flrand(-20,20);
  				}
  				else
  				{
  					T_Damage(trace.ent, caster, caster, forward, trace.endpos, forward,
  							irand(HELLLASER_DAMAGE_MIN, HELLLASER_DAMAGE_MAX), 0, DAMAGE_SPELL,MOD_P_HELLSTAFF);
  					gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER_BURN, CEF_FLAG6, trace.endpos, "t", 
  							forward);
  				}
				tracebuddy = trace.ent;
			}	// Don't trace again since there really should only be one thing between the player and startpos
			
			// Set up for main laser damaging loop.
			laser_dist = HELLLASER_DIST;

			do
			{
				AngleVectors(aimangles, forward, NULL, NULL);
				VectorMA(startpos, laser_dist, forward, endpos);
				gi.trace(startpos, min, max, endpos, tracebuddy, MASK_SHOT,&trace);
				if(level.fighting_beast)
				{
					edict_t *ent;
					
					if(ent = check_hit_beast(caster->s.origin, trace.endpos))
						trace.ent = ent;
				}

				if(trace.fraction < .99 )
				{
					// if we hit anything that won't take damage, kill the beam
					if (!trace.ent->takedamage)
						break;

					// This is possible if the tracebuddy is not the caster because a new one was on the way to startpos.
					if (trace.ent != caster)
					{
						// did we hit something that reflects ?
  						if(EntReflecting(trace.ent, true, true))
  						{
  							// reflect it off into space - powerless now, so it won't hurt anyone it hits

  							// draw line to this point
  							VectorSubtract(trace.endpos, startpos, vect);
  							blen = (byte)(VectorLength(vect)/8.0);

  							gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER, 0, startpos, "tb", 
  								forward, blen);
  							// re-constitute aimangle
  							aimangles[1] += flrand(160,200);
  							aimangles[0] += flrand(-20,20);
  						}
  						else
  						{
  							T_Damage(trace.ent, caster, caster, forward, trace.endpos, forward,
  									irand(HELLLASER_DAMAGE_MIN, HELLLASER_DAMAGE_MAX), 0, DAMAGE_SPELL,MOD_P_HELLSTAFF);
  							gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER_BURN, CEF_FLAG6, trace.endpos, "t", 
  									forward);
  						}
					}
					// this seems to alleviate the problem of a trace hitting the same ent multiple times...
					VectorSubtract(trace.endpos, startpos, vect);
					laser_dist -= VectorLength(vect);

					VectorCopy(trace.endpos, startpos);
					VectorSubtract(endpos, startpos, vect);
					if(VectorLength(vect) > 16.0)
					{
						VectorMA(startpos, 16.0, forward, startpos);
					}
					tracebuddy = trace.ent;
					numHit++;
				}

			} while((trace.fraction < .99) && !(trace.contents & MASK_SOLID) && (numHit < HELLSTAFF_LASER_MAX_TARGETS) );
		}

		// We altenate sound slots to make the quick-firing work well with the sounds.
		if (caster->client->playerinfo.flags & PLAYER_FLAG_ALTFIRE)
		{	// Use the alternate slot, clear the flag.
			gi.sound(caster, CHAN_WEAPON2, gi.soundindex("weapons/HellLaserFire.wav"), 1, ATTN_NORM, 0);
			caster->client->playerinfo.flags &= ~PLAYER_FLAG_ALTFIRE;
		}
		else
		{	// Use the regular slot, set the flag.
			gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/HellLaserFire.wav"), 1, ATTN_NORM, 0);
			caster->client->playerinfo.flags |= PLAYER_FLAG_ALTFIRE;
		}

		VectorSubtract(trace.endpos, startpos, vect);
		blen = (byte)(VectorLength(vect)/8.0);

		// decide if we need a scorch mark or not
		if(IsDecalApplicable(caster, trace.ent, caster->s.origin, trace.surface, &trace.plane, planedir))
			gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER, CEF_FLAG6|CEF_FLAG7, startpos, "tb", 
					forward, blen);
		else
			gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER, CEF_FLAG6, startpos, "tb", 
					forward, blen);
	}	
	else
	{	// Unpowered version of this weapon-- hellbolts.
		// Spawn the hellbolt
		hellbolt = G_Spawn();

		VectorCopy(loc, hellbolt->s.origin);

		//Check ahead first to see if it's going to hit anything at this angle
		AngleVectors(aimangles, forward, NULL, NULL);
		if(caster->client->playerinfo.flags & PLAYER_FLAG_NO_LARM)
		{
			VectorScale(forward, HELLBOLT_SPEED, hellbolt->velocity);
		}
		else
		{
			VectorMA(loc, HELLBOLT_SPEED, forward, endpos);
			gi.trace(loc, vec3_origin, vec3_origin, endpos, caster, MASK_MONSTERSOLID,&trace);
			if(trace.ent && ok_to_autotarget(caster, trace.ent))
			{//already going to hit a valid target at this angle- so don't autotarget
				VectorScale(forward, HELLBOLT_SPEED, hellbolt->velocity);
			}
			else
			{//autotarget current enemy
				GetAimVelocity(caster->enemy, hellbolt->s.origin, HELLBOLT_SPEED, aimangles, hellbolt->velocity);
			}
		}

		hellbolt->owner = caster;
		VectorNormalize2(hellbolt->velocity, hellbolt->movedir);
		create_hellbolt(hellbolt);
		hellbolt->reflect_debounce_time = MAX_REFLECT;
		G_LinkMissile(hellbolt); 

		// This alternation avoids cutting sounds out prematurely.
		if (caster->client->playerinfo.flags & PLAYER_FLAG_ALTFIRE)
		{	// Use the alternate slot, clear the flag.
			gi.sound(caster, CHAN_WEAPON2, gi.soundindex("weapons/HellFire.wav"), 1, ATTN_NORM, 0);
			caster->client->playerinfo.flags &= ~PLAYER_FLAG_ALTFIRE;
		}
		else
		{	// Use the regular slot, set the flag.
			gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/HellFire.wav"), 1, ATTN_NORM, 0);
			caster->client->playerinfo.flags |= PLAYER_FLAG_ALTFIRE;
		}

		gi.trace(hellbolt->s.origin, vec3_origin, vec3_origin, hellbolt->s.origin, caster, MASK_PLAYERSOLID,&trace);
		if (trace.startsolid)
		{
			HellboltTouch(hellbolt, trace.ent, &trace.plane, trace.surface);
			return;
		}
		gi.CreateEffect(&hellbolt->s, FX_WEAPON_HELLBOLT, CEF_OWNERS_ORIGIN, NULL, "t", hellbolt->velocity);
	}
}
// end
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
#include "random.h"

#define ARROW_RADIUS			2.0F
#define ARROW_BACKUP			(45.0F - ARROW_RADIUS)

extern void AlertMonsters (edict_t *self, edict_t *enemy, float lifetime, qboolean ignore_shadows);

void create_redarrow(edict_t *redarrow);

void RedRainRemove(edict_t *self)
{
	gi.RemoveEffects(&self->s, 0);
	G_SetToFree(self);
}

void RedRainThink(edict_t *self)
{
	edict_t *victim=NULL;
	vec3_t startpos, endpos, diffpos, min, max, hitpos, vec;
	trace_t trace;
	qboolean poweredup;
	float	lradius, rradius, rad_dmg;
	int		damage;

	if(deathmatch->value)
		rad_dmg = self->dmg * 0.25;
	else
		rad_dmg = self->dmg;

	// find all the entities in the volume
	while(victim = findinblocking(victim, self))
	{					
		if(victim != self->owner && victim->takedamage && 
				(victim->client || victim->svflags & SVF_MONSTER) && !(victim->svflags & SVF_DEADMONSTER))		
		{	// No damage to casting player
			VectorSubtract(self->pos1, victim->s.origin, vec);
			VectorNormalize(vec);
			VectorMA(victim->s.origin, victim->maxs[0], vec, hitpos);
			
			if (victim->svflags & SVF_BOSS)
			{
				T_Damage(victim, self, self->owner, vec3_origin, hitpos, vec3_origin, 
						rad_dmg/2, 0, DAMAGE_SPELL,MOD_STORM); 
			}
			else
			{
				T_Damage(victim, self, self->owner, vec3_origin, hitpos, vec3_origin, 
						rad_dmg, 0, DAMAGE_SPELL,MOD_STORM); 
			}
		}
	}

	if (self->delay <= level.time || (self->owner->red_rain_count - self->red_rain_count) > NUM_STORMS_PER_PLAYER)
	{
		self->owner->red_rain_count--;
		self->s.effects |= EF_DISABLE_EXTRA_FX;
		self->nextthink = level.time + 1.0;//lasts another 1.0 secs
		self->think = RedRainRemove;
	}
	else
	{
		// Powerup value comes from the health in the edict.
		poweredup = self->health;

		// Check for lightning
		if (self->delay - level.time < RED_RAIN_LIGHTNING_DURATION && irand(0, RED_RAIN_LIGHTNING_CHANCE) == 0)
		{	// First check the area for a potential victim.
			if (!poweredup)
			{
				rradius = RED_RAIN_RADIUS;
				lradius = RED_RAIN_LIGHTNING_RADIUS;
			}
			else
			{
				rradius = POWER_RAIN_RADIUS;
				lradius = POWER_RAIN_LIGHTNING_RADIUS;
			}

			// Find the bounds to search under.
			VectorSet(min, -lradius, -lradius, self->mins[2]);
			// Only search the lower half of the area for lightning hits.
			VectorSet(max, lradius, lradius, self->mins[2] + ((self->maxs[2] - self->mins[2])*0.5));
			VectorAdd(self->s.origin, min, min);
			VectorAdd(self->s.origin, max, max);
			victim=NULL;
			while (victim = findinbounds(victim, min, max))
			{
				if (victim != self->owner && victim->takedamage &&
						(victim->client || victim->svflags & SVF_MONSTER) && !(victim->svflags & SVF_DEADMONSTER))
					break;
			}
			if (victim)
			{	// Try to zap somebody with lightning
				VectorSet(startpos, 
							flrand(-rradius*0.6, rradius*0.6), 
							flrand(-rradius*0.6, rradius*0.6), 
							self->maxs[2]);
				VectorAdd(startpos, self->s.origin, startpos);
				VectorSet(endpos,
							flrand(victim->mins[0]*0.5, victim->maxs[0]*0.5),
							flrand(victim->mins[1]*0.5, victim->maxs[1]*0.5),
							flrand(victim->mins[2]*0.5, victim->maxs[2]*0.5));
				VectorAdd(endpos, victim->s.origin, endpos);

				gi.trace(startpos, vec3_origin, vec3_origin, endpos, self->owner, MASK_SOLID,&trace);
				if (!trace.startsolid && trace.fraction == 1.0)
				{	// FINALLY!  A clear lightning strike!
					VectorSubtract(endpos, startpos, diffpos);
					VectorNormalize(diffpos);
					if (!poweredup)
					{
						gi.CreateEffect(NULL, FX_LIGHTNING, CEF_FLAG6, 
								startpos, "vbb", endpos, (byte)RED_RAIN_LIGHTNING_WIDTH, (byte)0);
						gi.sound(victim,CHAN_WEAPON,gi.soundindex("weapons/Lightning.wav"),1,ATTN_NORM,0);

						// Do a nasty looking blast at the impact point
						gi.CreateEffect(&victim->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN | CEF_FLAG7, NULL, "t", diffpos);

						if(!(EntReflecting(victim, true, true)))
						{
							T_Damage(victim, self, self->owner, diffpos, endpos, vec3_origin, 
									irand(RED_RAIN_DMG_LIGHTNING_MIN, RED_RAIN_DMG_LIGHTNING_MAX), 0, DAMAGE_SPELL,MOD_STORM); 
						}
					}
					else
					{
						gi.CreateEffect(NULL, FX_POWER_LIGHTNING, 0, 
								startpos, "vb", endpos, (byte)POWER_RAIN_LIGHTNING_WIDTH);
						gi.sound(victim,CHAN_WEAPON,gi.soundindex("weapons/LightningPower.wav"),1,ATTN_NORM,0);

						if(!(EntReflecting(victim, true, true)))
						{
							damage = irand(POWER_RAIN_DMG_LIGHTNING_MIN, POWER_RAIN_DMG_LIGHTNING_MAX); 
							
							T_DamageRadiusFromLoc(endpos, self, self->owner, self->owner, POWER_RAIN_DMG_LIGHTNING_RADIUS, 
									damage, damage*0.25, DAMAGE_SPELL,MOD_P_STORM);
						}
					}
				}
			}
			else
			{
				VectorSet(startpos, 
							flrand(-rradius*0.75, rradius*0.75), 
							flrand(-rradius*0.75, rradius*0.75), 
							self->maxs[2]);
				VectorAdd(startpos, self->s.origin, startpos);
				VectorSet(endpos, 
							flrand(-rradius, rradius), 
							flrand(-rradius, rradius), 
							self->mins[2]);
				VectorAdd(endpos, self->s.origin, endpos);
				if (!poweredup)
				{
					gi.CreateEffect(NULL, FX_LIGHTNING, CEF_FLAG6, 
							startpos, "vbb", endpos, (byte)RED_RAIN_LIGHTNING_WIDTH, (byte)0);
					gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/Lightning.wav"), 2, ATTN_NORM,0);
				}
				else
				{
					gi.CreateEffect(NULL, FX_POWER_LIGHTNING, 0, 
							startpos, "vb", endpos, (byte)POWER_RAIN_LIGHTNING_WIDTH);
					gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/LightningPower.wav"), 2, ATTN_NORM,0);

					// The lightning does radius damage even if no target.
					damage = irand(POWER_RAIN_DMG_LIGHTNING_MIN, POWER_RAIN_DMG_LIGHTNING_MAX); 
					T_DamageRadiusFromLoc(endpos, self, self->owner, self->owner, POWER_RAIN_DMG_LIGHTNING_RADIUS, 
							damage, damage*0.25, DAMAGE_SPELL,MOD_P_STORM);
				}
			}
		}
		self->nextthink = level.time + self->wait;
	}
}


// ****************************************************************************
// RedRainMissile reflect
// ****************************************************************************

edict_t *RedRainMissileReflect(edict_t *self, edict_t *other, vec3_t vel)
{
	edict_t *redarrow;

	// create a new missile to replace the old one - this is necessary cos physics will do nasty shit
	// with the existing one,since we hit something. Hence, we create a new one totally.
	redarrow = G_Spawn();
	VectorCopy(self->s.origin, redarrow->s.origin);
	redarrow->health = self->health;
	redarrow->owner = other;
	redarrow->enemy = self->owner;
	redarrow->owner->red_rain_count++;
	self->owner->red_rain_count--;
	create_redarrow(redarrow);

	VectorCopy(vel, redarrow->velocity);
	redarrow->reflect_debounce_time = self->reflect_debounce_time -1;
	redarrow->reflected_time=self->reflected_time;
	G_LinkMissile(redarrow); 
	gi.CreateEffect(&redarrow->s, FX_WEAPON_REDRAINMISSILE, 
			CEF_OWNERS_ORIGIN|(redarrow->health<<5)|CEF_FLAG8, NULL, "t", redarrow->velocity);

	// kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point
	gi.CreateEffect(&redarrow->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", redarrow->velocity);

	return(redarrow);
}


// ****************************************************************************
// RedRainMissile touch
// ****************************************************************************

void RedRainMissileTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surface)
{
	vec3_t				org, end;
	edict_t				*damagearea;
	trace_t				trace;
	float				radius;

	// has the target got reflection turned on ?
	if (self->reflect_debounce_time)
	{
		if(EntReflecting(other, true, true))
		{
			Create_rand_relect_vect(self->velocity, self->velocity);
			Vec3ScaleAssign(RED_ARROW_SPEED/2, self->velocity);
			RedRainMissileReflect(self, other, self->velocity);

			return;
		}
	}

	if(other->takedamage)
	{	// Damage from direct impact of arrow, normal or powered up.
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, self->dmg, DAMAGE_SPELL,MOD_STORM);
	}
	AlertMonsters (self, self->owner, 5, false);

	// Backup effect a little so it doesn`t appear in the wall (but only if we hit the wall)
	if(other->svflags & SVF_MONSTER)
	{
		VectorCopy(self->s.origin, org);
	}
	else
	{
		VectorNormalize2(self->velocity, org);
		Vec3ScaleAssign(-ARROW_BACKUP, org);
		Vec3AddAssign(self->s.origin, org);
	}
	VectorClear(self->velocity);

	// Create a damage handling effect
	damagearea = G_Spawn();
	VectorCopy(org, damagearea->s.origin);
	damagearea->think = RedRainThink;
	damagearea->nextthink = level.time + RED_RAIN_DAMAGE_INTERVAL;
	damagearea->solid = SOLID_NOT;
	damagearea->clipmask = CONTENTS_EMPTY;
	damagearea->movetype = MOVETYPE_FLYMISSILE;				// Necessary for proper processing of thinkers
	damagearea->wait = RED_RAIN_DAMAGE_INTERVAL;
	if(deathmatch->value)
		damagearea->delay = level.time + RED_RAIN_DURATION - 2;//5 secs in DM
	else
		damagearea->delay = level.time + RED_RAIN_DURATION;
	damagearea->owner = self->owner;
	damagearea->red_rain_count = self->owner->red_rain_count;
	damagearea->classname = "Spell_RedRain";
	damagearea->health = self->health;						// Copy over the powerup status.
	damagearea->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	if (self->health == 0)
	{	// Not powered up
		damagearea->dmg = RED_RAIN_DAMAGE;
		radius = RED_RAIN_RADIUS;
	}
	else
	{	// Powered up rain
		damagearea->dmg = POWER_RAIN_DAMAGE;
		radius = POWER_RAIN_RADIUS;
	}

	// Find the top of the damage area.  Check down in an area less than the max size.
	VectorSet(damagearea->mins, -radius*0.5, -radius*0.5, -1.0F);
	VectorSet(damagearea->maxs, radius*0.5, radius*0.5, 1.0F);
	VectorCopy(org, end);
	end[2] += MAX_REDRAINHEIGHT;

	gi.trace(org, damagearea->mins, damagearea->maxs, end, damagearea, MASK_SOLID,&trace);
//	if(trace.startsolid)						// Ignore startsolids.
//		damagearea->maxs[2] = 1.0;
//	else
	if (trace.fraction == 1.0F)
		damagearea->maxs[2] = MAX_REDRAINHEIGHT;			// Put the bounds up all the way
	else
		damagearea->maxs[2] = trace.endpos[2] - org[2];		// Set the bounds up only part way

	// Find the bottom of the damage area.
	end[2] = org[2] - MAX_REDRAINFALLDIST;

	gi.trace(org, damagearea->mins, damagearea->maxs, end, damagearea, MASK_SOLID,&trace);
//	if(trace.startsolid)						// Startsolids mean that the area is too close to a wall
//		damagearea->mins[2] = -1.0;
//	else 
	if (trace.fraction == 1.0F)
		damagearea->mins[2] = -MAX_REDRAINFALLDIST;			// Put the bounds down all the way
	else
		damagearea->mins[2] = trace.endpos[2] - org[2];		// Set the bounds down where the trace stopped

	// Put the bounds of the damage area out to the max position now.
	damagearea->mins[0] = damagearea->mins[1] = -radius;
	damagearea->maxs[0] = damagearea->maxs[1] = radius;
	
	VectorSet(damagearea->pos1, damagearea->s.origin[0], damagearea->s.origin[1], damagearea->maxs[2]);

	gi.linkentity(damagearea);

	// Start the red rain
	// Send along the health as a flag, to indicate if powered up.
	gi.CreateEffect(&damagearea->s, FX_WEAPON_REDRAIN, CEF_BROADCAST|(self->health<<5), org, "");
	
//	gi.sound(damagearea, CHAN_VOICE, gi.soundindex("weapons/RedRainFall.wav"), 2, ATTN_NORM,0);
	damagearea->s.sound = gi.soundindex("weapons/RedRainFall.wav");
	damagearea->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;

	// Turn off the client effect
	gi.RemoveEffects(&self->s, FX_WEAPON_REDRAINMISSILE);
	G_SetToFree(self);
}


// ****************************************************************************
// RedRainMissile think
// ****************************************************************************

void RedRainMissileThink(edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
}

// create the guts of the red rain missile
void create_redarrow(edict_t *redarrow)
{
	redarrow->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	redarrow->svflags |= SVF_ALWAYS_SEND;
	redarrow->movetype = MOVETYPE_FLYMISSILE;

	VectorSet(redarrow->mins, -ARROW_RADIUS, -ARROW_RADIUS, -ARROW_RADIUS);
	VectorSet(redarrow->maxs, ARROW_RADIUS, ARROW_RADIUS, ARROW_RADIUS);

	redarrow->solid = SOLID_BBOX;
	redarrow->clipmask = MASK_SHOT;
	redarrow->touch = RedRainMissileTouch;
	redarrow->think = RedRainMissileThink;
	redarrow->classname = "Spell_RedRainArrow";
	redarrow->nextthink = level.time + 0.1;
	if (redarrow->health==1)
	{	// powerup arrow
		redarrow->dmg = irand(POWER_RAIN_DMG_ARROW_MIN, POWER_RAIN_DMG_ARROW_MAX);
	}
	else
	{
		redarrow->dmg = irand(RED_RAIN_DMG_ARROW_MIN, RED_RAIN_DMG_ARROW_MAX);
	}
}

// ****************************************************************************
// SpellCastRedRain
// ****************************************************************************

void SpellCastRedRain(edict_t *Caster, vec3_t StartPos, vec3_t AimAngles, vec3_t unused, float value)
{
	edict_t		*redarrow;
	trace_t		trace;
	vec3_t		dir, forward, endpos;
	qboolean	powerup;

	redarrow = G_Spawn();

	Caster->red_rain_count++;
	// health indicates a level of powerup
	if (Caster->client->playerinfo.powerup_timer > level.time)
	{	// Shoot powered up red rain.
 		redarrow->health = 1;
		powerup=true;
	}
	else
	{	// Normal red rain arrow
		redarrow->health = 0;
		powerup=false;
	}

	VectorCopy(StartPos, redarrow->s.origin);
	//Check ahead first to see if it's going to hit anything at this angle
	AngleVectors(AimAngles, forward, NULL, NULL);
	VectorMA(StartPos, RED_ARROW_SPEED, forward, endpos);
	gi.trace(StartPos, vec3_origin, vec3_origin, endpos, Caster, MASK_MONSTERSOLID,&trace);
	if(trace.ent && ok_to_autotarget(Caster, trace.ent))
	{//already going to hit a valid target at this angle- so don't autotarget
		VectorScale(forward, RED_ARROW_SPEED, redarrow->velocity);
	}
	else
	{//autotarget current enemy
		GetAimVelocity(Caster->enemy, redarrow->s.origin, RED_ARROW_SPEED, AimAngles, redarrow->velocity);
	}
	VectorNormalize2(redarrow->velocity, dir);
	// naughty naughty - this requires a normalised vector
	AnglesFromDir(dir, redarrow->s.angles);

	create_redarrow(redarrow);
	redarrow->reflect_debounce_time = MAX_REFLECT;
	
	redarrow->owner = Caster;
	G_LinkMissile(redarrow);

	gi.RemoveEffects(&Caster->s, FX_WEAPON_REDRAINGLOW);

	if (powerup)
	{	// Play powerup firing sound
		gi.sound(Caster, CHAN_WEAPON, gi.soundindex("weapons/RedRainPowerFire.wav"), 1, ATTN_NORM, 0);
	}
	else
	{	// Player normal red rain firing sound
		gi.sound(Caster, CHAN_WEAPON, gi.soundindex("weapons/RedRainFire.wav"), 1, ATTN_NORM, 0);
	}

	// remove the bow ready sound
	Caster->s.sound = 0;

	// Trace from the player's origin because then if we hit a wall, the effect won't be inside it...
	gi.trace(Caster->s.origin, redarrow->mins, redarrow->maxs, redarrow->s.origin, Caster, MASK_PLAYERSOLID,&trace);
	if (trace.startsolid || trace.fraction < .99)
	{
		if (trace.startsolid)
			VectorCopy(Caster->s.origin, redarrow->s.origin);
		else
			VectorCopy(trace.endpos, redarrow->s.origin);
		RedRainMissileTouch(redarrow, trace.ent, &trace.plane, trace.surface);
		return;
	}

	// Create the missile and trail effect only if we successfully launch the missile
	if (powerup)
	{	// Magenta trail
		gi.CreateEffect(&redarrow->s, FX_WEAPON_REDRAINMISSILE, CEF_OWNERS_ORIGIN|CEF_FLAG6, 
					NULL, "t", redarrow->velocity);
	}
	else
	{	// Red trail
		gi.CreateEffect(&redarrow->s, FX_WEAPON_REDRAINMISSILE, CEF_OWNERS_ORIGIN, 
					NULL, "t", redarrow->velocity);
	}
}

// end

//
// spl_RedRain.c
//
// Copyright 1998 Raven Software
//

#include "spl_RedRain.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define ARROW_RADIUS	2.0f
#define ARROW_BACKUP	(45.0f - ARROW_RADIUS)

void create_redarrow(edict_t *redarrow);

static void RedRainRemove(edict_t* self)
{
	gi.RemoveEffects(&self->s, 0); //mxd. Type 0 means remove all effects.
	G_SetToFree(self);
}

static void RedRainLighning(edict_t* self) //mxd. Split from RedRainThink().
{
	// Powerup value comes from the health in the edict.
	const qboolean is_powered = (self->health > 0);

	// First check the area for a potential victim.
	const float rain_radius = (is_powered ? POWER_RAIN_RADIUS : RED_RAIN_RADIUS); //mxd
	const float lightning_radius = (is_powered ? POWER_RAIN_LIGHTNING_RADIUS : RED_RAIN_LIGHTNING_RADIUS); //mxd

	// Find the bounds to search under.
	vec3_t min;
	VectorSet(min, -lightning_radius, -lightning_radius, self->mins[2]);

	// Only search the lower half of the area for lightning hits.
	vec3_t max;
	VectorSet(max, lightning_radius, lightning_radius, self->mins[2] + ((self->maxs[2] - self->mins[2]) * 0.5f));

	Vec3AddAssign(self->s.origin, min);
	Vec3AddAssign(self->s.origin, max);

	// Find a target to zap.
	edict_t* victim = NULL;
	while ((victim = FindInBounds(victim, min, max)) != NULL)
		if (victim != self->owner && victim->takedamage != DAMAGE_NO && (victim->client != NULL || victim->svflags & SVF_MONSTER) && !(victim->svflags & SVF_DEADMONSTER))
			break;

	if (victim != NULL)
	{
		// Try to zap somebody with lightning.
		vec3_t start_pos;
		for (int i = 0; i < 2; i++)
			start_pos[i] = flrand(-rain_radius * 0.6f, rain_radius * 0.6f);
		start_pos[2] = self->maxs[2];

		Vec3AddAssign(self->s.origin, start_pos);

		vec3_t end_pos;
		for (int i = 0; i < 3; i++)
			end_pos[i] = flrand(victim->mins[i] * 0.5f, victim->maxs[i] * 0.5f);

		Vec3AddAssign(victim->s.origin, end_pos);

		trace_t trace;
		gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, self->owner, MASK_SOLID, &trace);

		if (!trace.startsolid && trace.fraction == 1.0f)
		{
			// FINALLY! A clear lightning strike!
			vec3_t dir;
			VectorSubtract(end_pos, start_pos, dir);
			VectorNormalize(dir);

			if (!is_powered)
			{
				gi.CreateEffect(NULL, FX_LIGHTNING, CEF_FLAG6, start_pos, "vbb", end_pos, (byte)RED_RAIN_LIGHTNING_WIDTH, (byte)0);
				gi.sound(victim, CHAN_WEAPON, gi.soundindex("weapons/Lightning.wav"), 1.0f, ATTN_NORM, 0.0f);

				// Do a nasty looking blast at the impact point
				gi.CreateEffect(&victim->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN | CEF_FLAG7, NULL, "t", dir);

				if (!EntReflecting(victim, true, true))
				{
					const int damage = irand(RED_RAIN_DMG_LIGHTNING_MIN, RED_RAIN_DMG_LIGHTNING_MAX); //mxd
					T_Damage(victim, self, self->owner, dir, end_pos, vec3_origin, damage, 0, DAMAGE_SPELL, MOD_STORM);
				}
			}
			else
			{
				gi.CreateEffect(NULL, FX_POWER_LIGHTNING, 0, start_pos, "vb", end_pos, (byte)POWER_RAIN_LIGHTNING_WIDTH);
				gi.sound(victim, CHAN_WEAPON, gi.soundindex("weapons/LightningPower.wav"), 1.0f, ATTN_NORM, 0.0f);

				if (!EntReflecting(victim, true, true))
				{
					const float damage = flrand(POWER_RAIN_DMG_LIGHTNING_MIN, POWER_RAIN_DMG_LIGHTNING_MAX); //mxd. int / irand() in original logic.
					T_DamageRadiusFromLoc(end_pos, self, self->owner, self->owner, POWER_RAIN_DMG_LIGHTNING_RADIUS,
						damage, damage / 4, DAMAGE_SPELL, MOD_P_STORM);
				}
			}
		}
	}
	else
	{
		vec3_t start_pos;
		for (int i = 0; i < 2; i++)
			start_pos[i] = flrand(-rain_radius * 0.75f, rain_radius * 0.75f);
		start_pos[2] = self->maxs[2];

		Vec3AddAssign(self->s.origin, start_pos);

		vec3_t end_pos;
		for (int i = 0; i < 2; i++)
			end_pos[i] = flrand(-rain_radius, rain_radius);
		end_pos[2] = self->mins[2];

		Vec3AddAssign(self->s.origin, end_pos);

		if (!is_powered)
		{
			gi.CreateEffect(NULL, FX_LIGHTNING, CEF_FLAG6, start_pos, "vbb", end_pos, (byte)RED_RAIN_LIGHTNING_WIDTH, (byte)0);
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/Lightning.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?
		}
		else
		{
			gi.CreateEffect(NULL, FX_POWER_LIGHTNING, 0, start_pos, "vb", end_pos, (byte)POWER_RAIN_LIGHTNING_WIDTH);
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/LightningPower.wav"), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?

			// The lightning does radius damage even if no target.
			const float damage = flrand(POWER_RAIN_DMG_LIGHTNING_MIN, POWER_RAIN_DMG_LIGHTNING_MAX); //mxd. int / irand() in original logic.
			T_DamageRadiusFromLoc(end_pos, self, self->owner, self->owner, POWER_RAIN_DMG_LIGHTNING_RADIUS,
				damage, damage / 4, DAMAGE_SPELL, MOD_P_STORM);
		}
	}
}

static void RedRainThink(edict_t* self)
{
	const int base_damage = (DEATHMATCH ? self->dmg / 4 : self->dmg); //mxd

	// Damage all the entities in the volume.
	edict_t* victim = NULL;
	while ((victim = FindInBlocking(victim, self)) != NULL)
	{
		// No damage to casting player.
		if (victim == self->owner || victim->takedamage == DAMAGE_NO || (victim->client == NULL && !(victim->svflags & SVF_MONSTER)) || (victim->svflags & SVF_DEADMONSTER))
			continue;

		vec3_t vec;
		VectorSubtract(self->pos1, victim->s.origin, vec);
		VectorNormalize(vec);

		vec3_t hit_pos;
		VectorMA(victim->s.origin, victim->maxs[0], vec, hit_pos);

		const int damage = ((victim->svflags & SVF_BOSS) ? base_damage / 2 : base_damage); //mxd
		T_Damage(victim, self, self->owner, vec3_origin, hit_pos, vec3_origin, damage, 0, DAMAGE_SPELL, MOD_STORM);
	}

	if (self->delay <= level.time || self->owner->red_rain_count - self->red_rain_count > NUM_STORMS_PER_PLAYER)
	{
		self->owner->red_rain_count--;
		self->s.effects |= EF_DISABLE_EXTRA_FX;
		self->nextthink = level.time + 1.0f; // Lasts another second.
		self->think = RedRainRemove;
	}
	else
	{
		// Check for lightning.
		if (self->delay - level.time < RED_RAIN_LIGHTNING_DURATION && irand(0, RED_RAIN_LIGHTNING_CHANCE) == 0)
			RedRainLighning(self);

		self->nextthink = level.time + self->wait;
	}
}

static void RedRainMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	// Has the target got reflection turned on?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(RED_ARROW_SPEED / 2, self->velocity);
		RedRainMissileReflect(self, other, self->velocity);

		return;
	}

	// Damage from direct impact of arrow, normal or powered up.
	if (other->takedamage != DAMAGE_NO)
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, self->dmg, DAMAGE_SPELL, MOD_STORM);

	AlertMonsters(self, self->owner, 5.0f, false);

	// Backup effect a little so it doesn't appear in the wall (but only if we hit the wall).
	vec3_t origin;
	if (other->svflags & SVF_MONSTER)
	{
		VectorCopy(self->s.origin, origin);
	}
	else
	{
		VectorNormalize2(self->velocity, origin);
		Vec3ScaleAssign(-ARROW_BACKUP, origin);
		Vec3AddAssign(self->s.origin, origin);
	}

	VectorClear(self->velocity);

	// Create a damage handling effect.
	edict_t* damage_area = G_Spawn();

	VectorCopy(origin, damage_area->s.origin);
	damage_area->think = RedRainThink;
	damage_area->nextthink = level.time + RED_RAIN_DAMAGE_INTERVAL;
	damage_area->solid = SOLID_NOT;
	damage_area->clipmask = CONTENTS_EMPTY;
	damage_area->movetype = MOVETYPE_FLYMISSILE; // Necessary for proper processing of thinkers.
	damage_area->wait = RED_RAIN_DAMAGE_INTERVAL;

	damage_area->delay = level.time + RED_RAIN_DURATION;
	if (DEATHMATCH)
		damage_area->delay -= 2; // 5 seconds in DM.

	damage_area->owner = self->owner;
	damage_area->red_rain_count = self->owner->red_rain_count;
	damage_area->classname = "Spell_RedRain";
	damage_area->health = self->health; // Copy over the powerup status.
	damage_area->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	const qboolean is_powered = (self->health > 0);
	damage_area->dmg = (is_powered ? POWER_RAIN_DAMAGE : RED_RAIN_DAMAGE); //mxd
	const float radius = (is_powered ? POWER_RAIN_RADIUS : RED_RAIN_RADIUS); //mxd

	// Find the top of the damage area.  Check down in an area less than the max size.
	VectorSet(damage_area->mins, -radius * 0.5f, -radius * 0.5f, -1.0f);
	VectorSet(damage_area->maxs, radius * 0.5f, radius * 0.5f, 1.0f);

	vec3_t end;
	VectorCopy(origin, end);
	end[2] += MAX_REDRAINHEIGHT;

	trace_t trace;
	gi.trace(origin, damage_area->mins, damage_area->maxs, end, damage_area, MASK_SOLID, &trace);

	if (trace.fraction == 1.0f)
		damage_area->maxs[2] = MAX_REDRAINHEIGHT; // Put the bounds up all the way.
	else
		damage_area->maxs[2] = trace.endpos[2] - origin[2]; // Set the bounds up only part way.

	// Find the bottom of the damage area.
	end[2] = origin[2] - MAX_REDRAINFALLDIST;

	gi.trace(origin, damage_area->mins, damage_area->maxs, end, damage_area, MASK_SOLID, &trace);

	if (trace.fraction == 1.0f)
		damage_area->mins[2] = -MAX_REDRAINFALLDIST; // Put the bounds down all the way.
	else
		damage_area->mins[2] = trace.endpos[2] - origin[2]; // Set the bounds down where the trace stopped.

	// Put the bounds of the damage area out to the max position now.
	damage_area->mins[0] = -radius;
	damage_area->mins[1] = -radius;
	damage_area->maxs[0] = radius;
	damage_area->maxs[1] = radius;

	VectorSet(damage_area->pos1, damage_area->s.origin[0], damage_area->s.origin[1], damage_area->maxs[2]);

	gi.linkentity(damage_area);

	// Start the red rain. Send along the health as a flag, to indicate if powered up.
	gi.CreateEffect(&damage_area->s, FX_WEAPON_REDRAIN, CEF_BROADCAST | (self->health << 5), origin, "");

	damage_area->s.sound = (byte)gi.soundindex("weapons/RedRainFall.wav");
	damage_area->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;

	// Turn off the client effect.
	gi.RemoveEffects(&self->s, FX_WEAPON_REDRAINMISSILE);
	G_SetToFree(self);
}

static void RedRainMissileThink(edict_t* self)
{
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
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
	if(trace.ent && OkToAutotarget(Caster, trace.ent))
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

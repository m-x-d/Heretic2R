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

void RedRainRemove(edict_t* self)
{
	gi.RemoveEffects(&self->s, FX_REMOVE_EFFECTS);
	G_SetToFree(self);
}

static void RedRainLightning(edict_t* self) //mxd. Split from RedRainThink().
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

void RedRainThink(edict_t* self)
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
		if (self->delay - level.time < RED_RAIN_LIGHTNING_DURATION && irand(0, RED_RAIN_LIGHTNING_CHANCE - 1) == 0) //mxd. irand(0, RED_RAIN_LIGHTNING_CHANCE) in original logic. Let's actually make it 1 in 6 chance...
			RedRainLightning(self);

		self->nextthink = level.time + self->wait;
	}
}

void RedRainMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
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
		damage_area->delay -= 2.0f; // 5 seconds in DM. //TODO: actually, 6 seconds in DM. Adjust to match comment?..

	damage_area->owner = self->owner;
	damage_area->red_rain_count = self->owner->red_rain_count;
	damage_area->classname = "Spell_RedRain";
	damage_area->health = self->health; // Copy over the powerup status.
	damage_area->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	const qboolean is_powered = (self->health > 0);
	damage_area->dmg = (is_powered ? POWER_RAIN_DAMAGE : RED_RAIN_DAMAGE); //mxd
	const float radius = (is_powered ? POWER_RAIN_RADIUS : RED_RAIN_RADIUS); //mxd

	// Find the top of the damage area. Check down in an area less than the max size.
	VectorSet(damage_area->mins, -radius * 0.5f, -radius * 0.5f, -1.0f);
	VectorSet(damage_area->maxs, radius * 0.5f, radius * 0.5f, 1.0f);

	vec3_t end = VEC3_INITA(origin, 0.0f, 0.0f, MAX_REDRAINHEIGHT);

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
	int flags = CEF_BROADCAST; //mxd
	if (is_powered)
		flags |= CEF_FLAG6;
	gi.CreateEffect(&damage_area->s, FX_WEAPON_REDRAIN, flags, origin, "");

	damage_area->s.sound = (byte)gi.soundindex("weapons/RedRainFall.wav");
	damage_area->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;

	// Turn off the client effect.
	gi.RemoveEffects(&self->s, FX_WEAPON_REDRAINMISSILE);
	G_SetToFree(self);
}

void RedRainMissileThink(edict_t* self)
{
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
}

// Create the guts of the red rain arrow.
static edict_t* CreateRedRainArrow(const qboolean is_powered) // Named 'create_redarrow' in original logic. //mxd. +is_powered arg.
{
	edict_t* arrow = G_Spawn();

	arrow->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	arrow->svflags |= SVF_ALWAYS_SEND;
	arrow->movetype = MOVETYPE_FLYMISSILE;
	arrow->classname = "Spell_RedRainArrow";
	arrow->solid = SOLID_BBOX;
	arrow->clipmask = MASK_SHOT;
	arrow->health = (is_powered ? 1 : 0); // Health indicates a level of powerup.

	VectorSet(arrow->mins, -ARROW_RADIUS, -ARROW_RADIUS, -ARROW_RADIUS);
	VectorSet(arrow->maxs,  ARROW_RADIUS,  ARROW_RADIUS,  ARROW_RADIUS);

	if (is_powered) // Powered arrow?
		arrow->dmg = irand(POWER_RAIN_DMG_ARROW_MIN, POWER_RAIN_DMG_ARROW_MAX);
	else
		arrow->dmg = irand(RED_RAIN_DMG_ARROW_MIN, RED_RAIN_DMG_ARROW_MAX);

	arrow->touch = RedRainMissileTouch;
	arrow->think = RedRainMissileThink;
	arrow->nextthink = level.time + FRAMETIME; //mxd. Use define.

	return arrow;
}

edict_t* RedRainMissileReflect(edict_t* self, edict_t* other, const vec3_t vel)
{
	// Create a new missile to replace the old one - this is necessary because physics will do nasty things
	// with the existing one, since we hit something. Hence, we create a new one totally.
	const qboolean is_powered = (self->health > 0); //mxd
	edict_t* arrow = CreateRedRainArrow(is_powered);

	// Copy everything across.
	VectorCopy(self->s.origin, arrow->s.origin);
	VectorCopy(vel, arrow->velocity);
	arrow->owner = other;
	arrow->reflect_debounce_time = self->reflect_debounce_time - 1; // So it doesn't infinitely reflect in one frame somehow.
	arrow->reflected_time = self->reflected_time;

	arrow->owner->red_rain_count++;
	self->owner->red_rain_count--;

	G_LinkMissile(arrow);

	// Create new trails for the new missile.
	int flags = CEF_OWNERS_ORIGIN | CEF_FLAG8; //mxd
	if (is_powered)
		flags |= CEF_FLAG6;
	gi.CreateEffect(&arrow->s, FX_WEAPON_REDRAINMISSILE, flags, NULL, "t", arrow->velocity);

	// Kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it.
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point.
	gi.CreateEffect(&arrow->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", arrow->velocity);

	return arrow;
}

void SpellCastRedRain(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles)
{
	caster->red_rain_count++;
	gi.RemoveEffects(&caster->s, FX_WEAPON_REDRAINGLOW);

	const qboolean is_powered = (caster->client->playerinfo.powerup_timer > level.time); //mxd
	edict_t* arrow = CreateRedRainArrow(is_powered);

	arrow->reflect_debounce_time = MAX_REFLECT;
	VectorCopy(start_pos, arrow->s.origin);

	// If we have current enemy, we've already traced to its position and can hit it. Also, crosshair is currently aimed at it --mxd.
	if (caster->enemy != NULL)
		GetAimVelocity(caster->enemy, arrow->s.origin, RED_ARROW_SPEED, aim_angles, arrow->velocity);
	else
		AdjustAimVelocity(caster, start_pos, aim_angles, RED_ARROW_SPEED, 22.0f, arrow->velocity); //mxd

	arrow->owner = caster;
	G_LinkMissile(arrow);

	// Play firing sound.
	const char* snd_name = (is_powered ? "weapons/RedRainPowerFire.wav" : "weapons/RedRainFire.wav"); //mxd
	gi.sound(caster, CHAN_WEAPON, gi.soundindex(snd_name), 1.0f, ATTN_NORM, 0.0f);

	// Remove the bow ready sound.
	caster->s.sound = 0;

	// Trace from the player's origin because then if we hit a wall, the effect won't be inside it...
	trace_t back_trace;
	gi.trace(caster->s.origin, arrow->mins, arrow->maxs, arrow->s.origin, caster, MASK_PLAYERSOLID, &back_trace);

	if (back_trace.startsolid || back_trace.fraction < 0.99f)
	{
		const vec3_t* pos = (back_trace.startsolid ? &caster->s.origin : &back_trace.endpos); //mxd
		VectorCopy(*pos, arrow->s.origin);
		RedRainMissileTouch(arrow, back_trace.ent, &back_trace.plane, back_trace.surface);
	}
	else
	{
		// Create the missile trail effect only if we can successfully launch the missile.
		int fx_flags = CEF_OWNERS_ORIGIN; // Red trail.
		if (is_powered)
			fx_flags |= CEF_FLAG6; // Magenta trail.

		gi.CreateEffect(&arrow->s, FX_WEAPON_REDRAINMISSILE, fx_flags, NULL, "t", arrow->velocity);
	}
}
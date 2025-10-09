//
// spl_wall.c //TODO: rename to spl_FireWall.c?
//
// Copyright 1998 Raven Software
//

#include "spl_wall.h" //mxd
#include "g_combat.h" //mxd
#include "g_Physics.h"
#include "g_playstats.h"
#include "FX.h"
#include "Utilities.h" //mxd
#include "Vector.h"
#include "g_local.h"

#define FIREWALL_DOT_MIN		0.25f
#define FIREWORM_LIFETIME		1.0f
#define MAX_FIREBLAST_BOUNCES	3 //mxd

#pragma region ========================== FireBlast (unpowered) ==========================

static edict_t* CreateFireBlast(const vec3_t start_pos, const vec3_t angles, edict_t* owner, const int health, const float timestamp)
{
	edict_t* wall = G_Spawn();

	VectorSet(wall->mins, -FIREBLAST_PROJ_RADIUS, -FIREBLAST_PROJ_RADIUS, -FIREBLAST_PROJ_RADIUS);
	VectorSet(wall->maxs,  FIREBLAST_PROJ_RADIUS,  FIREBLAST_PROJ_RADIUS,  FIREBLAST_PROJ_RADIUS);

	VectorCopy(start_pos, wall->s.origin);
	VectorCopy(angles, wall->s.angles);
	AngleVectors(angles, wall->movedir, NULL, NULL);
	VectorScale(wall->movedir, FIREBLAST_SPEED, wall->velocity);

	wall->mass = 250;
	wall->elasticity = ELASTICITY_NONE;
	wall->friction = 0.0f;
	wall->gravity = 0.0f;

	wall->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	wall->svflags |= SVF_ALWAYS_SEND | SVF_DO_NO_IMPACT_DMG;
	wall->movetype = PHYSICSTYPE_FLY;
	wall->isBlocked = FireBlastBlocked;

	wall->classname = "Spell_FireBlast";
	wall->solid = SOLID_BBOX;
	wall->clipmask = MASK_DRIP;
	wall->owner = owner;
	wall->dmg_radius = FIREBLAST_RADIUS;
	wall->dmg = FIREBLAST_DAMAGE;

	wall->health = health; // Can bounce 3 times.
	wall->fire_timestamp = timestamp; // This marks the wall with a more-or-less unique value so the wall doesn't damage twice.

	wall->think = FireBlastStartThink;
	wall->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(wall);

	const short angle_yaw = ANGLE2SHORT(angles[YAW]);
	const short angle_pitch = ANGLE2SHORT(angles[PITCH]);
	gi.CreateEffect(&wall->s, FX_WEAPON_FIREBURST, CEF_OWNERS_ORIGIN, NULL, "ss", angle_yaw, angle_pitch);

	return wall;
}

// This called when missile touches anything (world or edict).
static void FireBlastBlocked(edict_t* self, trace_t* trace)
{
	assert(trace != NULL);

	// If we haven't damaged what we are hitting yet, damage it now. Mainly for the Trial Beast.
	if (trace->ent != NULL && trace->ent->takedamage != DAMAGE_NO && self->fire_timestamp > trace->ent->fire_timestamp)
	{
		// If we have reflection on, then no damage; no damage to casting player.
		if (!EntReflecting(trace->ent, true, true) && trace->ent != self->owner)
		{
			T_Damage(trace->ent, self, self->owner, self->movedir, self->s.origin, vec3_origin, self->dmg, self->dmg, DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_FIREWALL);

			gi.CreateEffect(&(trace->ent->s), FX_FLAREUP, CEF_OWNERS_ORIGIN, NULL, "");
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/FirewallDamage.wav"), 1.0f, ATTN_NORM, 0.0f);

			trace->ent->fire_timestamp = self->fire_timestamp; // Mark so the fire doesn't damage an ent twice.
		}
	}

	if (self->health > 0 && !(trace->contents & CONTENTS_WATER) && (trace->plane.normal[2] > FIREWALL_DOT_MIN || trace->plane.normal[2] < -FIREWALL_DOT_MIN))
	{
		const float dot = DotProduct(self->movedir, trace->plane.normal); // Potentially uninitialized in original logic --mxd.
		const float min_dot = (self->health == MAX_FIREBLAST_BOUNCES ? -1.0f : -0.67f); //mxd. For the first collision, allow almost perpendicular bounce - fixes fireblast disappearing when cast almost directly downwards.

		if (dot > min_dot && dot < 0.0f) // Slide on all but the most extreme angles.
		{
			vec3_t surf_vel;
			VectorMA(self->movedir, -dot, trace->plane.normal, surf_vel); // Vel then holds the velocity negated by the impact.

			vec3_t surf_dir;
			const float factor = VectorNormalize2(surf_vel, surf_dir); // Yes, there is the tiniest chance this could be a zero vect, 

			if (factor > 0.0f)
			{
				vec3_t test_pos;
				VectorMA(self->s.origin, 16.0f, surf_dir, test_pos); // Test distance.

				trace_t new_trace;
				gi.trace(self->s.origin, self->mins, self->maxs, test_pos, self, MASK_SHOT, &new_trace);

				if (new_trace.fraction > 0.99f)
				{
					// If this is successful, then we can make another fireblast moving in the new direction.
					vec3_t new_ang;
					vectoangles(surf_dir, new_ang);
					new_ang[PITCH] *= -1.0f; //TODO: this pitch inconsistency needs fixing...

					CreateFireBlast(self->s.origin, new_ang, self->owner, self->health - 1, level.time);
				}
			}
		}
	}

	// Well, whatever happened, free the current blast.
	VectorClear(self->velocity);

	self->s.effects |= EF_ALTCLIENTFX; // Indicate to the wall that it's time to die.
	G_SetToFree(self);
}

// Check in the area and try to damage anything in the immediate area.
static void FireBlastThink(edict_t* self)
{
	// Set up the checking volume.
	vec3_t min = { -self->dmg_radius, -self->dmg_radius, -FIREBLAST_VRADIUS };
	Vec3AddAssign(self->s.origin, min);

	vec3_t max = { self->dmg_radius, self->dmg_radius, FIREBLAST_VRADIUS };
	Vec3AddAssign(self->s.origin, max);

	// Find all the entities in the volume.
	edict_t* ent = NULL;
	while ((ent = FindInBounds(ent, min, max)) != NULL)
	{
		if (ent->takedamage == DAMAGE_NO || self->fire_timestamp <= ent->fire_timestamp || EntReflecting(ent, true, true) || ent == self->owner) // No damage to casting player.
			continue;

		T_Damage(ent, self, self->owner, self->movedir, self->s.origin, vec3_origin, self->dmg, self->dmg, DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_FIREWALL);

		gi.CreateEffect(&ent->s, FX_FLAREUP, CEF_OWNERS_ORIGIN, NULL, "");
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/FirewallDamage.wav"), 1.0f, ATTN_NORM, 0.0f);

		ent->fire_timestamp = self->fire_timestamp; // Mark so the fire doesn't damage an ent twice.
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	self->dmg_radius += FIREBLAST_DRADIUS * 0.1f;
	self->dmg = max(FIREBLAST_DAMAGE_MIN, self->dmg - 3);
}

static void FireBlastStartThink(edict_t* self)
{
	self->svflags |= SVF_NOCLIENT; // Allow transmission to client.

	FireBlastThink(self);
	self->think = FireBlastThink;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void CastFireBlast(edict_t* caster, vec3_t start_pos, vec3_t aim_angles)
{
	vec3_t angles;
	VectorCopy(aim_angles, angles);

	//mxd. Replicate II_WEAPON_FIREWALL case from Get_Crosshair()...
	vec3_t forward;
	AngleVectors(angles, forward, NULL, NULL);

	vec3_t end;
	const vec3_t view_pos = { caster->s.origin[0], caster->s.origin[1], caster->s.origin[2] + (float)caster->viewheight + 18.0f };
	VectorMA(view_pos, 1000.0f, forward, end);

	//mxd. Adjust aim angles to match crosshair logic...
	trace_t tr;
	gi.trace(view_pos, vec3_origin, vec3_origin, end, caster, MASK_SHOT, &tr);

	vec3_t dir;
	VectorSubtract(tr.endpos, start_pos, dir);
	VectorNormalize(dir);

	vectoangles(dir, angles);
	angles[PITCH] *= -1.0f; //TODO: this pitch inconsistency needs fixing...

	edict_t* wall = CreateFireBlast(start_pos, angles, caster, MAX_FIREBLAST_BOUNCES, level.time); // Bounce 3 times.

	// Check to see if this is a legit spawn.
	trace_t trace;
	gi.trace(caster->s.origin, wall->mins, wall->maxs, wall->s.origin, caster, MASK_SOLID, &trace);

	if (trace.startsolid || trace.fraction < 0.99f)
	{
		if (!trace.startsolid)
			VectorCopy(trace.endpos, wall->s.origin);

		FireBlastBlocked(wall, &trace);
	}
	else
	{
		FireBlastThink(wall);
	}
}

#pragma endregion

#pragma region ========================== FireWall (powered up) ==========================

static edict_t* CreateFireWall(vec3_t start_pos, vec3_t angles, edict_t* owner, const int health, const float timestamp, const float side_speed)
{
	edict_t* wall = G_Spawn();

	VectorSet(wall->mins, -FIREWAVE_PROJ_RADIUS, -FIREWAVE_PROJ_RADIUS, -FIREWAVE_PROJ_RADIUS);
	VectorSet(wall->maxs,  FIREWAVE_PROJ_RADIUS,  FIREWAVE_PROJ_RADIUS,  FIREWAVE_PROJ_RADIUS);

	VectorCopy(start_pos, wall->s.origin);
	VectorCopy(angles, wall->s.angles);

	vec3_t right;
	AngleVectors(angles, wall->movedir, right, NULL);

	int flags = 0;
	if (DEATHMATCH)
	{
		flags |= CEF_FLAG8;
		VectorScale(wall->movedir, FIREWAVE_DM_SPEED, wall->velocity); // Goes faster in deathmatch.
	}
	else
	{
		VectorScale(wall->movedir, FIREWAVE_SPEED, wall->velocity);
	}

	VectorMA(wall->velocity, side_speed, right, wall->velocity);

	if (side_speed < 0.0f)
		flags |= CEF_FLAG6;
	else if (side_speed > 0.0f)
		flags |= CEF_FLAG7;

	wall->mass = 250;
	wall->elasticity = ELASTICITY_NONE;
	wall->friction = 0.0f;
	wall->gravity = 0.0f;

	wall->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	wall->svflags |= SVF_ALWAYS_SEND;
	wall->movetype = PHYSICSTYPE_FLY;
	wall->isBlocked = FireWallMissileBlocked;

	wall->classname = "Spell_FireWall";
	wall->solid = SOLID_BBOX;
	wall->clipmask = MASK_DRIP;
	wall->owner = owner;
	wall->dmg = FIREWAVE_DAMAGE;
	wall->dmg_radius = FIREWAVE_RADIUS;

	wall->health = health; // Can bounce 3 times
	wall->fire_timestamp = timestamp; // Mark the wall so it can't damage something twice.

	wall->think = FireWallMissileStartThink;
	wall->nextthink = level.time + FRAMETIME; //mxd. Use define.

	gi.linkentity(wall);

	const short angle_yaw = ANGLE2SHORT(angles[YAW]);
	const short angle_pitch = ANGLE2SHORT(angles[PITCH]);
	gi.CreateEffect(&wall->s, FX_WEAPON_FIREWAVE, CEF_OWNERS_ORIGIN | flags, start_pos, "ss", angle_yaw, angle_pitch);

	return wall;
}

static void FireWallMissileWormThink(edict_t* self)
{
	T_DamageRadius(self, self->owner, self->owner, 64.0f, FIREWAVE_WORM_DAMAGE, FIREWAVE_WORM_DAMAGE, DAMAGE_FIRE, MOD_FIREWALL);
	G_SetToFree(self);
}

// This called when missile touches anything (world or edict).
static void FireWallMissileBlocked(edict_t* self, trace_t* trace)
{
	assert(trace != NULL);

	// If we haven't damaged what we are hitting yet, damage it now.  Mainly for the Trial Beast.
	if (trace->ent != NULL && trace->ent->takedamage != DAMAGE_NO && self->fire_timestamp > trace->ent->fire_timestamp)
	{
		// If we have reflection on, then no damage; no damage to casting player.
		if (!EntReflecting(trace->ent, true, true) && trace->ent != self->owner)
		{
			T_Damage(trace->ent, self, self->owner, self->movedir, self->s.origin, vec3_origin, self->dmg, self->dmg, DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_FIREWALL);
			gi.CreateEffect(&(trace->ent->s), FX_FLAREUP, CEF_OWNERS_ORIGIN, NULL, "");

			trace->ent->fire_timestamp = self->fire_timestamp;
			gi.CreateEffect(NULL, FX_WEAPON_FIREWAVEWORM, 0, trace->ent->s.origin, "t", self->movedir);

			edict_t* worm = G_Spawn();
			VectorCopy(trace->ent->s.origin, worm->s.origin);
			worm->think = FireWallMissileWormThink;
			worm->nextthink = level.time + FIREWORM_LIFETIME;
			worm->solid = SOLID_NOT;
			worm->clipmask = MASK_DRIP;
			worm->owner = self->owner;
			gi.linkentity(worm);

			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/FirewallDamage.wav"), 1.0f, ATTN_NORM, 0.0f);
		}
	}

	float dot = 0.0f; //BUGFIX: mxd. Uninitialized in original version.
	if (self->health > 0 && !(trace->contents & CONTENTS_WATER) && (trace->plane.normal[2] > FIREWALL_DOT_MIN || trace->plane.normal[2] < -FIREWALL_DOT_MIN))
		dot = DotProduct(self->movedir, trace->plane.normal);

	if (dot > -0.67f && dot < 0.0f) // Slide on all but the most extreme angles.
	{
		vec3_t surf_vel;
		VectorMA(self->movedir, -dot, trace->plane.normal, surf_vel); // Vel then holds the velocity negated by the impact.

		vec3_t surf_dir;
		const float factor = VectorNormalize2(surf_vel, surf_dir); // Yes, there is the tiniest chance this could be a zero vect, 

		if (factor > 0.0f)
		{
			vec3_t test_pos;
			VectorMA(self->s.origin, 16.0f, surf_dir, test_pos); // Test distance.

			trace_t	new_trace;
			gi.trace(self->s.origin, self->mins, self->maxs, test_pos, self, MASK_SOLID, &new_trace);

			if (new_trace.fraction > 0.99f)
			{
				// If this is successful, then we can make another fireblast moving in the new direction.
				vec3_t new_ang;
				vectoangles(surf_dir, new_ang);
				CreateFireWall(self->s.origin, new_ang, self->owner, self->health - 1, level.time, 0.0f);
			}
		}
	}

	// Well, whatever happened, free the current blast.
	VectorClear(self->velocity);

	self->s.effects |= EF_ALTCLIENTFX; // Indicate to the wall that it's time to die.
	G_SetToFree(self);
}

// Check in the area and try to damage anything in the immediate area.
static void FireWallMissileThink(edict_t* self)
{
	// Set up the checking volume
	vec3_t min = { -self->dmg_radius, -self->dmg_radius, -FIREWAVE_DOWN };
	Vec3AddAssign(self->s.origin, min);

	vec3_t max = { self->dmg_radius, self->dmg_radius, FIREWAVE_UP };
	Vec3AddAssign(self->s.origin, max);

	// Find all the entities in the volume.
	edict_t* ent = NULL;
	while ((ent = FindInBounds(ent, min, max)) != NULL)
	{
		if (ent->takedamage == DAMAGE_NO || ent->fire_timestamp >= self->fire_timestamp || EntReflecting(ent, true, true) || ent == self->owner) // No damage to casting player.
			continue;

		T_Damage(ent, self, self->owner, self->movedir, self->s.origin, vec3_origin, self->dmg, self->dmg, DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_FIREWALL);
		gi.CreateEffect(&ent->s, FX_FLAREUP, CEF_OWNERS_ORIGIN, NULL, "");

		ent->fire_timestamp = self->fire_timestamp;
		gi.CreateEffect(NULL, FX_WEAPON_FIREWAVEWORM, 0, ent->s.origin, "t", self->movedir);

		edict_t* worm = G_Spawn();
		VectorCopy(ent->s.origin, worm->s.origin);
		worm->think = FireWallMissileWormThink;
		worm->nextthink = level.time + FIREWORM_LIFETIME;
		worm->solid = SOLID_NOT;
		worm->clipmask = MASK_DRIP;
		worm->owner = self->owner;
		gi.linkentity(worm);

		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/FirewallDamage.wav"), 1.0f, ATTN_NORM, 0.0f);
	}

	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	self->dmg_radius += FIREWAVE_DRADIUS * 0.1f;
	self->dmg = max(FIREWAVE_DAMAGE_MIN, self->dmg - 3);
}

static void FireWallMissileStartThink(edict_t* self)
{
	self->svflags |= SVF_NOCLIENT; // Allow transmission to client.

	FireWallMissileThink(self);
	self->think = FireWallMissileThink;
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

static void CastFireWall(edict_t* caster, vec3_t start_pos, vec3_t aim_angles)
{
	// Big wall is powered up.
	vec3_t fwd;
	vec3_t right;
	AngleVectors(aim_angles, fwd, right, NULL);

	// Spawn walls to the left and right.
	for (int i = 0; i < 2; i++)
	{
		const float radius = FIREWAVE_RADIUS * (i == 0 ? -1.0f : 1.0f); //mxd

		vec3_t spawn_pos;
		VectorMA(start_pos, radius, right, spawn_pos);

		edict_t* wall = CreateFireWall(spawn_pos, aim_angles, caster, 3, level.time, radius);

		// Check to see if this is a legit spawn.
		trace_t trace;
		gi.trace(caster->s.origin, wall->mins, wall->maxs, wall->s.origin, caster, MASK_SOLID, &trace);

		if (trace.startsolid || trace.fraction < 0.99f)
		{
			if (trace.startsolid)
				VectorCopy(caster->s.origin, wall->s.origin);
			else
				VectorCopy(trace.endpos, wall->s.origin);

			FireWallMissileBlocked(wall, &trace);
		}
		else
		{
			FireWallMissileThink(wall);
		}
	}
}

#pragma endregion

// The Firewall spell is cast.
void SpellCastFireWall(edict_t* caster, vec3_t start_pos, vec3_t aim_angles)
{
	int snd_index; //mxd

	if (caster->client->playerinfo.powerup_timer <= level.time)
	{
		// Not powered up.
		CastFireBlast(caster, start_pos, aim_angles); //mxd. Starts at start_pos[2] + 16 in original logic.
		snd_index = gi.soundindex("weapons/FirewallCast.wav");
	}
	else
	{
		// Powered up, cast big wall o' doom.
		CastFireWall(caster, start_pos, aim_angles);
		snd_index = gi.soundindex("weapons/FirewallPowerCast.wav");
	}

	gi.sound(caster, CHAN_WEAPON, snd_index, 1.0f, ATTN_NORM, 0.0f);
}
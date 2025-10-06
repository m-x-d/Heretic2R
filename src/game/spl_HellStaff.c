//
// spl_hellstaff.c
//
// Copyright 1998 Raven Software
//

#include "spl_HellStaff.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "p_main.h"
#include "g_playstats.h"
#include "m_beast.h"
#include "Decals.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

// Radius of zero seems to prevent collision between bolts.
#define HELLBOLT_RADIUS		0.0f

static void HellboltTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	// Did we hit the sky? 
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// Has the target got reflection turned on?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(HELLBOLT_SPEED / 2.0f, self->velocity);
		HellboltReflect(self, other, self->velocity);

		return;
	}

	VectorNormalize2(self->velocity, self->movedir);

	if (other->takedamage != DAMAGE_NO)
	{
		T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, self->dmg, self->dmg * 2, DAMAGE_SPELL, MOD_HELLSTAFF);
	}
	else
	{
		// Back off the origin for the damage a bit.
		// We are a point and this will help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(self->s.origin, -8.0f, self->movedir, self->s.origin);
	}

	AlertMonsters(self, self->owner, 1.0f, false); //mxd. Done in 'Back off the origin for the damage a bit' case in original logic. Why?..

	int fx_flags = 0;
	if (IsDecalApplicable(other, self->s.origin, surface, plane, NULL))
		fx_flags = CEF_FLAG6;

	gi.CreateEffect(NULL, FX_WEAPON_HELLBOLTEXPLODE, fx_flags, self->s.origin, "d", self->movedir); //mxd. Created with CEF_OWNERS_ORIGIN flag in original logic (won't work after G_SetToFree() -> G_FreeEdict() change).
	G_FreeEdict(self); //mxd. G_SetToFree() in original logic. Fixes client effect/dynamic light staying active for 100 ms. after this.
}

static void HellboltThink(edict_t* self)
{
	// Prevent any further transmission of this entity to clients.
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
}

// Guts of creating a hell bolt.
static void CreateHellbolt(edict_t* hellbolt)
{
	hellbolt->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	hellbolt->svflags |= SVF_ALWAYS_SEND;
	hellbolt->movetype = MOVETYPE_FLYMISSILE;
	hellbolt->dmg = irand(HELLBOLT_DAMAGE_MIN, HELLBOLT_DAMAGE_MAX);
	hellbolt->classname = "Spell_Hellbolt";
	hellbolt->solid = SOLID_BBOX;
	hellbolt->clipmask = MASK_SHOT;

	VectorSet(hellbolt->mins, -HELLBOLT_RADIUS, -HELLBOLT_RADIUS, -HELLBOLT_RADIUS);
	VectorSet(hellbolt->maxs,  HELLBOLT_RADIUS,  HELLBOLT_RADIUS,  HELLBOLT_RADIUS);

	hellbolt->touch = HellboltTouch;
	hellbolt->think = HellboltThink;
	hellbolt->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

edict_t* HellboltReflect(edict_t* self, edict_t* other, const vec3_t vel)
{
	// Create a new missile to replace the old one - this is necessary because physics will do nasty things
	// with the existing one, since we hit something. Hence, we create a new one totally.
	edict_t* hellbolt = G_Spawn();

	// Copy everything across.
	CreateHellbolt(hellbolt);
	VectorCopy(self->s.origin, hellbolt->s.origin);
	VectorCopy(vel, hellbolt->velocity);
	VectorNormalize2(vel, hellbolt->movedir);
	vectoangles(hellbolt->movedir, hellbolt->s.angles); //TODO: FlyingFistReflect() uses AnglesFromDir() here. Why?
	hellbolt->owner = other;
	hellbolt->reflect_debounce_time = self->reflect_debounce_time - 1; // So it doesn't infinitely reflect in one frame somehow.
	hellbolt->reflected_time = self->reflected_time;

	G_LinkMissile(hellbolt);

	// Create new trails for the new missile.
	gi.CreateEffect(&hellbolt->s, FX_WEAPON_HELLBOLT, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "t", hellbolt->velocity);

	// Kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point. Do this rarely, since we hit a lot.
	if (irand(0, 10) == 0)
		gi.CreateEffect(&hellbolt->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", hellbolt->velocity);

	return hellbolt;
}

//mxd. Added to reduce code duplication. Modifies 'aim_angles' when reflected!
static void HellLaserDamage(edict_t* caster, const vec3_t start_pos, vec3_t aim_angles, const vec3_t forward, const trace_t* trace)
{
	// Did we hit something that reflects?
	if (EntReflecting(trace->ent, true, true))
	{
		// Reflect it off into space - powerless now, so it won't hurt anyone it hits.

		// Draw line to this point.
		vec3_t vect;
		VectorSubtract(trace->endpos, start_pos, vect);
		const byte b_len = (byte)(VectorLength(vect) / 8.0f);

		gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER, 0, start_pos, "tb", forward, b_len);

		// Re-constitute aimangle.
		aim_angles[1] += flrand(160.0f, 200.0f);
		aim_angles[0] += flrand(-20.0f, 20.0f);
	}
	else
	{
		const int damage = irand(HELLLASER_DAMAGE_MIN, HELLLASER_DAMAGE_MAX); //mxd
		T_Damage(trace->ent, caster, caster, forward, trace->endpos, forward, damage, 0, DAMAGE_SPELL, MOD_P_HELLSTAFF);

		gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER_BURN, CEF_FLAG6, trace->endpos, "t", forward);
	}
}

//mxd. Added to reduce code duplication.
static void PlayHellstaffFiringSound(edict_t* caster, const char* snd_name)
{
	int snd_channel; //mxd

	// This alternation avoids cutting sounds out prematurely.
	if (caster->client->playerinfo.flags & PLAYER_FLAG_ALTFIRE)
	{
		// Use the alternate slot, clear the flag.
		snd_channel = CHAN_WEAPON2;
		caster->client->playerinfo.flags &= ~PLAYER_FLAG_ALTFIRE;
	}
	else
	{
		// Use the regular slot, set the flag.
		snd_channel = CHAN_WEAPON;
		caster->client->playerinfo.flags |= PLAYER_FLAG_ALTFIRE;
	}

	gi.sound(caster, snd_channel, gi.soundindex(snd_name), 1.0f, ATTN_NORM, 0.0f);
}

// Powered up version of this weapon - a laser.
static void FireHellLaser(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles) //mxd. Split from SpellCastHellstaff().
{
#define HELLSTAFF_LASER_MAX_TARGETS		8

	const vec3_t mins = { -4.0f, -4.0f, -4.0f };
	const vec3_t maxs = {  4.0f,  4.0f,  4.0f };

	//mxd. Setup aiming direction...
	vec3_t end;

	//mxd. Replicate II_WEAPON_HELLSTAFF case from Get_Crosshair()...
	const vec3_t view_pos = { caster->s.origin[0], caster->s.origin[1], caster->s.origin[2] + (float)caster->viewheight + 14.0f };

	if (caster->enemy != NULL) //mxd. Auto-target current enemy?
	{
		VectorAverage(caster->enemy->mins, caster->enemy->maxs, end); // Get center of model.
		Vec3AddAssign(caster->enemy->s.origin, end);
	}
	else
	{
		// Check ahead first to see if it's going to hit anything at this angle.
		vec3_t fwd;
		AngleVectors(aim_angles, fwd, NULL, NULL);

		VectorScale(fwd, HELLLASER_DIST, end);
		Vec3AddAssign(view_pos, end);
	}

	//mxd. Aim beam at world hit location. Can't stop at monsters, because beam won't be stopped by them...
	trace_t tr;
	gi.trace(view_pos, vec3_origin, vec3_origin, end, caster, MASK_SOLID, &tr); //TODO: damage trace will be also stopped by non-damageable entities.

	vec3_t dir;
	VectorSubtract(tr.endpos, start_pos, dir);
	VectorNormalize(dir);

	vec3_t cur_aim_angles; //mxd. Don't modify aim_angles.
	vectoangles(dir, cur_aim_angles);
	cur_aim_angles[PITCH] *= -1.0f; //TODO: this pitch inconsistency needs fixing...

	vec3_t cur_start_pos;
	VectorCopy(start_pos, cur_start_pos);

	// Do the damage...
	trace_t trace;
	vec3_t forward;
	float laser_dist = HELLLASER_DIST;
	const edict_t* ignore_ent = caster;

	for (int i = 0; i < HELLSTAFF_LASER_MAX_TARGETS && laser_dist > 0.0f; i++)
	{
		AngleVectors(cur_aim_angles, forward, NULL, NULL);

		vec3_t end_pos;
		VectorMA(cur_start_pos, laser_dist, forward, end_pos);

		gi.trace(cur_start_pos, mins, maxs, end_pos, ignore_ent, MASK_SHOT, &trace);

		if (level.fighting_beast)
		{
			edict_t* ent = TBeastCheckHit(caster->s.origin, trace.endpos);

			if (ent != NULL)
				trace.ent = ent;
		}

		// If we hit anything that won't take damage, kill the beam.
		if (trace.ent == NULL || trace.ent->takedamage == DAMAGE_NO || trace.fraction == 1.0f || (trace.contents & MASK_SOLID))
			break;

		if (trace.ent != caster)
			HellLaserDamage(caster, cur_start_pos, cur_aim_angles, forward, &trace); //mxd. Modifies 'cur_aim_angles' when reflected!

		vec3_t diff;
		VectorSubtract(trace.endpos, cur_start_pos, diff);
		laser_dist -= VectorLength(diff);

		VectorCopy(trace.endpos, cur_start_pos);
		ignore_ent = trace.ent;
	}

	PlayHellstaffFiringSound(caster, "weapons/HellLaserFire.wav"); //mxd

	vec3_t diff;
	VectorSubtract(trace.endpos, cur_start_pos, diff);
	const byte b_len = (byte)(VectorLength(diff) / 8.0f);

	// Decide if we need a scorch mark or not.
	int fx_flags = CEF_FLAG6; // Create HellLaserBurn fx --mxd.
	if (IsDecalApplicable(trace.ent, caster->s.origin, trace.surface, &trace.plane, NULL))
		fx_flags |= CEF_FLAG7;

	// Draw last (when reflected) or the only segment of the line.
	gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER, fx_flags, cur_start_pos, "tb", forward, b_len);
}

// Unpowered version of this weapon - hellbolts.
static void FireHellbolt(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles) //mxd. Split from SpellCastHellstaff().
{
	edict_t* hellbolt = G_Spawn();
	CreateHellbolt(hellbolt);
	hellbolt->reflect_debounce_time = MAX_REFLECT;
	VectorCopy(start_pos, hellbolt->s.origin);

	if (caster->enemy != NULL) // Auto-target current enemy?
	{
		// If we have current enemy, we've already traced to its position and can hit it. Also, crosshair is currently aimed at it --mxd.
		GetAimVelocity(caster->enemy, hellbolt->s.origin, HELLBOLT_SPEED, aim_angles, hellbolt->velocity);
	}
	else
	{
		// Check ahead first to see if it's going to hit anything at this angle.
		vec3_t forward;
		AngleVectors(aim_angles, forward, NULL, NULL);

		//mxd. Replicate II_WEAPON_HELLSTAFF case from Get_Crosshair()...
		vec3_t end;
		const vec3_t view_pos = { caster->s.origin[0], caster->s.origin[1], caster->s.origin[2] + (float)caster->viewheight + 14.0f };
		VectorMA(view_pos, HELLBOLT_SPEED, forward, end);

		trace_t trace;
		gi.trace(view_pos, vec3_origin, vec3_origin, end, caster, MASK_MONSTERSOLID, &trace);

		vec3_t dir;
		VectorSubtract(trace.endpos, hellbolt->s.origin, dir);
		VectorNormalize(dir);
		VectorScale(dir, HELLBOLT_SPEED, hellbolt->velocity);
	}

	hellbolt->owner = caster;

	// Remember velocity in case we have to reverse it.
	VectorNormalize2(hellbolt->velocity, hellbolt->movedir);

	G_LinkMissile(hellbolt);
	PlayHellstaffFiringSound(caster, "weapons/HellFire.wav"); //mxd

	// Make sure we don't start in a solid.
	trace_t back_trace;
	gi.trace(caster->s.origin, vec3_origin, vec3_origin, hellbolt->s.origin, caster, MASK_PLAYERSOLID, &back_trace); //H2_BUGFIX: mxd. Both 'start' and 'end' use hellbolt->s.origin in original logic.

	if (back_trace.startsolid)
	{
		VectorCopy(back_trace.endpos, hellbolt->s.origin); //mxd
		HellboltTouch(hellbolt, back_trace.ent, &back_trace.plane, back_trace.surface);

		return;
	}

	// Spawn effect after it has been determined it has not started in wall.
	// This is so it won't try to remove it before it exists.
	gi.CreateEffect(&hellbolt->s, FX_WEAPON_HELLBOLT, CEF_OWNERS_ORIGIN, NULL, "t", hellbolt->velocity);
}

void SpellCastHellstaff(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles)
{
	assert(caster->client != NULL);

	if (caster->client->playerinfo.powerup_timer > level.time)
		FireHellLaser(caster, start_pos, aim_angles);
	else
		FireHellbolt(caster, start_pos, aim_angles);
}
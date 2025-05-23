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
		AlertMonsters(self, self->owner, 1.0f, false);
		VectorMA(self->s.origin, -8.0f, self->movedir, self->s.origin);
	}

	int fx_flags = 0;
	if (IsDecalApplicable(other, self->s.origin, surface, plane, NULL))
		fx_flags = CEF_FLAG6;

	gi.CreateEffect(&self->s, FX_WEAPON_HELLBOLTEXPLODE, CEF_OWNERS_ORIGIN | fx_flags, NULL, "d", self->movedir);
	G_SetToFree(self);
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

	hellbolt->touch = HellboltTouch;
	hellbolt->think = HellboltThink;
	hellbolt->classname = "Spell_Hellbolt";
	hellbolt->nextthink = level.time + 0.1f;
	VectorSet(hellbolt->mins, -HELLBOLT_RADIUS, -HELLBOLT_RADIUS, -HELLBOLT_RADIUS);
	VectorSet(hellbolt->maxs,  HELLBOLT_RADIUS,  HELLBOLT_RADIUS,  HELLBOLT_RADIUS);

	hellbolt->solid = SOLID_BBOX;
	hellbolt->clipmask = MASK_SHOT;
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

//mxd. Added to reduce code duplication.
static void HellboltTryApplyDamage(edict_t* caster, const vec3_t start_pos, vec3_t aim_angles, const vec3_t forward, const trace_t* trace)
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
		T_Damage(trace->ent, caster, caster, forward, trace->endpos, forward,
			damage, 0, DAMAGE_SPELL, MOD_P_HELLSTAFF);

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

void SpellCastHellstaff(edict_t* caster, const vec3_t loc, vec3_t aim_angles)
{
#define HELLSTAFF_LASER_MAX_TARGETS		8

	const vec3_t min = { -4.0f, -4.0f, -4.0f };
	const vec3_t max = { 4.0f,  4.0f,  4.0f };

	assert(caster->client != NULL);

	if (caster->client->playerinfo.powerup_timer > level.time)
	{
		// Powered up version of this weapon - a laser.
		// We must trace from the player's centerpoint to the casting location to assure we don't hit anything before the laser starts.
		const edict_t* trace_buddy = caster;

		vec3_t start_pos;
		VectorCopy(loc, start_pos);

		trace_t trace;
		gi.trace(caster->s.origin, min, max, start_pos, caster, MASK_SHOT, &trace);

		if (level.fighting_beast)
		{
			edict_t* ent = TBeastCheckHit(caster->s.origin, trace.endpos);

			if (ent != NULL)
				trace.ent = ent;
		}

		if (trace.fraction > 0.99f || !(trace.contents & MASK_SOLID))
		{
			// It's okay to continue with the shot. If not, we should skip right to the impact.
			// Now then, if we hit something on the way to the start location, damage him NOW!
			if (trace.ent != NULL && trace.ent->takedamage != DAMAGE_NO)
			{
				vec3_t forward;
				AngleVectors(aim_angles, forward, NULL, NULL); //BUGFIX: mxd. Uninitialized in original version.

				HellboltTryApplyDamage(caster, start_pos, aim_angles, forward, &trace); //mxd

				trace_buddy = trace.ent;
			} // Don't trace again since there really should only be one thing between the player and start_pos.

			// Set up for main laser damaging loop.
			float laser_dist = HELLLASER_DIST;
			int num_hit = 0; // Can hit no more than 8 guys...

			do
			{
				vec3_t forward;
				AngleVectors(aim_angles, forward, NULL, NULL);

				vec3_t end_pos;
				VectorMA(start_pos, laser_dist, forward, end_pos);

				gi.trace(start_pos, min, max, end_pos, trace_buddy, MASK_SHOT, &trace);

				if (level.fighting_beast)
				{
					edict_t* ent = TBeastCheckHit(caster->s.origin, trace.endpos);

					if (ent != NULL)
						trace.ent = ent;
				}

				if (trace.fraction < 0.99f)
				{
					// If we hit anything that won't take damage, kill the beam.
					if (trace.ent->takedamage == DAMAGE_NO)
						break;

					// This is possible if the trace_buddy is not the caster because a new one was on the way to start_pos.
					if (trace.ent != caster)
						HellboltTryApplyDamage(caster, start_pos, aim_angles, forward, &trace); //mxd

					// This seems to alleviate the problem of a trace hitting the same ent multiple times...
					vec3_t vect;
					VectorSubtract(trace.endpos, start_pos, vect);
					laser_dist -= VectorLength(vect);

					VectorCopy(trace.endpos, start_pos);
					VectorSubtract(end_pos, start_pos, vect);

					if (VectorLength(vect) > 16.0f)
						VectorMA(start_pos, 16.0f, forward, start_pos);

					trace_buddy = trace.ent;
					num_hit++;
				}

			} while (trace.fraction < 0.99f && !(trace.contents & MASK_SOLID) && num_hit < HELLSTAFF_LASER_MAX_TARGETS);
		}

		PlayHellstaffFiringSound(caster, "weapons/HellLaserFire.wav"); //mxd

		vec3_t vect;
		VectorSubtract(trace.endpos, start_pos, vect);
		const byte b_len = (byte)(VectorLength(vect) / 8.0f);

		vec3_t forward;
		AngleVectors(aim_angles, forward, NULL, NULL); //BUGFIX: mxd. Potentially uninitialized in original version.

		// Decide if we need a scorch mark or not.
		vec3_t plane_dir;
		int fx_flags = CEF_FLAG6; //mxd

		if (IsDecalApplicable(trace.ent, caster->s.origin, trace.surface, &trace.plane, plane_dir))
			fx_flags |= CEF_FLAG7;

		gi.CreateEffect(NULL, FX_WEAPON_HELLSTAFF_POWER, fx_flags, start_pos, "tb", forward, b_len);
	}
	else
	{
		// Unpowered version of this weapon - hellbolts.
		edict_t* hellbolt = G_Spawn();
		CreateHellbolt(hellbolt);
		VectorCopy(loc, hellbolt->s.origin);

		// Check ahead first to see if it's going to hit anything at this angle.
		vec3_t forward;
		AngleVectors(aim_angles, forward, NULL, NULL);

		if (caster->client->playerinfo.flags & PLAYER_FLAG_NO_LARM)
		{
			VectorScale(forward, HELLBOLT_SPEED, hellbolt->velocity);
		}
		else
		{
			vec3_t end_pos;
			VectorMA(loc, HELLBOLT_SPEED, forward, end_pos);

			trace_t trace;
			gi.trace(loc, vec3_origin, vec3_origin, end_pos, caster, MASK_MONSTERSOLID, &trace);

			if (trace.ent != NULL && OkToAutotarget(caster, trace.ent))
				VectorScale(forward, HELLBOLT_SPEED, hellbolt->velocity); // Already going to hit a valid target at this angle, so don't auto-target.
			else
				GetAimVelocity(caster->enemy, hellbolt->s.origin, HELLBOLT_SPEED, aim_angles, hellbolt->velocity); // Auto-target current enemy.
		}

		hellbolt->owner = caster;
		VectorNormalize2(hellbolt->velocity, hellbolt->movedir);
		hellbolt->reflect_debounce_time = MAX_REFLECT;

		G_LinkMissile(hellbolt);

		PlayHellstaffFiringSound(caster, "weapons/HellFire.wav"); //mxd

		trace_t trace;
		gi.trace(hellbolt->s.origin, vec3_origin, vec3_origin, hellbolt->s.origin, caster, MASK_PLAYERSOLID, &trace);

		if (trace.startsolid)
			HellboltTouch(hellbolt, trace.ent, &trace.plane, trace.surface);
		else
			gi.CreateEffect(&hellbolt->s, FX_WEAPON_HELLBOLT, CEF_OWNERS_ORIGIN, NULL, "t", hellbolt->velocity);
	}
}
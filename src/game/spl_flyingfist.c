//
// spl_flyingfist.c
//
// Copyright 1998 Raven Software
//

#include "spl_flyingfist.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "Decals.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define FIST_RADIUS		2.0f

static void FlyingFistFizzleThink(edict_t* self)
{
	// Don't fizzle in deathmatch, or if powered up.
	if (!DEATHMATCH && self->health == 0)
	{
		self->dmg = max(FIREBALL_MIN_FIZZLE_DAMAGE, self->dmg - 2);
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
}

static void FlyingFistInitThink(edict_t* self)
{
	self->svflags |= SVF_NOCLIENT;
	self->think = FlyingFistFizzleThink;

	FlyingFistFizzleThink(self);
}

static void FlyingFistTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface) //mxd. Moved to avoid forward declaration...
{
	if (other == self->owner) // Don't touch owner.
		return;

	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	const qboolean powerup = (self->health > 0); // Powered up meteor?
	int fx_flags = (powerup ? CEF_FLAG7 : 0);

	const qboolean wimpy = (self->flags & FL_NO_KNOCKBACK); // Wimpy out-of-ammo weapon.
	if (wimpy)
		fx_flags |= CEF_FLAG8;

	// Has the target got reflection turned on?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);

		// Scale speed down.
		Vec3ScaleAssign(FLYING_FIST_SPEED / 2.0f, self->velocity);
		FlyingFistReflect(self, other, self->velocity);

		return;
	}

	AlertMonsters(self, self->owner, 1.0f, false);

	if (other->takedamage != DAMAGE_NO)
	{
		const int dmg_div = (DEATHMATCH ? 2 : 1); //mxd

		if (powerup) // Check for powered up meteor.
		{
			int damage = irand(FIREBALL_DAMAGE_MIN_POWER / dmg_div, FIREBALL_DAMAGE_MAX_POWER / dmg_div);

			if (wimpy) // Wimpy (no mana) shots do half damage.
			{
				damage /= 2;

				T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal,
					damage, damage, DAMAGE_SPELL, MOD_FIREBALL); // No blast damage, just direct.
			}
			else
			{
				// Half goes directly to target, blast does rest.
				T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal,
					damage / 2, damage, DAMAGE_SPELL | DAMAGE_EXTRA_KNOCKBACK, MOD_FIREBALL);

				T_DamageRadius(self, self->owner, self->owner, FIREBALL_RADIUS,
					FIREBALL_DAMAGE_MAX_POWER, FIREBALL_DAMAGE_MIN_POWER, DAMAGE_SPELL, MOD_FIREBALL);
			}
		}
		else
		{
			int damage = irand(FIREBALL_DAMAGE_MIN / dmg_div, FIREBALL_DAMAGE_MAX / dmg_div);

			if (wimpy) // Wimpy (no mana) shots do half damage.
				damage /= 2;

			T_Damage(other, self, self->owner, self->movedir, self->s.origin, plane->normal, damage, damage, DAMAGE_SPELL, MOD_FIREBALL);
		}
	}
	else
	{
		VectorMA(self->s.origin, -8.0f, self->movedir, self->s.origin);
	}

	// Attempt to apply a scorchmark decal to the thing I hit.
	if (IsDecalApplicable(other, self->s.origin, surface, plane, NULL))
		fx_flags |= CEF_FLAG6;

	gi.CreateEffect(NULL, FX_WEAPON_FLYINGFISTEXPLODE, fx_flags, self->s.origin, "d", self->movedir);
	G_FreeEdict(self); //mxd. G_SetToFree() in original logic. Fixes client effect/dynamic light staying active for 100 ms. after this.
}

static void CreateFlyingFist(edict_t* flying_fist)
{
	flying_fist->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	flying_fist->svflags |= SVF_ALWAYS_SEND;
	flying_fist->movetype = MOVETYPE_FLYMISSILE;
	flying_fist->classname = "Spell_FlyingFist";
	flying_fist->solid = SOLID_BBOX;
	flying_fist->clipmask = MASK_SHOT;

	VectorSet(flying_fist->mins, -FIST_RADIUS, -FIST_RADIUS, -FIST_RADIUS);
	VectorSet(flying_fist->maxs,  FIST_RADIUS,  FIST_RADIUS,  FIST_RADIUS);

	flying_fist->touch = FlyingFistTouch;
	flying_fist->think = FlyingFistInitThink;
	flying_fist->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

edict_t* FlyingFistReflect(edict_t* self, edict_t* other, const vec3_t vel)
{
	// Create a new missile to replace the old one - this is necessary because physics will do nasty things
	// with the existing one, since we hit something. Hence, we create a new one totally.
	edict_t* flying_fist = G_Spawn();

	// Copy everything across.
	CreateFlyingFist(flying_fist);
	VectorCopy(self->s.origin, flying_fist->s.origin);
	VectorCopy(vel, flying_fist->velocity);
	VectorNormalize2(vel, flying_fist->movedir);
	AnglesFromDir(flying_fist->movedir, flying_fist->s.angles);
	flying_fist->owner = other;
	flying_fist->health = self->health;
	flying_fist->flags |= (self->flags & FL_NO_KNOCKBACK); //TODO: why is it reflected as wimpy? Why is the CEF_FLAG8 flag not applied?
	flying_fist->reflect_debounce_time = self->reflect_debounce_time - 1; // So it doesn't infinitely reflect in one frame somehow.
	flying_fist->reflected_time = self->reflected_time;

	G_LinkMissile(flying_fist);

	// Create new trails for the new missile. //TODO: powered/wimpy flags are not carried over!
	gi.CreateEffect(&flying_fist->s, FX_WEAPON_FLYINGFIST, CEF_OWNERS_ORIGIN | CEF_FLAG6, NULL, "t", flying_fist->velocity);

	// Kill the existing missile, since it's a pain in the ass to modify it so the physics won't screw it. 
	G_SetToFree(self);

	// Do a nasty looking blast at the impact point.
	gi.CreateEffect(&flying_fist->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", flying_fist->velocity);

	return flying_fist;
}

void SpellCastFlyingFist(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles)
{
	// Spawn the flying-fist (fireball).
	edict_t* flying_fist = G_Spawn();
	const playerinfo_t* info = &caster->client->playerinfo;

	const qboolean wimpy = (info->pers.inventory.Items[info->weap_ammo_index] < info->pers.weapon->quantity);
	int fx_flags = (wimpy ? CEF_FLAG8 : 0);

	if (info->powerup_timer > level.time)
	{
		// Powered up flying fist. Make it a meteor!
		fx_flags |= CEF_FLAG7;
		flying_fist->health = 1;

		const float volume = (wimpy ? 0.5f : 1.0f); // Play it quiet?
		gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/FireballPowerCast.wav"), volume, ATTN_NORM, 0.0f);
	}
	else
	{
		// Not powered up.
		const char* snd_name = (wimpy ? "weapons/FireballNoMana.wav" : "weapons/FlyingFistCast.wav"); // Play special wimpy sound?
		gi.sound(caster, CHAN_WEAPON, gi.soundindex(snd_name), 1.0f, ATTN_NORM, 0.0f);
	}

	CreateFlyingFist(flying_fist);
	flying_fist->reflect_debounce_time = MAX_REFLECT;
	VectorCopy(start_pos, flying_fist->s.origin);

	if (wimpy)
		flying_fist->flags |= FL_NO_KNOCKBACK; // Just using the no knockback flag to indicate a wussy weapon.

	// Auto-target current enemy?
	if (caster->enemy != NULL)
	{
		// If we have current enemy, we've already traced to its position and can hit it. Also, crosshair is currently aimed at it --mxd.
		GetAimVelocity(caster->enemy, flying_fist->s.origin, FLYING_FIST_SPEED, aim_angles, flying_fist->velocity);
	}
	else
	{
		// Check ahead to see if it's going to hit anything at this angle.
		vec3_t forward;
		AngleVectors(aim_angles, forward, NULL, NULL);

		//mxd. Replicate II_WEAPON_FLYINGFIST case from Get_Crosshair()...
		vec3_t end;
		const vec3_t view_pos = { caster->s.origin[0], caster->s.origin[1], caster->s.origin[2] + (float)caster->viewheight + 20.0f };
		VectorMA(view_pos, FLYING_FIST_SPEED, forward, end);

		trace_t trace;
		gi.trace(view_pos, vec3_origin, vec3_origin, end, caster, MASK_MONSTERSOLID, &trace);

		vec3_t dir;
		VectorSubtract(trace.endpos, flying_fist->s.origin, dir);
		VectorNormalize(dir);
		VectorScale(dir, FLYING_FIST_SPEED, flying_fist->velocity);
	}

	flying_fist->owner = caster;

	// Remember velocity in case we have to reverse it.
	VectorNormalize2(flying_fist->velocity, flying_fist->movedir);

	G_LinkMissile(flying_fist);

	// Make sure we don't start in a solid.
	trace_t back_trace;
	gi.trace(caster->s.origin, vec3_origin, vec3_origin, flying_fist->s.origin, caster, MASK_PLAYERSOLID, &back_trace);

	if (back_trace.startsolid || back_trace.fraction < 1.0f)
	{
		VectorCopy(back_trace.endpos, flying_fist->s.origin);
		FlyingFistTouch(flying_fist, back_trace.ent, &back_trace.plane, back_trace.surface);

		return;
	}

	// Spawn effect after it has been determined it has not started in wall.
	// This is so it won`t try to remove it before it exists.
	gi.CreateEffect(&flying_fist->s, FX_WEAPON_FLYINGFIST, fx_flags | CEF_OWNERS_ORIGIN, NULL, "t", flying_fist->velocity);
}
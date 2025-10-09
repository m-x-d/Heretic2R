//
// spl_Phoenix.c
//
// Copyright 1998 Raven Software
//

#include "spl_Phoenix.h" //mxd
#include "g_playstats.h"
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "Decals.h"
#include "FX.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define ARROW_RADIUS	4.0f

static void PhoenixMissileTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	// Did we hit the sky? 
	if (surface != NULL && (surface->flags & SURF_SKY))
	{
		SkyFly(self);
		return;
	}

	// Are we reflecting?
	if (self->reflect_debounce_time > 0 && EntReflecting(other, true, true))
	{
		Create_rand_relect_vect(self->velocity, self->velocity);
		Vec3ScaleAssign(PHOENIX_ARROW_SPEED / 2.0f, self->velocity);
		PhoenixMissileReflect(self, other, self->velocity);

		return;
	}

	// Kill the travel sound.
	gi.sound(self, CHAN_WEAPON, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f);

	// I'm gonna set the enemy to what I hit... The DAMAGE_ENEMY_MAX assures that what I hit takes full damage.
	// I do this rather than do direct damage AND radius because that tends to do double damage to what is hit...
	self->enemy = ((other != NULL && other->takedamage != DAMAGE_NO) ? other : NULL);

	const qboolean is_powered = (self->health > 0); //mxd

	if (is_powered)
	{
		// Must be powered up version. Storing in health is not so good, though...
		// Powered up Phoenix will NOT damage the shooter. //TODO: doesn't work. Shouldn't 'ignore' arg be set for this to work?
		T_DamageRadius(self, self->owner, NULL, PHOENIX_EXPLODE_RADIUS_POWER,
			PHOENIX_EXPLODE_DAMAGE_POWER, PHOENIX_EXPLODE_DAMAGE_POWER >> 4,
			DAMAGE_NORMAL | DAMAGE_FIRE | DAMAGE_EXTRA_KNOCKBACK | DAMAGE_POWERPHOENIX | DAMAGE_ENEMY_MAX | DAMAGE_PHOENIX,
			MOD_P_PHOENIX_SPL);
	}
	else
	{
		T_DamageRadius(self, self->owner, NULL, PHOENIX_EXPLODE_RADIUS,
			PHOENIX_EXPLODE_DAMAGE, PHOENIX_EXPLODE_DAMAGE >> 4,
			DAMAGE_NORMAL | DAMAGE_FIRE | DAMAGE_EXTRA_KNOCKBACK | DAMAGE_ENEMY_MAX | DAMAGE_PHOENIX,
			MOD_PHOENIX_SPL);
	}

	AlertMonsters(self, self->owner, 3.0f, false);

	// Attempt to apply a scorchmark decal to the thing I hit.
	int fx_flags = CEF_BROADCAST;
	vec3_t plane_dir;
	if (IsDecalApplicable(other, self->s.origin, surface, plane, plane_dir))
		fx_flags |= CEF_FLAG8;

	VectorNormalize2(self->velocity, self->movedir);

	// Start the explosion effect.
	const vec3_t* dir = (Vec3NotZero(plane->normal) ? &plane->normal : &self->movedir); //BUGFIX: mxd. 'if (plane->normal)' in original version (always true).

	if (is_powered)
		fx_flags |= CEF_FLAG6;
	gi.CreateEffect(&self->s, FX_WEAPON_PHOENIXEXPLODE, fx_flags, self->s.origin, "td", *dir, self->movedir);

	VectorClear(self->velocity);

	// Turn off the client effect.
	gi.RemoveEffects(&self->s, FX_WEAPON_PHOENIXMISSILE);

	G_SetToFree(self);
}

static void PhoenixMissileThink(edict_t* self)
{
	self->svflags |= SVF_NOCLIENT;
	self->think = NULL;
}

// Create the guts of a phoenix arrow.
static void CreatePhoenixArrow(edict_t* arrow)
{
	arrow->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	arrow->svflags |= SVF_ALWAYS_SEND;
	arrow->movetype = MOVETYPE_FLYMISSILE;
	arrow->classname = "Spell_PhoenixArrow";
	arrow->solid = SOLID_BBOX;
	arrow->clipmask = MASK_SHOT;

	VectorSet(arrow->mins, -ARROW_RADIUS, -ARROW_RADIUS, -ARROW_RADIUS);
	VectorSet(arrow->maxs,  ARROW_RADIUS,  ARROW_RADIUS,  ARROW_RADIUS);

	arrow->touch = PhoenixMissileTouch;
	arrow->think = PhoenixMissileThink;
	arrow->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

edict_t* PhoenixMissileReflect(edict_t* self, edict_t* other, vec3_t vel)
{
	// Create a new missile to replace the old one - this is necessary because physics will do nasty things
	// with the existing one, since we hit something. Hence, we create a new one totally.
	edict_t* arrow = G_Spawn();

	// Copy everything across.
	CreatePhoenixArrow(arrow);
	VectorCopy(vel, arrow->velocity);
	VectorCopy(self->s.origin, arrow->s.origin);
	arrow->owner = other;
	arrow->enemy = self->owner;
	arrow->health = self->health;
	arrow->reflect_debounce_time = self->reflect_debounce_time - 1; // So it doesn't infinitely reflect in one frame somehow.
	arrow->reflected_time = self->reflected_time;

	G_LinkMissile(arrow);

	// Create new trails for the new missile.
	gi.CreateEffect(&arrow->s, FX_WEAPON_PHOENIXMISSILE, CEF_OWNERS_ORIGIN | (arrow->health << 5) | CEF_FLAG8, NULL, "t", arrow->velocity);

	// Kill the existing missile, since its a pain in the ass to modify it so the physics won't screw it.
	G_SetToFree(self);

	// Play travel sound on the arrow itself - the one on the original arrow will be deleted when the object is removed.
	arrow->s.sound = (byte)gi.soundindex("weapons/PhoenixTravel.wav");
	arrow->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
	self->s.sound = 0; // But just to be safe...

	// Do a nasty looking blast at the impact point.
	gi.CreateEffect(&arrow->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", arrow->velocity);

	return arrow;
}

void SpellCastPhoenix(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles)
{
	edict_t* arrow = G_Spawn();

	CreatePhoenixArrow(arrow);
	VectorCopy(start_pos, arrow->s.origin);

	// Check ahead first to see if it's going to hit anything at this angle.
	vec3_t forward;
	AngleVectors(aim_angles, forward, NULL, NULL);

	vec3_t end_pos;
	VectorMA(start_pos, PHOENIX_ARROW_SPEED, forward, end_pos);

	trace_t trace;
	gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, caster, MASK_MONSTERSOLID, &trace);

	if (trace.ent != NULL && OkToAutotarget(caster, trace.ent))
		VectorScale(forward, PHOENIX_ARROW_SPEED, arrow->velocity); // Already going to hit a valid target at this angle - so don't auto-target.
	else
		GetAimVelocity(caster->enemy, arrow->s.origin, PHOENIX_ARROW_SPEED, aim_angles, arrow->velocity); // Auto-target current enemy.

	VectorCopy(aim_angles, arrow->s.angles);
	arrow->reflect_debounce_time = MAX_REFLECT;
	arrow->owner = caster;

	G_LinkMissile(arrow);

	// Play travel sound on the arrow itself.
	arrow->s.sound = (byte)gi.soundindex("weapons/PhoenixTravel.wav");
	arrow->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;

	// Play firing sound on the player.
	const qboolean is_powered = (caster->client->playerinfo.powerup_timer > level.time); //mxd
	const char* snd_name = (is_powered ? "weapons/PhoenixPowerFire.wav" : "weapons/PhoenixFire.wav"); //mxd
	gi.sound(caster, CHAN_WEAPON, gi.soundindex(snd_name), 1.0f, ATTN_NORM, 0.0f);
	arrow->health = is_powered;

	// Remove the bow ready sound.
	caster->s.sound = 0;

	// Trace from the player's origin because then if we hit a wall, the effect won't be inside it...
	trace_t back_trace;
	gi.trace(caster->s.origin, arrow->mins, arrow->maxs, arrow->s.origin, caster, MASK_PLAYERSOLID, &back_trace);

	if (back_trace.startsolid || back_trace.fraction < 0.99f)
	{
		const vec3_t* pos = (back_trace.startsolid ? &caster->s.origin : &back_trace.endpos); //mxd
		VectorCopy(*pos, arrow->s.origin);
		PhoenixMissileTouch(arrow, back_trace.ent, &back_trace.plane, back_trace.surface);
	}
	else
	{
		int fx_flags = CEF_OWNERS_ORIGIN; //mxd
		if (is_powered)
			fx_flags |= CEF_FLAG6;

		gi.CreateEffect(&arrow->s, FX_WEAPON_PHOENIXMISSILE, fx_flags, NULL, "t", arrow->velocity);
	}
}
//
// spl_tornado.c
//
// Copyright 1998 Raven Software
//

#include "spl_tornado.h" //mxd
#include "spl_BlueRing.h" //mxd
#include "g_combat.h" //mxd
#include "g_playstats.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h" //mxd
#include "Vector.h"
#include "g_local.h"

#define TORNADO_EFFECT_RADIUS	100.0f //mxd. Named 'TORN_EFFECT_RADIUS' in original logic.
#define TORNADO_KNOCKBACK_SCALE	200.0f //mxd. Named 'TORN_KNOCKBACK_SCALE' in original logic.
#define TORNADO_KNOCKBACK_BASE	250.0f //mxd. Named 'TORN_KNOCKBACK_BASE' in original logic.
#define TORNADO_MASS_FACTOR		200.0f //mxd. Named 'TORN_MASS_FACTOR' in original logic.

// Do the think for the tornado ring.
void TornadoThink(edict_t* self)
{
	edict_t* ent = NULL;

	// Stolen wholesale from the ring of repulsion...
	while ((ent = FindRingRadius(ent, self->s.origin, TORNADO_EFFECT_RADIUS, self)) != NULL)
	{
		if (ent->mass == 0 || ent == self->owner)
			continue;

		vec3_t vel;
		VectorSubtract(ent->s.origin, self->s.origin, vel);

		float scale = (TORNADO_EFFECT_RADIUS - VectorLength(vel)) * (TORNADO_KNOCKBACK_SCALE / TORNADO_EFFECT_RADIUS) * sqrtf(TORNADO_MASS_FACTOR / (float)ent->mass) + TORNADO_KNOCKBACK_BASE;
		scale *= 20.0f; // Just for yucks.

		VectorNormalize(vel);
		Vec3ScaleAssign(scale, vel);
		vel[2] += 200.0f;

		// Vel is just passing the direction of the knockback.
		G_PostMessage(ent, MSG_REPULSE, PRI_DIRECTIVE, "fff", vel[0], vel[1], vel[2]);

		// Double the damage if this tornado is powered up.
		if (ent->takedamage != DAMAGE_NO)
		{
			// Do a nasty looking blast at the impact point.
			gi.CreateEffect(&ent->s, FX_LIGHTNING_HIT, CEF_OWNERS_ORIGIN, NULL, "t", ent->velocity);
			VectorClear(ent->velocity);

			// No damage if reflection is on.
			const int damage = (EntReflecting(ent, true, true) ? 0 : TORNADO_DAMAGE);

			vec3_t hit_pos;
			VectorMA(ent->s.origin, -ent->maxs[0], vel, hit_pos);

			T_Damage(ent, ent, self->targetEnt, vel, hit_pos, vec3_origin, damage, 600, DAMAGE_RADIUS | DAMAGE_SPELL, MOD_TORN);
		}
	}

	if (self->jump_time < level.time)
	{
		const vec3_t angles = { flrand(ANGLE_0, ANGLE_360), flrand(ANGLE_0, ANGLE_360), ANGLE_0 };

		vec3_t end_pos;
		DirFromAngles(angles, end_pos);

		Vec3ScaleAssign(flrand(0.0f, 110.0f), end_pos);
		end_pos[2] = 100.0f;

		Vec3AddAssign(self->s.origin, end_pos);

		gi.CreateEffect(NULL, FX_LIGHTNING, 0, self->s.origin, "vbb", end_pos, (byte)RED_RAIN_LIGHTNING_WIDTH, (byte)0);
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/Lightning.wav"), 1.0f, ATTN_NORM, 0.0f);

		self->jump_time = level.time + flrand(0.2f, 1.0f);
	}

	if ((float)self->count < level.time)
	{
		self->think = G_SetToFree;
		self->s.effects &= ~EF_SPEED_ACTIVE;
		self->s.effects &= ~EF_HIGH_MAX;

		self->nextthink = level.time + 1.0f;
	}
	else
	{
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.
	}
}

// Time's up, create the tornado effect.
void TornadoCreateThink(edict_t* tornado) //mxd. Named 'create_tornado' in original logic.
{
	tornado->classname = "Spell_Tornado";
	tornado->timestamp = level.time;
	tornado->count = (int)(level.time + TORNADO_SPIN_DURATION);
	tornado->gravity = 0.0f;
	tornado->alert_time = level.time + flrand(0.6f, 1.2f);
	tornado->jump_time = level.time + flrand(0.2f, 1.0f);

	tornado->s.sound = (byte)gi.soundindex("weapons/tornadospell.wav");
	tornado->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_NORM;
	tornado->s.effects |= EF_SPEED_ACTIVE;

	tornado->think = TornadoThink;
	tornado->nextthink = level.time + FRAMETIME; //mxd. Use define.

	tornado->PersistantCFX = gi.CreatePersistantEffect(&tornado->s, FX_TORNADO, CEF_BROADCAST | CEF_OWNERS_ORIGIN, NULL, "");
}

// We just cast/dropped the tornado, set up a timer so it doesn't erupt immediately and hit the caster.
void SpellCastDropTornado(edict_t* caster, const vec3_t start_pos)
{
	static const char* spawn_checks[] = { "info_player_start", "info_player_deathmatch", "info_player_coop" }; //mxd. Made static const.

	edict_t* tornado = G_Spawn();

	tornado->movetype = PHYSICSTYPE_NONE;
	tornado->classname = "Spell_Tornado_time";
	tornado->think = TornadoCreateThink;
	tornado->nextthink = level.time + TORNADO_DURATION;
	tornado->takedamage = DAMAGE_NO;
	tornado->owner = caster;

	// Use the speed active ef_flag to tell the client effect when the effect is over.
	tornado->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	tornado->svflags |= SVF_ALWAYS_SEND;
	tornado->solid = SOLID_NOT;
	tornado->clipmask = MASK_SOLID;
	tornado->targetEnt = caster;

	VectorCopy(start_pos, tornado->s.origin);
	tornado->s.origin[2] += 1.0f;

	gi.linkentity(tornado);

	const vec3_t end = VEC3_INITA(tornado->s.origin, 0.0f, 0.0f, -256.0f);

	trace_t trace;
	gi.trace(tornado->s.origin, NULL, NULL, end, tornado, MASK_SOLID, &trace);

	VectorCopy(trace.endpos, tornado->s.origin);
	tornado->s.origin[2] += 3.0f; //BUGFIX: mxd. Set BEFORE VectorCopy() in original version.

	//TODO: do spawnpoint check in DM only (meaningless in SP, mostly meaningless in COOP)?
	// Check to see if we are over a spawn point - this won't catch specific teleport arrival points, but will get some of them.
	int game_type = 0;
	if (DEATHMATCH)
		game_type = 1;
	else if (COOP)
		game_type = 2;

	// Search for spawn points.
	edict_t* spot = NULL;
	while ((spot = G_Find(spot, FOFS(classname), spawn_checks[game_type])) != NULL)
	{
		// If we are over a spawn spot, explode the tornado immediately.
		vec3_t diff;
		VectorSubtract(spot->s.origin, tornado->s.origin, diff);

		if (VectorLength(diff) < 80.0f)
		{
			tornado->think = G_SetToFree;
			tornado->nextthink = level.time + FRAMETIME; //mxd. Use define.
			gi.CreateEffect(NULL, FX_TORNADO_BALL_EXPLODE, 0, tornado->s.origin, "");

			return;
		}
	}

	gi.CreateEffect(&tornado->s, FX_TORNADO_BALL, CEF_OWNERS_ORIGIN, NULL, "");
}
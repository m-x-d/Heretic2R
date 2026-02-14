//
// spl_ripper.c
//
// Copyright 1998 Raven Software
//

#include "spl_ripper.h" //mxd
#include "g_playstats.h"
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "m_beast.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h" //mxd
#include "Vector.h"
#include "g_local.h"

#define RIPPER_RADIUS				12.0f
#define RIPPER_EXPLODE_BALL_RADIUS	8.0f
#define RIPPER_MAX_DISTANCE			2000.0f

void RipperExplodeBallThink(edict_t* self)
{
	trace_t trace;

	vec3_t start_pos = VEC3_INIT(self->s.origin);
	const vec3_t end_pos = VEC3_INIT(self->last_org);

	const edict_t* trace_buddy = self;
	int num_hit = 0; // Can't hit more than 6 guys...

	do
	{
		gi.trace(start_pos, self->mins, self->maxs, end_pos, trace_buddy, MASK_SHOT, &trace);

		// If we hit anything that won't take damage, kill the beam.
		if (trace.ent->takedamage == DAMAGE_NO || trace.fraction >= 0.99f)
			break;

		// Did we hit something that reflects?
		if (!EntReflecting(trace.ent, true, true))
		{
			T_Damage(trace.ent, self, self->owner, self->movedir, trace.endpos, vec3_origin, self->dmg, 0, DAMAGE_SPELL | DAMAGE_EXTRA_BLOOD, MOD_IRONDOOM);

			if (trace.ent->svflags & SVF_MONSTER)
			{
				// Let's spawn a big gush of blood in the traveling direction.
				vec3_t vel;
				VectorScale(self->movedir, RIPPER_EXPLODE_SPEED * 0.25f, vel);

				const int fx_flags = (trace.ent->materialtype == MAT_INSECT ? CEF_FLAG8 : 0); //mxd
				gi.CreateEffect(NULL, FX_BLOOD, fx_flags, self->last_org, "ub", vel, (byte)20);

				gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/RipperDamage.wav"), 1.0f, ATTN_NORM, 0.0f);
			}
		}

		// This seems to alleviate the problem of a trace hitting the same ent multiple times...
		VectorCopy(trace.endpos, start_pos);

		if (VectorSeparation(end_pos, start_pos) > 16.0f)
		{
			vec3_t forward;
			VectorSubtract(start_pos, end_pos, forward);
			VectorNormalize(forward);
			VectorMA(start_pos, 16.0f, forward, start_pos);
		}

		trace_buddy = trace.ent;
		num_hit++;
	} while (!(trace.contents & MASK_SOLID) && num_hit < 6);

	// Prevent any further transmission of this entity to clients.
	self->svflags |= SVF_NOCLIENT;

	VectorCopy(self->s.origin, self->last_org);
	self->nextthink = level.time + FRAMETIME; //mxd. Use define.
}

void RipperExplodeBallTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surface)
{
	G_SetToFree(self);
}

// This is like a touch function, except since the ripper is instant now...
static void RipperImpact(edict_t* caster, edict_t* other, const vec3_t start_pos, const vec3_t end_pos, const vec3_t angles)
{
	short ball_array[RIPPER_BALLS];

	// Get the forward vector for various calculations.
	vec3_t fwd;
	AngleVectors(angles, fwd, NULL, NULL);

	AlertMonsters(caster, caster, 2.0f, false);

	vec3_t hit_pos;
	if (other != NULL && other->takedamage != DAMAGE_NO)
	{
		const int dmg = irand(RIPPER_DAMAGE_MIN, RIPPER_DAMAGE_MAX);
		VectorCopy(end_pos, hit_pos);
		T_Damage(other, caster, caster, fwd, end_pos, fwd, dmg, dmg * 2, DAMAGE_SPELL, MOD_IRONDOOM);
	}
	else
	{
		// Back off the origin for the damage a bit.
		// We are a point and this will help fix hitting base of a stair and not hurting a guy on next step up.
		VectorMA(end_pos, -8.0f, fwd, hit_pos);
	}

	// Shoot out ripper balls. Add half-increment so that the balls don't come right back.
	float cur_yaw = (angles[YAW] * ANGLE_TO_RAD) + (RIPPER_BALL_ANGLE * 0.5f);

	// Store the current yaw in radians for sin/cos operations.
	const byte b_yaw = (byte)(cur_yaw * RAD_TO_BYTEANGLE); // Reduce precision to a 0-255 byte...
	cur_yaw = (float)b_yaw * BYTEANGLE_TO_RAD; // ...and pass this imprecision back to the float yaw.
	edict_t* ripper = NULL;

	for (int i = 0; i < RIPPER_BALLS; i++)
	{
		ripper = G_Spawn();

		// Set up the angle vectors.
		VectorSet(ripper->movedir, cosf(cur_yaw), sinf(cur_yaw), 0.0f);

		// Place the ball.	
		VectorCopy(hit_pos, ripper->s.origin);
		VectorScale(ripper->movedir, RIPPER_EXPLODE_SPEED, ripper->velocity);

		// Set up the net transmission attributes.
		ripper->s.effects |= EF_ALWAYS_ADD_EFFECTS;
		ripper->svflags |= SVF_ALWAYS_SEND;
		ripper->movetype = MOVETYPE_FLYMISSILE;

		// Set up the dimensions.
		VectorSet(ripper->mins, -RIPPER_EXPLODE_BALL_RADIUS, -RIPPER_EXPLODE_BALL_RADIUS, -RIPPER_EXPLODE_BALL_RADIUS);
		VectorSet(ripper->maxs,  RIPPER_EXPLODE_BALL_RADIUS,  RIPPER_EXPLODE_BALL_RADIUS,  RIPPER_EXPLODE_BALL_RADIUS);

		// Set up physics attributes.
		ripper->solid = SOLID_BBOX;
		ripper->clipmask = MASK_SOLID;
		ripper->classname = "Spell_Ripper";
		ripper->dmg = RIPPER_EXPLODE_DAMAGE;
		ripper->owner = caster;

		// last_org is used for damaging things.
		VectorCopy(ripper->s.origin, ripper->last_org);

		ripper->touch = RipperExplodeBallTouch;
		ripper->think = RipperExplodeBallThink;
		ripper->nextthink = level.time + FRAMETIME; //mxd. Use define.

		// Store the entity numbers for sending with the effect.
		ball_array[i] = ripper->s.number;

		cur_yaw += RIPPER_BALL_ANGLE;
	}

	gi.sound(ripper, CHAN_WEAPON, gi.soundindex("weapons/RipperImpact.wav"), 1.0f, ATTN_NORM, 0.0f);

	// Now we send the effect. There's a lot here, but this is WAY more efficient than sending out eight
	// (well, nine with the impact) separate effects...

	// Send it attached to the last ball...
	assert(ripper != NULL);
	assert(RIPPER_BALLS == 8);

	// So we send this with:
	//	-- The position of the caster (for the launch trail effect).
	//	-- The byte angle to shoot things at.
	//	-- All eight entity numbers to attach things to.  
	//		Okay, so I only need seven (the eighth comes with the effect), but for consistency I'm going to send all eight.
	//	(NOTE: Further optimization would maybe pass a pitch and distance, and make the yaw precise, but that would save
	//		little and make things more confusing than they are...).
	//	Currently this sends 29 bytes plus the entity it's attached to.
	//		This is down from (12 plus the entity) times eight, plus another effect for impact, plus another for the trail.
	gi.CreateEffect(&ripper->s, FX_WEAPON_RIPPEREXPLODE, CEF_OWNERS_ORIGIN, NULL, "vbssssssss",
		start_pos, b_yaw, ball_array[0], ball_array[1], ball_array[2], ball_array[3], ball_array[4], ball_array[5], ball_array[6], ball_array[7]);
}

void SpellCastRipper(edict_t* caster, const vec3_t start_pos, const vec3_t aim_angles)
{
	static const vec3_t mins = { -RIPPER_RADIUS, -RIPPER_RADIUS, -RIPPER_RADIUS }; //mxd. Made static.
	static const vec3_t maxs = {  RIPPER_RADIUS,  RIPPER_RADIUS,  RIPPER_RADIUS }; //mxd. Made static.

	gi.sound(caster, CHAN_WEAPON, gi.soundindex("weapons/RipperFire.wav"), 1.0f, ATTN_NORM, 0.0f);

	// Make sure we don't spawn in a wall.
	trace_t trace;
	gi.trace(caster->s.origin, mins, maxs, start_pos, caster, MASK_PLAYERSOLID, &trace);

	if (trace.startsolid || trace.fraction < 0.99f)
	{
		RipperImpact(caster, trace.ent, caster->s.origin, trace.endpos, aim_angles);
		return;
	}

	vec3_t angles;
	AdjustAimAngles(caster, start_pos, aim_angles, 21.0f, angles); //mxd

	// Get the forward angle.
	vec3_t forward;
	AngleVectors(angles, forward, NULL, NULL);

	// Now trace from the starting point to the final destination.
	vec3_t end_pos;
	VectorMA(start_pos, RIPPER_MAX_DISTANCE, forward, end_pos);

	gi.trace(start_pos, mins, maxs, end_pos, caster, MASK_SHOT, &trace);

	if (level.fighting_beast)
	{
		edict_t* ent = TBeastCheckHit(start_pos, trace.endpos);

		if (ent != NULL)
			trace.ent = ent;
	}

	RipperImpact(caster, trace.ent, start_pos, trace.endpos, angles);
}
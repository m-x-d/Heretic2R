//
// g_PhysicsQ2.c -- mxd. g_phys.c in original version.
//
// Copyright 1998 Raven Software
//

#include "g_local.h"
#include "g_Physics.h" //mxd
#include "Vector.h"

static void SV_CheckVelocity(edict_t* ent)
{
	// Bound velocity.
	for (int i = 0; i < 3; i++)
		ent->velocity[i] = Clamp(ent->velocity[i], -sv_maxvelocity->value, sv_maxvelocity->value);
}

// Runs thinking code for this frame if necessary.
static qboolean SV_RunThink(edict_t* ent)
{
	const float think_time = ent->nextthink;

	assert(ent->inuse);

	if (ent->msgHandler != NULL)
		G_ProcessMessages(ent);

	assert(ent->inuse);

	if (ent->think != NULL && think_time > 0.0f && think_time <= level.time + 0.001f)
	{
		ent->nextthink = THINK_NEVER; //mxd. '0' in original logic. Changed for consistency sake.
		ent->think(ent);
	}

	return true; //NOTE: is this what we want to return if it gets this far?
}

// Two entities have touched, so run their touch functions.
static void SV_Impact(edict_t* e1, trace_t* trace)
{
	edict_t* e2 = trace->ent;

	if (e1->touch != NULL && e1->solid != SOLID_NOT)
		e1->touch(e1, e2, &trace->plane, trace->surface);

	if (e2->touch != NULL && e2->solid != SOLID_NOT)
		e2->touch(e2, e1, NULL, NULL);
}

// Slide off of the impacting object.
static void ClipVelocity(const vec3_t in, const vec3_t normal, vec3_t out) //mxd. Removed return value (unused), removed non-MOVETYPE_FLYMISSILE logic.
{
	const float backoff = DotProduct(in, normal);

	for (int i = 0; i < 3; i++)
	{
		const float change = normal[i] * backoff;
		out[i] = in[i] - change;

		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0.0f;
	}
}

// Does not change the entities velocity at all.
static trace_t SV_PushEntity(edict_t* ent, vec3_t push) //mxd. Removed non-MOVETYPE_FLYMISSILE logic.
{
	static trace_t trace; //mxd. Made local static.

	const vec3_t start = VEC3_INIT(ent->s.origin);

	vec3_t end;
	VectorAdd(start, push, end);

	while (true) //mxd. Use while loop instead of goto.
	{
		const int mask = (ent->clipmask != 0 ? ent->clipmask : MASK_SOLID);
		gi.trace(start, ent->mins, ent->maxs, end, ent, mask, &trace);

		VectorCopy(trace.endpos, ent->s.origin);
		G_LinkMissile(ent);

		if (trace.fraction != 1.0f)
		{
			SV_Impact(ent, &trace);

			// If the pushed entity went away and the pusher is still there...
			if (!trace.ent->inuse && ent->inuse)
			{
				//...move the pusher back and try again.
				VectorCopy(start, ent->s.origin);
				gi.linkentity(ent);

				continue; // Try again...
			}
		}

		if (ent->inuse)
			G_TouchTriggers(ent);

		return trace;
	}
}

// Fly movement. When on ground, do nothing.
static void SV_Physics_Toss(edict_t* ent) //mxd. Removed non-MOVETYPE_FLYMISSILE logic.
{
	// Regular thinking.
	SV_RunThink(ent);

	// If not a team captain, so movement will be handled elsewhere.
	if (ent->flags & FL_TEAMSLAVE) //TODO: ever used on missiles?
		return;

	// Check for the groundentity going away.
	if (ent->velocity[2] > 0.0f || (ent->groundentity != NULL && !ent->groundentity->inuse))
		ent->groundentity = NULL;

	// If on ground, return without moving.
	if (ent->groundentity != NULL)
		return;

	SV_CheckVelocity(ent);

	// Move angles.
	VectorMA(ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);

	// Move origin.
	vec3_t move;
	VectorScale(ent->velocity, FRAMETIME, move);

	const trace_t trace = SV_PushEntity(ent, move);
	if (!ent->inuse)
		return;

	if (trace.fraction < 1.0f)
	{
		ClipVelocity(ent->velocity, trace.plane.normal, ent->velocity);

		// Stop if on ground.
		if (trace.plane.normal[2] > 0.7f)
		{
			ent->groundentity = trace.ent;
			ent->groundentity_linkcount = trace.ent->linkcount;
			VectorCopy(vec3_origin, ent->velocity);
			VectorCopy(vec3_origin, ent->avelocity);
		}
	}

	// Check for water transition.
	PhysicsCheckWaterTransition(ent);

	// Move teamslaves. //TODO: ever used on missiles?
	for (edict_t* slave = ent->teamchain; slave != NULL; slave = slave->teamchain)
	{
		VectorCopy(ent->s.origin, slave->s.origin);
		gi.linkentity(slave);
	}
}

//mxd. Used only for MOVETYPE_FLYMISSILE.
void G_RunEntity(edict_t* ent)
{
	assert(ent->inuse);
	assert(ent->movetype == MOVETYPE_FLYMISSILE);

	SV_Physics_Toss(ent);
}
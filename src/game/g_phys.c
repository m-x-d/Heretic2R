//
// g_PhysicsQ2.h
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
		ProcessMessages(ent);

	assert(ent->inuse);

	if (ent->think != NULL && think_time > 0.0f && think_time <= level.time + 0.001f)
	{
		ent->nextthink = 0.0f;
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

/*
==================
ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1

int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i, blocked;
	
	blocked = 0;
	if (normal[2] > 0)
		blocked |= 1;		// floor
	if (!normal[2])
		blocked |= 2;		// step
	
	backoff = DotProduct (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
	
	return blocked;
}

/*
============
SV_AddGravity

============
*/
void SV_AddGravity(edict_t* ent)
{
	ent->velocity[2] -= ent->gravity * sv_gravity->value * FRAMETIME;
}

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
SV_PushEntity

Does not change the entities velocity at all
============
*/
trace_t SV_PushEntity (edict_t *ent, vec3_t push)
{
	trace_t	trace;
	vec3_t	start;
	vec3_t	end;
	int		mask;

	VectorCopy (ent->s.origin, start);
	VectorAdd (start, push, end);

retry:
	if (ent->clipmask)
		mask = ent->clipmask;
	else
		mask = MASK_SOLID;

	gi.trace (start, ent->mins, ent->maxs, end, ent, mask,&trace);
	
	VectorCopy (trace.endpos, ent->s.origin);

	if (ent->movetype == MOVETYPE_FLYMISSILE)
	{
		G_LinkMissile(ent);
	}
	else
	{
		gi.linkentity (ent);
	}

	if (trace.fraction != 1.0)
	{
		SV_Impact (ent, &trace);

		// if the pushed entity went away and the pusher is still there
		if (!trace.ent->inuse && ent->inuse)
		{
			// move the pusher back and try again
			VectorCopy (start, ent->s.origin);
			gi.linkentity (ent);
			goto retry;
		}
	}

	if (ent->inuse)
		G_TouchTriggers (ent);

	return trace;
}					

/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
void PhysicsCheckWaterTransition(edict_t *self);
void SV_Physics_Toss (edict_t *ent)
{
	trace_t		trace;
	vec3_t		move;
	float		backoff;
	edict_t		*slave;
//	qboolean	wasinwater;
//	qboolean	isinwater;
	vec3_t		old_origin;

// regular thinking
	SV_RunThink (ent);

	// if not a team captain, so movement will be handled elsewhere
	if ( ent->flags & FL_TEAMSLAVE)
		return;

	if (ent->velocity[2] > 0)
		ent->groundentity = NULL;

// check for the groundentity going away
	if (ent->groundentity)
		if (!ent->groundentity->inuse)
			ent->groundentity = NULL;

// if onground, return without moving
	if ( ent->groundentity )
		return;

	VectorCopy (ent->s.origin, old_origin);

	SV_CheckVelocity (ent);

// add gravity
	if (ent->movetype != MOVETYPE_FLY
	&& ent->movetype != MOVETYPE_FLYMISSILE)
		SV_AddGravity (ent);

// move angles
	VectorMA (ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);

// move origin
	VectorScale (ent->velocity, FRAMETIME, move);
	trace = SV_PushEntity (ent, move);
	if (!ent->inuse)
		return;

	if (trace.fraction < 1)
	{
		if (ent->movetype == MOVETYPE_BOUNCE)
			backoff = 1.5;
		else
			backoff = 1;

		ClipVelocity (ent->velocity, trace.plane.normal, ent->velocity, backoff);

	// stop if on ground
		if (trace.plane.normal[2] > 0.7)
		{		
			if (ent->velocity[2] < 60 || ent->movetype != MOVETYPE_BOUNCE )
			{
				ent->groundentity = trace.ent;
				ent->groundentity_linkcount = trace.ent->linkcount;
				VectorCopy (vec3_origin, ent->velocity);
				VectorCopy (vec3_origin, ent->avelocity);
			}
		}

//		if (ent->touch)
//			ent->touch (ent, trace.ent, &trace.plane, trace.surface);
	}
	
// check for water transition
	PhysicsCheckWaterTransition(ent);

/*
	wasinwater = (ent->watertype & MASK_WATER);
	ent->watertype = gi.pointcontents (ent->s.origin);
	isinwater = ent->watertype & MASK_WATER;

	if (isinwater)
		ent->waterlevel = 1;
	else
		ent->waterlevel = 0;

	if (!wasinwater && isinwater)
		gi.positioned_sound (old_origin, g_edicts, CHAN_AUTO, gi.soundindex("misc/h2ohit1.wav"), 1, 1, 0);
	else if (wasinwater && !isinwater)
		gi.positioned_sound (ent->s.origin, g_edicts, CHAN_AUTO, gi.soundindex("misc/h2ohit1.wav"), 1, 1, 0);
*/
// move teamslaves
	for (slave = ent->teamchain; slave; slave = slave->teamchain)
	{
		VectorCopy (ent->s.origin, slave->s.origin);
		gi.linkentity (slave);
	}
}

//============================================================================
/*
================
G_RunEntity

================
*/
//mxd. Used only for MOVETYPE_FLYMISSILE.
void G_RunEntity(edict_t* ent)
{
	assert(ent->inuse);
	assert(ent->movetype == MOVETYPE_FLYMISSILE);

	if (ent->prethink != NULL)
		ent->prethink(ent);

	SV_Physics_Toss(ent);
}
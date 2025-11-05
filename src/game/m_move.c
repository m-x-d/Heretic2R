//
// m_move.c -- Monster movement.
//
// Copyright 1998 Raven Software
//

#include "m_move.h" //mxd
#include "mg_ai.h" //mxd
#include "Vector.h"
#include "g_local.h"

static qboolean SimpleBottomCornersCheck(const vec3_t mins, const vec3_t maxs) //mxd. Added to simplify logic a bit.
{
	vec3_t pos;
	pos[2] = mins[2] - 1.0f;

	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			pos[0] = (x == 1 ? maxs[0] : mins[0]);
			pos[1] = (y == 1 ? maxs[1] : mins[1]);

			if (gi.pointcontents(pos) != CONTENTS_SOLID)
				return false;
		}
	}

	return true;
}

// Returns false if any part of the bottom of the entity is off an edge that is not a staircase.
static qboolean M_CheckBottom(const edict_t* ent)
{
	vec3_t mins;
	vec3_t maxs;
	VectorAdd(ent->s.origin, ent->mins, mins);
	VectorAdd(ent->s.origin, ent->maxs, maxs);

	// If all of the points under the corners are solid world, don't bother with the tougher checks.
	// The corners must be within 16 of the midpoint.
	if (SimpleBottomCornersCheck(mins, maxs)) //mxd
		return true; // We got out easy.

	// Check it for real... The midpoint must be within 16 of the bottom.
	vec3_t start = { (mins[0] + maxs[0]) * 0.5f, (mins[1] + maxs[1]) * 0.5f, mins[2] };
	vec3_t stop =  { start[0], start[1], start[2] - STEP_SIZE * 2.0f };

	trace_t	trace;
	gi.trace(start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID, &trace);

	if (trace.fraction == 1.0f)
		return false;

	const float mid = trace.endpos[2];
	float bottom = mid;

	// The corners must be within 16 of the midpoint
	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			start[0] = (x == 1 ? maxs[0] : mins[0]);
			start[1] = (y == 1 ? maxs[1] : mins[1]);
			stop[0] = start[0];
			stop[1] = start[1];

			gi.trace(start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID, &trace);

			if (trace.fraction != 1.0f && trace.endpos[2] > bottom)
				bottom = trace.endpos[2];

			if (trace.fraction == 1.0f || mid - trace.endpos[2] > STEP_SIZE)
				return false;
		}
	}

	return true;
}

static qboolean SV_MoveStep_Walk(edict_t* ent, const vec3_t move, const qboolean relink) //mxd. Split from SV_movestep().
{
	// Try the move.
	vec3_t initial_org;
	VectorCopy(ent->s.origin, initial_org);

	// Push down from a step height above the wished position.
	const float step_size = ((ent->monsterinfo.aiflags & AI_NOSTEP) ? 1.0f : STEP_SIZE);

	vec3_t new_org;
	VectorAdd(ent->s.origin, move, new_org);
	new_org[2] += step_size;

	vec3_t end;
	VectorCopy(new_org, end);
	end[2] -= step_size * 2.0f;

	trace_t trace;
	gi.trace(new_org, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID, &trace);

	if (trace.allsolid)
	{
		G_PostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return false;
	}

	if (trace.startsolid)
	{
		// Can't step up, try down.
		new_org[2] -= step_size;
		gi.trace(new_org, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID, &trace);

		if (trace.allsolid || trace.startsolid)
		{
			G_PostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
			return false;
		}
	}

	// Don't go in to water unless only 40% height deep.
	if (ent->waterlevel == 0)
	{
		// Not currently in water.
		vec3_t pos;
		VectorCopy(trace.endpos, pos);
		pos[2] += ent->mins[2] + (ent->maxs[2] - ent->mins[2]) * 0.4f;

		if (gi.pointcontents(pos) & MASK_WATER) //TODO: MG_MoveStep_Walk() also checks for FL_AMPHIBIAN here.
			return false;
	}

	if (trace.fraction == 1.0f)
	{
		// If monster had the ground pulled out, go ahead and fall.
		if (ent->flags & FL_PARTIALGROUND)
		{
			VectorAdd(ent->s.origin, move, ent->s.origin);

			if (relink)
			{
				gi.linkentity(ent);
				G_TouchTriggers(ent);
			}

			return true;
		}

		G_PostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return false; // Walked off an edge.
	}

	// Check point traces down for dangling corners.
	VectorCopy(trace.endpos, ent->s.origin);

	if (!M_CheckBottom(ent))
	{
		if (ent->flags & FL_PARTIALGROUND)
		{
			// Entity had floor mostly pulled out from underneath it and is trying to correct.
			if (relink)
			{
				gi.linkentity(ent);
				G_TouchTriggers(ent);
			}

			return true;
		}

		// Let's not make that move after all.
		VectorCopy(initial_org, ent->s.origin);
		G_PostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);

		return false;
	}

	if (ent->flags & FL_PARTIALGROUND)
		ent->flags &= ~FL_PARTIALGROUND;

	ent->groundentity = trace.ent;
	ent->groundentity_linkcount = trace.ent->linkcount;

	// The move is ok.
	if (relink)
	{
		gi.linkentity(ent);
		G_TouchTriggers(ent);
	}

	return true;
}

// Called by monster program code.
// The move will be adjusted for slopes and stairs, but if the move isn't possible, no move is done, false is returned,
// and pr_global_struct->trace_normal is set to the normal of the blocking wall.
// FIXME since we need to test end position contents here, can we avoid doing it again later in categorize position?
//TODO: modifies 'move' vector. Is that intentional?
qboolean SV_movestep(edict_t* ent, vec3_t move, const qboolean relink)
{
	// Scale here, not before!
	if (ent->monsterinfo.scale > 0.0f && ent->monsterinfo.scale != 1.0f) //TODO: check cases when ent->monsterinfo.scale == 0.
		Vec3ScaleAssign(ent->monsterinfo.scale, move);

	// Swim and fly monsters. Flying monsters don't step up.
	if (ent->flags & (FL_SWIM | FL_FLY))
	{
		const trace_t tr = MG_MoveStep_SwimOrFly(ent, move, relink); //mxd. Reuse MG_MoveStep_SwimOrFly() logic.
		return tr.succeeded;
	}

	// Walk monsters.
	return SV_MoveStep_Walk(ent, move, relink); //mxd
}

float M_ChangeYaw(edict_t* ent) //TODO: VERY similar to MG_ChangeYaw() (the only difference is anglemod() / anglemod_old() usage). Use MG_ChangeYaw() instead?
{
	const float current = anglemod(ent->s.angles[YAW]);
	const float ideal = ent->ideal_yaw;
	float move = ideal - current;

	if (FloatIsZeroEpsilon(move)) //mxd. Use FloatIsZeroEpsilon() instead of direct comparison.
		return 0.0f;

	if (ideal > current)
	{
		if (move >= 180.0f)
			move -= 360.0f;
	}
	else
	{
		if (move <= -180.0f)
			move += 360.0f;
	}

	move = Clamp(move, -ent->yaw_speed, ent->yaw_speed);
	ent->s.angles[YAW] = anglemod(current + move);

	return move;
}

qboolean M_walkmove(edict_t* ent, float yaw, const float dist)
{
	if (ent->groundentity != NULL || (ent->flags & (FL_FLY | FL_SWIM)))
	{
		yaw *= ANGLE_TO_RAD;
		vec3_t move = { cosf(yaw) * dist, sinf(yaw) * dist, 0.0f };

		return SV_movestep(ent, move, true);
	}

	return false;
}
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

/*
=============
SV_movestep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink)
{
	float		dz;
	vec3_t		oldorg, neworg, end;
	trace_t		trace;
	int			i;
	float		stepsize;
	vec3_t		test;
	int			contents;

// try the move	
	VectorCopy (ent->s.origin, oldorg);
	VectorAdd (ent->s.origin, move, neworg);

	if(ent->monsterinfo.scale)
	{//scale here, not before!
		VectorScale(move, ent->monsterinfo.scale, move);
	}

	// flying monsters don't step up
	if ( ent->flags & (FL_SWIM | FL_FLY) )
	{
	// try one move with vertical motion, then one without
		for (i=0 ; i<2 ; i++)
		{
			VectorAdd (ent->s.origin, move, neworg);
			if (i == 0 && ent->enemy)
			{
				if (!ent->goalentity)
					ent->goalentity = ent->enemy;
				dz = ent->s.origin[2] - ent->goalentity->s.origin[2];
				if (ent->goalentity->client)
				{
					if (dz > 40)
						neworg[2] -= 8;
					if (!((ent->flags & FL_SWIM) && (ent->waterlevel < 2)))
						if (dz < 30)
							neworg[2] += 8;
				}
				else
				{
					if (dz > 8)
						neworg[2] -= 8;
					else if (dz > 0)
						neworg[2] -= dz;
					else if (dz < -8)
						neworg[2] += 8;
					else
						neworg[2] += dz;
				}
			}
			gi.trace (ent->s.origin, ent->mins, ent->maxs, neworg, ent, MASK_MONSTERSOLID,&trace);
	
			// fly monsters don't enter water voluntarily
			if (ent->flags & FL_FLY)
			{
				if (!ent->waterlevel)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (contents & MASK_WATER)
					{
						QPostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
						return false;
					}
				}
			}

			// swim monsters don't exit water voluntarily
			if (ent->flags & FL_SWIM)
			{
				if (ent->waterlevel < 2)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (!(contents & MASK_WATER))
					{
						QPostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
						return false;
					}
				}
			}

			if (trace.fraction == 1)
			{
				VectorCopy (trace.endpos, ent->s.origin);
				if (relink)
				{
					gi.linkentity (ent);
					G_TouchTriggers (ent);
				}
				return true;
			}
			
			if (!ent->enemy)
				break;
		}
		
		return false;
	}

// push down from a step height above the wished position
	if (!(ent->monsterinfo.aiflags & AI_NOSTEP))
		stepsize = STEP_SIZE;
	else
		stepsize = 1;

	neworg[2] += stepsize;
	VectorCopy (neworg, end);
	end[2] -= stepsize*2;

	gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID,&trace);

	if (trace.allsolid)
	{
		QPostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return false;
	}

	if (trace.startsolid)
	{
		neworg[2] -= stepsize;
		gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID,&trace);
		if (trace.allsolid || trace.startsolid)
		{
			QPostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
			return false;
		}
	}


	// don't go in to water unless only 40% hieght deep
	if (ent->waterlevel == 0)
	{
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2] + ent->mins[2];// + 1;
		test[2] += (ent->maxs[2] - ent->mins[2]) * 0.4;
		contents = gi.pointcontents(test);

		if (contents & MASK_WATER)
			return false;
	}

	if (trace.fraction == 1)
	{
	// if monster had the ground pulled out, go ahead and fall
		if ( ent->flags & FL_PARTIALGROUND )
		{
			VectorAdd (ent->s.origin, move, ent->s.origin);
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
//			ent->groundentity = NULL;
//	SV_Printf ("fall down\n"); 
			return true;
		}
		QPostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return false;		// walked off an edge
	}

// check point traces down for dangling corners
	VectorCopy (trace.endpos, ent->s.origin);
	
	if (!M_CheckBottom (ent))
	{
		if ( ent->flags & FL_PARTIALGROUND )
		{	// entity had floor mostly pulled out from underneath it
			// and is trying to correct
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
			return true;
		}
		VectorCopy (oldorg, ent->s.origin);
		QPostMessage(ent, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return false;
	}

	if ( ent->flags & FL_PARTIALGROUND )
	{
//		SV_Printf ("back on ground\n"); 
		ent->flags &= ~FL_PARTIALGROUND;
	}
	ent->groundentity = trace.ent;
	ent->groundentity_linkcount = trace.ent->linkcount;

// the move is ok
	if (relink)
	{
		gi.linkentity (ent);
		G_TouchTriggers (ent);
	}
	return true;
}


//============================================================================

/*
===============
M_ChangeYaw

===============
*/
float M_ChangeYaw (edict_t *ent)
{
	float	ideal;
	float	current;
	float	move;
	float	speed;
	
	current = anglemod(ent->s.angles[YAW]);
	ideal = ent->ideal_yaw;

	if (current == ideal)
		return false;

	move = ideal - current;
	speed = ent->yaw_speed;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}
	
	ent->s.angles[YAW] = anglemod (current + move);
	return move;
}

/*
======================
SV_FixCheckBottom

======================
*/
void SV_FixCheckBottom (edict_t *ent)
{
//	SV_Printf ("SV_FixCheckBottom\n");
	
	ent->flags |= FL_PARTIALGROUND;
}

/*
===============
M_movetoside - move creature to the side determined by the given yaw
===============
*/
void M_movetoside (edict_t *self,float yaw, float dist)
{
	M_walkmove (self, yaw, dist);
}

/*
===============
M_walkmove
===============
*/
qboolean M_walkmove (edict_t *ent, float yaw, float dist)
{
	vec3_t	move;
	
	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)))
		return false;

	yaw = yaw*M_PI*2 / 360;
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	return SV_movestep(ent, move, true);
}

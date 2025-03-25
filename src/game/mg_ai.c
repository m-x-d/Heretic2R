//
// mg_ai.c -- New low-level movement code.
//
// Copyright 1998 Raven Software
//

#include "mg_ai.h" //mxd
#include "mg_guide.h" //mxd
#include "g_monster.h"
#include "g_HitLocation.h"
#include "m_beast.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define YAW_IDEAL		true
#define YAW_BEST_MOVE	false

void ssithraCheckJump(edict_t* self); //TODO: move to m_plagueSsithra.h.
void SV_FixCheckBottom(edict_t* ent); //TODO: move to m_move.h
float ai_face_goal(edict_t* self); //TODO: move to g_ai.h

// Returns true if the entity is in front (dot > 0.8) of self.
qboolean ahead(const edict_t* self, const edict_t* other) //TODO: rename to MG_IsAheadOf?
{
	vec3_t check_angles;

	if (Vec3NotZero(self->v_angle_ofs))
		VectorAdd(self->v_angle_ofs, self->s.angles, check_angles);
	else
		VectorCopy(self->s.angles, check_angles);

	vec3_t forward;
	AngleVectors(check_angles, forward, NULL, NULL);

	vec3_t vec;
	VectorSubtract(other->s.origin, self->s.origin, vec);
	VectorNormalize(vec);

	return (DotProduct(vec, forward) > 0.8f);
}

// Returns true if there is an unobstructed spot from start to end.
static qboolean HaveLOS(const edict_t* self, const vec3_t start, const vec3_t end) //mxd. Named 'LOS' in original logic.
{
	trace_t trace;
	gi.trace(start, vec3_origin, vec3_origin, end, self, MASK_SOLID, &trace);

	return (trace.fraction == 1.0f);
}

// Returns true if the entity is visible to self, even if not infront().
qboolean visible_pos(const edict_t* self, const vec3_t pos) //TODO: rename to MG_IsVisiblePos?
{
	vec3_t start;
	VectorCopy(self->s.origin, start);
	start[2] += (float)self->viewheight;

	trace_t trace;
	gi.trace(start, vec3_origin, vec3_origin, pos, self, MASK_OPAQUE, &trace);

	return (trace.fraction == 1.0f);
}

// Returns false if any 2 adjacent cornerpoints of the bottom of the entity are off an edge that is not a staircase.
static qboolean MG_CheckBottom(edict_t* ent)
{
	// Normal corner checking.
	if (ent->classID == CID_TBEAST)
		return TB_CheckBottom(ent);

	// Lenient, max 16 corner checking.
	vec3_t mins;
	vec3_t maxs;
	VectorCopy(ent->mins, mins);
	VectorCopy(ent->maxs, maxs);

	// Keep corner checks within 16 of center.
	for (int i = 0; i < 2; i++)
	{
		// Some leniency is ok here, no?
		mins[i] = max(-16.0f, mins[i] * 0.75f);
		maxs[i] = min(16.0f, maxs[i] * 0.75f);
	}

	float step_size = 0.0f;

	if (ent->maxs[0] > maxs[0])
		step_size = STEP_SIZE + (ent->maxs[0] - maxs[0]);

	VectorAdd(ent->s.origin, mins, mins);
	VectorAdd(ent->s.origin, maxs, maxs);

	// If all of the points under the corners are solid world, don't bother with the tougher checks.
	qboolean corner_ok[4];
	qboolean easy_ok[2][2];
	qboolean do_real_check = false;

	vec3_t check_pos = { 0.0f, 0.0f, mins[2] - 1.0f };

	// The corners must be within 16 of the midpoint.
	int corner_index = 0;

	for (int x = 0; x < 2; x++) // 0, 0; 0, 1; 1, 0; 1, 1;
	{
		for (int y = 0; y < 2; y++)
		{
			check_pos[0] = (x == 1 ? maxs[0] : mins[0]);
			check_pos[1] = (y == 1 ? maxs[1] : mins[1]);

			if (gi.pointcontents(check_pos) != CONTENTS_SOLID)
			{
				// Only do real_check if two adjacent corners are off-ledge.
				if (((corner_index == 1 || corner_index == 2) && !corner_ok[0]) ||
					(corner_index == 3 && (!corner_ok[1] || !corner_ok[2])))
				{
					do_real_check = true;
				}

				easy_ok[x][y] = false;
				corner_ok[corner_index] = false;
			}
			else
			{
				// Check them all to make real_check faster.
				easy_ok[x][y] = true;
				corner_ok[corner_index] = true;
			}

			corner_index++;
		}
	}

	if (!do_real_check)
		return true; // We got out easy.

	// Check it for real...
	vec3_t start = { 0.0f, 0.0f, mins[2] }; // Bottom.
	vec3_t stop = { 0.0f, 0.0f, start[2] - step_size + 1.0f };

	// The corners must be within 16 of the midpoint.
	corner_index = 0;

	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			if (easy_ok[x][y])
			{
				corner_ok[corner_index] = true;
				continue;
			}

			// Don't trace the ones that were ok in the easy check.
			start[0] = (x == 1 ? maxs[0] : mins[0]);
			start[1] = (y == 1 ? maxs[1] : mins[1]);
			stop[0] = start[0];
			stop[1] = start[1];

			trace_t	trace;
			gi.trace(start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID, &trace);

			if (trace.fraction == 1.0f) //mxd. '>= 1.0f' in original logic.
			{
				// This point is off too high of a step.
				if (((corner_index == 1 || corner_index == 2) && !corner_ok[0]) ||
					(corner_index == 3 && (!corner_ok[1] || !corner_ok[2])))
				{
					return false;
				}

				corner_ok[corner_index] = false;
			}
			else
			{
				// Only return false if two adjacent corners are off-ledge.
				if (ent->classID == CID_TBEAST)
					return true; // Super hack - let big guy go up slopes.

				corner_ok[corner_index] = true;
			}

			corner_index++;
		}
	}

	return true;
}

static trace_t MG_MoveStep_SwimOrFly(edict_t* self, vec3_t move, const qboolean relink) //mxd. Split from MG_MoveStep().
{
	trace_t trace = { 0 }; //mxd. Initialize to avoid static analysis warning.

	// Try one move with vertical motion, then one without.
	for (int i = 0; i < 2; i++)
	{
		vec3_t test_org;
		VectorAdd(self->s.origin, move, test_org);

		if (i == 0 && self->enemy != NULL)
		{
			if (self->goalentity == NULL)
				self->goalentity = self->enemy;

			const float dz = self->s.origin[2] - self->goalentity->s.origin[2];

			if (self->goalentity->client != NULL)
			{
				if (dz > 40.0f)
					test_org[2] -= 8.0f;
				else if (dz < 30.0f && !((self->flags & FL_SWIM) && self->waterlevel < 2)) // When not swimming on water surface.
					test_org[2] += 8.0f;
			}
			else
			{
				test_org[2] -= Clamp(dz, -8.0f, 8.0f);
			}
		}

		gi.trace(self->s.origin, self->mins, self->maxs, test_org, self, MASK_MONSTERSOLID, &trace);

		if ((self->flags & FL_FLY) && self->waterlevel == 0) // Fly monsters don't enter water voluntarily.
		{
			const vec3_t pos = { trace.endpos[0], trace.endpos[1], trace.endpos[2] + self->mins[2] + 1.0f };

			if (gi.pointcontents(pos) & MASK_WATER)
			{
				QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
				return trace;
			}
		}
		else if ((self->flags & FL_SWIM) && self->waterlevel < 2) // Swim monsters don't exit water voluntarily.
		{
			const vec3_t pos = { trace.endpos[0], trace.endpos[1], trace.endpos[2] + self->mins[2] + 1.0f };

			if (!(gi.pointcontents(pos) & MASK_WATER))
			{
				QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
				return trace;
			}
		}

		if (trace.fraction == 1.0f)
		{
			VectorCopy(trace.endpos, self->s.origin);

			if (relink)
			{
				gi.linkentity(self);
				G_TouchTriggers(self);
			}

			return trace; // OK to move. //TODO: should also set trace.succeeded to true?
		}

		if (self->enemy == NULL)
			break;
	}

	return trace;
}

static trace_t MG_MoveStep_Walk(edict_t* self, vec3_t move, const qboolean relink) //mxd. Split from MG_MoveStep().
{
	vec3_t initial_org;
	VectorCopy(self->s.origin, initial_org);

	// Push down from a step height above the wished position.
	int clipmask = MASK_MONSTERSOLID;
	float step_size;

	if (!(self->monsterinfo.aiflags & AI_NOSTEP))
	{
		if (self->classID == CID_TBEAST)
		{
			clipmask = MASK_SOLID;
			step_size = STEP_SIZE * 3.0f;
		}
		else
		{
			step_size = STEP_SIZE;
		}
	}
	else
	{
		step_size = 1.0f;
	}

	vec3_t test_org;
	VectorAdd(self->s.origin, move, test_org);
	test_org[2] += step_size;

	vec3_t end;
	VectorCopy(test_org, end);
	end[2] -= step_size * 2.0f;

	trace_t trace;
	gi.trace(test_org, self->mins, self->maxs, end, self, clipmask, &trace);

	// The step up/down is all solid in front.
	if (trace.allsolid)
	{
		QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		return trace;
	}

	//NOTE: if did the forward trace above, CAN'T have startsolid, so this is ok.
	if (trace.startsolid)
	{
		// Can't step up, try down.
		test_org[2] -= step_size;
		gi.trace(test_org, self->mins, self->maxs, end, self, clipmask, &trace);

		if (trace.allsolid || trace.startsolid)
		{
			const qboolean slip_under = (trace.ent != NULL && trace.ent->client != NULL); // Lets rats walk between legs.

			if (!slip_under)
			{
				QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
				return trace;
			}
		}
	}

	int contents = CONTENTS_EMPTY;

	// Don't go in to water unless only 40% height deep or an amphibian.
	if (self->waterlevel == 0)
	{
		// Not currently in water.
		vec3_t pos;
		VectorCopy(trace.endpos, pos);
		pos[2] += self->mins[2] + (self->maxs[2] - self->mins[2]) * 0.4f;

		contents = gi.pointcontents(pos);

		if ((contents & MASK_WATER) && !(self->flags & FL_AMPHIBIAN))
			return trace;
	}

	const qboolean is_amphibian = ((contents & MASK_WATER) && (self->flags & FL_AMPHIBIAN)); //mxd

	if (trace.fraction == 1.0f)
	{
		// Too long of a step down.
		if ((self->flags & FL_PARTIALGROUND) || (self->svflags & SVF_FLOAT) || self->classID == CID_TBEAST || is_amphibian) // Allow amphibian monsters to step off ledges into water.
		{
			// If monster had the ground pulled out, go ahead and fall.
			VectorAdd(self->s.origin, move, self->s.origin);

			if (relink)
			{
				gi.linkentity(self);
				G_TouchTriggers(self);
			}

			self->groundentity = NULL;
			trace.succeeded = true; // OK to move.
		}
		else
		{
			QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		}

		return trace;
	}

	// Check point traces down for dangling corners.
	VectorCopy(trace.endpos, self->s.origin);

	if (!is_amphibian && !MG_CheckBottom(self))
	{
		// Not completely on solid ground.
		if (self->flags & FL_PARTIALGROUND || self->svflags & SVF_FLOAT)
		{
			// Entity had floor mostly pulled out from underneath it and is trying to correct or can float.
			if (relink)
			{
				gi.linkentity(self);
				G_TouchTriggers(self);
			}

			trace.succeeded = true; // OK to move.
		}
		else
		{
			// Whoops, let's not make that move after all.
			VectorCopy(initial_org, self->s.origin);
			QPostMessage(self, MSG_BLOCKED, PRI_DIRECTIVE, NULL);
		}

		return trace;
	}

	// OK, we're on the ground completely now.
	if (self->flags & FL_PARTIALGROUND)
		self->flags &= ~FL_PARTIALGROUND;

	self->groundentity = trace.ent;
	self->groundentity_linkcount = trace.ent->linkcount;

	// The move is ok.
	if (relink)
	{
		gi.linkentity(self);
		G_TouchTriggers(self);
	}
	else
	{
		VectorCopy(initial_org, self->s.origin);
	}

	trace.succeeded = true;
	return trace; // OK to move.
}

// Called by monster program code. The move will be adjusted for slopes and stairs, but if the move isn't possible,
// no move is done, false is returned, and pr_global_struct->trace_normal is set to the normal of the blocking wall.
// Only relinks if move succeeds.
//FIXME: since we need to test end position contents here, can we avoid doing it again later in PM_CatagorizePosition()?
//TODO: modifies 'move' vector. Is that intentional?
trace_t MG_MoveStep(edict_t* self, vec3_t move, const qboolean relink)
{
	// Scale movement by monster's scale. Scale here, not before since any function can call this.
	if (self->monsterinfo.scale != 0.0f) //TODO: add ' && self->monsterinfo.scale != 1.0f'?
		VectorScale(move, self->monsterinfo.scale, move);

	// Swim and fly monsters. Flying monsters don't step up.
	if (self->flags & (FL_SWIM | FL_FLY))
		return MG_MoveStep_SwimOrFly(self, move, relink); //mxd

	// Walk monsters.
	return MG_MoveStep_Walk(self, move, relink); //mxd
}

float MG_ChangeWhichYaw(edict_t* self, const qboolean ideal_yaw)
{
	const float current = anglemod(self->s.angles[YAW]);
	const float ideal = (ideal_yaw ? self->ideal_yaw : self->best_move_yaw);
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

	move = Clamp(move, -self->yaw_speed, self->yaw_speed);
	self->s.angles[YAW] = anglemod_old(current + move); // Normal anglemod doesn't have the precision I need to slide along walls.

	return move;
}

float MG_ChangeYaw(edict_t* self)
{
	return MG_ChangeWhichYaw(self, YAW_IDEAL);
}

static qboolean MG_GetGoalPos(edict_t* self, vec3_t goal_pos)
{
	const qboolean charge_enemy = ((self->monsterinfo.aiflags & AI_STRAIGHT_TO_ENEMY) && self->enemy != NULL);

	if (!charge_enemy)
	{
		if (self->monsterinfo.searchType == SEARCH_BUOY)
		{
			if (self->buoy_index < 0 || self->buoy_index > level.active_buoys)
				return false;

			VectorCopy(level.buoy_list[self->buoy_index].origin, self->monsterinfo.nav_goal);
			VectorCopy(self->monsterinfo.nav_goal, goal_pos);

			return true;
		}

		if (self->goalentity != NULL)
		{
			if (self->goalentity == self->enemy && (self->ai_mood_flags & AI_MOOD_FLAG_PREDICT) && !(self->spawnflags & MSF_FIXED))
				M_PredictTargetPosition(self->enemy, self->enemy->velocity, 8.0f, goal_pos); // Predict where he's going.
			else
				VectorCopy(self->goalentity->s.origin, goal_pos);

			return true;
		}
	}

	if (self->enemy != NULL)
	{
		if ((self->ai_mood_flags & AI_MOOD_FLAG_PREDICT) && !(self->spawnflags & MSF_FIXED))
			M_PredictTargetPosition(self->enemy, self->enemy->velocity, 8.0f, goal_pos); // Predict where he's going.
		else
			VectorCopy(self->enemy->s.origin, goal_pos);

		return true;
	}

	VectorClear(goal_pos);
	return false;
}

float MG_FaceGoal(edict_t* self, const qboolean do_turn)
{
	vec3_t goal_pos;
	if (!MG_GetGoalPos(self, goal_pos))
		return 0.0f;

	vec3_t vec;
	VectorSubtract(goal_pos, self->s.origin, vec);
	self->ideal_yaw = VectorYaw(vec);

	return (do_turn ? MG_ChangeYaw(self) : 0.0f);
}

// Turns to the movement direction, and walks the current distance if facing it.
static qboolean MG_StepDirection(edict_t* self, const float yaw, const float distance)
{
	// Find vector offset (move to add to origin).
	const vec3_t test_angles = { 0.0f, yaw, 0.0f };

	vec3_t forward;
	AngleVectors(test_angles, forward, NULL, NULL);

	vec3_t move;
	VectorScale(forward, distance, move);

	// See if can move that way, but don't actually move.
	const trace_t trace = MG_MoveStep(self, move, false);

	if (trace.succeeded)
	{
		// Move was allowed.
		self->best_move_yaw = yaw;
		self->monsterinfo.idle_time = level.time + flrand(0.5f, 1.25f);
	}

	return trace.succeeded;
}

static void MG_NewDir(edict_t* self, const float dist)
{
#define DI_NODIR	(-1.0f)

	vec3_t goal_org;
	if (!MG_GetGoalPos(self, goal_org)) //FIXME: how did we get here with no enemy?
		return;

	const float old_yaw = anglemod((float)((int)(self->ideal_yaw / 45.0f) * 45));
	const float turn_around = anglemod(old_yaw - 180.0f);

	const float delta_x = goal_org[0] - self->s.origin[0];
	const float delta_y = goal_org[1] - self->s.origin[1];

	float d[3];

	if (delta_x > 10.0f)
		d[1] = 0.0f;
	else if (delta_x < -10.0f)
		d[1] = 180.0f;
	else
		d[1] = DI_NODIR;

	if (delta_y < -10.0f)
		d[2] = 270.0f;
	else if (delta_y > 10.0f)
		d[2] = 90.0f;
	else
		d[2] = DI_NODIR;

	// Try direct route.
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		float target_yaw;

		if (d[1] == 0.0f)
			target_yaw = (d[2] == 90.0f ? 45.0f : 315.0f);
		else
			target_yaw = (d[2] == 90.0f ? 135.0f : 215.0f);

		if (target_yaw != turn_around && MG_StepDirection(self, target_yaw, dist))
			return;
	}

	// Try other directions.
	if (irand(0, 1) == 1 || fabsf(delta_y) > fabsf(delta_x))
	{
		const float tmp = d[1];
		d[1] = d[2];
		d[2] = tmp;
	}

	if (d[1] != DI_NODIR && d[1] != turn_around && MG_StepDirection(self, d[1], dist))
		return;

	if (d[2] != DI_NODIR && d[2] != turn_around && MG_StepDirection(self, d[2], dist))
		return;

	// There is no direct path to the player, so pick another direction.
	if (old_yaw != DI_NODIR && MG_StepDirection(self, old_yaw, dist))
		return;

	// Randomly determine direction of search.
	if (irand(0, 1) == 1)
	{
		for (int yaw = 0; yaw <= 315; yaw += 45)
			if ((float)yaw != turn_around && MG_StepDirection(self, (float)yaw, dist))
				return;
	}
	else
	{
		for (int yaw = 315; yaw >= 0; yaw -= 45)
			if ((float)yaw != turn_around && MG_StepDirection(self, (float)yaw, dist))
				return;
	}

	if (turn_around != DI_NODIR && MG_StepDirection(self, turn_around, dist))
		return;

	// Can't move, restore yaw?

	// If a bridge was pulled out from underneath a monster, it may not have a valid standing position at all.
	if (!MG_CheckBottom(self))
		SV_FixCheckBottom(self);
}

/*
=============
infront_pos

returns 1 if the spot is in front (in sight) of self
=============
*/
qboolean infront_pos (edict_t *self, vec3_t pos)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward, check_angles;
	
	if(Vec3NotZero(self->v_angle_ofs))
		VectorAdd(self->v_angle_ofs,self->s.angles,check_angles);
	else
		VectorCopy(self->s.angles,check_angles);
	
	AngleVectors (check_angles, forward, NULL, NULL);
	VectorSubtract (pos, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
	if (dot > 0.3)
		return true;
	return false;
}

qboolean MG_ExtraCheckJump (edict_t *self)
{
	vec3_t		vf, source, source2, targ_org, targ_mins;
	vec3_t		maxs, mins, save_org;
	trace_t		trace;
	float		hgt_diff, jump_fdist;
	qboolean	jump_up_check = false;
	qboolean	check_down = false;
	qboolean	can_move = false;

#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("Extra Check Jump\n");
#endif
	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return false;

		VectorCopy(level.buoy_list[self->buoy_index].origin, targ_org);

		if (!(infront_pos(self, targ_org)))
			return false;

		if (targ_org[2] < self->s.origin[2] - 28)
		{
			check_down = true;
		}
		else
		{
			check_down = false;
		}
		VectorClear(targ_mins);
	}
	else
	{
		if(!self->goalentity)
			return false;
		
		if (!(infront(self, self->goalentity)))
			return false;
		
		if (self->goalentity->s.origin[2] < self->s.origin[2] - 28)
		{
			check_down = true;
		}
		else
		{
			check_down = false;
		}

		VectorCopy(self->goalentity->s.origin, targ_org);
		VectorCopy(self->goalentity->mins, targ_mins);
	}

	if (check_down)
	{//jumping down
		//Setup the trace
		int inwater;

#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("check jump down\n");
#endif
		inwater = (gi.pointcontents(self->s.origin) & CONTENTS_WATER);

#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("checking jump down: ");
#endif
		if(inwater)
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("checkdown allsolid\n");
#endif
			return false;
		}

		VectorCopy(self->maxs, maxs);
		VectorCopy(self->s.origin, source);
		AngleVectors(self->s.angles, vf, NULL, NULL);
		jump_fdist = vhlen(targ_org, self->s.origin);
		if(jump_fdist > 128)
			jump_fdist = 128;

		VectorMA(source, 128, vf, source);

		maxs[2] += 16;
		gi.trace (self->s.origin, self->mins, maxs, source, self, MASK_MONSTERSOLID,&trace);

		if (trace.fraction == 1)
		{//clear ahead and above
			VectorCopy(source, source2);

			source2[2] -= 1024;
			//trace down
			gi.trace (source, self->mins, self->maxs, source2, self, MASK_ALL,&trace);
			
			if (trace.allsolid || trace.startsolid)
			{
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("checkdown allsolid\n");
#endif
				return false;
			}

			if (trace.fraction == 1)
			{
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("checkdown- too far\n");
#endif
				return false;
			}
			else
			{
				if (trace.contents != CONTENTS_SOLID && trace.ent != self->enemy)
				{//didn't hit ground
					return false;
				}
				else
				{
#ifdef _DEVEL
					if(MGAI_DEBUG)
						gi.dprintf("checkjump down->whichjump\n");
#endif
					VectorSubtract(trace.endpos, self->s.origin, source2);
					VectorNormalize(source2);
					self->ideal_yaw = VectorYaw(source2);
					
					VectorMA(self->velocity, 300, vf, self->velocity);
					self->velocity[2]+=150;

					if(classStatics[self->classID].msgReceivers[MSG_JUMP])
					{
						if(self->classID != CID_RAT && self->classID != CID_SSITHRA)
						{//save vel so can crouch first
							VectorCopy(self->velocity, self->movedir);
							VectorClear(self->velocity);
						}
						QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
						self->nextthink = level.time + 0.01;
					}
					else
						self->nextthink = level.time + 0.3;

					self->monsterinfo.jump_time = level.time + 1;
#ifdef _DEVEL
					if(MGAI_DEBUG)
						gi.dprintf("Extra jump down\n");
#endif
				}
			}
		}
#ifdef _DEVEL
		else if(MGAI_DEBUG)
			gi.dprintf("checkdown: not clear infront\n");
#endif
	}
	else
	{
		VectorCopy(self->s.origin, save_org);
		can_move = M_walkmove (self, self->s.angles[YAW], 64);
		VectorCopy(save_org, self->s.origin);
		
		if(can_move)
			return false;
		else
		{//check to jump over something

#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("check jump over\n");
#endif
			AngleVectors(self->s.angles, vf, NULL, NULL);
			VectorCopy(self->s.origin, source);
			VectorMA(source, 128, vf, source2);
			VectorCopy(self->mins, mins);
			mins[2]+=24;//can clear it
			gi.trace(source, mins, self->maxs, source2, self, MASK_SOLID,&trace);
			
			if((!trace.allsolid&&!trace.startsolid&&trace.fraction==1.0) || trace.ent == self->enemy)
			{//Go for it!
				
				VectorMA(self->velocity, 500*trace.fraction, vf, self->velocity);
				self->velocity[2] += 225;

				if(classStatics[self->classID].msgReceivers[MSG_JUMP])
				{
					if(self->classID != CID_RAT && self->classID != CID_SSITHRA)
					{//save vel so can crouch first
						VectorCopy(self->velocity, self->movedir);
						VectorClear(self->velocity);
					}
					QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
					self->nextthink = level.time + 0.01;
				}
				else
					self->nextthink = level.time + 0.3;
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("Extra jump over\n");
#endif
			}
			else
			{
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("check jump up\n");
#endif
				VectorCopy(self->maxs, maxs);
				VectorCopy(self->s.origin, source);

				hgt_diff = (targ_org[2] + targ_mins[2]) - (self->s.origin[2] + self->mins[2]) + 32;
				source[2] += hgt_diff;
				gi.trace (self->s.origin, self->mins, self->maxs, source, self, MASK_MONSTERSOLID,&trace);

				if (trace.fraction == 1)
				{//clear above
					VectorCopy(source, source2);

					AngleVectors(self->s.angles, vf, NULL, NULL);
					VectorMA(source, 64, vf, source2);
					source2[2] -= 24;
					//trace forward and down a little
					gi.trace (source, self->mins, self->maxs, source2, self, MASK_ALL,&trace);
					
					if (trace.allsolid || trace.startsolid)
						return false;

					if (trace.fraction < 0.1)
					{
#ifdef _DEVEL
						if(MGAI_DEBUG)
							gi.dprintf("Can't jump up, no ledge\n");
#endif
						return false;
					}
		//			{
		//				if (stricmp(trace.ent->classname, "worldspawn"))
		//					return;
		//			}
					else
					{
						VectorSubtract(trace.endpos, self->s.origin, source2);
						source2[2] = 0;
						VectorNormalize(source2);
						self->ideal_yaw = VectorYaw(source2);
						
						VectorMA(self->s.origin, 64, source2, source);
						gi.trace(self->s.origin, vec3_origin, vec3_origin, source, self, MASK_SOLID,&trace);

						VectorScale(source2, 480*trace.fraction, self->velocity);
						self->velocity[2] = hgt_diff*3 + 200;

						if(classStatics[self->classID].msgReceivers[MSG_JUMP])
						{
							if(self->classID != CID_RAT && self->classID != CID_SSITHRA)
							{//save vel so can crouch first
								VectorCopy(self->velocity, self->movedir);
								VectorClear(self->velocity);
							}
							QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
							self->nextthink = level.time + 0.01;
						}
						else
							self->nextthink = level.time + 0.3;
						self->monsterinfo.jump_time = level.time + 1;
#ifdef _DEVEL
						if(MGAI_DEBUG)
							gi.dprintf("Extra jump up\n");
#endif
					}
				}
				else
				{
#ifdef _DEVEL
					if(MGAI_DEBUG)
						gi.dprintf("Can't jump up, blocked\n");
#endif
					return false;
				}

			}
		}

	}
	return true;
}

/*
==================================================================
MG_CheckJump()
Checks to see if the enemy is not at the same level as monster
or something is blocking the path of the monster.  If there is 
a clear jump arc to the enemy and the monster will not land in
water or lava, the monster will attempt to jump the distance.
==================================================================
*/
qboolean MG_CheckJump (edict_t *self)
{
	static const vec3_t jump_mins = { -8.0f, -8.0f, 0.0f }; //mxd. Made local static.
	static const vec3_t jump_maxs = { 8.0f,  8.0f, 4.0f }; //mxd. Made local static.

	vec3_t spot1, spot2, jumpdir, forward, right, targ_absmin;
	vec3_t up, cont_spot, vis_check_spot, end_spot, targ_org;
	float jump_height, sub_len;
	int	contents;
	qboolean ignore_height;
	qboolean jumpup = false;
	trace_t trace;

	if(irand(1,100) > self->jump_chance)//JumpChanceForClass[self->classID])
		return false;

	if(self->classID == CID_TBEAST)
		return TB_CheckJump(self);

#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("Check Jump\n");
#endif
	//FIXME: Allow jump in/out of water if not too deep
	if(self->flags & FL_INWATER)// && !(self->flags&FL_AMPHIBIAN))?
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Can't jump- inwater\n");
#endif
		return false;
	}

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return false;

		VectorCopy(level.buoy_list[self->buoy_index].origin, targ_org);
		VectorCopy(targ_org, targ_absmin);

		if (!(infront_pos(self, targ_org)))
			return false;
	}
	else
	{
		if(!self->goalentity)
			return false;
		
		if (!(infront(self, self->goalentity)))
			return false;
		
		if(!self->goalentity->groundentity && self->classID != CID_GORGON)
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("goalentity in air\n");
#endif
			return false;
		}

		VectorCopy(self->goalentity->s.origin, targ_org);
		VectorAdd(targ_org, self->goalentity->mins, targ_absmin);
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorSubtract(targ_org, self->s.origin, jumpdir);
	VectorNormalize(jumpdir);
	jumpdir[2] = 0;
	jump_height=DotProduct(jumpdir, forward);

	if(jump_height<0.3)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("jump direction more than 60 degrees off of forward\n");
#endif
		return false;
	}

    VectorCopy(self->s.origin, spot1);
    VectorCopy(targ_org, spot2);

	jump_height = 16;

	VectorMA(spot1, 24, forward, cont_spot);
	cont_spot[2] -= 10;
	if(!(gi.pointcontents(cont_spot)&CONTENTS_SOLID))
		ignore_height = true;

	sub_len = vhlen(spot1, spot2);
//	if(self->classname!="monster_mezzoman"&&!self->spiderType&&self->model!="models/yakman.mdl")
	if(sub_len > 256)
		ignore_height = false;

	VectorMA(spot1, self->size[0] * 2, forward, vis_check_spot);
	VectorMA(vis_check_spot, self->size[2] * 1.5, up, vis_check_spot);
//also check to make sure you can't walkmove forward
    if(self->monsterinfo.jump_time > level.time)            //Don't jump too many times in a row
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("just jumped\n");
#endif
		return false;
	}
    else if(!ignore_height&&targ_absmin[2]+36>=self->absmin[2])//&&self->think!=SpiderJumpBegin&&self->classname!="monster_mezzoman"&&self->model!="models/yakman.mdl")
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("not above goalentity, and not spider\n");
#endif
		return false;
	}
    else if(!self->groundentity)//flags&FL_ONGROUND)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("not on ground\n");
#endif
		return false;
	}
    else if(sub_len>777 && !ignore_height)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("too far away\n");
#endif
		return false;
	}
    else if(sub_len <= 100)//&&self->think!=SpiderMeleeBegin)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("too close & not spider\n");
#endif
		return false;
	}
	//sfs--sure, it's just a dotproduct, but the other checks are a little cheaper
    else if(!infront_pos(self, targ_org))
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("goalentity not in front\n");
#endif
		return false;
	}
	//sfs--save the trace line for after the easy checks
    else if(!clear_visible_pos(self, targ_org)&&!HaveLOS(self, vis_check_spot, spot2))
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("can't see goalentity\n");
#endif
		return false;
	}

	//sfs--holding off on the point contents too
	contents = gi.pointcontents(spot2);
	if(!(self->monsterinfo.aiflags&AI_SWIM_OK)&&
		(contents&CONTENTS_WATER||contents&CONTENTS_SLIME||contents&CONTENTS_LAVA))
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("goalentity in water or lava\n");
#endif
		return false;
	}

	VectorCopy(self->s.origin, spot1);
	//	spot1=self->s.origin;
	spot1[2] += self->maxs[2];
    //spot1_z=self->absmax_z;
	VectorCopy(spot1, spot2);
	//spot2=spot1;
	spot2[2] += 36;
	//spot2_z+=36;

    gi.trace(spot1, self->mins, self->maxs, spot2, self, MASK_MONSTERSOLID,&trace);

    if(trace.fraction<1.0||trace.allsolid||trace.startsolid)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("not enough room above\n");
#endif
		return false;
	}

	if(!jumpup)		// This variable is used without being initialised
	{
//	        spot1+=normalize(v_forward)*((self->maxs_x+self->maxs_y)*0.5);
	    VectorMA(spot1, (self->maxs[0]+self->maxs[1])*0.5, jumpdir, spot1);
		//spot1+=jumpdir*((self->maxs_x+self->maxs_y)*0.5);
		VectorCopy(spot1, end_spot);
		end_spot[2] += 36;

		gi.trace(self->s.origin, self->mins, self->maxs, end_spot, self, MASK_MONSTERSOLID,&trace);

	    if(trace.fraction<1.0||trace.allsolid||trace.startsolid)
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("not enough room in front\n");
#endif
			return false;
		}
		VectorMA(spot1, 64, jumpdir, end_spot);
		end_spot[2] -= 500;
		gi.trace(spot1, jump_mins, jump_maxs, end_spot, self, MASK_MONSTERSOLID,&trace);
//	        traceline(spot1,spot1+jumpdir*64 - '0 0 500',false,self);

		contents = gi.pointcontents(trace.endpos);
		if(contents&CONTENTS_WATER||contents&CONTENTS_SLIME||contents&CONTENTS_LAVA)
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("won't jump in water\n");
#endif
			return false;
		}
	}

	MG_FaceGoal(self, true);
//FIXME: make them do whatever jump function they have if they have one
	self->monsterinfo.jump_time = level.time + 2;        //Only try to jump once every 7 seconds
	if(!self->s.scale)
		self->s.scale = 1.0;
	if(!jumpup)
	{
		VectorScale(jumpdir, jump_height*18*self->s.scale, self->velocity);
		//self->velocity=jumpdir*jump_height*18*self->scale;//was 18
		self->velocity[2] = jump_height*14*self->s.scale;//was 12
	}
	else
	{
		VectorScale(jumpdir, jump_height*14*self->s.scale, self->velocity);
		//self->velocity=jumpdir*jump_height*14*self->scale;//was 10
		self->velocity[2] = jump_height*17*self->s.scale;//was 14
	}
	//self->groundentity = NULL?
	if(classStatics[self->classID].msgReceivers[MSG_JUMP])
	{
		if(self->classID != CID_RAT && self->classID != CID_SSITHRA)
		{//save vel so can crouch first
			VectorCopy(self->velocity, self->movedir);
			VectorClear(self->velocity);
		}
		QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
		self->nextthink = level.time + 0.01;
	}
	else
		self->nextthink = level.time + 0.3;
	
#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("JUMP!!!\n");
#endif
	return true;
}

/*
===============
MG_WalkMove

  Tries to step forward dist, returns the trace
===============
*/
trace_t MG_WalkMove (edict_t *self, float yaw, float dist)
{
	vec3_t	move, endpos;
	trace_t trace;
	
	yaw = yaw*M_PI*2 / 360;
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	trace = MG_MoveStep(self, move, true);
	if(trace.succeeded)
	{
		return trace;
	}
//FaileD? ok, so what's in front of us
	VectorAdd(self->s.origin, move, endpos);
	//up mins for stairs?
	gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&trace);
	trace.succeeded = false;
	return trace;
}

/*
===============
MG_BoolWalkMove

  Tries to step forward dist, returns true/false
===============
*/
qboolean MG_BoolWalkMove (edict_t *self, float yaw, float dist)
{
	vec3_t	move;
	trace_t trace;
	
	yaw = yaw*M_PI*2 / 360;
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	trace = MG_MoveStep(self, move, true);
	if(trace.succeeded)
	{
		return true;
	}

	return false;
}

/*
===============
MG_TestMove

  Sees if it can step dist in yaw, but doesn't do the move
===============
*/
qboolean MG_TestMove (edict_t *self, float yaw, float dist)
{
	vec3_t	move;
	trace_t trace;
	
	yaw = yaw*M_PI*2 / 360;
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	trace = MG_MoveStep(self, move, false);
	if(trace.succeeded)
		return true;

	return false;
}

void MG_CheckEvade (edict_t *self)
{//FIXME: only check my enemy?  See if he's fired (last_attack) recently?
	int hl;
	float ent_dist, proj_offset;
	vec3_t proj_dir, endpos, bad_dir;
	trace_t trace;
	edict_t	*ent = NULL;
	vec3_t	total_dist;
	float	eta;
	
	if(!skill->value)
		return;
	//else if(flrand(0, 3) > skill->value)
	//	return;

	while(ent = FindInRadius(ent, self->s.origin, 500))
	{
		if(ent->movetype == MOVETYPE_FLYMISSILE && ent->solid && ent->owner!=self)
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("Monster checking evade from %s projectile\n", ent->classname);
#endif
			if(Vec3IsZero(ent->velocity))
			{
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("ERROR: NULL velocity on %s projectile!\n", ent->classname);
#endif
			}
			else
			{
				VectorCopy(ent->velocity, proj_dir);
				VectorNormalize(proj_dir);
				VectorMA(ent->s.origin, 600, proj_dir, endpos);

				gi.trace(ent->s.origin, ent->mins, ent->maxs, endpos, ent, MASK_MONSTERSOLID,&trace);
				if(trace.ent == self)
				{//going to get hit!
					hl = MG_GetHitLocation(self, ent, trace.endpos, vec3_origin);
					VectorSubtract(trace.endpos, ent->s.origin, total_dist);
					eta = VectorLength(total_dist)/VectorLength(ent->velocity);
					QPostMessage(self, MSG_EVADE, PRI_DIRECTIVE, "eif", ent, hl, eta);
				}
				else if(!irand(0,2))
				{
					VectorSubtract(self->s.origin, ent->s.origin, bad_dir);
					ent_dist = VectorNormalize(bad_dir);
					proj_offset = DotProduct(bad_dir, proj_dir);
#ifdef _DEVEL
					if(MGAI_DEBUG)
						gi.dprintf("Proj dot prod: %f\n", proj_offset);
#endif
					if(proj_offset > ent_dist/600)//farther it is, smaller angle deviation allowed for evasion
					{//coming pretty close
						VectorMA(ent->s.origin, ent_dist, proj_dir, endpos);//extrapolate to close to me
						gi.trace(endpos, ent->mins, ent->maxs, self->s.origin, ent, MASK_MONSTERSOLID,&trace);
						if(trace.ent == self)
						{
							hl = MG_GetHitLocation(self, ent, trace.endpos, vec3_origin);
							VectorSubtract(trace.endpos, ent->s.origin, total_dist);
							eta = VectorLength(total_dist)/VectorLength(ent->velocity);
							QPostMessage(self, MSG_EVADE, PRI_DIRECTIVE, "eif", ent, hl, eta);
						}
					}
				}
			}
		}
	}
}

/*
=============
ai_run

The monster has an enemy it is trying to kill or the monster is fleeing
=============
*/
void old_ai_run (edict_t *self, float dist);
void ai_run (edict_t *self, float dist)
{
	float	turnamt, i;
	
	//if dumb fleeing or fleeing and can't use buoys...
	if((DEACTIVATE_BUOYS||!(self->monsterinfo.aiflags & AI_USING_BUOYS))
				&&
				(self->monsterinfo.aiflags & AI_COWARD || 
					(self->monsterinfo.aiflags&AI_FLEE
					&&
					self->monsterinfo.flee_finished >= level.time)
				)
			)
	{
		ai_flee(self, dist);
		return;
	}
	else if(self->ai_mood_flags & AI_MOOD_FLAG_DUMB_FLEE)
	{
		if(MG_GoToRandomBuoy(self))
			self->monsterinfo.searchType = SEARCH_BUOY;
		else
		{
			ai_flee(self, dist);
			return;
		}
	}

	if(!DEACTIVATE_BUOYS)
	{
		if(self->monsterinfo.aiflags & AI_USING_BUOYS)
		{
			if(!(self->monsterinfo.aiflags&AI_STRAIGHT_TO_ENEMY))
			{
				if(self->pathfind_nextthink<=level.time)
				{
					MG_BuoyNavigate(self);
					self->pathfind_nextthink = level.time + 0.1;//0.3;-maybe TOO optimized, trying every frame again, take out of generic mood think?
					//don't pathfind for the next 3 frames.
				}
			}
		}
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{//just face enemy
		turnamt = Q_fabs(ai_face_goal(self));
		return;
	}
	
#ifdef _DEVEL
	if(self->goalentity == self->enemy)
	{
		//sfs--only do this visibility check when debugging (gets expensive at big distances)
		if(MGAI_DEBUG && !visible(self, self->enemy))
		{
			gi.dprintf("ERROR: goal is invis enemy!\n");
		}
	}
#endif

	if(dist)
		if(!MG_MoveToGoal (self, dist))
		{
			if(self->classID == CID_SSITHRA)
				ssithraCheckJump(self);
		}
		else
			i = 0;
	
	if(self->classID!=CID_ASSASSIN)//does his own checks
		if(classStatics[self->classID].msgReceivers[MSG_EVADE])
		{//check for if going to be hit and evade
			MG_CheckEvade(self);
		}
}

void mg_ai_charge (edict_t *self, float dist)
{
	vec3_t	v;

	if(!self->enemy)
	{
#ifdef _DEVEL
		gi.dprintf("ERROR: AI_CHARGE at a NULL enemy!\n");
#endif
		return;//send stand MSG?
	}

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);

	self->ideal_yaw = VectorYaw(v);

	MG_ChangeYaw (self);

	if (dist)
		MG_WalkMove (self, self->s.angles[YAW], dist);

	if(self->classID!=CID_ASSASSIN)//does his own checks
		if(classStatics[self->classID].msgReceivers[MSG_EVADE])
		{//check for if going to be hit and evade
			MG_CheckEvade(self);
		}
}

qboolean canmove (edict_t *self)
{
	if(	self->movetype!=PHYSICSTYPE_NONE &&
		self->movetype!=PHYSICSTYPE_PUSH)
		return true;
	return false;
}

void mg_remove_body(edict_t *self)
{
	vec3_t	origin;
	int		flag = 0;

	VectorCopy(self->s.origin, origin);
	origin[2] += (self->mins[2] + 8.0f);
	if(self->classID == CID_RAT)
		flag |= CEF_FLAG6;
	gi.CreateEffect(NULL, FX_CORPSE_REMOVE, flag, origin, "");
	G_SetToFree(self);
}

void body_phase_out (edict_t *self)
{
	int	interval = 30;

	if(self->s.color.a > interval)
	{
		self->s.color.a -= irand(interval/2, interval);
		self->post_think = body_phase_out;
		self->next_post_think = level.time + 0.05;
	}
	else 
	{
		self->s.color.a = 0;
		self->post_think = NULL;
		self->next_post_think = -1;

		G_SetToFree(self);
	}
}

trace_t MG_AirMove(edict_t *self, vec3_t goalpos, float dist)
{
	trace_t		trace;
	vec3_t		endpos, movedir;

	VectorSubtract(goalpos, self->s.origin, movedir);
	VectorNormalize(movedir);

	VectorMA(self->s.origin, dist, movedir, endpos);

	gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID, &trace);

	if(trace.allsolid || trace.startsolid || trace.fraction <= 0.01)
	{
		trace.succeeded = false;
		return trace;
	}

	VectorCopy(trace.endpos, self->s.origin);

	gi.linkentity(self);

	trace.succeeded = true;
	return trace;
}

static int FRONT = 0;
static int BACK	= 1;
static int RIGHT = 2;
static int LEFT	= 3;
static float MIN_DROP_DIST = 0.125f;

void MG_PostDeathThink (edict_t *self)
{
	float	mostdist;
	trace_t trace1, trace2, trace3, trace4, movetrace;
	vec3_t	org, endpos, startpos, forward, right;
	int		whichtrace = 0;
	float	cornerdist[4];
	qboolean	frontbackbothclear = false;
	qboolean	rightleftbothclear = false;

	self->post_think = body_phase_out;
	self->next_post_think = level.time + 10;

	if(!self->groundentity || Vec3NotZero(self->velocity))
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Falling!\n");
#endif
		if(self->groundentity&&self->friction == 1.0)//check avelocity?
			pitch_roll_for_slope(self, NULL);

		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
	}

	cornerdist[FRONT] = cornerdist[BACK] = cornerdist[RIGHT] = cornerdist[LEFT] = 0.0f;

	mostdist = MIN_DROP_DIST;

	AngleVectors(self->s.angles, forward, right, NULL);
	VectorCopy(self->s.origin, org);
	org[2]+=self->mins[2];

	VectorMA(org, self->dead_size, forward, startpos);
	VectorCopy(startpos, endpos);
	endpos[2]-=128;
	gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_SOLID,&trace1);
	if(!trace1.allsolid&&!trace1.startsolid)
	{
		cornerdist[FRONT] = trace1.fraction;
		if(trace1.fraction>mostdist)
		{
			mostdist = trace1.fraction;
			whichtrace = 1;
		}
	}

	VectorMA(org, -self->dead_size, forward, startpos);
	VectorCopy(startpos, endpos);
	endpos[2]-=128;
	gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_SOLID,&trace2);
	if(!trace2.allsolid&&!trace2.startsolid)
	{
		cornerdist[BACK] = trace2.fraction;
		if(trace2.fraction>mostdist)
		{
			mostdist = trace2.fraction;
			whichtrace = 2;
		}
	}

	VectorMA(org, self->dead_size/2, right, startpos);
	VectorCopy(startpos, endpos);
	endpos[2]-=128;
	gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_SOLID,&trace3);
	if(!trace3.allsolid&&!trace3.startsolid)
	{
		cornerdist[RIGHT] = trace3.fraction;
		if(trace3.fraction>mostdist)
		{
			mostdist = trace3.fraction;
			whichtrace = 3;
		}
	}

	VectorMA(org, -self->dead_size/2, right, startpos);
	VectorCopy(startpos, endpos);
	endpos[2]-=128;
	gi.trace(startpos, vec3_origin, vec3_origin, endpos, self, MASK_SOLID,&trace4);
	if(!trace4.allsolid&&!trace4.startsolid)
	{
		cornerdist[LEFT] = trace4.fraction;
		if(trace4.fraction>mostdist)
		{
			mostdist = trace4.fraction;
			whichtrace = 4;
		}
	}
	
	//OK!  Now if two opposite sides are hanging, use a third if any, else, do nothing
	if(cornerdist[FRONT] > MIN_DROP_DIST && cornerdist[BACK] > MIN_DROP_DIST)
		frontbackbothclear = true;

	if(cornerdist[RIGHT] > MIN_DROP_DIST && cornerdist[LEFT] > MIN_DROP_DIST)
		rightleftbothclear = true;

	if(frontbackbothclear && rightleftbothclear)
		return;

	if(frontbackbothclear)
	{
		if(cornerdist[RIGHT] > MIN_DROP_DIST)
			whichtrace = 3;
		else if(cornerdist[LEFT] > MIN_DROP_DIST)
			whichtrace = 4;
		else
			return;
	}

	if(rightleftbothclear)
	{
		if(cornerdist[FRONT] > MIN_DROP_DIST)
			whichtrace = 1;
		else if(cornerdist[BACK] > MIN_DROP_DIST)
			whichtrace = 2;
		else
			return;
	}
	
	switch(whichtrace)
	{//check for stuck
	case 1:
		VectorMA(self->s.origin, self->maxs[0], forward, endpos);
		gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&movetrace);
		if(movetrace.allsolid||movetrace.startsolid||movetrace.fraction<1.0)
			if(canmove(movetrace.ent))
				whichtrace = -1;
			else
				whichtrace = 0;
		break;
	case 2:
		VectorMA(self->s.origin, -self->maxs[0], forward, endpos);
		gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&movetrace);
		if(movetrace.allsolid||movetrace.startsolid||movetrace.fraction<1.0)
			if(canmove(movetrace.ent))
				whichtrace = -1;
			else
				whichtrace = 0;
		break;
	case 3:
		VectorMA(self->s.origin, self->maxs[0], right, endpos);
		gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&movetrace);
		if(movetrace.allsolid||movetrace.startsolid||movetrace.fraction<1.0)
			if(canmove(movetrace.ent))
				whichtrace = -1;
			else
				whichtrace = 0;
		break;
	case 4:
		VectorMA(self->s.origin, -self->maxs[0], right, endpos);
		gi.trace(self->s.origin, self->mins, self->maxs, endpos, self, MASK_MONSTERSOLID,&movetrace);
		if(movetrace.allsolid||movetrace.startsolid||movetrace.fraction<1.0)
			if(canmove(movetrace.ent))
				whichtrace = -1;
			else
				whichtrace = 0;
		break;
	}
	
	switch(whichtrace)
	{
	case 1:
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Forward trace %f\n", trace1.fraction);
#endif
		VectorMA(self->velocity, 200, forward, self->velocity);
		if(trace1.fraction >= 0.9)
		{
//can't anymore, origin not in center of deathframe!
//			self->avelocity[PITCH] = -300;
			self->friction = 1.0;
		}
		else
		{
			pitch_roll_for_slope(self, &trace1.plane.normal);
			self->friction = trace1.plane.normal[2] * 0.1;
		}
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
		break;
		
	case 2:
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("back trace %f\n", trace2.fraction);
#endif
		VectorMA(self->velocity, -200, forward, self->velocity);
		if(trace2.fraction >= 0.9)
		{
//can't anymore, origin not in center of deathframe!
//			self->avelocity[PITCH] = 300;
			self->friction = 1.0;
		}
		else
		{
			pitch_roll_for_slope(self, &trace2.plane.normal);
			self->friction = trace2.plane.normal[2] * 0.1;
		}
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
		break;
		
	case 3:
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Right trace %f\n", trace3.fraction);
#endif
		VectorMA(self->velocity, 200, right, self->velocity);
		if(trace3.fraction >= 0.9)
		{
//can't anymore, origin not in center of deathframe!
//			self->avelocity[ROLL] = -300;
			self->friction = 1.0;
		}
		else
		{
			pitch_roll_for_slope(self, &trace3.plane.normal);
			self->friction = trace3.plane.normal[2] * 0.1;
		}
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
		break;
		
	case 4:
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Left trace %f\n", trace4.fraction);
#endif
		VectorMA(self->velocity, -200, right, self->velocity);
		if(trace4.fraction >= 0.9)
		{
//can't anymore, origin not in center of deathframe!
//			self->avelocity[ROLL] = 300;
			self->friction = 1.0;
		}
		else
		{
			pitch_roll_for_slope(self, &trace4.plane.normal);
			self->friction = trace4.plane.normal[2] * 0.1;
		}
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1;
		return;
		break;
	}
	//on solid ground
	if(whichtrace == -1)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Deadmonster slide = stuck! (size is %4.2f)\n", self->dead_size);
#endif
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 2;
		return;
	}
#ifdef _DEVEL
	else if(MGAI_DEBUG)
		gi.dprintf("Deadmonster slide = On ground (size was %4.2f)\n", self->dead_size);
#endif
	self->friction = 1.0;

	VectorClear(self->avelocity);
	pitch_roll_for_slope(self, NULL);

	if(!self->s.color.r)
		self->s.color.r = 255;
	if(!self->s.color.g)
		self->s.color.g = 255;
	if(!self->s.color.b)
		self->s.color.b = 255;
	self->s.color.a = 255;

	self->post_think = body_phase_out;
	if(self->classID == CID_RAT)
		self->next_post_think = level.time + flrand(3, 7);
	else
		self->next_post_think = level.time + flrand(10, 20);

	gi.linkentity (self);
}

void MG_CheckLanded (edict_t *self, float next_anim)
{
	vec3_t pos;
	
#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("self->velocity %f %f %f\n", self->velocity[0], self->velocity[1], self->velocity[2]);
#endif

	if(self->groundentity)
		SetAnim(self, (int)next_anim);
	else if(self->velocity[2]<0)
	{
		VectorCopy(self->s.origin, pos);
		pos[2] += self->mins[2];
		VectorMA(pos, 0.5, self->velocity, pos);
		if(gi.pointcontents(pos)&CONTENTS_SOLID)
			SetAnim(self, (int)next_anim);
	}
}

void MG_InAirMove (edict_t *self, float fwdspd,float upspd,float rtspd)
{//simple addition of velocity, if on ground or not
	vec3_t up, forward, right;

	if(self->groundentity)//on ground
		return;

	AngleVectors(self->s.angles, forward, right, up);
	
	VectorMA(self->velocity, upspd, up, self->velocity);
	VectorMA(self->velocity, fwdspd, forward, self->velocity);
	VectorMA(self->velocity, rtspd, right, self->velocity);
}

void MG_ApplyJump (edict_t *self)
{
	self->jump_time = level.time + 0.5;
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);
#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("Jump velocity will be: %4.2f %4.2f %4.2f\n", self->velocity[0], self->velocity[1], self->velocity[2]);
#endif
	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
}

void MG_NoBlocking (edict_t *self)
{
	self->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
	self->msgHandler = DeadMsgHandler;	// no more messages at all
}

qboolean MG_GetTargOrg (edict_t *self, vec3_t targ_org)
{
	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if(self->buoy_index < 0 || self->buoy_index > level.active_buoys)
		{
			VectorClear(targ_org);
			return false;
		}
		VectorCopy(level.buoy_list[self->buoy_index].origin, self->monsterinfo.nav_goal);
		VectorCopy(self->monsterinfo.nav_goal, targ_org);
	}
	else
	{
		if(!self->goalentity)
		{
			VectorClear(targ_org);
			return false;
		}
	
		VectorCopy(self->goalentity->s.origin, targ_org);
	}
	return true;
}

/*
qboolean EqualAngle(float angle1, float angle2, float leniency)

Sees if the two angles are within leniency degrees of each other
*/
qboolean EqualAngle(float angle1, float angle2, float leniency)
{
	float diff;

	if(angle1 < -180)
		angle1 += 360;
	else if(angle1 > 180)
		angle1 -= 360;

	if(angle2 < -180)
		angle2 += 360;
	else if(angle2 > 180)
		angle2 -= 360;

	diff = angle1 - angle2;

	if(diff < -180)
		diff += 360;
	else if(diff > 180)
		diff -= 360;

	if(fabs(diff) > leniency)
		return false;

	return true;
}

/*
qboolean ok_to_break (edict_t *target)

  Ok to just smash this damn thing in my way?
*/
qboolean ok_to_break (edict_t *target)
{
	if(!target)
		return false;

	if(!target->takedamage)
		return false;

	if(target->health>MAX_BLOCKING_THING_HEALTH)//general damage for pots, barrels, etc. 
		return false;

	if(target->targetname)//supposed to be triggered for some reason
		return false;

	if(target->svflags&SVF_MONSTER)//another monster
		return false;

	if(Vec3IsZero(target->s.origin))//breakable_brushes have no origin
		return false;

	return true;//break it!
}

qboolean MG_MoveToGoal (edict_t *self, float dist)
{
	trace_t trace;
	float turnamt, distloss, adj_dist, save_yaw, save_yaw_speed, WallDot;//, save_ideal_yaw;
	vec3_t	mins, maxs, source, goal_dir;//, vec, save_org;
	qboolean	goal_vis=false, hitworld = false, new_best_yaw = false;
	float		stepsize, goal_dist, oby;

	if(self->classID == CID_TBEAST)
		stepsize = STEP_SIZE * 3;
	else
		stepsize = STEP_SIZE;

	if(!self->groundentity&&!(self->flags&FL_SWIM)&&!(self->flags&FL_FLY))
		return false;//in air!

	trace.succeeded = false;

	if(self->classID != CID_GORGON)//they do their own yawing
		MG_FaceGoal(self, false);//get ideal yaw, but don't turn

	//are we very close to our goal? problem: what if something in between?
	if(!EqualAngle(self->s.angles[YAW], self->ideal_yaw, self->yaw_speed))
	{//we aren't really facing our ideal yet
		if(self->monsterinfo.searchType == SEARCH_BUOY||self->ai_mood == AI_MOOD_NAVIGATE)
		{
			VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, goal_dir);
			goal_dist = VectorNormalize(goal_dir);
			if(goal_dist < (self->maxs[0] + 24 + dist))
			{//we're close to our goal
				MG_ChangeWhichYaw(self, YAW_IDEAL);
				return true;//so close to enemy, just turn, no movement - not if rat?
			}
		}
		else if(self->enemy)
		{
			VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, goal_dir);
			goal_dist = VectorNormalize(goal_dir);
			if(goal_dist < (self->maxs[0] + self->enemy->maxs[0] + dist*2))
			{//we're close to our goal
				MG_ChangeWhichYaw(self, YAW_IDEAL);
				return true;//so close to enemy, just turn, no movement - not if rat?
			}
		}
	}

	if(self->monsterinfo.idle_time == -1)
	{//have been told to just turn to ideal_yaw
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
		//keep turning towards ideal until facing it
		if(turnamt < 1)
			self->monsterinfo.idle_time = 0;
		else
			return true;
	}
	else if(self->monsterinfo.idle_time > level.time)
	{//using best_move_yaw
		if(EqualAngle(self->s.angles[YAW], self->best_move_yaw, 5))
		{//do a test move in the direction I would like to go:
			if(MG_TestMove(self, self->ideal_yaw, dist))
			{//can move in that dir turn there for rest of this
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("Move in ideal tested true while using best_move...!\n");
#endif
				turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
				//keep turning towards ideal until facing it
				if(turnamt < 1)
					self->monsterinfo.idle_time = 0;
				else
				{
					self->monsterinfo.idle_time = -1;
					return true;
				}
			}
		}
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));//turn to temp yaw
	}
	else
	{//using ideal_yaw
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
	}

	distloss = turnamt/self->yaw_speed * 0.8;//0.3;
	adj_dist = dist - (dist * distloss);
	
	trace = MG_WalkMove(self, self->s.angles[YAW], dist);
	if(trace.succeeded)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Move forward succeeded!\n");
#endif
		return true;
	}
	else if(self->classID == CID_TBEAST)
	{
		if(trace.fraction<1.0)
			VectorCopy(trace.endpos, self->s.origin);
	}

//if facing best_move_yaw and can't move that way, stop trying in that dir now.
	if(self->monsterinfo.idle_time > level.time && self->s.angles[YAW] == self->best_move_yaw)
	{
		new_best_yaw = true;
		oby = self->best_move_yaw;
		self->monsterinfo.idle_time = 0;
	}

//bumped into something
	if(trace.ent)
	{
		if(!stricmp(trace.ent->classname, "worldspawn"))
			hitworld = true;
		else
			hitworld = false;
		
		if(trace.ent == self->enemy)
		{//bumped into enemy, go get him!
			if(!(self->monsterinfo.aiflags & AI_COWARD) &&
				(!(self->monsterinfo.aiflags&AI_FLEE) || self->monsterinfo.flee_finished < level.time))
			{
				if(!(self->monsterinfo.aiflags&AI_NO_MELEE))
				{
					if(!(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY))
					{
						if(classStatics[self->classID].msgReceivers[MSG_MELEE] && infront(self, self->enemy))
						{
							QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
							return true;
						}
					}
				}
			}
		}
		else if(trace.ent->svflags & SVF_MONSTER)
		{//if bumped into a monster that's not after an enemy but not ambushing, bring him along
			if(!trace.ent->enemy)
			{
				if(trace.ent->health>0 && !trace.ent->monsterinfo.awake)
				{
					if(!(trace.ent->spawnflags & MSF_AMBUSH))
					{
						if(self->enemy)
						{
							if(self->enemy->client)
							{
								trace.ent->enemy = self->enemy;
								FoundTarget(trace.ent, false);
							}
						}
					}
				}
			}
		}

/*			
//FIXME: this needs to make sure they can break it
//also: do not do this if the monsters' enemy is visible (MASK_SOLID check, though)
		else if(!irand(0, 5))
		{//fixme: need to make sure the melee anims can break this
			if(self->classID > CID_RAT && classStatics[self->classID].msgReceivers[MSG_MELEE])
			{
				if(!(self->monsterinfo.aiflags&AI_NO_MELEE))
				{
					if(ok_to_break(trace.ent))
					{
						if(infront(self, trace.ent))
						{//smash it!
							if(MGAI_DEBUG)
								gi.dprintf("%s breaking blocking %s!\n", self->classname, trace.ent->classname);
							self->monsterinfo.aiflags |= AI_STRAIGHT_TO_ENEMY;//go straight at enemy, not buoys
							self->oldenemy_debounce_time = level.time + 7;//attack it for 7 seconds max
							self->oldenemy = self->enemy;//remember who I was after
							self->enemy = self->goalentity = trace.ent;//let's nail this sucker
							QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);//SMACK!
							return true;
						}
					}
				}
			}
		}
*/
		if(!hitworld)
		{
			if(self->monsterinfo.idle_time < level.time)
			{//not already following a weird dir
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("Move forward hit wall, newdir\n");
#endif
				self->monsterinfo.idle_time = level.time + flrand(0.5, 1.2);
				self->best_move_yaw = anglemod(180 + self->ideal_yaw);
				MG_NewDir(self, dist);
			}
			return false;
		}
#ifdef _DEVEL
		else if(MGAI_DEBUG)
			gi.dprintf("Bumped world - t_f: %f t_allsolid: %d, t_startsolid %d\n",trace.fraction, trace.allsolid, trace.startsolid);
#endif
	}

//Ledge?
	if((trace.fraction >= 0.5 + distloss ||self->classID == CID_ASSASSIN) && !trace.allsolid && !trace.startsolid)//a ledge
	{//why not tracefraction == 1.0?
		if(!(self->spawnflags & MSF_FIXED))
		{
			if(MG_CheckJump(self))//can jump off it
			{
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("Move forward jumped off ledge!\n");
#endif
				return true;
			}
			else if(self->classID == CID_ASSASSIN)
			{
				if(MG_ExtraCheckJump(self))
				{
#ifdef _DEVEL
					if(MGAI_DEBUG)
						gi.dprintf("Move straight to goal extra jumped off ledge!\n");
#endif
					return true;
				}
			}
		}

		if(trace.fraction >= 0.5)//even assassins skip this
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("Can't jump off, getting newdir\n");
#endif
			if(self->monsterinfo.idle_time < level.time)
			{//not already following some other dir, pick one
				self->monsterinfo.idle_time = level.time + flrand(1, 2);
				self->best_move_yaw = anglemod(180 + self->ideal_yaw);
				MG_NewDir(self, dist);//what if this fails to set one?
			}
			return false;
		}
	}

#ifdef _DEVEL
	if(MGAI_DEBUG)
		if(trace.allsolid || trace.startsolid)
			gi.dprintf("Move forward allsolid or startsolid!\n");
#endif
	//FROM HERE ON, ONLY CHANGES DIR, WILL NOT MOVE!

	//otherwise, go around it... this ONLY???
	//lock into this new yaw for a bit
	if(self->monsterinfo.idle_time > level.time)
	{//heading somewhere for a few secs, turn here
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Turning to newdir, not bumping\n");
#endif
		/*		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
		distloss = turnamt/self->yaw_speed * 0.3;
		dist -= (dist * distloss);*/
		return false;
	}

	if((hitworld || irand(0,10)<6)&&!goal_vis)
		self->monsterinfo.idle_time = level.time + flrand(1, 2);
	else
		self->monsterinfo.idle_time = level.time + flrand(0.5, 1.25);

	self->best_move_yaw = anglemod(180 + self->ideal_yaw);

	//if hit a wall and close to ideal yaw (with 5), try a new dir
	if(Vec3NotZero(trace.plane.normal)&&
		EqualAngle(self->s.angles[YAW], self->ideal_yaw, 5))
	{//a wall?
		vec3_t	wall_angles, wall_right, self_forward, new_forward, vf;
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Move forward hit wall, checking left/right/back...\n");
#endif

		//If facing a wall, turn faster, more facing the wall, faster the turn
		save_yaw_speed = self->yaw_speed;
		AngleVectors(self->s.angles, self_forward, NULL, NULL);
		WallDot = DotProduct(trace.plane.normal, self_forward);
		if(WallDot>0)
			WallDot = 0;//-1 to 0
		self->yaw_speed *= 1.25 - WallDot;//facing wall head-on = 2.25 times normal yaw speed

		vectoangles(trace.plane.normal, wall_angles);
		AngleVectors(wall_angles, NULL, wall_right, NULL);
		
		if(goal_vis)
		{//can see goal, turn towards IT first
			VectorSubtract(self->goalentity->s.origin, self->s.origin, self_forward);
			VectorNormalize(self_forward);
		}
		
		//Get closest angle off that wall to move in
		if(DotProduct(wall_right,self_forward)>0)
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("turning left\n");
#endif
			VectorCopy(wall_right, new_forward);
		}
		else
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("turning right\n");
#endif
			VectorScale(wall_right, -1, new_forward);
		}

		if(irand(0,10)<3)//30% chance of trying other way first
			VectorScale(new_forward, -1, new_forward);
		
		self->best_move_yaw=VectorYaw(new_forward);

		if(new_best_yaw && self->best_move_yaw == oby)
		{
			VectorScale(new_forward, -1, new_forward);
		
			self->best_move_yaw=VectorYaw(new_forward);
		}

		//make sure we can move in chosen dir
		//set up mins and maxes for these moves
		VectorCopy(self->mins, mins);
		VectorCopy(self->maxs, maxs);
		//Account for STEPSIZE
		mins[2] += stepsize;
		if(mins[2] >= self->maxs[2])
			mins[2] = self->maxs[2] - 1;
		//remember yaw in case all these fail!
		save_yaw = self->s.angles[YAW];

		//Haven't yawed yet, so this is okay
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
		distloss = turnamt/self->yaw_speed * 0.8;//0.3;
		adj_dist = dist - (dist * distloss);
		
		VectorCopy(new_forward, vf);
		//AngleVectors(self->s.angles, vf, NULL, NULL);

		VectorCopy(self->s.origin, source);
		VectorMA(source, adj_dist, vf, source);
		
		gi.trace (self->s.origin, mins, self->maxs, source, self, MASK_SOLID,&trace);//was MASK_SHOT

		if (trace.fraction < 1||trace.allsolid||trace.startsolid)
		{//Uh oh, try other way
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("turn other way\n");
#endif
			VectorScale(new_forward, -1, new_forward);
			self->best_move_yaw=VectorYaw(new_forward);			
			//restore yaw
			self->s.angles[YAW] = save_yaw;
			//try new dir
			turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
			distloss = turnamt/self->yaw_speed * 0.8;//0.3;
			adj_dist = dist - (dist * distloss);

			VectorCopy(new_forward, vf);
			//AngleVectors(self->s.angles, vf, NULL, NULL);
	
			VectorMA(source, adj_dist, vf, source);

			//Account for STEPSIZE
			mins[2] += stepsize;
			if(mins[2] >= self->maxs[2])
				mins[2] = self->maxs[2] - 1;
			
			gi.trace (self->s.origin, mins, self->maxs, source, self, MASK_SOLID,&trace);//was MASK_SHOT
			if (trace.fraction < 1||trace.allsolid||trace.startsolid)
			{//Uh oh!  Go straight away from wall
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("turn all the way around\n");
#endif
				self->best_move_yaw=wall_angles[YAW];
				//restore yaw
				self->s.angles[YAW] = save_yaw;
				//start turning this move, but don't actually move until next time
				MG_ChangeWhichYaw(self, YAW_BEST_MOVE);
			}
		}
		self->yaw_speed = save_yaw_speed;
		return false;
	}
	else//keep turning to ideal
		self->monsterinfo.idle_time = 0;

	//Must have bumped into something very strange (other monster?)
	//just pick a new random dir
#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("Don't know what I hit, choosing newdir for a second\n");
#endif	
	MG_NewDir(self, dist);
	return false;
}

qboolean MG_SwimFlyToGoal (edict_t *self, float dist)
{
	trace_t		trace;
	float		turnamt, distloss, adj_dist, save_yaw, save_yaw_speed, WallDot;//, save_ideal_yaw;
	vec3_t		mins, maxs, source, goal_dir, goalpos;//, vec, save_org;
	qboolean	goal_vis=false, hitworld = false, new_best_yaw = false;
	float		goal_dist, oby;

	trace.succeeded = false;

	if(self->classID != CID_GORGON)//they do their own yawing
		MG_FaceGoal(self, false);//get ideal yaw, but don't turn

	//are we very close to our goal? problem: what if something in between?
	if(!EqualAngle(self->s.angles[YAW], self->ideal_yaw, self->yaw_speed))
	{//we aren't really facing our ideal yet
		if(self->monsterinfo.searchType == SEARCH_BUOY||self->ai_mood == AI_MOOD_NAVIGATE)
		{
			VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, goal_dir);
			goal_dist = VectorNormalize(goal_dir);
			if(goal_dist < (self->maxs[0] + 24 + dist))
			{//we're close to our goal
				MG_ChangeWhichYaw(self, YAW_IDEAL);
				return true;//so close to enemy, just turn, no movement - not if rat?
			}
		}
		else if(self->enemy)
		{
			VectorSubtract(self->monsterinfo.nav_goal, self->s.origin, goal_dir);
			goal_dist = VectorNormalize(goal_dir);
			if(goal_dist < (self->maxs[0] + self->enemy->maxs[0] + dist*2))
			{//we're close to our goal
				MG_ChangeWhichYaw(self, YAW_IDEAL);
				return true;//so close to enemy, just turn, no movement - not if rat?
			}
		}
	}

	if(self->monsterinfo.idle_time == -1)
	{//have been told to just turn to ideal_yaw
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
		//keep turning towards ideal until facing it
		if(turnamt < 1)
			self->monsterinfo.idle_time = 0;
		else
			return true;
	}
	else if(self->monsterinfo.idle_time > level.time)
	{//using best_move_yaw
		if(EqualAngle(self->s.angles[YAW], self->best_move_yaw, 5))
		{//do a test move in the direction I would like to go:
			if(MG_TestMove(self, self->ideal_yaw, dist))
			{//can move in that dir turn there for rest of this
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("Move in ideal tested true while using best_move...!\n");
#endif
				turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
				//keep turning towards ideal until facing it
				if(turnamt < 1)
					self->monsterinfo.idle_time = 0;
				else
				{
					self->monsterinfo.idle_time = -1;
					return true;
				}
			}
		}
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));//turn to temp yaw
	}
	else
	{//using ideal_yaw
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_IDEAL));
	}

	distloss = turnamt/self->yaw_speed * 0.8;//0.3;
	adj_dist = dist - (dist * distloss);
	
	MG_GetGoalPos(self, goalpos);

	trace = MG_AirMove(self, goalpos, dist);
	if(trace.succeeded)
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Move forward succeeded!\n");
#endif
		return true;
	}

//if facing best_move_yaw and can't move that way, stop trying in that dir now.
	if(self->monsterinfo.idle_time > level.time && self->s.angles[YAW] == self->best_move_yaw)
	{
		new_best_yaw = true;
		oby = self->best_move_yaw;
		self->monsterinfo.idle_time = 0;
	}

//bumped into something
	if(trace.ent)
	{
		if(!stricmp(trace.ent->classname, "worldspawn"))
			hitworld = true;
		else
			hitworld = false;
		
		if(trace.ent == self->enemy)
		{//bumped into enemy, go get him!
			if(!(self->monsterinfo.aiflags & AI_COWARD) &&
				(!(self->monsterinfo.aiflags&AI_FLEE) || self->monsterinfo.flee_finished < level.time))
			{
				if(!(self->monsterinfo.aiflags&AI_NO_MELEE))
				{
					if(!(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY))
					{
						if(classStatics[self->classID].msgReceivers[MSG_MELEE] && infront(self, self->enemy))
						{
							QPostMessage(self, MSG_MELEE, PRI_DIRECTIVE, NULL);
							return true;
						}
					}
				}
			}
		}
		else if(trace.ent->svflags & SVF_MONSTER)
		{//if bumped into a monster that's not after an enemy but not ambushing, bring him along
			if(!trace.ent->enemy)
			{
				if(trace.ent->health>0 && !trace.ent->monsterinfo.awake)
				{
					if(!(trace.ent->spawnflags & MSF_AMBUSH))
					{
						if(self->enemy)
						{
							if(self->enemy->client)
							{
								trace.ent->enemy = self->enemy;
								FoundTarget(trace.ent, false);
							}
						}
					}
				}
			}
		}

		if(!hitworld)
		{
			if(self->monsterinfo.idle_time < level.time)
			{//not already following a weird dir
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("Move forward hit wall, newdir\n");
#endif
				self->monsterinfo.idle_time = level.time + flrand(0.5, 1.2);
				self->best_move_yaw = anglemod(180 + self->ideal_yaw);
				MG_NewDir(self, dist);
			}
			return false;
		}
#ifdef _DEVEL
		else if(MGAI_DEBUG)
			gi.dprintf("Bumped world - t_f: %f t_allsolid: %d, t_startsolid %d\n",trace.fraction, trace.allsolid, trace.startsolid);
#endif
	}

//Ledge?
/*	if(trace.fraction >= 0.5 + distloss && !trace.allsolid && !trace.startsolid)//a ledge
	{//why not tracefraction == 1.0?
		if(trace.fraction >= 0.5)//even assassins skip this
		{
			if(MGAI_DEBUG)
				gi.dprintf("Can't jump off, getting newdir\n");

			if(self->monsterinfo.idle_time < level.time)
			{//not already following some other dir, pick one
				self->monsterinfo.idle_time = level.time + flrand(1, 2);
				self->best_move_yaw = anglemod(180 + self->ideal_yaw);
				MG_NewDir(self, dist);//what if this fails to set one?
			}
			return false;
		}
	}*/

#ifdef _DEVEL
	if(MGAI_DEBUG)
		if(trace.allsolid || trace.startsolid)
			gi.dprintf("Move forward allsolid or startsolid!\n");
#endif
	//FROM HERE ON, ONLY CHANGES DIR, WILL NOT MOVE!

	//otherwise, go around it... this ONLY???
	//lock into this new yaw for a bit
	if(self->monsterinfo.idle_time > level.time)
	{//heading somewhere for a few secs, turn here
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Turning to newdir, not bumping\n");
#endif
		/*		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
		distloss = turnamt/self->yaw_speed * 0.3;
		dist -= (dist * distloss);*/
		return false;
	}

	if((hitworld || irand(0,10)<6)&&!goal_vis)
		self->monsterinfo.idle_time = level.time + flrand(1, 2);
	else
		self->monsterinfo.idle_time = level.time + flrand(0.5, 1.25);

	self->best_move_yaw = anglemod(180 + self->ideal_yaw);

	//if hit a wall and close to ideal yaw (with 5), try a new dir
	if(Vec3NotZero(trace.plane.normal)&&
		EqualAngle(self->s.angles[YAW], self->ideal_yaw, 5))
	{//a wall?
		vec3_t	wall_angles, wall_right, self_forward, new_forward, vf;
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("Move forward hit wall, checking left/right/back...\n");
#endif
		//If facing a wall, turn faster, more facing the wall, faster the turn
		save_yaw_speed = self->yaw_speed;
		AngleVectors(self->s.angles, self_forward, NULL, NULL);
		WallDot = DotProduct(trace.plane.normal, self_forward);
		if(WallDot>0)
			WallDot = 0;//-1 to 0
		self->yaw_speed *= 1.25 - WallDot;//facing wall head-on = 2.25 times normal yaw speed

		vectoangles(trace.plane.normal, wall_angles);
		AngleVectors(wall_angles, NULL, wall_right, NULL);
		
		if(goal_vis)
		{//can see goal, turn towards IT first
			VectorSubtract(self->goalentity->s.origin, self->s.origin, self_forward);
			VectorNormalize(self_forward);
		}
		
		//Get closest angle off that wall to move in
		if(DotProduct(wall_right,self_forward)>0)
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("turning left\n");
#endif
			VectorCopy(wall_right, new_forward);
		}
		else
		{
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("turning right\n");
#endif
			VectorScale(wall_right, -1, new_forward);
		}

		if(irand(0,10)<3)//30% chance of trying other way first
			VectorScale(new_forward, -1, new_forward);
		
		self->best_move_yaw=VectorYaw(new_forward);

		if(new_best_yaw && self->best_move_yaw == oby)
		{
			VectorScale(new_forward, -1, new_forward);
		
			self->best_move_yaw=VectorYaw(new_forward);
		}

		//make sure we can move in chosen dir
		//set up mins and maxes for these moves
		VectorCopy(self->mins, mins);
		VectorCopy(self->maxs, maxs);

		//remember yaw in case all these fail!
		save_yaw = self->s.angles[YAW];

		//Haven't yawed yet, so this is okay
		turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
		distloss = turnamt/self->yaw_speed * 0.8;//0.3;
		adj_dist = dist - (dist * distloss);
		
		VectorCopy(new_forward, vf);
		//AngleVectors(self->s.angles, vf, NULL, NULL);

		VectorCopy(self->s.origin, source);
		VectorMA(source, adj_dist, vf, source);
		
		gi.trace (self->s.origin, mins, self->maxs, source, self, MASK_SOLID,&trace);//was MASK_SHOT

		if (trace.fraction < 1||trace.allsolid||trace.startsolid)
		{//Uh oh, try other way
#ifdef _DEVEL
			if(MGAI_DEBUG)
				gi.dprintf("turn other way\n");
#endif
			VectorScale(new_forward, -1, new_forward);
			self->best_move_yaw=VectorYaw(new_forward);			
			//restore yaw
			self->s.angles[YAW] = save_yaw;
			//try new dir
			turnamt = Q_fabs(MG_ChangeWhichYaw(self, YAW_BEST_MOVE));
			distloss = turnamt/self->yaw_speed * 0.8;//0.3;
			adj_dist = dist - (dist * distloss);

			VectorCopy(new_forward, vf);
			//AngleVectors(self->s.angles, vf, NULL, NULL);
	
			VectorMA(source, adj_dist, vf, source);

			gi.trace (self->s.origin, mins, self->maxs, source, self, MASK_SOLID,&trace);//was MASK_SHOT
			if (trace.fraction < 1||trace.allsolid||trace.startsolid)
			{//Uh oh!  Go straight away from wall
#ifdef _DEVEL
				if(MGAI_DEBUG)
					gi.dprintf("turn all the way around\n");
#endif
				self->best_move_yaw=wall_angles[YAW];
				//restore yaw
				self->s.angles[YAW] = save_yaw;
				//start turning this move, but don't actually move until next time
				MG_ChangeWhichYaw(self, YAW_BEST_MOVE);
			}
		}
		self->yaw_speed = save_yaw_speed;
		return false;
	}
	else//keep turning to ideal
		self->monsterinfo.idle_time = 0;

	//Must have bumped into something very strange (other monster?)
	//just pick a new random dir
#ifdef _DEVEL
	if(MGAI_DEBUG)
		gi.dprintf("Don't know what I hit, choosing newdir for a second\n");
#endif
	
	MG_NewDir(self, dist);
	return false;
}
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

// Returns true if the spot is in front (in sight) of self.
qboolean infront_pos(const edict_t* self, const vec3_t pos) //TODO: rename to MG_IsInforntPos?
{
	vec3_t check_angles;

	if (Vec3NotZero(self->v_angle_ofs))
		VectorAdd(self->v_angle_ofs, self->s.angles, check_angles);
	else
		VectorCopy(self->s.angles, check_angles);

	vec3_t forward;
	AngleVectors(check_angles, forward, NULL, NULL);

	vec3_t vec;
	VectorSubtract(pos, self->s.origin, vec);
	VectorNormalize(vec);

	return (DotProduct(vec, forward) > 0.3f);
}

static void MG_PostJump(edict_t* self) //mxd. Added to reduce code duplication.
{
	if (classStatics[self->classID].msgReceivers[MSG_JUMP])
	{
		if (self->classID != CID_RAT && self->classID != CID_SSITHRA)
		{
			// Save velocity so can crouch first.
			VectorCopy(self->velocity, self->movedir);
			VectorClear(self->velocity);
		}

		QPostMessage(self, MSG_JUMP, PRI_DIRECTIVE, NULL);
		self->nextthink = level.time + 0.01f;
	}
	else
	{
		self->nextthink = level.time + 0.3f;
	}
}

static qboolean MG_AssassinCheckJump(edict_t* self) //mxd. Named 'MG_ExtraCheckJump' in original logic.
{
	vec3_t targ_org;
	vec3_t targ_mins;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return false;

		VectorCopy(level.buoy_list[self->buoy_index].origin, targ_org);

		if (!infront_pos(self, targ_org))
			return false;

		VectorClear(targ_mins);
	}
	else
	{
		if (self->goalentity == NULL || !infront(self, self->goalentity))
			return false;

		VectorCopy(self->goalentity->s.origin, targ_org);
		VectorCopy(self->goalentity->mins, targ_mins);
	}

	const qboolean check_down = (targ_org[2] < self->s.origin[2] - 28.0f);

	if (check_down) // Jumping down.
	{
		if (gi.pointcontents(self->s.origin) & CONTENTS_WATER) // Don't jump down when in water!
			return false;

		// Setup the trace.
		vec3_t source;
		VectorCopy(self->s.origin, source);

		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(source, 128.0f, forward, source);

		vec3_t maxs;
		VectorCopy(self->maxs, maxs);
		maxs[2] += 16.0f;

		trace_t trace;
		gi.trace(self->s.origin, self->mins, maxs, source, self, MASK_MONSTERSOLID, &trace);

		if (trace.fraction < 1.0f)
			return true; //TODO: MGAI_DEBUG: "checkdown: not clear infront" -- should return false?

		// Clear ahead and above.
		vec3_t source_bottom;
		VectorCopy(source, source_bottom);
		source_bottom[2] -= 1024.0f;

		// Trace down.
		gi.trace(source, self->mins, self->maxs, source_bottom, self, MASK_ALL, &trace);

		if (trace.allsolid || trace.startsolid)
			return false;

		if (trace.fraction == 1.0f || (trace.contents != CONTENTS_SOLID && trace.ent != self->enemy)) // Ground too far down...
			return false;

		VectorSubtract(trace.endpos, self->s.origin, source_bottom);
		VectorNormalize(source_bottom);
		self->ideal_yaw = VectorYaw(source_bottom);

		VectorMA(self->velocity, 300.0f, forward, self->velocity);
		self->velocity[2] += 150.0f;

		MG_PostJump(self); //mxd
		self->monsterinfo.jump_time = level.time + 1.0f;
	}
	else // Jumping over something.
	{
		vec3_t save_org;
		VectorCopy(self->s.origin, save_org);
		const qboolean can_move = M_walkmove(self, self->s.angles[YAW], 64.0f);
		VectorCopy(save_org, self->s.origin);

		if (can_move) // Can walk, no jumping required.
			return false;

		// Check to jump over something.
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);

		vec3_t source;
		VectorCopy(self->s.origin, source);

		vec3_t source2;
		VectorMA(source, 128.0f, forward, source2);

		vec3_t mins;
		VectorCopy(self->mins, mins);
		mins[2] += 24.0f; // Can clear it.

		trace_t trace;
		gi.trace(source, mins, self->maxs, source2, self, MASK_SOLID, &trace);

		if ((!trace.allsolid && !trace.startsolid && trace.fraction == 1.0f) || trace.ent == self->enemy)
		{
			// Go for it!
			VectorMA(self->velocity, trace.fraction * 500.0f, forward, self->velocity);
			self->velocity[2] += 225.0f;

			MG_PostJump(self); //mxd
			//TODO: should also set self->monsterinfo.jump_time here?
		}
		else // Check jump up.
		{
			const float height_diff = (targ_org[2] + targ_mins[2]) - (self->s.origin[2] + self->mins[2]) + 32.0f;

			vec3_t pos_top;
			VectorCopy(self->s.origin, pos_top);
			pos_top[2] += height_diff;

			gi.trace(self->s.origin, self->mins, self->maxs, pos_top, self, MASK_MONSTERSOLID, &trace);

			if (trace.fraction == 1.0f)
			{
				// Clear above.
				vec3_t pos_fwd;
				VectorCopy(pos_top, pos_fwd);

				AngleVectors(self->s.angles, forward, NULL, NULL);
				VectorMA(pos_top, 64.0f, forward, pos_fwd);
				pos_fwd[2] -= 24.0f;

				// Trace forward and down a little.
				gi.trace(pos_top, self->mins, self->maxs, pos_fwd, self, MASK_ALL, &trace);

				if (trace.allsolid || trace.startsolid)
					return false;

				if (trace.fraction < 0.1f)
					return false; // Can't jump up, no ledge.

				vec3_t jump_dir;
				VectorSubtract(trace.endpos, self->s.origin, jump_dir);
				jump_dir[2] = 0.0f;
				VectorNormalize(jump_dir);
				self->ideal_yaw = VectorYaw(jump_dir);

				VectorMA(self->s.origin, 64.0f, jump_dir, pos_fwd);

				// Trace directly forward to determine velocity.
				gi.trace(self->s.origin, vec3_origin, vec3_origin, pos_fwd, self, MASK_SOLID, &trace);

				VectorScale(jump_dir, trace.fraction * 480.0f, self->velocity);
				self->velocity[2] = height_diff * 3.0f + 200.0f;

				MG_PostJump(self); //mxd
				self->monsterinfo.jump_time = level.time + 1.0f;
			}
			else
			{
				return false; // Can't jump up, blocked.
			}
		}
	}

	return true;
}

// Checks to see if the enemy is not at the same level as monster or something is blocking the path of the monster.
// If there is a clear jump arc to the enemy and the monster will not land in water or lava, the monster will attempt to jump the distance.
static qboolean MG_CheckJump(edict_t* self)
{
#define JUMP_HEIGHT	16.0f //mxd

	static const vec3_t jump_mins = { -8.0f, -8.0f, 0.0f }; //mxd. Made local static.
	static const vec3_t jump_maxs = {  8.0f,  8.0f, 4.0f }; //mxd. Made local static.

	if (irand(1, 100) > self->jump_chance)
		return false;

	if (self->classID == CID_TBEAST)
		return TB_CheckJump(self);

	//FIXME: Allow jump in/out of water if not too deep (also check for '!(self->flags & FL_AMPHIBIAN)')?
	if (self->flags & FL_INWATER)
		return false; // Can't jump while in water.

	vec3_t targ_org;
	vec3_t targ_mins;

	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (self->buoy_index < 0 || self->buoy_index > level.active_buoys)
			return false;

		VectorCopy(level.buoy_list[self->buoy_index].origin, targ_org);

		if (!infront_pos(self, targ_org))
			return false;

		VectorCopy(targ_org, targ_mins);
	}
	else
	{
		if (self->goalentity == NULL || !infront(self, self->goalentity))
			return false;

		if (self->goalentity->groundentity == NULL && self->classID != CID_GORGON)
			return false; // goalentity in air.

		VectorCopy(self->goalentity->s.origin, targ_org);
		VectorAdd(targ_org, self->goalentity->mins, targ_mins);
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(self->s.angles, forward, NULL, up);

	vec3_t jump_dir;
	VectorSubtract(targ_org, self->s.origin, jump_dir);
	VectorNormalize(jump_dir);
	jump_dir[2] = 0.0f;

	if (DotProduct(jump_dir, forward) < 0.3f)
		return false; // Jump direction more than 60 degrees off of forward.

	vec3_t spot1;
	VectorCopy(self->s.origin, spot1);

	vec3_t spot2;
	VectorCopy(targ_org, spot2);

	vec3_t contents_spot;
	VectorMA(spot1, 24.0f, forward, contents_spot);
	contents_spot[2] -= 10.0f;

	qboolean ignore_height = !(gi.pointcontents(contents_spot) & CONTENTS_SOLID);

	const float spot_diff = vhlen(spot1, spot2);
	if (spot_diff > 256.0f)
		ignore_height = false;

	// Also check to make sure you can't walkmove forward.
	if (self->monsterinfo.jump_time > level.time) // Don't jump too many times in a row.
		return false;

	if (!ignore_height && targ_mins[2] + 36.0f >= self->absmin[2])
		return false; // Jump target too high.

	if (self->groundentity == NULL)
		return false; // Can't jump when not on ground.

	if (!ignore_height && spot_diff > 777.0f)
		return false; // Jump target too far away.

	if (spot_diff <= 100.0f)
		return false; // Jump target too close.

	//sfs -- Sure, it's just a dotproduct, but the other checks are a little cheaper.
	if (!infront_pos(self, targ_org))
		return false; // Goalentity not in front.

	//sfs -- Save the trace line for after the easy checks.
	if (!clear_visible_pos(self, targ_org))
	{
		vec3_t vis_check_spot;
		VectorMA(spot1, self->size[0] * 2.0f, forward, vis_check_spot);
		VectorMA(vis_check_spot, self->size[2] * 1.5f, up, vis_check_spot);

		if (!HaveLOS(self, vis_check_spot, spot2))
			return false; // Can't see goalentity.
	}

	if (!(self->monsterinfo.aiflags & AI_SWIM_OK) && (gi.pointcontents(spot2) & MASK_WATER))
		return false; // Goalentity in water, slime or lava.

	vec3_t start;
	VectorCopy(self->s.origin, start);
	start[2] += self->maxs[2];

	vec3_t end;
	VectorCopy(start, end);
	end[2] += 36.0f;

	trace_t trace;
	gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID, &trace);

	if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
		return false; // Not enough room above.

	VectorMA(start, (self->maxs[0] + self->maxs[1]) * 0.5f, jump_dir, start);

	vec3_t end_spot;
	VectorCopy(start, end_spot);
	end_spot[2] += 36.0f;

	gi.trace(self->s.origin, self->mins, self->maxs, end_spot, self, MASK_MONSTERSOLID, &trace);

	if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
		return false; // Not enough room in front.

	VectorMA(start, 64.0f, jump_dir, end_spot);
	end_spot[2] -= 500.0f;

	gi.trace(start, jump_mins, jump_maxs, end_spot, self, MASK_MONSTERSOLID, &trace);

	if (gi.pointcontents(trace.endpos) & MASK_WATER)
		return false; // Won't jump in water.

	MG_FaceGoal(self, true);

	//FIXME: make them do whatever jump function they have if they have one.
	self->monsterinfo.jump_time = level.time + 2.0f; // Only try to jump once every 2 seconds.

	assert(self->s.scale != 0.0f); //mxd. Does this ever happen?
	if (self->s.scale == 0.0f) //TODO: why set scale here, of all places?..
		self->s.scale = 1.0f;

	VectorScale(jump_dir, JUMP_HEIGHT * 18.0f * self->s.scale, self->velocity);
	self->velocity[2] = JUMP_HEIGHT * 14.0f * self->s.scale;

	MG_PostJump(self); //mxd

	return true; // Can jump.
}

// Tries to step forward dist, returns the trace.
trace_t MG_WalkMove(edict_t* self, float yaw, const float dist)
{
	yaw *= ANGLE_TO_RAD;
	vec3_t move = { cosf(yaw) * dist, sinf(yaw) * dist, 0.0f };
	trace_t trace = MG_MoveStep(self, move, true);

	if (!trace.succeeded)
	{
		// Failed? Find what's in front of us.
		vec3_t end_pos;
		VectorAdd(self->s.origin, move, end_pos);

		// Up mins for stairs?
		gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);
		trace.succeeded = false;
	}

	return trace;
}

// Tries to step forward dist, returns true/false.
qboolean MG_BoolWalkMove(edict_t* self, float yaw, const float dist)
{
	yaw *= ANGLE_TO_RAD;
	vec3_t move = { cosf(yaw) * dist, sinf(yaw) * dist, 0.0f };
	const trace_t trace = MG_MoveStep(self, move, true);

	return trace.succeeded;
}

// Checks if it can step dist in yaw, but doesn't do the move.
static qboolean MG_TestMove(edict_t* self, float yaw, const float dist) //TODO: same as above, except 'relink' MG_MoveStep arg.
{
	yaw *= ANGLE_TO_RAD;
	vec3_t move = { cosf(yaw) * dist, sinf(yaw) * dist, 0.0f };
	const trace_t trace = MG_MoveStep(self, move, false);

	return trace.succeeded;
}

void MG_CheckEvade(edict_t* self)
{
	//FIXME: only check my enemy? See if he's fired (last_attack) recently?
	if (SKILL == SKILL_EASY)
		return;

	edict_t* ent = NULL;
	while ((ent = FindInRadius(ent, self->s.origin, 500.0f)) != NULL)
	{
		if (ent->movetype != MOVETYPE_FLYMISSILE || ent->solid == SOLID_NOT || ent->owner == self || Vec3IsZero(ent->velocity))
			continue; // Not an evadeable projectile.

		vec3_t proj_dir;
		VectorCopy(ent->velocity, proj_dir);
		VectorNormalize(proj_dir);

		vec3_t end_pos;
		VectorMA(ent->s.origin, 600.0f, proj_dir, end_pos);

		trace_t trace;
		gi.trace(ent->s.origin, ent->mins, ent->maxs, end_pos, ent, MASK_MONSTERSOLID, &trace);

		if (trace.ent != self && irand(0, 2) == 0) //TODO: scale irand chance by difficulty?
		{
			vec3_t ent_dir;
			VectorSubtract(self->s.origin, ent->s.origin, ent_dir);
			const float ent_dist = VectorNormalize(ent_dir);
			const float proj_offset = DotProduct(ent_dir, proj_dir);

			if (proj_offset > ent_dist / 600.0f) // Farther it is, smaller angle deviation allowed for evasion.
			{
				// Coming pretty close.
				vec3_t ent_pos;
				VectorMA(ent->s.origin, ent_dist, proj_dir, ent_pos); // Extrapolate to close to me.

				gi.trace(ent_pos, ent->mins, ent->maxs, self->s.origin, ent, MASK_MONSTERSOLID, &trace);
			}
		}

		if (trace.ent != self)
			continue;

		// Perform evasive maneuvers.
		const HitLocation_t hit_loc = MG_GetHitLocation(self, ent, trace.endpos, vec3_origin);

		vec3_t total_dist;
		VectorSubtract(trace.endpos, ent->s.origin, total_dist);

		const float eta = VectorLength(total_dist) / VectorLength(ent->velocity);
		QPostMessage(self, MSG_EVADE, PRI_DIRECTIVE, "eif", ent, hit_loc, eta);
	}
}

// The monster has an enemy it is trying to kill or the monster is fleeing.
void ai_run(edict_t* self, const float dist) //TODO: rename to MGAI_Run.
{
	// Skip when fleeing and can't use buoys...
	if ((DEACTIVATE_BUOYS || !(self->monsterinfo.aiflags & AI_USING_BUOYS)) &&
		((self->monsterinfo.aiflags & AI_COWARD) || ((self->monsterinfo.aiflags & AI_FLEE) && self->monsterinfo.flee_finished >= level.time)))
	{
		ai_flee(self, dist);
		return;
	}

	if (self->ai_mood_flags & AI_MOOD_FLAG_DUMB_FLEE)
	{
		if (MG_GoToRandomBuoy(self))
		{
			self->monsterinfo.searchType = SEARCH_BUOY;
		}
		else
		{
			ai_flee(self, dist);
			return;
		}
	}

	if (!DEACTIVATE_BUOYS && (self->monsterinfo.aiflags & AI_USING_BUOYS) && !(self->monsterinfo.aiflags & AI_STRAIGHT_TO_ENEMY) && self->pathfind_nextthink <= level.time)
	{
		MG_BuoyNavigate(self);
		self->pathfind_nextthink = level.time + FRAMETIME;
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		ai_face_goal(self); // Just face thy enemy.
		return;
	}

	assert(dist >= 0.0f); //mxd. Original logic used 'if(dist)' check below, so...

	if (dist > 0.0f && !MG_MoveToGoal(self, dist) && self->classID == CID_SSITHRA)
		ssithraCheckJump(self);

	if (self->classID != CID_ASSASSIN && classStatics[self->classID].msgReceivers[MSG_EVADE] != NULL) // Assassin does his own checks.
		MG_CheckEvade(self); // Check if going to be hit and evade.
}

void mg_ai_charge(edict_t* self, const float dist) //TODO: rename to MGAI_Charge.
{
	if (self->enemy == NULL)
		return; //FIXME: send stand MSG?

	vec3_t diff;
	VectorSubtract(self->enemy->s.origin, self->s.origin, diff);

	self->ideal_yaw = VectorYaw(diff);

	MG_ChangeYaw(self);

	assert(dist >= 0.0f); //mxd. Original logic used 'if(dist)' check below, so...

	if (dist > 0.0f)
		MG_WalkMove(self, self->s.angles[YAW], dist);

	if (self->classID != CID_ASSASSIN && classStatics[self->classID].msgReceivers[MSG_EVADE] != NULL) // Assassin does his own checks. //TODO: mg_ai_charge is used ONLY by CID_ASSASSIN!
		MG_CheckEvade(self); // Check if going to be hit and evade.
}

static void BodyPhaseOutPostThink(edict_t* self) //mxd. Named 'body_phase_out' in original logic.
{
#define PHASE_OUT_STEP	30 //mxd

	if (self->s.color.a > PHASE_OUT_STEP)
	{
		self->s.color.a -= irand(PHASE_OUT_STEP / 2, PHASE_OUT_STEP);
		self->next_post_think = level.time + 0.05f;
	}
	else
	{
		self->s.color.a = 0;
		self->post_think = NULL;
		self->next_post_think = -1.0f;

		G_SetToFree(self);
	}
}

static trace_t MG_AirMove(edict_t* self, const vec3_t goal_pos, const float dist)
{
	vec3_t move_dir;
	VectorSubtract(goal_pos, self->s.origin, move_dir);
	VectorNormalize(move_dir);

	vec3_t end_pos;
	VectorMA(self->s.origin, dist, move_dir, end_pos);

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &trace);

	if (trace.allsolid || trace.startsolid || trace.fraction <= 0.01f)
	{
		trace.succeeded = false;
	}
	else
	{
		trace.succeeded = true;
		VectorCopy(trace.endpos, self->s.origin);
		gi.linkentity(self);
	}

	return trace;
}

void MG_PostDeathThink(edict_t* self)
{
#define FRONT			0
#define BACK			1
#define RIGHT			2
#define LEFT			3
#define MIN_DROP_DIST	0.125f

	self->post_think = BodyPhaseOutPostThink;
	self->next_post_think = level.time + 10.0f;

	if (self->groundentity == NULL || Vec3NotZero(self->velocity))
	{
		if (self->groundentity != NULL && self->friction == 1.0f) //FIXME: check avelocity?
			pitch_roll_for_slope(self, NULL);

		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 0.1f;

		return;
	}

	float cornerdist[4] = { 0 };
	float most_dist = MIN_DROP_DIST;
	int which_trace = 0;

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	vec3_t org;
	VectorCopy(self->s.origin, org);
	org[2] += self->mins[2];

	// Trace forwards.
	vec3_t start_pos;
	VectorMA(org, self->dead_size, forward, start_pos);

	vec3_t end_pos;
	VectorCopy(start_pos, end_pos);
	end_pos[2] -= 128.0f;

	trace_t trace1;
	gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, self, MASK_SOLID, &trace1);

	if (!trace1.allsolid && !trace1.startsolid)
	{
		cornerdist[FRONT] = trace1.fraction;

		if (trace1.fraction > most_dist)
		{
			most_dist = trace1.fraction;
			which_trace = 1;
		}
	}

	// Trace backwards.
	VectorMA(org, -self->dead_size, forward, start_pos);
	VectorCopy(start_pos, end_pos);
	end_pos[2] -= 128.0f;

	trace_t trace2;
	gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, self, MASK_SOLID, &trace2);

	if (!trace2.allsolid && !trace2.startsolid)
	{
		cornerdist[BACK] = trace2.fraction;

		if (trace2.fraction > most_dist)
		{
			most_dist = trace2.fraction;
			which_trace = 2;
		}
	}

	// Trace right.
	VectorMA(org, self->dead_size / 2.0f, right, start_pos);
	VectorCopy(start_pos, end_pos);
	end_pos[2] -= 128.0f;

	trace_t trace3;
	gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, self, MASK_SOLID, &trace3);

	if (!trace3.allsolid && !trace3.startsolid)
	{
		cornerdist[RIGHT] = trace3.fraction;

		if (trace3.fraction > most_dist)
		{
			most_dist = trace3.fraction;
			which_trace = 3;
		}
	}

	// Trace left.
	VectorMA(org, -self->dead_size / 2.0f, right, start_pos);
	VectorCopy(start_pos, end_pos);
	end_pos[2] -= 128.0f;

	trace_t trace4;
	gi.trace(start_pos, vec3_origin, vec3_origin, end_pos, self, MASK_SOLID, &trace4);

	if (!trace4.allsolid && !trace4.startsolid)
	{
		cornerdist[LEFT] = trace4.fraction;

		if (trace4.fraction > most_dist)
		{
			most_dist = trace4.fraction;
			which_trace = 4;
		}
	}

	// OK! Now if two opposite sides are hanging, use a third if any, else, do nothing.
	qboolean front_back_both_clear = (cornerdist[FRONT] > MIN_DROP_DIST && cornerdist[BACK] > MIN_DROP_DIST);
	qboolean right_left_both_clear = (cornerdist[RIGHT] > MIN_DROP_DIST && cornerdist[LEFT] > MIN_DROP_DIST);

	if (front_back_both_clear && right_left_both_clear)
		return;

	if (front_back_both_clear)
	{
		if (cornerdist[RIGHT] > MIN_DROP_DIST)
			which_trace = 3;
		else if (cornerdist[LEFT] > MIN_DROP_DIST)
			which_trace = 4;
		else
			return;
	}

	if (right_left_both_clear)
	{
		if (cornerdist[FRONT] > MIN_DROP_DIST)
			which_trace = 1;
		else if (cornerdist[BACK] > MIN_DROP_DIST)
			which_trace = 2;
		else
			return;
	}

	switch (which_trace)
	{
		// Check for stuck.
		case 1: // Front.
		{
			VectorMA(self->s.origin, self->maxs[0], forward, end_pos);

			trace_t move_trace;
			gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &move_trace);

			if (move_trace.allsolid || move_trace.startsolid || move_trace.fraction < 1.0f)
				which_trace = (movable(move_trace.ent) ? -1 : 0);
		} break;

		case 2: // Back.
		{
			VectorMA(self->s.origin, -self->maxs[0], forward, end_pos);

			trace_t move_trace;
			gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &move_trace);

			if (move_trace.allsolid || move_trace.startsolid || move_trace.fraction < 1.0f)
				which_trace = (movable(move_trace.ent) ? -1 : 0);
		} break;

		case 3: // Right.
		{
			VectorMA(self->s.origin, self->maxs[0], right, end_pos);

			trace_t move_trace;
			gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &move_trace);

			if (move_trace.allsolid || move_trace.startsolid || move_trace.fraction < 1.0f)
				which_trace = (movable(move_trace.ent) ? -1 : 0);
		} break;

		case 4: // Left.
		{
			VectorMA(self->s.origin, -self->maxs[0], right, end_pos);

			trace_t move_trace;
			gi.trace(self->s.origin, self->mins, self->maxs, end_pos, self, MASK_MONSTERSOLID, &move_trace);

			if (move_trace.allsolid || move_trace.startsolid || move_trace.fraction < 1.0f)
				which_trace = (movable(move_trace.ent) ? -1 : 0);
		} break;
	}

	switch (which_trace)
	{
		case 1: // Forward.
			VectorMA(self->velocity, 200.0f, forward, self->velocity);
			if (trace1.fraction >= 0.9f)
			{
				self->friction = 1.0f;
			}
			else
			{
				pitch_roll_for_slope(self, &trace1.plane.normal); //TODO: should pass trace1.plane.normal instead of pointer to it?
				self->friction = trace1.plane.normal[2] * 0.1f;
			}

			self->post_think = MG_PostDeathThink;
			self->next_post_think = level.time + 0.1f;
			return;

		case 2: // Back.
			VectorMA(self->velocity, -200.0f, forward, self->velocity);
			if (trace2.fraction >= 0.9f)
			{
				self->friction = 1.0f;
			}
			else
			{
				pitch_roll_for_slope(self, &trace2.plane.normal); //TODO: should pass trace1.plane.normal instead of pointer to it?
				self->friction = trace2.plane.normal[2] * 0.1f;
			}

			self->post_think = MG_PostDeathThink;
			self->next_post_think = level.time + 0.1f;
			return;

		case 3: // Right.
			VectorMA(self->velocity, 200.0f, right, self->velocity);
			if (trace3.fraction >= 0.9f)
			{
				self->friction = 1.0f;
			}
			else
			{
				pitch_roll_for_slope(self, &trace3.plane.normal); //TODO: should pass trace1.plane.normal instead of pointer to it?
				self->friction = trace3.plane.normal[2] * 0.1f;
			}
			self->post_think = MG_PostDeathThink;
			self->next_post_think = level.time + 0.1f;
			return;

		case 4: // Left.
			VectorMA(self->velocity, -200.0f, right, self->velocity);
			if (trace4.fraction >= 0.9f)
			{
				self->friction = 1.0f;
			}
			else
			{
				pitch_roll_for_slope(self, &trace4.plane.normal); //TODO: should pass trace1.plane.normal instead of pointer to it?
				self->friction = trace4.plane.normal[2] * 0.1f;
			}
			self->post_think = MG_PostDeathThink;
			self->next_post_think = level.time + 0.1f;
			return;
	}

	// On solid ground.
	if (which_trace == -1)
	{
		self->post_think = MG_PostDeathThink;
		self->next_post_think = level.time + 2.0f;

		return;
	}

	self->friction = 1.0f;

	VectorClear(self->avelocity);
	pitch_roll_for_slope(self, NULL);

	if (self->s.color.r == 0)
		self->s.color.r = 255;

	if (self->s.color.g == 0)
		self->s.color.g = 255;

	if (self->s.color.b == 0)
		self->s.color.b = 255;

	self->s.color.a = 255;

	self->post_think = BodyPhaseOutPostThink;
	const float delay = (self->classID == CID_RAT ? flrand(3.0f, 7.0f) : flrand(10.0f, 20.0f)); //mxd
	self->next_post_think = level.time + delay;

	gi.linkentity(self);
}

void MG_CheckLanded(edict_t* self, const float next_anim)
{
	if (self->groundentity != NULL)
	{
		SetAnim(self, (int)next_anim);
	}
	else if (self->velocity[2] < 0.0f)
	{
		vec3_t pos;
		VectorCopy(self->s.origin, pos);
		pos[2] += self->mins[2];

		VectorMA(pos, 0.5f, self->velocity, pos);

		if (gi.pointcontents(pos) & CONTENTS_SOLID)
			SetAnim(self, (int)next_anim);
	}
}

// Simple addition of velocity when not on ground.
void MG_InAirMove(edict_t* self, const float forward_speed, const float up_speed, const float right_speed)
{
	if (self->groundentity == NULL)
	{
		vec3_t forward;
		vec3_t right;
		vec3_t up;
		AngleVectors(self->s.angles, forward, right, up);

		VectorMA(self->velocity, up_speed, up, self->velocity);
		VectorMA(self->velocity, forward_speed, forward, self->velocity);
		VectorMA(self->velocity, right_speed, right, self->velocity);
	}
}

void MG_ApplyJump(edict_t* self)
{
	VectorCopy(self->movedir, self->velocity);
	VectorNormalize(self->movedir);

	self->jump_time = level.time + 0.5f;
	self->monsterinfo.aiflags &= ~AI_OVERRIDE_GUIDE;
}

void MG_NoBlocking(edict_t* self) //TODO: rename to MG_SetNoBlocking?
{
	self->svflags |= SVF_DEADMONSTER; // Now treat as a different content type.
	self->msgHandler = DeadMsgHandler; // No more messages at all.
}

qboolean MG_GetTargOrg(edict_t* self, vec3_t target_origin) //TODO: rename to MG_GetTargetOrigin?
{
	if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		if (self->buoy_index < 0 || self->buoy_index > level.active_buoys)
		{
			VectorClear(target_origin);
			return false;
		}

		VectorCopy(level.buoy_list[self->buoy_index].origin, self->monsterinfo.nav_goal);
		VectorCopy(self->monsterinfo.nav_goal, target_origin);
	}
	else
	{
		if (self->goalentity == NULL)
		{
			VectorClear(target_origin);
			return false;
		}

		VectorCopy(self->goalentity->s.origin, target_origin);
	}

	return true;
}

// Sees if the two angles are within leniency degrees of each other.
qboolean EqualAngle(float angle1, float angle2, const float leniency) //TODO: rename to IsEqualAngle?
{
	if (angle1 < -180.0f)
		angle1 += 360.0f;
	else if (angle1 > 180.0f)
		angle1 -= 360.0f;

	if (angle2 < -180.0f)
		angle2 += 360.0f;
	else if (angle2 > 180.0f)
		angle2 -= 360.0f;

	float diff = angle1 - angle2;

	if (diff < -180.0f)
		diff += 360.0f;
	else if (diff > 180.0f)
		diff -= 360.0f;

	return (fabsf(diff) <= leniency);
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
				if(MG_AssassinCheckJump(self))
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
//
// mg_guide.c -- High level monster guide information using BUOYAH! Navigation System(tm). Mike Gummelt & Josh Weier.
//
// Copyright 1998 Raven Software
//

//FIXME: way to send monsters to a buoy out of water/lava if they are drowning or burning.
//FIXME: when you get to a buoy, do a trace to the next. If it's blocked, you need to find another way...
//FIXME: way to handle lots of monsters gathered around the same buoy... The one that is at it can't get out, the others can't get to it, they just crowd it.

#include "mg_guide.h"
#include "g_monster.h"
#include "m_assassin.h" //mxd
#include "m_stats.h"
#include "Random.h"
#include "Vector.h"
#include "FX.h" //TODO: remove
#include "g_local.h"

// 10 seconds between choosing a buoy and getting there.
#define	BUOY_SEARCH_TIME	10	

// Number of passes through buoy list when searching.
// Only accept buoys closer than 1 / BUOY_SEARCH_PASSES * MAX_BUOY_DIST	for first pass, etc.
// If a buoy is found after a pass, we know we've got the closest buoy, and further passes can be skipped.
#define BUOY_SEARCH_PASSES	6

#pragma region ========================== Helper functions ==========================

qboolean MG_ReachedBuoy(const edict_t* self, const vec3_t p_spot)
{
	vec3_t spot;

	if (p_spot == NULL)
		VectorCopy(self->monsterinfo.nav_goal, spot);
	else
		VectorCopy(p_spot, spot);

	const float center_z = (self->absmin[2] + self->absmax[2]) * 0.5f;
	if (Q_fabs(spot[2] - center_z) > self->size[2])
		return false;

	const float dist = vhlen(spot, self->s.origin);
	const float radius = 24 + max(16.0f, self->maxs[0]);

	return (dist < radius + 24.0f);
}

static qboolean IsClearPath(const edict_t* self, const vec3_t end) //mxd. Named 'Clear_Path' in original version.
{
	if (DEACTIVATE_BUOYS || !gi.inPVS(self->s.origin, end)) // Quicker way to discard points that are very not in a clear path.
		return false;

	vec3_t mins;
	vec3_t maxs;
	VectorScale(self->mins, 0.5f, mins);
	VectorScale(self->maxs, 0.5f, maxs);

	if (self->mins[2] + 18.0f > mins[2]) // Need to account for stepheight.
	{
		// Took off less than 18.
		mins[2] = self->mins[2] + 18.0f;
		maxs[2] = max(mins[2], maxs[2]);
	}

	trace_t trace;
	gi.trace(self->s.origin, mins, maxs, end, self, MASK_SOLID, &trace);

	return (trace.fraction == 1.0f || trace.ent == self->enemy);
}

// Returns true if the spot is visible, but not through transparencies.
qboolean clear_visible_pos(const edict_t* self, const vec3_t spot) //TODO: rename to MG_ClearVisiblePos.
{
	if (self == NULL || !gi.inPVS(self->s.origin, spot)) // Quicker way to discard points that are very not visible.
		return false;

	vec3_t start;
	VectorCopy(self->s.origin, start);
	start[2] += (float)self->viewheight;

	if (self->classID == CID_TBEAST)
	{
		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(start, self->maxs[0], forward, start);
	}

	trace_t trace;
	gi.trace(start, vec3_origin, vec3_origin, spot, self, MASK_SOLID, &trace);

	return (trace.fraction == 1.0f);
}

void MG_RemoveBuoyEffects(const edict_t* self) //TODO: remove.
{
}

int MG_SetFirstBuoy(edict_t* self)
{
	if (self->client == NULL && !(self->monsterinfo.aiflags & AI_USING_BUOYS))
	{
		self->ai_mood = AI_MOOD_PURSUE;
		return NULL_BUOY;
	}

	if (DEACTIVATE_BUOYS)
		return NULL_BUOY;

	// First, pre-calculate all distances.
	for (int i = 0; i <= level.active_buoys; i++)
	{
		buoy_t* buoy = &level.buoy_list[i];

		vec3_t vec;
		VectorSubtract(self->s.origin, buoy->origin, vec);
		buoy->temp_dist = VectorLength(vec);
	}

	const float search_pass_interval = MAX_BUOY_DIST / BUOY_SEARCH_PASSES;
	const buoy_t* best_buoy = NULL;
	float best_dist = 9999999.0f;

	// Now, do all the passes, going from closest to farthest.
	for (int i = 0; i < BUOY_SEARCH_PASSES && best_buoy == NULL; i++)
	{
		for (int c = 0; c <= level.active_buoys; c++)
		{
			const buoy_t* buoy = &level.buoy_list[c];
			const float dist = buoy->temp_dist;

			// Only consider buoys in the current interval. Closer ones have already been checked, and we'll save farther ones for later.
			if (dist < best_dist && dist > search_pass_interval * (float)i && dist < search_pass_interval * (float)(i + 1))
			{
				if (IsClearPath(self, buoy->origin))
				{
					best_dist = dist;
					best_buoy = buoy;
				}
			}
		}
	}

	if (best_buoy == NULL)
		return NULL_BUOY;

	if (self->client == NULL)
	{
		self->lastbuoy = NULL_BUOY;
		self->buoy_index = best_buoy->id;
		VectorCopy(best_buoy->origin, self->monsterinfo.nav_goal);
	}

	return best_buoy->id;
}

qboolean MG_GoToRandomBuoy(edict_t* self)
{
	if (MG_SetFirstBuoy(self) == NULL_BUOY)
		return false;

	const buoy_t* found_buoy = &level.buoy_list[self->buoy_index];
	qboolean dead_end = false;
	qboolean branch_checked[MAX_BUOY_BRANCHES];
	int last_buoy_id = NULL_BUOY;

	for (int i = 0; i < self->mintel && !dead_end; i++)
	{
		for (int j = 0; j < MAX_BUOY_BRANCHES; j++)
			branch_checked[j] = false;

		while (true)
		{
			const int next_branch = irand(0, MAX_BUOY_BRANCHES - 1);
			branch_checked[next_branch] = true;

			int branches_checked = false;

			for (int j = 0; j < MAX_BUOY_BRANCHES; j++)
				if (branch_checked[j])
					branches_checked++;

			if (found_buoy->nextbuoy[next_branch] > NULL_BUOY &&
				found_buoy->nextbuoy[next_branch] != self->buoy_index &&
				(found_buoy->nextbuoy[next_branch] != self->lastbuoy || branches_checked >= MAX_BUOY_BRANCHES) &&
				(found_buoy->nextbuoy[next_branch] != last_buoy_id || branches_checked >= MAX_BUOY_BRANCHES))
			{
				// nextbuoy off this one is not null, not my start buoy, not my last buoy, and not last buoy found.
				last_buoy_id = found_buoy->id;
				found_buoy = &level.buoy_list[found_buoy->nextbuoy[next_branch]];

				// Break inner loop.
				break;
			}

			if (branches_checked >= MAX_BUOY_BRANCHES)
			{
				// A dead end, checked all 3 branches.
				if (i < MAX_BUOY_BRANCHES) // Can't run away far enough.
					return false;

				// Break both loops.
				dead_end = true;
				break;
			}
		}
	}

	if (self->ai_mood == AI_MOOD_FLEE)
		self->ai_mood_flags |= AI_MOOD_FLAG_IGNORE_ENEMY;
	else
		self->ai_mood = AI_MOOD_NAVIGATE; // Wander?

	self->ai_mood_flags &= ~AI_MOOD_FLAG_DUMB_FLEE;
	self->ai_mood_flags |= AI_MOOD_FLAG_FORCED_BUOY;
	self->forced_buoy = found_buoy->id;

	QPostMessage(self, MSG_RUN, PRI_DIRECTIVE, NULL);

	MG_RemoveBuoyEffects(self);
	MG_MakeConnection(self, NULL, false);

	return true;
}

// Assigns the start_buoy as the monster's buoy.
static void MG_AssignMonsterNextBuoy(edict_t* self, const buoy_t* start_buoy)
{
	VectorCopy(start_buoy->origin, self->monsterinfo.nav_goal);

	self->lastbuoy = self->buoy_index;
	self->buoy_index = start_buoy->id;
	self->last_buoy_time = level.time;
}

// See if this entity and this buoy are ok to be associated (clear path, etc.).
static qboolean MG_IsValidBestBuoyForEnt(edict_t* ent, const buoy_t* test_buoy) //mxd. Named 'MG_ValidBestBuoyForEnt' in original version.
{
	vec3_t diff;
	VectorSubtract(ent->s.origin, test_buoy->origin, diff);

	if (VectorLengthSquared(diff) <= 250000.0f) // 500 squared.
		return MG_CheckClearPathToSpot(ent, test_buoy->origin);

	return false; // Too far.
}

// Makes the connection between two buoys.
static qboolean MG_ResolveBuoyConnection(edict_t* self, const buoy_t* best_buoy, const buoy_t* e_best_buoy, const vec3_t goal_pos, qboolean dont_use_last, const qboolean skip_jump)
{
	// When called directly, this does not and should not set player_buoy.

	//FIXME: Allow assassins to take any buoy even if can't make a connection, since they can teleport.
	// Basically, pick the player's buoy and randomly pick one off of it or even that one?..
	if (self->lastbuoy == e_best_buoy->id && !MG_ReachedBuoy(self, e_best_buoy->origin) && !clear_visible_pos(self, goal_pos))
	{
		self->lastbuoy = NULL_BUOY;
		dont_use_last = false;
	}

	if ((best_buoy->modflags & BUOY_JUMP) && best_buoy->jump_target_id == e_best_buoy->id)
	{
		self->monsterinfo.searchType = SEARCH_BUOY;

		if (self->ai_mood != AI_MOOD_FLEE)
			self->ai_mood = AI_MOOD_NAVIGATE;

		const buoy_t* dest = (skip_jump ? e_best_buoy : best_buoy); //mxd
		VectorCopy(dest->origin, self->monsterinfo.nav_goal);

		self->lastbuoy = self->buoy_index;
		self->buoy_index = dest->id;
		self->last_buoy_time = level.time;

		return true;
	}

	// What if we're touching our e_bestbuoy buoy? Also, if this is a jump buoy and e_bestboy is the target of it, force use of bestbuoy.
	if (best_buoy == e_best_buoy)
	{
		if (dont_use_last && self->lastbuoy == best_buoy->id) // Found same buoy just touched as next buoy.
			return false;

		self->monsterinfo.searchType = SEARCH_BUOY;

		if (self->ai_mood != AI_MOOD_FLEE)
			self->ai_mood = AI_MOOD_NAVIGATE;

		VectorCopy(best_buoy->origin, self->monsterinfo.nav_goal);

		self->lastbuoy = self->buoy_index;
		self->buoy_index = best_buoy->id;
		self->last_buoy_time = level.time;

		return true;
	}

	if (best_buoy != NULL && e_best_buoy != NULL)
	{
		const buoy_t* dest = FindNextBuoy(self, best_buoy->id, e_best_buoy->id);

		if (dest != NULL)
		{
			if (dont_use_last && self->lastbuoy == dest->id) // Found same buoy just touched as next buoy.
				return false;

			self->monsterinfo.searchType = SEARCH_BUOY;

			if (self->ai_mood != AI_MOOD_FLEE)
				self->ai_mood = AI_MOOD_NAVIGATE;

			if ((!(best_buoy->modflags & BUOY_JUMP) || skip_jump) && MG_ReachedBuoy(self, best_buoy->origin))
				MG_AssignMonsterNextBuoy(self, dest);
			else
				MG_AssignMonsterNextBuoy(self, best_buoy);

			return true;
		}
	}

	return false;
}

// Attempts to make a connection between a buoy and a monster's enemy.
// This function itself just finds the two buoys to attempt to make the connection between,
// MG_ResolveBuoyConnection() actually makes the connection between two buoys.
static qboolean MG_MakeStartForcedConnection(edict_t* self, const int forced_buoy, const qboolean dont_use_last, const qboolean skip_jump)
{
	if (DEACTIVATE_BUOYS)
		return false;

	self->last_buoyed_enemy = self->enemy; // Remember the last enemy I looked for.

	vec3_t goal_pos;
	VectorCopy(self->enemy->s.origin, goal_pos);
	goal_pos[2] += (float)self->viewheight;

	const buoy_t* e_best_buoy = NULL;
	float e_best_dist = 9999999.0f;
	const float e_radius = 24.0f + max(16.0f, self->enemy->maxs[0]);

	// First, pre-calculate all distances.
	for (int i = 0; i <= level.active_buoys; i++)
	{
		buoy_t* buoy = &level.buoy_list[i];

		vec3_t vec;
		VectorSubtract(goal_pos, buoy->origin, vec);
		buoy->temp_e_dist = VectorLength(vec);

		if (buoy->temp_e_dist < e_radius + 24.0f)
		{
			e_best_buoy = buoy;
			e_best_dist = buoy->temp_dist;

			break;
		}
	}

	const float search_pass_interval = MAX_BUOY_DIST / BUOY_SEARCH_PASSES;
	const buoy_t* best_buoy = &level.buoy_list[forced_buoy];

	// Now, do all the passes, going from closest to farthest.
	for (int i = 0; i < BUOY_SEARCH_PASSES && (best_buoy == NULL || e_best_buoy == NULL); i++)
	{
		for (int c = 0; c <= level.active_buoys; c++)
		{
			const buoy_t* buoy = &level.buoy_list[c];
			const float e_dist = buoy->temp_e_dist;

			// Only consider buoys in the current interval. Closer ones have already been checked, and we'll save farther ones for later.
			if (e_dist < e_best_dist && e_dist > search_pass_interval * (float)i && e_dist < search_pass_interval * (float)(i + 1))
			{
				if (IsClearPath(self->enemy, buoy->origin))
				{
					e_best_dist = e_dist;
					e_best_buoy = buoy;
				}
			}
		}
	}

	if (e_best_buoy == NULL && irand(0, 10) < 5)
	{
		// Clear_Path too restrictive, try just clear_visible.
		// Distances are pre-calculated already, so skip that step.

		// Now, do all the passes, going from closest to farthest.
		for (int i = 0; (i < BUOY_SEARCH_PASSES) && (!e_best_buoy); i++)
		{
			for (int c = 0; c <= level.active_buoys; c++)
			{
				const buoy_t* found_buoy = &level.buoy_list[c];
				const float e_dist = found_buoy->temp_e_dist;

				// Only consider buoys in the current interval. Closer ones have already been checked, and we'll save farther ones for later.
				if (e_dist < e_best_dist && e_dist > search_pass_interval * (float)i && e_dist < search_pass_interval * (float)(i + 1))
				{
					if (clear_visible_pos(self->enemy, found_buoy->origin))
					{
						e_best_dist = e_dist;
						e_best_buoy = found_buoy;
					}
				}
			}
		}
	}

	// Closest buoy too far away...
	if (e_best_dist > MAX_BUOY_DIST)
		return false;

	// Don't skip jump buoys, they're crucial.
	if ((!(best_buoy->modflags & BUOY_JUMP) || skip_jump) && e_best_buoy != NULL)
	{
		vec3_t e_buoy_vec;
		VectorSubtract(e_best_buoy->origin, self->s.origin, e_buoy_vec);
		const float e_buoy_dist = VectorLength(e_buoy_vec);

		// Enemy best buoy is farther away from me and not my buoy.
		if (best_buoy != e_best_buoy && e_buoy_dist > 0.0f && IsClearPath(self, e_best_buoy->origin))
			best_buoy = e_best_buoy; // Can go straight at enemy best buoy even though farther away.
	}

	// If going after a player, set his buoy for other monsters this frame.
	if (e_best_buoy != NULL && self->enemy->client != NULL)
		level.player_buoy[self->enemy->s.number - 1] = e_best_buoy->id;

	return MG_ResolveBuoyConnection(self, best_buoy, e_best_buoy, goal_pos, dont_use_last, skip_jump);
}

// Attempts to make a connection between a monster and its forced_buoy.
// This function itself just finds the two buoys to attempt to make the connection between,
// MG_ResolveBuoyConnection actually makes the connection between two buoys.
static qboolean MG_MakeForcedConnection(edict_t* self, const int forced_buoy, const qboolean dont_use_last, const qboolean skip_jump)
{
	if (DEACTIVATE_BUOYS)
		return false;

	const buoy_t* e_best_buoy = &level.buoy_list[forced_buoy];

	vec3_t goal_pos;
	VectorCopy(e_best_buoy->origin, goal_pos);

	const buoy_t* best_buoy = NULL;
	float best_dist = 9999999.0f;
	const float radius = 24.0f + max(16.0f, self->maxs[0]);

	// First, pre-calculate all distances.
	for (int i = 0; i <= level.active_buoys; i++)
	{
		buoy_t* buoy = &level.buoy_list[i];

		vec3_t vec;
		VectorSubtract(self->s.origin, buoy->origin, vec);
		buoy->temp_dist = VectorLength(vec);

		if (buoy->temp_dist < radius + 24.0f)
		{
			best_buoy = buoy;
			best_dist = buoy->temp_dist;

			break;
		}
	}

	const float search_pass_interval = MAX_BUOY_DIST / BUOY_SEARCH_PASSES;

	// Now, do all the passes, going from closest to farthest.
	for (int i = 0; i < BUOY_SEARCH_PASSES && (best_buoy == NULL || e_best_buoy == NULL); i++)
	{
		for (int c = 0; c <= level.active_buoys; c++)
		{
			const buoy_t* found_buoy = &level.buoy_list[c];
			const float dist = found_buoy->temp_dist;

			// Only consider buoys in the current interval. Closer ones have already been checked, and we'll save farther ones for later.
			if (dist < best_dist && dist > search_pass_interval * (float)i && dist < search_pass_interval * (float)(i + 1))
			{
				if (IsClearPath(self, found_buoy->origin))
				{
					best_dist = dist;
					best_buoy = found_buoy;
				}
			}
		}
	}

	if (best_dist > MAX_BUOY_DIST)
	{
		self->pathfind_nextthink = level.time + 3; // Wait 3 seconds before trying to use buoys again.
		return false;
	}

	// Don't skip jump buoys, they're crucial.
	if (!(best_buoy->modflags & BUOY_JUMP) || skip_jump)
	{
		vec3_t e_buoy_vec;
		VectorSubtract(e_best_buoy->origin, self->s.origin, e_buoy_vec);
		const float e_buoy_dist = VectorLength(e_buoy_vec);

		// Enemy best buoy is farther away from me and not my buoy.
		if (best_buoy != e_best_buoy && e_buoy_dist > best_dist && IsClearPath(self, e_best_buoy->origin))
			best_buoy = e_best_buoy; // Can go straight at enemy best buoy even though farther away.
	}

	return MG_ResolveBuoyConnection(self, best_buoy, e_best_buoy, goal_pos, dont_use_last, skip_jump);
}

// Attempts to make a buoy connection between a monster and its enemy.
// This function itself just finds the two buoys to attempt to make the connection between,
// MG_ResolveBuoyConnection actually makes the connection between two buoys.
static qboolean MG_MakeNormalConnection(edict_t* self, const qboolean dont_use_last, const qboolean skip_jump)
{
	if (DEACTIVATE_BUOYS)
		return false;

	vec3_t goal_pos;
	VectorCopy(self->enemy->s.origin, goal_pos);
	goal_pos[2] += (float)self->viewheight;

	const buoy_t* best_buoy = NULL;
	float best_dist = 9999999.0f;
	const float radius = 24.0f + max(16.0f, self->maxs[0]);

	const buoy_t* e_best_buoy = NULL;
	float e_best_dist = 9999999.0f;
	const float e_radius = 24.0f + max(16.0f, self->enemy->maxs[0]);

	// First, pre-calculate all distances.
	for (int i = 0; i <= level.active_buoys; i++)
	{
		buoy_t* buoy = &level.buoy_list[i];

		if (best_buoy == NULL)
		{
			vec3_t vec;
			VectorSubtract(self->s.origin, buoy->origin, vec);
			buoy->temp_dist = VectorLength(vec);

			if (buoy->temp_dist < radius + 24.0f)
			{
				best_buoy = buoy;
				best_dist = buoy->temp_dist;
			}
		}

		if (e_best_buoy == NULL)
		{
			vec3_t vec;
			VectorSubtract(goal_pos, buoy->origin, vec);
			buoy->temp_e_dist = VectorLength(vec);

			if (buoy->temp_e_dist < e_radius + 24.0f)
			{
				e_best_buoy = buoy;
				e_best_dist = buoy->temp_dist;
			}
		}

		if (e_best_buoy && best_buoy)
			break;
	}

	const float search_pass_interval = MAX_BUOY_DIST / BUOY_SEARCH_PASSES;

	// Now, do all the passes, going from closest to farthest.
	for (int i = 0; i < BUOY_SEARCH_PASSES && (best_buoy == NULL || e_best_buoy == NULL); i++)
	{
		for (int c = 0; c <= level.active_buoys; c++)
		{
			const buoy_t* buoy = &level.buoy_list[c];
			const float dist = buoy->temp_dist;
			const float e_dist = buoy->temp_e_dist;

			// Only consider buoys in the current interval. Closer ones have already been checked, and we'll save farther ones for later.
			if (dist < best_dist && dist > search_pass_interval * (float)i && dist < search_pass_interval * (float)(i + 1))
			{
				if (IsClearPath(self, buoy->origin))
				{
					best_dist = dist;
					best_buoy = buoy;
				}
			}

			// Only consider buoys in the current interval. Closer ones have already been checked, and we'll save farther ones for later.
			if (e_dist < e_best_dist && e_dist > search_pass_interval * (float)i && e_dist < search_pass_interval * (float)(i + 1))
			{
				if (IsClearPath(self->enemy, buoy->origin))
				{
					e_best_dist = e_dist;
					e_best_buoy = buoy;
				}
			}
		}
	}

	// Closest buoy too far away...
	if (best_dist > MAX_BUOY_DIST)
		return false;

	if (best_buoy == NULL && e_best_buoy == NULL)
		return false;

	if (e_best_buoy == NULL && irand(0, 10) < 5)
	{
		// Clear_Path too restrictive, try just clear_visible.
		// Distances are pre-calculated already, so skip that step.

		// Now, do all the passes, going from closest to farthest.
		for (int i = 0; i < BUOY_SEARCH_PASSES && e_best_buoy == NULL; i++)
		{
			for (int c = 0; c <= level.active_buoys; c++)
			{
				const buoy_t* buoy = &level.buoy_list[c];
				const float e_dist = buoy->temp_e_dist;

				// Only consider buoys in the current interval. Closer ones have already been checked, and we'll save farther ones for later.
				if (e_dist < e_best_dist && e_dist > search_pass_interval * (float)i && e_dist < search_pass_interval * (float)(i + 1))
				{
					if (clear_visible_pos(self->enemy, buoy->origin))
					{
						e_best_dist = e_dist;
						e_best_buoy = buoy;
					}
				}
			}
		}
	}

	// Closest buoy too far away...
	if (e_best_dist > MAX_BUOY_DIST)
		return false;

	// Don't skip jump buoys, they're crucial.
	if ((!(best_buoy->modflags & BUOY_JUMP) || skip_jump) && e_best_buoy != NULL)
	{
		vec3_t e_buoy_vec;
		VectorSubtract(e_best_buoy->origin, self->s.origin, e_buoy_vec);
		const float e_buoy_dist = VectorLength(e_buoy_vec);

		// Enemy best buoy is farther away from me and not my buoy.
		if (best_buoy != e_best_buoy && e_buoy_dist > best_dist && IsClearPath(self, e_best_buoy->origin))
			best_buoy = e_best_buoy; // Can go straight at enemy best buoy even though farther away.
	}

	// If going after a player, set his buoy for other monsters this frame.
	if (e_best_buoy != NULL && self->enemy->client != NULL)
		level.player_buoy[self->enemy->s.number - 1] = e_best_buoy->id;

	return MG_ResolveBuoyConnection(self, best_buoy, e_best_buoy, goal_pos, dont_use_last, skip_jump);
}

// Determines if monster should be pursuing a forced_buoy. If not, calls normal enemy-tracking buoy connection finding function.
static int MG_TryMakeConnection(edict_t* self, const buoy_t* first_buoy, const qboolean skip_jump) //mxd. Named 'MG_MakeConnection_Go' in original logic.
{
	if (self->spawnflags & MSF_FIXED)
		return 0;

	MG_RemoveBuoyEffects(self);

	qboolean dont_use_last;
	qboolean found_path = false;
	qboolean last_buoy_clear = false;

	if (self->enemy != NULL && !(self->ai_mood_flags & AI_MOOD_FLAG_IGNORE_ENEMY) && self->ai_mood != AI_MOOD_FLEE)
	{
		self->ai_mood_flags &= ~AIMF_CANT_FIND_ENEMY;

		if (self->enemy != self->last_buoyed_enemy) // Current enemy wasn't the last enemy I looked for...
		{
			dont_use_last = false;
			self->lastbuoy = NULL_BUOY; // So forget last buoy I used.
		}
		else
		{
			dont_use_last = (self->monsterinfo.searchType == SEARCH_BUOY);
		}

		self->last_buoyed_enemy = self->enemy; // Remember the last enemy I looked for.

		// See if this player already has a best_buoy found by a previous monster this frame.
		if (self->enemy->client != NULL && level.player_buoy[self->enemy->s.number - 1] > NULL_BUOY) // Entity numbers 0 - 8 should be players.
		{
			//FIXME: if not, try his last player_buoy first!
			self->forced_buoy = level.player_buoy[self->enemy->s.number - 1]; // Just stores id in player_buoy.

			if (first_buoy != NULL)
			{
				const buoy_t* forced_buoy = &level.buoy_list[self->forced_buoy];
				found_path = MG_ResolveBuoyConnection(self, first_buoy, forced_buoy, forced_buoy->origin, dont_use_last, skip_jump);
			}
			else
			{
				found_path = MG_MakeForcedConnection(self, self->forced_buoy, dont_use_last, skip_jump);
			}
		}
	}
	else
	{
		self->last_buoyed_enemy = NULL;

		if (self->ai_mood_flags & AI_MOOD_FLAG_FORCED_BUOY && self->forced_buoy != -1)
		{
			if (first_buoy)
			{
				const buoy_t* forced_buoy = &level.buoy_list[self->forced_buoy];
				return MG_ResolveBuoyConnection(self, first_buoy, forced_buoy, forced_buoy->origin, false, skip_jump);
			}

			return MG_MakeForcedConnection(self, self->forced_buoy, false, skip_jump);
		}

		self->lastbuoy = NULL_BUOY; // So forget last buoy I used.
		return 0;
	}

	if (!found_path)
	{
		if (self->ai_mood == AI_MOOD_FLEE)
			return 0;

		if (self->enemy->client != NULL && level.player_last_buoy[self->enemy->s.number - 1] > NULL_BUOY)
		{
			// See if the player_last_buoy is a valid buoy for the enemy, if so, go for it.
			if (MG_IsValidBestBuoyForEnt(self->enemy, &level.buoy_list[level.player_last_buoy[self->enemy->s.number - 1]]))
			{
				last_buoy_clear = true;
				goto last_resort;
			}
		}

		if (first_buoy)
			found_path = MG_MakeStartForcedConnection(self, first_buoy->id, dont_use_last, skip_jump);
		else
			found_path = MG_MakeNormalConnection(self, dont_use_last, skip_jump);
	}

	if (found_path)
		return 1;

	// Can't find ANY buoy connections, let's go with player_last_buoy even if it can't connect to you!
	self->ai_mood_flags |= AIMF_CANT_FIND_ENEMY;

last_resort:
	// Try the player_last_buoy, don't care if it can connect to player!
	if (self->enemy->client != NULL && level.player_last_buoy[self->enemy->s.number - 1] > NULL_BUOY)
	{
		//FIXME: require that the player be withing 250 of this buoy at least?

		// We don't actually set self->forced_buoy since we want to find a better buoy next time we look.
		const buoy_t* forced_buoy = &level.buoy_list[level.player_last_buoy[self->enemy->s.number - 1]];

		if (!last_buoy_clear && (self->ai_mood_flags & AIMF_SEARCHING || MG_ReachedBuoy(self, forced_buoy->origin)))
			return 3;

		if (first_buoy && MG_ResolveBuoyConnection(self, first_buoy, forced_buoy, forced_buoy->origin, dont_use_last, skip_jump))
			return 2;

		if (MG_MakeForcedConnection(self, forced_buoy->id, dont_use_last, skip_jump))
			return 2;
	}

	// Damn, lost him!
	return 0;
}

qboolean MG_MakeConnection(edict_t* self, const buoy_t* first_buoy, const qboolean skip_jump)
{
	// Just for debug info.
	int result = MG_TryMakeConnection(self, first_buoy, skip_jump);

	if (!(self->ai_mood_flags & AIMF_CANT_FIND_ENEMY))
	{
		self->monsterinfo.last_successful_enemy_tracking_time = level.time;
		self->ai_mood_flags &= ~AIMF_SEARCHING;
	}

	//TODO: result 2 is not handled.
	if (result != 1)
	{
		// If can't find him (not including player_last_buoys) for 5 - 10 seconds, go into wander mode...
		if (result == 3 && !(self->ai_mood_flags & AIMF_SEARCHING))
		{
			self->monsterinfo.last_successful_enemy_tracking_time = level.time;
			self->monsterinfo.searchType = SEARCH_COMMON;
			self->ai_mood = AI_MOOD_PURSUE;
			self->ai_mood_flags |= AIMF_SEARCHING;
		}
		else if (self->enemy != NULL && self->ai_mood != AI_MOOD_FLEE && !(self->ai_mood_flags & AI_MOOD_FLAG_IGNORE_ENEMY) && self->monsterinfo.last_successful_enemy_tracking_time + MONSTER_SEARCH_TIME < level.time)
		{
			// Give up, can't see him or find path to him for ten seconds now...
			if (self->classID == CID_ASSASSIN && self->monsterinfo.last_successful_enemy_tracking_time + MONSTER_SEARCH_TIME + 20 > level.time)
			{
				// Assassins get an extra 20 seconds to look for the enemy and try to teleport to him.
			}
			else
			{
				if (self->enemy->client != NULL)
					self->oldenemy = self->enemy;

				self->enemy = NULL;
				self->ai_mood = ((result == 0 && self->ai_mood == AI_MOOD_WANDER) ? AI_MOOD_STAND : AI_MOOD_WANDER); //mxd
			}
		}
		else if (result == 0 && self->ai_mood != AI_MOOD_FLEE && self->enemy != NULL)
		{
			self->monsterinfo.searchType = SEARCH_COMMON;
			self->ai_mood = AI_MOOD_PURSUE;
		}

		if (result == 0 && self->ai_mood == AI_MOOD_WANDER)
		{
			self->monsterinfo.pausetime = 0;
			self->ai_mood = AI_MOOD_STAND;
		}

		if (result == 3)
			result = 0;
	}

	return result;
}

qboolean MG_CheckClearPathToEnemy(edict_t* self)
{
	if (self->enemy == NULL)
		return false;

	vec3_t mins;
	VectorCopy(self->mins, mins);
	mins[2] += 18.0f;

	trace_t	trace;
	gi.trace(self->s.origin, mins, self->maxs, self->enemy->s.origin, self, MASK_SOLID, &trace);

	if (trace.ent != NULL && trace.ent == self->enemy)
		return true; // Bumped into our enemy!

	if (trace.allsolid || trace.startsolid) // Trace is through a wall next to me?
		return false;

	if (trace.fraction < 1.0f)
	{
		// Couldn't get to enemy.
		vec3_t enemy_diff;
		VectorSubtract(self->enemy->s.origin, trace.endpos, enemy_diff);

		if (VectorLength(enemy_diff) > 48.0f || !visible(self, self->enemy))
			return false; // Couldn't even get close to a visible enemy.
	}

	if (self->groundentity == NULL || (self->flags & FL_INWATER && self->enemy->flags & FL_INWATER))
		return true;

	if (self->flags & FL_FLY || self->movetype == PHYSICSTYPE_FLY || self->gravity == 0.0f || self->classID == CID_GORGON)
		return true;

	if (((self->flags & FL_INWATER) || (self->flags & FL_SWIM) || (self->flags & FL_AMPHIBIAN)) && self->enemy->flags & FL_INWATER)
		return true;

	// Now lets see if there is a solid ground or steps path to the enemy.
	//FIXME: what about jumping monsters? Call a jump message?
	vec3_t center;
	VectorAverage(self->absmin, self->absmax, center);

	vec3_t enemy_diff;
	VectorSubtract(self->enemy->s.origin, center, enemy_diff);
	const float dist = VectorNormalize(enemy_diff);

	vec3_t origin;
	VectorCopy(self->s.origin, origin);
	origin[2] += self->mins[2];

	for (int i = 0; (float)i < dist; i += 8)
	{
		// Check to see if ground is there all the way to enemy.
		vec3_t check_spot;
		VectorMA(origin, (float)i, enemy_diff, check_spot);
		check_spot[2] -= 3.0f;

		if (!(gi.pointcontents(check_spot) & CONTENTS_SOLID))
		{
			check_spot[2] -= 16.0f; // Not solid underneath.

			if (!(gi.pointcontents(check_spot) & CONTENTS_SOLID))
				return false; // Not a step down.
		}
	}

	return true;
}

static qboolean MG_CheckClearPathToSpot(edict_t* self, const vec3_t spot)
{
	vec3_t mins;
	VectorCopy(self->mins, mins);
	mins[2] += 18.0f;

	trace_t	trace;
	gi.trace(self->s.origin, mins, self->maxs, spot, self, MASK_SOLID, &trace);

	if (trace.ent != NULL && trace.ent == self->enemy)
		return true; // Bumped into our enemy!

	if (trace.allsolid || trace.startsolid) // Trace is through a wall next to me?
		return false;

	if (trace.fraction < 1.0f)
	{
		// Couldn't get to enemy.
		vec3_t enemy_diff;
		VectorSubtract(spot, trace.endpos, enemy_diff);

		if (VectorLength(enemy_diff) > 48.0f || !visible_pos(self, spot))
			return false; // Couldn't even get close to a visible enemy.
	}

	if (self->groundentity == NULL || (self->flags & FL_INWATER && gi.pointcontents(spot) & MASK_WATER))
		return true;

	if (self->flags & FL_FLY || self->movetype == PHYSICSTYPE_FLY || self->gravity == 0.0f || self->classID == CID_GORGON)
		return true;

	// Now lets see if there is a solid ground or steps path to the enemy.
	//FIXME: what about jumping monsters? Call a jump message?
	vec3_t enemy_diff;
	VectorSubtract(spot, self->s.origin, enemy_diff);
	const float dist = VectorNormalize(enemy_diff);

	vec3_t origin;
	VectorCopy(self->s.origin, origin);
	origin[2] += self->mins[2];

	for (int i = 0; (float)i < dist; i += 8)
	{
		// Check to see if ground is there all the way to enemy.
		vec3_t check_spot;
		VectorMA(origin, (float)i, enemy_diff, check_spot);
		check_spot[2] -= 3.0f;

		if (!(gi.pointcontents(check_spot) & CONTENTS_SOLID))
		{
			check_spot[2] -= 16.0f; // Not solid underneath.

			if (!(gi.pointcontents(check_spot) & CONTENTS_SOLID))
				return false; // Not a step down.
		}
	}

	return true;
}

static qboolean MG_OkToShoot(const edict_t* self, const edict_t* target) //mxd. Named 'MG_OK_ToShoot' in original logic.
{
	return (target == self->enemy || (target->takedamage != DAMAGE_NO && (target->classID == CID_RAT || (!(target->svflags & SVF_MONSTER) && target->health < 50))));
}

static qboolean MG_CheckClearShotToEnemy(const edict_t* self)
{
	static const vec3_t zero_vec = { 0.0f, 0.0f, 0.0f }; //mxd. Made static.

	vec3_t start_pos;
	VectorCopy(self->s.origin, start_pos);
	start_pos[2] += (float)self->viewheight;

	vec3_t end_pos;
	VectorCopy(self->enemy->s.origin, end_pos);

	trace_t trace;
	gi.trace(start_pos, zero_vec, zero_vec, end_pos, self, MASK_MONSTERSOLID, &trace);

	return MG_OkToShoot(self, trace.ent);
}

static void MG_MonsterFirePathTarget(edict_t* self, const char* path_target)
{
	edict_t* ent = NULL;

	while ((ent = G_Find(ent, FOFS(pathtargetname), path_target)) != NULL)
		if (ent->use != NULL)
			ent->use(ent, self, self);
}

qboolean MG_MonsterAttemptTeleport(edict_t *self, vec3_t destination, qboolean ignoreLOS)
{
	qboolean no_teleport = false;
	trace_t trace;
	vec3_t	top, bottom, mins, maxs;
	edict_t	*ent = NULL;
	int		i;

	if(self->svflags & SVF_BOSS || self->classID == CID_OGLE)
		return false;

	if(self->classID != CID_ASSASSIN)
	{
		//if still SEE monsters cheat, re-enable the following 2 lines
		//if(skill->value < 2)
		//	return false;//only cheat on hard

		if(!ignoreLOS)
		{//check line of sight with all players
			ent = g_edicts;

			for(i = 0; i <= game.maxclients; i++)
			{
				ent = &g_edicts[i];

				if(ent->client)
				{
					edict_t	*temp;

					temp = G_Spawn();

					VectorSet(temp->s.origin,
						ent->client->playerinfo.pcmd.camera_vieworigin[0] * 0.125,
						ent->client->playerinfo.pcmd.camera_vieworigin[1] * 0.125,
						ent->client->playerinfo.pcmd.camera_vieworigin[2] * 0.125);

					if(gi.inPVS(temp->s.origin, destination))
					{
						no_teleport = true;
						G_FreeEdict(temp);
						break;
					}

					if(gi.inPVS(temp->s.origin, self->s.origin))
					{
						no_teleport = true;
						G_FreeEdict(temp);
						break;
					}

					G_FreeEdict(temp);
				}
			}
		}
	}

	if(!no_teleport)
	{//do traces
		VectorCopy(destination, bottom);
		bottom[2] -= self->size[2];
		
		VectorCopy(self->mins, mins);
		VectorCopy(self->maxs, maxs);
		mins[2] = 0;
		maxs[2] = 1;

		gi.trace(destination, mins, maxs, bottom, self, MASK_MONSTERSOLID, &trace);//self->clipmask

		if(trace.fraction<1.0)
		{
			VectorCopy(trace.endpos, bottom);
			VectorCopy(bottom, top);
			top[2] += self->size[2] - 1;
			gi.trace(bottom, mins, maxs, top, self, MASK_MONSTERSOLID, &trace);

			if(trace.allsolid || trace.startsolid)
				return false;

			if(trace.fraction == 1.0)
			{
				bottom[2] -= self->mins[2];

				if(self->classID == CID_ASSASSIN)
					assassinPrepareTeleportDest(self, bottom, false);
				else
				{
					VectorCopy(bottom, self->s.origin);
					gi.linkentity(self);
				}

				MG_RemoveBuoyEffects(self);
				self->lastbuoy = -1;
				return true;
			}
		}
		else if(!trace.allsolid && !trace.startsolid)
		{
			bottom[2] -= self->mins[2];
			if(self->classID == CID_ASSASSIN)
				assassinPrepareTeleportDest(self, bottom, false);
			else
			{
				VectorCopy(bottom, self->s.origin);
				gi.linkentity(self);
			}
			MG_RemoveBuoyEffects(self);
			self->lastbuoy = -1;
			return true;
		}
	}
	return false;
}

//FIXME:  If a monster CAN see player but can't get to him for a short while and does not have a clear path to him, use the buoys anyway!
void MG_Pathfind(edict_t *self, qboolean check_clear_path)
{
	buoy_t		*current_buoy = NULL;
	buoy_t		*last_buoy = NULL;
	buoy_t		*jump_buoy = NULL;
	qboolean	clear_path = false;

	if(self->spawnflags & MSF_FIXED)
		return;

	if(!(self->monsterinfo.aiflags & AI_USING_BUOYS))
	{
		self->ai_mood = AI_MOOD_PURSUE;
		return;
	}

	if(DEACTIVATE_BUOYS)
	{
		self->monsterinfo.searchType = SEARCH_COMMON;
		self->ai_mood = AI_MOOD_PURSUE;
		return;
	}

	if (self->monsterinfo.searchType == SEARCH_COMMON)
	{
		if(!self->enemy)
			return;

		if(check_clear_path)
			clear_path = MG_CheckClearPathToEnemy(self);
		else
			clear_path = false;

		if(!clear_path)
		{//this sucks- why should I do this every time pathfind is called- I need to know if the monster can get to the player directly.. if so, no Makeconnection attempt, less traces
			if(!MG_MakeConnection(self, NULL, false))//if(!MG_MakeConnection(self, true, false))
			{
			}
		}
	}
	else if (self->monsterinfo.searchType == SEARCH_BUOY)
	{
		current_buoy = &level.buoy_list[self->buoy_index];
		last_buoy = &level.buoy_list[self->lastbuoy];

		if(self->ai_mood != AI_MOOD_FLEE && self->ai_mood != AI_MOOD_WANDER)
			self->ai_mood = AI_MOOD_NAVIGATE;

		if (self->ai_mood == AI_MOOD_DELAY)
			self->ai_mood = AI_MOOD_NAVIGATE;

		if (self->ai_mood == AI_MOOD_JUMP && self->groundentity)
			self->ai_mood = AI_MOOD_NAVIGATE;

		if (MG_ReachedBuoy(self, NULL))
		{
			MG_RemoveBuoyEffects(self);
#ifdef _DEVEL
			if (BUOY_DEBUG)
			{
				gi.dprintf("Reached goal %s\n", current_buoy->targetname);
			}
#endif
			//Check the possibility of activating something
			if ((current_buoy->modflags & BUOY_ACTIVATE) && (self->ai_mood != AI_MOOD_DELAY))
			{
#ifdef _DEVEL
				if (BUOY_DEBUG)
					gi.dprintf("Activating target %s\n", current_buoy->pathtarget);
#endif
				if (self->wait < level.time)
				{
					self->wait = level.time + current_buoy->wait;
					MG_MonsterFirePathTarget(self, current_buoy->pathtarget);
					if (current_buoy->delay)
					{
						QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
						self->ai_mood = AI_MOOD_DELAY;
						self->mood_nextthink = level.time + current_buoy->delay;
						return;
					}
				}
			}


			//if in AI_MOOD_FORCED_BUOY mode and this buoy is my forced_buoy,
			//take off that ai_mood flag and clear forced_buoy
			//also, if AI_MOOD_IGNORE_ENEMY flag, remove it
			if(self->ai_mood_flags&AI_MOOD_FLAG_FORCED_BUOY && self->forced_buoy == current_buoy->id)
			{
				self->forced_buoy = -1;
				self->ai_mood_flags &= ~AI_MOOD_FLAG_FORCED_BUOY;

				if(self->ai_mood_flags&AI_MOOD_FLAG_GOTO_STAND)
				{
					self->ai_mood_flags &= ~AI_MOOD_FLAG_GOTO_STAND;
					self->enemy = NULL;
					self->ai_mood = AI_MOOD_STAND;
					QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, "");
					return;
				}

				if(self->ai_mood_flags&AI_MOOD_FLAG_GOTO_WANDER)
				{
					self->ai_mood_flags &= ~AI_MOOD_FLAG_GOTO_WANDER;
					self->enemy = NULL;
					self->ai_mood = AI_MOOD_WANDER;
					return;
				}

				if(self->ai_mood_flags&AI_MOOD_FLAG_GOTO_FIXED)
				{
					self->ai_mood_flags &= ~AI_MOOD_FLAG_GOTO_FIXED;
					self->spawnflags |= MSF_FIXED;
					if(self->enemy)
						self->ai_mood = AI_MOOD_PURSUE;
					else
						self->ai_mood = AI_MOOD_STAND;
				}
		
				if(self->ai_mood != AI_MOOD_FLEE)
					self->ai_mood_flags &= ~AI_MOOD_FLAG_IGNORE_ENEMY;
				else
				{//reached buoy was fleeing to now what?
					if(MG_GoToRandomBuoy(self))
					{
						self->monsterinfo.searchType = SEARCH_BUOY;
						return;
					}
					else
					{//couldn't flee using buoys, use dumb fleeing
						//FIXME: cowering if can't flee using buoys?
						self->ai_mood_flags |= AI_MOOD_FLAG_DUMB_FLEE;
						return;
					}
				}

				if(!M_ValidTarget(self, self->enemy))
				{//got to where I was going, no enemy, so chill, baby.
					if (self->monsterinfo.pausetime == -1)
					{
						self->spawnflags |= MSF_WANDER;
						self->ai_mood = AI_MOOD_WANDER;
					}
					else if (level.time > self->monsterinfo.pausetime)
						self->ai_mood = AI_MOOD_WALK;
					else
					{
						self->ai_mood = AI_MOOD_STAND;
						QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, "");
					}
					return;
				}
			}

			if ((current_buoy->modflags & BUOY_JUMP) && (self->ai_mood != AI_MOOD_JUMP))
			{
				if(MG_MakeConnection(self, current_buoy, true))//make a regular connection, allowing skipping of jump_buoys
				{
					jump_buoy = current_buoy;
					current_buoy = &level.buoy_list[self->buoy_index];
					if(jump_buoy == current_buoy)
					{//Shit, found same buoy, shouldn't happen with dont_use_last = true! unless switching enemies
#ifdef _DEVEL
						if (BUOY_DEBUG)
							gi.dprintf("Warning: %s found same next buoy as last buoy at jump buoy\n",self->classname);
#endif
					}
					else if(current_buoy->id == jump_buoy->jump_target_id)
					{//go ahead and jump
						if(self->groundentity)
						{
							vec3_t	jumpangles, jumpfwd, jump_spot;

#ifdef _DEVEL
							if (BUOY_DEBUG)
								gi.dprintf("Jumping after buoy %s at angle %4.2f with height %4.2f and speed %4.2f\n", 
										current_buoy->targetname, jump_buoy->jump_yaw, jump_buoy->jump_uspeed, jump_buoy->jump_fspeed);
#endif
							VectorSet(jumpangles, 0, jump_buoy->jump_yaw, 0);
							AngleVectors(jumpangles, jumpfwd, NULL, NULL);
							
							//since we may not be right on the buoy, find out where they want us to go by extrapolating and finding MY dir to there
							VectorMA(jump_buoy->origin, jump_buoy->jump_fspeed, jumpfwd, jump_spot);
							VectorSubtract(jump_spot, self->s.origin, jumpfwd);
							jumpfwd[2] = 0;
							VectorNormalize(jumpfwd);

							VectorScale(jumpfwd, jump_buoy->jump_fspeed, self->movedir);
							self->movedir[2] = jump_buoy->jump_uspeed;
							self->ai_mood = AI_MOOD_JUMP;//don't technically need this line
							self->mood_nextthink = level.time + 0.5;
							//as an alternative, call self->forced_jump(self);
							QPostMessage(self, MSG_CHECK_MOOD, PRI_DIRECTIVE, "i", AI_MOOD_JUMP);
							return;
						}
					}
					else
					{//follow the new path
#ifdef _DEVEL
						if (BUOY_DEBUG)
						{
							current_buoy = &level.buoy_list[self->buoy_index];
							gi.dprintf("Heading to new goal %s\n", current_buoy->targetname);
						}
#endif
						return;
//WAS: oops, not the right one, set it back and search down below again
//						current_buoy = jump_buoy;
//						MG_AssignMonsterNextBuoy(self, current_buoy, NULL);
					}
				}
				else
				{
					return;//?
				}
			}

			if (!MG_MakeConnection(self, current_buoy, false))
			{
				return;//?
			}
			else
			{
#ifdef _DEVEL
				if (BUOY_DEBUG)
				{
					gi.dprintf("Heading to new goal %s\n", current_buoy->targetname);
				}
#endif
			}
		}
		
		if(self->last_buoy_time > 0 && self->last_buoy_time + BUOY_SEARCH_TIME < level.time)
		{
#ifdef _DEVEL
			if (BUOY_DEBUG)
			{
				gi.dprintf("Buoy search timed out trying to get to %s\n", current_buoy->targetname);
			}
#endif
			
			if(self->classID == CID_ASSASSIN)
			{
				if(MG_MonsterAttemptTeleport(self, current_buoy->origin, true))
				{
					self->monsterinfo.aiflags |= AI_OVERRIDE_GUIDE;
#ifdef _DEVEL
					if(BUOY_DEBUG)
						gi.dprintf("%s teleported to buoy %s (ignoring player LOS)\n", 
								self->classname, current_buoy->targetname);
#endif
					return;
				}
			}
			else if(CHEATING_MONSTERS)
			{
				if(CHEATING_MONSTERS<2)
				{
#ifdef _DEVEL
					if(MG_MonsterAttemptTeleport(self, current_buoy->origin, false))
						if(BUOY_DEBUG)
							gi.dprintf("%s cheated and teleported to buoy %s\n", 
									self->classname, current_buoy->targetname);
#else
					MG_MonsterAttemptTeleport(self, current_buoy->origin, false);
#endif
				}
				else
				{
#ifdef _DEVEL
					if(MG_MonsterAttemptTeleport(self, current_buoy->origin, true))
						if(BUOY_DEBUG)
							gi.dprintf("%s cheated and teleported to buoy %s (ignoring player LOS)\n", 
									self->classname, current_buoy->targetname);
#else
					MG_MonsterAttemptTeleport(self, current_buoy->origin, true);
#endif
				}
			}
			if (!MG_MakeConnection(self, NULL, false))
			{
			}
		}
		else if(!irand(0, 4) && !clear_visible_pos(self, current_buoy->origin))
		{//DAMN!  Lost sight of buoy, let's re-aquire
#ifdef _DEVEL
			if (BUOY_DEBUG)
				gi.dprintf("%s Lost sight of buoy %s looking for another...\n", 
						self->classname, current_buoy->targetname);
#endif			
			if (!MG_MakeConnection(self, NULL, false))
			{
			}
		}
	}
}

#pragma endregion

#pragma region ========================== Guide functions ==========================

void MG_BuoyNavigate(edict_t *self)
{//Only handles buoy selection, some mood changing
//called from ai_run
	qboolean	valid_enemy = false;
	int			i;
	buoy_t		*found_buoy = NULL;
	qboolean	found;

	//See if my enemy is still valid
	valid_enemy = M_ValidTarget(self, self->enemy);

	if(self->spawnflags & MSF_FIXED)
		return;

	if(!(self->monsterinfo.aiflags & AI_USING_BUOYS))
	{
		self->ai_mood = AI_MOOD_PURSUE;//ai_mood_normal?
		return;
	}
	
//STEP 1: See if should be running away or wandering

	if(self->monsterinfo.flee_finished < level.time)
		self->monsterinfo.aiflags &= ~AI_FLEE;//clear the flee flag now

	if(self->monsterinfo.aiflags & AI_COWARD || 
		(self->monsterinfo.aiflags&AI_FLEE && self->monsterinfo.flee_finished >= level.time))
		self->ai_mood = AI_MOOD_FLEE;

	if(!valid_enemy)
	{//no enemy, now what?
		self->enemy = NULL;
		if(self->spawnflags & MSF_WANDER || self->monsterinfo.pausetime == -1)
		{
			self->spawnflags |= MSF_WANDER;
			self->ai_mood = AI_MOOD_WANDER;
		}
		else if (level.time > self->monsterinfo.pausetime)
			self->ai_mood = AI_MOOD_WALK;
		else
			self->ai_mood = AI_MOOD_STAND;
	}
	else
	{
		if(self->ai_mood == AI_MOOD_WANDER)
			self->ai_mood = AI_MOOD_PURSUE;
	}

	if(self->ai_mood == AI_MOOD_FLEE || self->ai_mood == AI_MOOD_WANDER)
	{//go off in a random buoy path

		if(!(self->ai_mood_flags&AI_MOOD_FLAG_FORCED_BUOY))
		{//first time, find closest buoy, alert other enemies
			if(self->ai_mood == AI_MOOD_FLEE)
			{//wake up enemies for next 10 seconds
				level.sight_entity = self;
				level.sight_entity_framenum = level.framenum + 100;
				level.sight_entity->light_level = 128;
			}

			if(MG_GoToRandomBuoy(self))
			{
				self->monsterinfo.searchType = SEARCH_BUOY;
				return;
			}
			else if(self->ai_mood == AI_MOOD_FLEE)
			{//couldn't flee using buoys, use dumb fleeing
				//FIXME: cowering if can't flee using buoys?
				self->ai_mood_flags |= AI_MOOD_FLAG_DUMB_FLEE;
				return;

			}
				//otherwise, want to wander, but can't, continue down the possibilities
		}
		else
		{
			self->monsterinfo.searchType = SEARCH_BUOY;
			MG_Pathfind(self, false);
			return;//already wandering normal buoy navigation
		}
	}
//STEP 2: Not running away or wandering, see what should be doing

	if(!valid_enemy)
	{//No enemy, Not wandering or can't, see if I have a homebuoy
		if(self->homebuoy)
		{//have a home base, let's get back there if no enemy
			for(i = 0; i <= level.active_buoys; i++)
			{
				found_buoy = &level.buoy_list[i];
				if(found_buoy->targetname && !stricmp(found_buoy->targetname, self->homebuoy))
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
#ifdef _DEVEL
				if(BUOY_DEBUG)
					gi.dprintf("ERROR: %s can't find it's homebuoy %s\n", self->classname, self->homebuoy);
#endif
				return;
			}

			if(!MG_ReachedBuoy(self, found_buoy->origin))
			{
#ifdef _DEVEL
				if(BUOY_DEBUG)
					gi.dprintf("%s heading for homebuoy %s\n", self->classname, self->homebuoy);
#endif
				self->ai_mood_flags|=AI_MOOD_FLAG_FORCED_BUOY;
				self->forced_buoy = found_buoy->id;

				if(MG_MakeConnection(self, NULL, false))
				{
					self->ai_mood = AI_MOOD_NAVIGATE;

					QPostMessage(self, MSG_WALK,PRI_DIRECTIVE, NULL);

					MG_RemoveBuoyEffects(self);
				}
				else
				{
					self->ai_mood_flags &= ~AI_MOOD_FLAG_FORCED_BUOY;
					self->forced_buoy = -1;
				}
			}
		}
//No enemy Not wandering, not going to homebuoy (or can't do these for some reason), so just stand around
		//do we really need to clear the enemy?  mAybe we shouldn't...
		if(self->enemy)
		{
			if(self->enemy->client)
				self->oldenemy = self->enemy;//remember last player enemy
		}
		self->enemy = NULL;

		self->mood_nextthink = level.time + 1;
		//fixme: check for a self->target also?
		if (self->monsterinfo.pausetime == -1)
		{
			self->spawnflags |= MSF_WANDER;
			self->ai_mood = AI_MOOD_WANDER;
		}
		else if (level.time > self->monsterinfo.pausetime)
			self->ai_mood = AI_MOOD_WALK;
		else
			self->ai_mood = AI_MOOD_STAND;
		return;
	}
	else if(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY)
	{//have an enemy, but being forced to use buoys, and ignore enemy until get to forced_buoy
		self->ai_mood = AI_MOOD_NAVIGATE;
		MG_Pathfind(self, false);
		return;
	}

//Actually have a valid enemy. let's try to get him
	self->ai_mood = AI_MOOD_PURSUE;

	MG_Pathfind(self, true);
	if(self->ai_mood == AI_MOOD_PURSUE)
		self->goalentity = self->enemy;
}

void Cvar_SetValue (char *var_name, float value);
void MG_GenericMoodSet(edict_t *self)
{
	vec3_t		v, forward, pursue_vel;
	float		enemydist;
	qboolean	coward = false;
	qboolean	can_attack_ranged = false;
	qboolean	can_attack_close = false;
	qboolean	clear_shot = false;
	qboolean	enemyvis = false;
	qboolean	enemyinfront = false;
	qboolean	found = false;
	qboolean	melee_go = false;
	int			i;
	buoy_t		*found_buoy;
	qboolean	valid_enemy = false;

	if(!level.active_buoys)
	{
		if(!DEACTIVATE_BUOYS)
		{
			gi.dprintf("WARNING: no buoys on this map!!!\n");
			if(!level.active_buoys)
			{//1st buoy, initialize a couple arrays
				for(i = 0; i < MAX_CLIENTS; i++)
				{
					level.player_buoy[i] = NULL_BUOY;				//stores current bestbuoy for a player enemy (if any)
					level.player_last_buoy[i] = NULL_BUOY;		//when player_buoy is invalid, saves it here so monsters can check it first instead of having to do a whole search
				}
			}
			Cvar_SetValue("deactivate_buoys", 1);
			DEACTIVATE_BUOYS = true;
		}
	}

	if(self->mood_nextthink > level.time || self->mood_nextthink <= 0.0f)
		return;

	//See if my enemy is still valid
	valid_enemy = M_ValidTarget(self, self->enemy);

	if(!(self->monsterinfo.aiflags & AI_USING_BUOYS))
	{//skip buoy stuff
		self->ai_mood = AI_MOOD_PURSUE;
	}
	else
	{//use buoys
//STEP 1: See if should be running away or wandering
		if(self->monsterinfo.flee_finished < level.time)
			self->monsterinfo.aiflags &= ~AI_FLEE;//clear the flee flag now

		if(self->monsterinfo.aiflags & AI_COWARD || 
			(self->monsterinfo.aiflags&AI_FLEE && self->monsterinfo.flee_finished >= level.time))
			self->ai_mood = AI_MOOD_FLEE;

		if(!valid_enemy)
		{//no enemy, now what?
			self->enemy = NULL;
			
			if(self->spawnflags & MSF_FIXED)
				return;

			if(self->spawnflags & MSF_WANDER || self->monsterinfo.pausetime == -1)
			{
				self->spawnflags |= MSF_WANDER;
				self->ai_mood = AI_MOOD_WANDER;
			}
			else if (level.time > self->monsterinfo.pausetime)
				self->ai_mood = AI_MOOD_WALK;
			else
				self->ai_mood = AI_MOOD_STAND;
		}
		else
		{
			if(self->spawnflags & MSF_FIXED)
				goto checkattacks;

			if(self->ai_mood == AI_MOOD_WANDER)
				self->ai_mood = AI_MOOD_PURSUE;
		}

		if(self->ai_mood == AI_MOOD_FLEE || self->ai_mood == AI_MOOD_WANDER)
		{//go off in a random buoy path
			if(!(self->ai_mood_flags&AI_MOOD_FLAG_FORCED_BUOY))
			{//first time, find closest buoy, alert other enemies
				if(self->ai_mood == AI_MOOD_FLEE)
				{//wake up enemies for next 10 seconds
					level.sight_entity = self;
					level.sight_entity_framenum = level.framenum + 100;
					level.sight_entity->light_level = 128;
				}

				if(MG_GoToRandomBuoy(self))
				{
					self->monsterinfo.searchType = SEARCH_BUOY;
					return;
				}
				else if(self->ai_mood == AI_MOOD_FLEE)
				{//couldn't flee using buoys, use dumb fleeing
					//FIXME: cowering if can't flee using buoys?
					self->ai_mood_flags |= AI_MOOD_FLAG_DUMB_FLEE;
					return;
				}
				//otherwise, want to wander, but can't, continue down the possibilities
			}
			else
			{
				self->monsterinfo.searchType = SEARCH_BUOY;
				MG_Pathfind(self, false);
				return;//already wandering normal buoy navigation
			}
		}
//STEP 2: Not running away or wandering, see what should be doing

		if(!valid_enemy)
		{//No enemy, Not wandering or can't, see if I have a homebuoy
			if(self->homebuoy)
			{//have a home base, let's get back there if no enemy
				for(i = 0; i <= level.active_buoys; i++)
				{
					found_buoy = &level.buoy_list[i];
					if(found_buoy->targetname && !stricmp(found_buoy->targetname, self->homebuoy))
					{
						found = true;
						break;
					}
				}

////////////////////////////  BUGBUGBUGBUG 7 lines above, targetname can be NULL if you just happen to die.

				if(!found)
				{
#ifdef _DEVEL
					if(BUOY_DEBUG)
						gi.dprintf("ERROR: %s can't find it's homebuoy %s\n", self->classname, self->homebuoy);
#endif
					return;
				}

				if(!MG_ReachedBuoy(self, found_buoy->origin))
				{
#ifdef _DEVEL
					if(BUOY_DEBUG)
						gi.dprintf("%s heading for homebuoy %s\n", self->classname, self->homebuoy);
#endif

					self->ai_mood_flags|=AI_MOOD_FLAG_FORCED_BUOY;
					self->forced_buoy = found_buoy->id;

					if(MG_MakeConnection(self, NULL, false))
					{
						self->ai_mood = AI_MOOD_NAVIGATE;

						QPostMessage(self, MSG_WALK,PRI_DIRECTIVE, NULL);

						MG_RemoveBuoyEffects(self);
					}
					else
					{
						self->ai_mood_flags &= ~AI_MOOD_FLAG_FORCED_BUOY;
						self->forced_buoy = -1;
					}
				}
			}
//No enemy Not wandering, not going to homebuoy (or can't do these for some reason), so just stand around
			//do we really need to clear the enemy?  mAybe we shouldn't...
			if(self->enemy)
			{
				if(self->enemy->client)
					self->oldenemy = self->enemy;//remember last player enemy
			}
			self->enemy = NULL;

			self->mood_nextthink = level.time + 1;
			//fixme: check for a self->target also?
			if (self->monsterinfo.pausetime == -1)
			{
				self->spawnflags |= MSF_WANDER;
				self->ai_mood = AI_MOOD_WANDER;
			}
			else if (level.time > self->monsterinfo.pausetime)
				self->ai_mood = AI_MOOD_WALK;
			else
				self->ai_mood = AI_MOOD_STAND;
			return;
		}
		else if(self->ai_mood_flags&AI_MOOD_FLAG_IGNORE_ENEMY)
		{//have an enemy, but being forced to use buoys, and ignore enemy until get to forced_buoy
			self->ai_mood = AI_MOOD_NAVIGATE;
			MG_Pathfind(self, false);
			return;
		}
	}

	if(!valid_enemy || !self->enemy)
	{
		if (self->monsterinfo.aiflags & AI_EATING)
			self->ai_mood = AI_MOOD_EAT;
		return;
	}
	
//STEP 3: OK, have a valid enemy, let's go get him!
checkattacks:
	self->ai_mood = AI_MOOD_PURSUE;

	//get distance to target, ignore Z diff if close
	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	if (v[2] <= 40)
		v[2] = 0;
	enemydist = VectorLength(v) - self->enemy->maxs[0];

	if ((self->monsterinfo.aiflags & AI_EATING) && (enemydist > self->wakeup_distance) && !self->monsterinfo.awake)
	{
		self->monsterinfo.last_successful_enemy_tracking_time = level.time;
		self->ai_mood = AI_MOOD_EAT;
		return;
	}

	if(self->monsterinfo.aiflags & AI_NO_MISSILE)
		self->spawnflags &= ~MSF_FIXED;//don't stand around if can't fire

	if(self->attack_debounce_time > level.time || self->monsterinfo.attack_finished > level.time)
	{//can't attack yet
		can_attack_ranged = false;
		clear_shot = false;
		can_attack_close = false;
	}
	else
	{
		if(classStatics[self->classID].msgReceivers[MSG_MISSILE] && !(self->monsterinfo.aiflags&AI_NO_MISSILE))
		{
			can_attack_ranged = true;
			clear_shot = MG_CheckClearShotToEnemy(self);
		}
		else
			clear_shot = false;
		
		if(classStatics[self->classID].msgReceivers[MSG_MELEE] && !(self->monsterinfo.aiflags&AI_NO_MELEE))
			can_attack_close = true;
	}

	if(enemyvis = visible(self, self->enemy))
	{
		self->ai_mood_flags &= ~AIMF_CANT_FIND_ENEMY;
		self->ai_mood_flags &= ~AIMF_SEARCHING;
		self->monsterinfo.last_successful_enemy_tracking_time = level.time;
		
		if(self->ai_mood_flags&AI_MOOD_FLAG_BACKSTAB)
		{//only approach and attack the enemy's back- be sure to take this off if hurt?
			if(enemydist < 128)
			{
				self->ai_mood_flags &= ~AI_MOOD_FLAG_BACKSTAB;
			}
			else if(infront(self->enemy, self))
			{
				self->ai_mood = AI_MOOD_DELAY;
				return;
			}
		}
	}
	enemyinfront = infront(self, self->enemy);

//HEY! What if too close- backpedal or flee for a bit?
//Also, need a chance of closing in anyway- a bypass_missile_chance?
	if(enemyvis && enemyinfront &&
		can_attack_ranged &&
		clear_shot &&
		enemydist <= self->missile_range)
	{//are they far enough away?
		if(irand(0, 100)>self->bypass_missile_chance)
		{
			if(enemydist >= self->min_missile_range)
			{//ranged attack!
				self->ai_mood = AI_MOOD_ATTACK;
				self->ai_mood_flags &= ~AI_MOOD_FLAG_MELEE;
				self->ai_mood_flags |= AI_MOOD_FLAG_MISSILE;
				self->attack_debounce_time = level.time + (3 - skill->value)/2;
				return;
			}
			else if(!can_attack_close)
			{//too close and can't melee!
				goto enemy_too_close;
			}
		}
	}

//otherwise, close in
	if (!MG_CheckClearPathToEnemy(self)&&self->classID!=CID_TBEAST)
	{//can't directly approach enemy
//		if(enemyinfront)//ie, failed to shoot for some other reason
//		{//can't shoot enemy, use buoys to get there
			MG_Pathfind(self, false);//false means don't do a mg_checkclearpath
			if(self->ai_mood == AI_MOOD_PURSUE)
				self->goalentity = self->enemy;
			if(self->cant_attack_think)
				self->cant_attack_think(self, enemydist, enemyvis, enemyinfront);
			return;
//		}
	}

//use dummy AI
	self->monsterinfo.searchType = SEARCH_COMMON;
	self->movetarget = self->goalentity = self->enemy;

//can directly approach player

	if(self->melee_range < 0)
	{//keep a distance
		if(enemydist <= -self->melee_range)
		{//hang back and wait until CAN fire a shot off
			goto enemy_too_close;
		}//else close in
	}

//check for melee range attack

	if(can_attack_close)
	{
		if(!enemyvis || !enemyinfront || enemydist > self->melee_range || enemydist < self->min_melee_range)
		{
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorSubtract(self->s.origin, self->s.old_origin, pursue_vel);
			melee_go  = M_PredictTargetEvasion(self, self->enemy, pursue_vel, self->enemy->velocity, self->melee_range, 5);//predict for next half second
		}
		else
			melee_go = true;
	}

	if(melee_go)
	{//Close enough to melee
		self->ai_mood = AI_MOOD_ATTACK;
		self->ai_mood_flags |= AI_MOOD_FLAG_MELEE;
		self->ai_mood_flags &= ~AI_MOOD_FLAG_MISSILE;
		self->attack_debounce_time = level.time + (3 - skill->value)/2;
		//OR: ok to missile too?
		return;
	}
	else if(enemydist < self->min_melee_range)
	{
		goto enemy_too_close;
		return;
	}

//Can't melee, so just run blindly
	self->ai_mood = AI_MOOD_PURSUE;
	if(self->cant_attack_think)
		self->cant_attack_think(self, enemydist, enemyvis, enemyinfront);
	return;

enemy_too_close:
	if(classStatics[self->classID].msgReceivers[MSG_FALLBACK])
	{//what if I hit a wall?  Go into attack anyway?
		self->ai_mood = AI_MOOD_BACKUP;//walk back while firing
	}
	else//maybe turn and run for a bit?
	{
#ifdef _DEVEL
		if(MGAI_DEBUG)
			gi.dprintf("%s running away to get some distance from %s\n", self->classname, self->enemy->classname);
#endif
		self->monsterinfo.aiflags |= AI_FLEE;
		self->monsterinfo.flee_finished = level.time + flrand(3, 6);
		//self->ai_mood = AI_MOOD_DELAY;//fixme: this is not good!
	}
	return;
}

void MG_InitMoods(edict_t *self)
{
	self->monsterinfo.searchType = SEARCH_COMMON;

	if(!self->mintel)
		self->mintel = MaxBuoysForClass[self->classID];

	self->mood_think = MG_GenericMoodSet;//we'll re-specialize these soon
	self->mood_nextthink = level.time + 0.1;
	if(self->mood_nextthink <= 0.0f)
		self->mood_nextthink = 0.1;

	//setup attack ranges for the mood functions to use
	//these can be set by the designer if desired and can be
	//affected later by the loss of a weapon or limb...
	if(!self->min_melee_range)
		self->min_melee_range = 0;//rendundant, I know, but clearer to see it here with other stuff

	if(!self->melee_range)
		self->melee_range = AttackRangesForClass[self->classID * 4 + 0];

	if(!self->missile_range)
		self->missile_range = AttackRangesForClass[self->classID * 4 + 1];
	
	if(!self->min_missile_range)
		self->min_missile_range = AttackRangesForClass[self->classID * 4 + 2];

	if(!self->bypass_missile_chance)
		self->bypass_missile_chance = AttackRangesForClass[self->classID * 4 + 3];

	if(!self->jump_chance)
		self->jump_chance = JumpChanceForClass[self->classID];

	if(!self->wakeup_distance)
		self->wakeup_distance = MAX_SIGHT_PLAYER_DIST;

	//so ai_run knows to call MG_BuoyNavigate...
	if(self->mintel > 0)
		self->monsterinfo.aiflags |= AI_USING_BUOYS;

	if(!skill->value)//no skill = 1/2 health monsters
		self->max_health = self->health = self->health * 0.5;

	self->lastbuoy = NULL_BUOY;
}

#pragma endregion
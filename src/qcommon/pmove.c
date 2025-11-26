//
// pmove.c
//
// Copyright 1998 Raven Software
//

#include "client.h" //mxd
#include "qcommon.h"
#include "game.h"
#include "q_Physics.h"
#include "turbsin.h"
#include "Vector.h"

// All of the locals will be zeroed before each pmove, just to make sure we don't have any differences when running on client or server.

typedef struct
{
	qboolean server; // H2
	vec3_t origin; // Full float precision.
	vec3_t velocity; // Full float precision.

	vec3_t forward;
	vec3_t right;
	vec3_t up;
	float frametime;

	csurface_t* groundsurface;
	cplane_t groundplane;
	int groundcontents;

	float gravity; // H2
	float max_velocity; // H2 //TODO: check if correct var name.
	float knockbackfactor; // H2
	float desired_water_height; // H2

	short previous_origin[3]; //mxd. vec3_t in H2.
	short snapped_origin[3]; // H2
} pml_t;

static pmove_t* pm;
static pml_t pml;

// Movement parameters.
#define PM_STOP_SPEED			100.0f
#define PM_MAX_SPEED			300.0f
#define PM_MAX_SWIM_SPEED		240.0f //mxd
#define PM_FRICTION				6.0f
#define PM_WATER_ACCELERATE		10.0f
#define PM_WATER_CURRENT_SPEED	400.0f
#define PM_CONVEYOR_SPEED		100.0f

#define MIN_STEP_NORMAL	0.7f // Can't step up onto very steep slopes.

typedef struct ssm_settings_s //mxd
{
	float time_step;
	vec3_t primal_velocity;
	vec3_t plane_normal;

	qboolean is_sliding; // When true, player is moving along an upwards-facing surface (including horizontal ones with [0 0 1] normal).
	float slide_vel;
	vec3_t slide_dir;
	float inv_slide_vel;

	int numplanes;
	int cur_plane;
	int prev_plane;
} ssm_settings_t;

#define MAX_CLIP_PLANES	5
static vec3_t clip_planes[MAX_CLIP_PLANES];

static float ClampVelocity(vec3_t vel, float max_speed, const qboolean run_shrine, const qboolean high_max)
{
	if (high_max || run_shrine)
		max_speed *= 2.0f;

	vec3_t dir;
	const float speed = VectorNormalize2(vel, dir);

	if (speed > max_speed)
	{
		VectorScale(dir, max_speed, vel);
		return max_speed;
	}

	return speed;
}

static qboolean CheckCollision(const float aimangle)
{
	vec3_t end;
	trace_t tr;

	end[0] = pml.origin[0] + cosf(aimangle) * 2.0f;
	end[1] = pml.origin[1] + sinf(aimangle) * 2.0f;
	end[2] = pml.origin[2];

	pm->trace(pml.origin, pm->mins, pm->maxs, end, &tr);

	if (tr.fraction < 1.0f && tr.architecture && tr.plane.normal[2] > -0.34f && tr.plane.normal[2] < 0.8f)
	{
		vec3_t aim_dir;
		VectorSubtract(end, pml.origin, aim_dir);
		aim_dir[2] = 0.0f;

		VectorNormalize(aim_dir);

		if (tr.plane.normal[0] * aim_dir[0] + tr.plane.normal[1] * aim_dir[1] < -0.5f)
		{
			pm->s.c_flags |= PC_COLLISION;
			return true;
		}
	}

	return false;
}

static qboolean PM_TryStepUp(const float step_height, const float frametime, trace_t* tr) // H2
{
	const vec3_t vel = VEC3_INITS(pml.velocity, frametime);

	// Can't step up if we aren't moving.
	if (vhlen(vel, vec3_origin) < 0.01f)
		return false;

	//mxd. Check horizontal move from initial position.
	vec3_t start = VEC3_INIT(pml.origin);
	vec3_t end = VEC3_INITA(pml.origin, vel[0], vel[1], 0.0f);

	trace_t trace;
	pm->trace(start, pm->mins, pm->maxs, end, &trace);

	//mxd. If we can move forwards, we don't need to step up (fixes successfully stepping up (and breaking ASEQ_SLIDE_FORWARD/ASEQ_SLIDE_BACKWARD animations as a result) while sliding down a slope...).
	if (trace.fraction == 1.0f)
		return false;

	// Check headroom.
	VectorCopy(pml.origin, end);
	end[2] += step_height + max(0.0f, vel[2]);

	pm->trace(start, pm->mins, pm->maxs, end, &trace);

	if (trace.startsolid || trace.allsolid || trace.endpos[2] < pml.origin[2] + 1.0f)
		return false;

	// Check horizontal move from step-up position.
	VectorCopy(trace.endpos, start);
	VectorCopy(trace.endpos, end);
	end[0] += vel[0];
	end[1] += vel[1];

	pm->trace(start, pm->mins, pm->maxs, end, &trace);

	if (trace.startsolid || trace.allsolid || trace.fraction == 0.0f)
		return false;

	// Check vertical move from previous position.
	const float prev_fraction = trace.fraction;

	VectorCopy(trace.endpos, start);
	VectorCopy(trace.endpos, end);
	end[2] = pml.origin[2] + min(0.0f, vel[2]);

	pm->trace(start, pm->mins, pm->maxs, end, &trace);

	// Abort if we can't move, won't move up or plane in front of us is too steep.
	if (trace.startsolid || trace.allsolid ||
		(prev_fraction < 1.0f && trace.endpos[2] <= pml.origin[2] + 0.01f) ||
		(trace.fraction < 1.0f && trace.plane.normal[2] <= MIN_STEP_NORMAL))
		return false;

	// Move player.
	trace.fraction = prev_fraction;
	memcpy(tr, &trace, sizeof(trace_t));
	VectorCopy(trace.endpos, pml.origin);

	return true;
}

static void PM_StepSlideFinishMove(const qboolean clear_velocity) //mxd. Split from PM_StepSlideMove().
{
	if (clear_velocity)
		VectorClear(pml.velocity);

	CheckCollision((float)pm->cmd.aimangles[YAW] * SHORT_TO_ANGLE * ANGLE_TO_RAD);
}

static qboolean PM_StepSlideSetupMove(ssm_settings_t* ssm, vec3_t velocity_step, const qboolean add_velocity_step) //mxd. Split from PM_StepSlideMove().
{
	ssm->slide_vel = pml.groundplane.normal[2] * pml.max_velocity;

	// Check if we should be sliding.
	if (pml.groundplane.normal[2] < MIN_STEP_NORMAL && pm->waterlevel == 0)
	{
		ssm->inv_slide_vel = -ssm->slide_vel * 0.1f;

		pm->s.c_flags |= PC_SLIDING;
		pml.velocity[2] = min(-40.0f, pml.velocity[2]);
	}
	else
	{
		ssm->inv_slide_vel = -ssm->slide_vel;
	}

	vec3_t move_dir;
	const float time_step_sq = ssm->time_step * ssm->time_step;
	float move_dist = ssm->inv_slide_vel * time_step_sq * 0.5f;

	if (add_velocity_step)
		move_dist += VectorNormalize2(velocity_step, move_dir);

	if (move_dist < 0.0f)
	{
		move_dist = 0.0f;
		ssm->inv_slide_vel = 0.0f;
	}

	if (DotProduct(ssm->slide_dir, ssm->plane_normal) < -FLOAT_ZERO_EPSILON)
	{
		VectorMA(pml.origin, 0.5f, clip_planes[ssm->cur_plane], pml.origin);
		ssm->prev_plane = ssm->cur_plane;
	}

	// Check if we should be sliding... again!
	if (pml.groundplane.normal[2] < MIN_STEP_NORMAL && pm->waterlevel == 0)
	{
		ssm->slide_vel = (1.0f - pml.groundplane.normal[2]) * pml.gravity - pml.groundplane.normal[2] * pml.max_velocity * 0.1f;
		pm->s.c_flags |= PC_SLIDING;
	}
	else
	{
		ssm->slide_vel = (1.0f - pml.groundplane.normal[2]) * pml.gravity - ssm->slide_vel;
	}

	ssm->slide_vel = max(0.0f, ssm->slide_vel);
	const float slide_dist = ssm->slide_vel * time_step_sq * 0.5f;

	// Set velocity_step.
	VectorScale(ssm->slide_dir, slide_dist, velocity_step);

	if (add_velocity_step)
		VectorScale(move_dir, move_dist, velocity_step);

	if (ssm->slide_vel + ssm->inv_slide_vel + move_dist == 0.0f)
	{
		PM_StepSlideFinishMove(true);
		return false;
	}

	return true;
}

static qboolean PM_StepSlideCheckMove(ssm_settings_t* ssm, vec3_t velocity_step, const qboolean first_bump) //mxd. Split from PM_StepSlideMove().
{
	// Get plane_normal perpendicular (parallel to plane, points in the same direction on xy axis as plane_normal; when plane_normal is [0 0 1], slide_direction is [0 0 0]) --mxd.
	ssm->slide_dir[0] = ssm->plane_normal[0] * ssm->plane_normal[2];
	ssm->slide_dir[1] = ssm->plane_normal[1] * ssm->plane_normal[2];
	ssm->slide_dir[2] = -(ssm->plane_normal[0] * ssm->plane_normal[0] + ssm->plane_normal[1] * ssm->plane_normal[1]);
	VectorNormalize(ssm->slide_dir);

	VectorScale(pml.velocity, ssm->time_step, velocity_step);

	// Slide along groundplane (including horizontal [0 0 1] planes too!) --mxd.
	// When groundplane.normal[2] == 0, either normal is empty, or normal[2] was explicitly set to 0 in PM_StepSlideMove() or PM_AddClipPlane() --mxd.
	if (pml.groundplane.normal[2] != 0.0f)
	{
		if (Vec3IsZero(pml.velocity))
		{
			if (!first_bump && pml.groundplane.normal[2] >= MIN_STEP_NORMAL && pml.groundplane.normal[2] >= pml.gravity / (pml.max_velocity + pml.gravity))
				return false;

			if (DotProduct(ssm->slide_dir, ssm->plane_normal) < -FLOAT_ZERO_EPSILON)
			{
				VectorMA(pml.origin, 0.5f, clip_planes[ssm->cur_plane], pml.origin);
				ssm->prev_plane = ssm->cur_plane;
			}

			ssm->is_sliding = true;
			return PM_StepSlideSetupMove(ssm, velocity_step, false);
		}

		vec3_t velocity_dir;
		VectorNormalize2(velocity_step, velocity_dir);

		if (fabsf(DotProduct(velocity_dir, ssm->plane_normal)) <= 0.05f)
		{
			ssm->is_sliding = true;
			return PM_StepSlideSetupMove(ssm, velocity_step, true);
		}
	}

	if (pm->groundentity == NULL)
	{
		const float time_step_sq = ssm->time_step * ssm->time_step;
		velocity_step[2] -= time_step_sq * pml.gravity * 0.5f;
	}

	return true;
}

static qboolean PM_AddClipPlane(ssm_settings_t* ssm, const trace_t* trace) //mxd. Split from PM_StepSlideMove().
{
	if (pm->groundentity != NULL && pml.groundplane.normal[2] > MIN_STEP_NORMAL && trace->plane.normal[2] <= MIN_STEP_NORMAL)
	{
		vec3_t v = VEC3_SET(trace->plane.normal[0], trace->plane.normal[1], 0.0f);
		VectorNormalize(v);

		if (ssm->numplanes == 0 || !VectorCompare(v, clip_planes[ssm->numplanes - 1]))
		{
			VectorCopy(v, clip_planes[ssm->numplanes]);
			ssm->numplanes++;
		}
	}
	else
	{
		VectorCopy(trace->plane.normal, pml.groundplane.normal);
		VectorCopy(trace->plane.normal, ssm->plane_normal);

		const qboolean have_trace_ent = (pml.server && (trace->ent->solid == SOLID_BSP || trace->ent->solid == SOLID_BBOX)) || (!pml.server && trace->ent != NULL); //mxd
		if (have_trace_ent && trace->plane.normal[2] > 0.0f)
		{
			pm->groundentity = trace->ent;
		}
		else
		{
			pm->groundentity = NULL;

			if (trace->plane.normal[2] < 0.0f)
				pml.groundplane.normal[2] = 0.0f;
		}

		if (ssm->numplanes == 0 || !VectorCompare(trace->plane.normal, clip_planes[ssm->numplanes - 1]))
		{
			VectorCopy(trace->plane.normal, clip_planes[ssm->numplanes]);
			ssm->numplanes++;
		}
		else
		{
			ssm->prev_plane = ssm->numplanes - 1;
			VectorMA(pml.origin, 0.5f, clip_planes[ssm->prev_plane], pml.origin);
		}
	}

	return (ssm->numplanes <= MAX_CLIP_PLANES - 1);
}

static qboolean PM_StepSlideTryMove(ssm_settings_t* ssm, trace_t* trace, const qboolean last_bump) //mxd. Split from PM_StepSlideMove().
{
	// Suggested move will get us stuck...
	if (trace->startsolid)
	{
		if (ssm->prev_plane == -1)
		{
			vec3_t mins = VEC3_INIT(pm->mins);
			vec3_t maxs = VEC3_INIT(pm->maxs);

			float offset = 1.0f;
			while (maxs[0] - offset >= mins[0] + offset)
			{
				VectorInc(mins);
				VectorDec(maxs);

				pm->trace(pml.origin, mins, maxs, pml.origin, trace);

				if (!trace->startsolid)
				{
					VectorCopy(pm->mins, pm->intentMins);
					VectorCopy(pm->maxs, pm->intentMaxs);
					VectorCopy(mins, pm->mins);
					VectorCopy(maxs, pm->maxs);

					// Found possible settings, restart.
					return PM_StepSlideTryMove(ssm, trace, last_bump);
				}

				offset += 1.0f;
			}

			VectorClear(pml.velocity);

			return true;
		}

		for (int i = 0; i < 3; i++)
			pml.origin[i] -= clip_planes[ssm->prev_plane][i] * 0.5f;

		if (last_bump)
			PM_StepSlideFinishMove(trace->fraction < 1.0f);

		return false;
	}

	// Check current move.
	if (trace->fraction > 0.0f)
	{
		// Actually covered some distance.
		VectorCopy(trace->endpos, pml.origin);

		const float time_step_frac = ssm->time_step * trace->fraction;

		if (ssm->is_sliding)
		{
			vec3_t vel_dir;
			float vel_dist = VectorNormalize2(pml.velocity, vel_dir);
			vel_dist += time_step_frac * ssm->inv_slide_vel;

			const float scaler = time_step_frac * ssm->slide_vel;

			for (int i = 0; i < 3; i++)
				pml.velocity[i] = ssm->slide_dir[i] * scaler + vel_dir[i] * vel_dist;
		}
		else
		{
			pml.velocity[2] -= time_step_frac * pml.gravity;
		}

		// Moved the entire distance.
		if (trace->fraction == 1.0f)
		{
			PM_StepSlideFinishMove(false);
			return true;
		}

		// Reset collision settings...
		ssm->numplanes = 0;
		ssm->prev_plane = -1;
		ssm->time_step -= time_step_frac;

		VectorCopy(pml.velocity, ssm->primal_velocity);
	}
	else if (Vec3IsZero(pml.velocity) && pml.groundplane.normal[2] >= pml.gravity / (pml.max_velocity + pml.gravity))
	{
		PM_StepSlideFinishMove(true);
		return true;
	}

	// Handle entity blocking.
	if (pml.server && trace->ent != NULL && trace->ent->isBlocking != NULL)
	{
		trace_t tr = *trace;

		tr.ent = pm->self;
		VectorInverse(tr.plane.normal);
		trace->ent->isBlocking(trace->ent, &tr);
	}

	// Save entity for contact.
	if (pm->numtouch < MAXTOUCH && trace->ent != NULL)
	{
		pm->touchents[pm->numtouch] = trace->ent;
		pm->numtouch++;
	}

	// Try stepping over obstacle.
	if (trace->plane.normal[2] <= MIN_STEP_NORMAL && trace->ent != NULL)
	{
		const qboolean check_step_up = (!pml.server || trace->ent->solid == SOLID_BSP || trace->ent->solid == SOLID_BBOX); //mxd
		if (check_step_up && PM_TryStepUp(STEP_SIZE, ssm->time_step, trace))
		{
			PM_StepSlideFinishMove(trace->fraction < 1.0f);
			return true;
		}
	}

	// Store trace.plane as current clip plane.
	if (!PM_AddClipPlane(ssm, trace))
	{
		PM_StepSlideFinishMove(trace->fraction < 1.0f);
		return true;
	}

	// Modify pml.velocity so it's parallel to all clip planes.
	vec3_t cur_slide_vel;

	for (int i = 0; i < ssm->numplanes; i++)
	{
		ssm->cur_plane = i;
		BounceVelocity(ssm->primal_velocity, clip_planes[i], cur_slide_vel, ELASTICITY_SLIDE);

		vec3_t cur_slide_dir;
		const float slide_dist = VectorNormalize2(cur_slide_vel, cur_slide_dir);

		// Can't slide along this plane.
		if (fabsf(slide_dist) < FLOAT_ZERO_EPSILON)
		{
			// If current plane normal points even a little bit upwards, slide along its perpendicular.
			if (clip_planes[i][2] > 0.0f)
			{
				// Get current plane perpendicular (parallel to plane, points in the same direction on xy axis as current plane; when current plane normal is [0 0 1], slide_direction is [0 0 0]) --mxd.
				cur_slide_dir[0] = -(clip_planes[i][2] * clip_planes[i][0]);
				cur_slide_dir[1] = -(clip_planes[i][1] * clip_planes[i][2]);
				cur_slide_dir[2] = clip_planes[i][0] * clip_planes[i][0] + clip_planes[i][1] * clip_planes[i][1];

				VectorNormalize(cur_slide_dir);
			}
			else
			{
				VectorCopy(vec3_down, cur_slide_dir);
			}
		}

		// If current slide direction is opposite to current plane normal, move us away from this plane.
		if (DotProduct(cur_slide_dir, clip_planes[i]) < -FLOAT_ZERO_EPSILON)
		{
			VectorMA(pml.origin, 0.5f, clip_planes[i], pml.origin);
			ssm->prev_plane = i;
		}

		// Check if slide velocity is opposite to any of clip planes. 
		int c;
		for (c = 0; c < ssm->numplanes; c++)
			if (c != i && DotProduct(cur_slide_vel, clip_planes[c]) <= 0.0f)
				break; // Not ok.

		// Stop checking clip planes when slide velocity is adjacent to all of them (e.g. current move won't move us into any of them). 
		if (c == ssm->numplanes)
			break;
	}

	// Go along the crease.
	if (ssm->numplanes != 2)
	{
		if (ssm->cur_plane == ssm->numplanes)
		{
			VectorClear(pml.velocity);
			return true;
		}

		VectorCopy(cur_slide_vel, pml.velocity);
	}
	else
	{
		vec3_t move_vel;
		CrossProduct(clip_planes[0], clip_planes[1], move_vel);

		if (DotProduct(move_vel, pml.velocity) < 0.0f)
			VectorInverse(move_vel);

		move_vel[2] += 0.1f;
		VectorNormalize(move_vel);

		VectorAdd(clip_planes[0], clip_planes[1], ssm->plane_normal);
		VectorNormalize(ssm->plane_normal);

		if (pm->groundentity != NULL && move_vel[2] > MIN_STEP_NORMAL)
		{
			PM_StepSlideFinishMove(true);
			return true;
		}

		const float d = DotProduct(move_vel, pml.velocity);

		if (d > 0.0f)
			VectorScale(move_vel, d, pml.velocity);
		else
			VectorClear(pml.velocity);
	}

	if (last_bump)
		PM_StepSlideFinishMove(trace->fraction < 1.0f);

	return false;
}

// Each intersection will try to step over the obstruction instead of sliding along it.
static void PM_StepSlideMove(void)
{
#define NUM_BUMPS	4

	trace_t trace;
	ssm_settings_t ssm; //mxd

	// Init settings.
	ssm.time_step = pml.frametime;
	ssm.slide_vel = 0.0f;
	ssm.inv_slide_vel = 0.0f;

	ssm.numplanes = 0;
	ssm.cur_plane = 0;
	ssm.prev_plane = -1;

	if (pm->groundentity == NULL)
		pml.groundplane.normal[2] = 0.0f;

	VectorCopy(pml.velocity, ssm.primal_velocity);
	VectorCopy(pml.groundplane.normal, ssm.plane_normal);

	for (int bumpcount = 0; bumpcount < NUM_BUMPS; bumpcount++)
	{
		// Plot current move.
		vec3_t velocity_step;
		ssm.is_sliding = false;

		if (!PM_StepSlideCheckMove(&ssm, velocity_step, bumpcount == 0))
			return;

		// Trace current move.
		vec3_t end;
		VectorAdd(pml.origin, velocity_step, end);
		pm->trace(pml.origin, pm->mins, pm->maxs, end, &trace);

		// Check & perform current move.
		if (PM_StepSlideTryMove(&ssm, &trace, bumpcount == NUM_BUMPS - 1))
			return;
	}
}

static void PM_AddCurrents(vec3_t wishvel)
{
	// H2: don't account for ladders.

	// Add water currents.
	if (pm->watertype & MASK_CURRENT)
	{
		vec3_t v = { 0 };

		if (pm->watertype & CONTENTS_CURRENT_0)
			v[0] += 1.0f;

		if (pm->watertype & CONTENTS_CURRENT_90)
			v[1] += 1.0f;

		if (pm->watertype & CONTENTS_CURRENT_180)
			v[0] -= 1.0f;

		if (pm->watertype & CONTENTS_CURRENT_270)
			v[1] -= 1.0f;

		if (pm->watertype & CONTENTS_CURRENT_UP)
			v[2] += 1.0f;

		if (pm->watertype & CONTENTS_CURRENT_DOWN)
			v[2] -= 1.0f;

		float s = PM_WATER_CURRENT_SPEED;
		if (pm->waterlevel == 1 && pm->groundentity != NULL) // Half speed when wading.
			s /= 2;

		VectorMA(wishvel, s, v, wishvel);
	}

	// Add conveyor belt velocities.
	if (pm->groundentity != NULL && (pml.groundcontents & MASK_CURRENT))
	{
		vec3_t v = { 0 };

		if (pml.groundcontents & CONTENTS_CURRENT_0)
			v[0] += 1.0f;

		if (pml.groundcontents & CONTENTS_CURRENT_90)
			v[1] += 1.0f;

		if (pml.groundcontents & CONTENTS_CURRENT_180)
			v[0] -= 1.0f;

		if (pml.groundcontents & CONTENTS_CURRENT_270)
			v[1] -= 1.0f;

		if (pml.groundcontents & (CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN)) // H2
			Com_DPrintf("CONTENTS_CURRENT_UP or CONTENTS_CURRENT_DOWN not supported on groundcontents (conveyor belts)\n"); //mxd. Com_Printf -> Com_DPrintf.

		VectorMA(wishvel, PM_CONVEYOR_SPEED, v, wishvel);
	}
}

static void PM_OnBboxSizeChanged(void) // H2 //mxd. Removed unused return value.
{
	int axis;
	float sign;
	float* v1;
	float* v2;
	vec3_t start;
	vec3_t end;
	vec3_t mins;
	vec3_t maxs;
	vec3_t intent_mins;
	vec3_t intent_maxs;
	trace_t trace;

	VectorCopy(pm->intentMins, intent_mins);
	VectorCopy(pm->intentMaxs, intent_maxs);
	VectorCopy(pm->mins, mins);
	VectorCopy(pm->maxs, maxs);
	VectorCopy(pml.origin, start);

	for (int i = 0; i < 6; i++)
	{
		if (i & 1)
		{
			if (i & 4)
				axis = 2;
			else if (i & 2)
				axis = 0;
			else
				axis = 1;

			v1 = maxs;
			v2 = intent_maxs;
			sign = 1.0f;
		}
		else
		{
			if (i & 4)
				axis = 2;
			else if (i & 2)
				axis = 1;
			else
				axis = 0;

			v1 = mins;
			v2 = intent_mins;
			sign = -1.0f;
		}

		float diff = v2[axis] - v1[axis];

		if (diff * sign > 0.0f)
		{
			VectorCopy(start, end);

			if (i == 4 && pm->groundentity != NULL)
			{
				trace.fraction = 0.0f;
			}
			else
			{
				end[axis] += diff;

				pm->trace(start, mins, maxs, end, &trace);

				if (trace.startsolid || trace.allsolid)
					return;
			}

			if (trace.fraction < 1.0f)
			{
				diff *= (1.0f - trace.fraction);
				end[axis] = start[axis] - diff;

				pm->trace(start, mins, maxs, end, &trace);

				if (trace.fraction != 1.0f)
					return;

				start[axis] -= diff;
			}
		}

		v1[axis] = v2[axis];
	}

	pm->trace(start, intent_mins, intent_maxs, start, &trace);

	if (!trace.startsolid)
	{
		VectorCopy(start, pml.origin);
		VectorCopy(intent_mins, pm->mins);
		VectorCopy(intent_maxs, pm->maxs);
	}
}

//mxd. Subtle acceleration/deceleration while walking on land.
static void PM_AirAccelerate(float* fmove, float* smove) //mxd
{
	static float fmove_lerp;
	static float smove_lerp;

	if (*fmove != 0.0f)
	{
		fmove_lerp = min(1.0f, fmove_lerp + pml.frametime * 2.0f);
		*fmove *= 1.0f + sinf(-ANGLE_90 + ANGLE_90 * fmove_lerp);
	}
	else
	{
		fmove_lerp = 0.0f;
	}

	if (*smove != 0.0f)
	{
		smove_lerp = min(1.0f, smove_lerp + pml.frametime * 2.5f);
		*smove *= 1.0f + sinf(-ANGLE_90 + ANGLE_90 * smove_lerp);
	}
	else
	{
		smove_lerp = 0.0f;
	}
}

static void PM_AirMove(void)
{
	float fmove = pm->cmd.forwardmove;
	float smove = pm->cmd.sidemove;
	qboolean run_shrine = false; // H2

	pml.gravity = pm->s.gravity; // H2

	if (!pm->high_max && pm->run_shrine && fmove > 0.0f) // H2
	{
		fmove *= 1.65f;
		run_shrine = true;
	}

	VectorNormalize(pml.forward);
	VectorNormalize(pml.right);

	PM_AirAccelerate(&fmove, &smove); //mxd

	vec3_t wishvel;
	for (int i = 0; i < 2; i++)
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	wishvel[2] = 0.0f;

	PM_AddCurrents(wishvel);

	if (pml.knockbackfactor > 0.0f) // H2
		pml.max_velocity *= (1.0f - pml.knockbackfactor);

	VectorMA(wishvel, pml.knockbackfactor, pml.velocity, wishvel);
	const float speed = ClampVelocity(wishvel, PM_MAX_SPEED, run_shrine, pm->high_max);

	if (pm->groundentity != NULL)
	{
		// Standing on ground.
		if (speed == 0.0f && pml.groundplane.normal[2] >= MIN_STEP_NORMAL && pml.groundplane.normal[2] >= pml.gravity / (pml.max_velocity + pml.gravity))
		{
			PM_StepSlideFinishMove(true); //mxd
			return;
		}

		VectorCopy(wishvel, pml.velocity);
	}

	PM_StepSlideMove();
}

// Handles water/slime/lava friction.
static void PM_Friction(void)
{
	const float speed = VectorLength(pml.velocity);

	if (speed < 1.0f)
	{
		pml.velocity[0] = 0.0f;
		pml.velocity[1] = 0.0f;

		return;
	}

	float drop = 0.0f;

	// Apply ground friction?
	if (pm->groundentity != NULL && pml.groundsurface != NULL && !(pml.groundsurface->flags & SURF_SLICK))
		drop = max(PM_STOP_SPEED, speed) * pml.frametime * PM_FRICTION;

	// Apply water friction?
	if (pm->waterlevel > 0)
	{
		const float waterfriction = (pm->watertype & (CONTENTS_SLIME | CONTENTS_LAVA) ? 0.5f : 0.75f);
		drop += speed * waterfriction * (float)pm->waterlevel * pml.frametime;
	}

	// Scale the velocity.
	const float newspeed = max(0.0f, speed - drop) / speed;
	Vec3ScaleAssign(newspeed, pml.velocity);
}

//mxd. Simplified version of PM_WaterMove().
static void PM_LavaAndSlimeMove(const float scaler)
{
	pml.gravity = 0.0f;

	PM_Friction();

	float fmove = pm->cmd.forwardmove;
	const float smove = pm->cmd.sidemove;
	qboolean run_shrine = false;

	if (pm->run_shrine && fmove > 0.0f)
	{
		fmove *= 1.65f;
		run_shrine = true;
	}

	// User intentions.
	vec3_t wishvel;
	for (int i = 0; i < 3; i++)
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;

	PM_AddCurrents(wishvel);
	Vec3ScaleAssign(scaler, wishvel);

	ClampVelocity(wishvel, PM_MAX_SPEED, run_shrine, false);
	VectorCopy(wishvel, pml.velocity);
}

static void PM_SlimeMove(void) // H2
{
	PM_LavaAndSlimeMove(0.35f);
	pml.velocity[2] = -12.0f;
	PM_StepSlideMove();
}

static void PM_LavaMove(void) // H2
{
	PM_LavaAndSlimeMove(0.45f);
	pml.velocity[2] = Clamp(pml.velocity[2], -64.0f, -32.0f);
	PM_StepSlideMove();
}

// Q2 counterpart (PM_Accelerate()).
// Handles user intended acceleration.
static void PM_WaterAccelerate(vec3_t wishdir, const float wishspeed, const float accel) //mxd. Used only by PM_TryWaterMove() in H2.
{
	const float currentspeed = DotProduct(pml.velocity, wishdir);
	const float addspeed = wishspeed - currentspeed;

	if (addspeed <= 0.0f)
		return;

	float accelspeed = accel * pml.frametime * wishspeed;
	accelspeed = min(addspeed, accelspeed);

	VectorMA(pml.velocity, accelspeed, wishdir, pml.velocity);
}

static qboolean PM_TryWaterMove(const float scaler) //TODO: scaler is always 0.5. Remove arg?
{
	pml.gravity = 0.0f;

	PM_Friction();

	float fmove = pm->cmd.forwardmove;
	const float smove = pm->cmd.sidemove;
	qboolean run_shrine = false;

	if (pm->run_shrine && fmove > 0.0f)
	{
		fmove *= 1.65f;
		run_shrine = true;
	}

	// User intentions.
	vec3_t wishvel;
	for (int i = 0; i < 3; i++)
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;

	PM_AddCurrents(wishvel);

	const float max_swim_speed = PM_MAX_SWIM_SPEED * scaler; //mxd. Use swim-specific max. speed.
	const float speed = ClampVelocity(wishvel, max_swim_speed, run_shrine, false);

	vec3_t wishdir;
	VectorNormalize2(wishvel, wishdir);
	PM_WaterAccelerate(wishdir, speed, PM_WATER_ACCELERATE);

	//mxd. Clamp to max. velocity again (with some room for inertia), because PM_WaterAccelerate() can greatly exceed it...
	if (VectorLength(pml.velocity) > max_swim_speed * 1.25f)
		VectorScale(wishdir, max_swim_speed * 1.25f, pml.velocity);

	if (pm->groundentity != NULL)
	{
		// Standing on ground.
		if (speed == 0.0f && pml.groundplane.normal[2] >= MIN_STEP_NORMAL && pml.groundplane.normal[2] >= pml.gravity / (pml.max_velocity + pml.gravity))
		{
			VectorClear(pml.velocity);
			return true;
		}
	}

	return false;
}

static void PM_UnderwaterMove(void) // H2
{
	if (PM_TryWaterMove(0.5f))
		return;

	PM_StepSlideMove();
}

// Swim on water surface or walk while in water.
static void PM_WaterMove(void) // H2
{
	if (PM_TryWaterMove(0.5f))
		return;

	if (pm->s.w_flags & WF_SINK)
	{
		pm->s.w_flags &= ~WF_SINK;
	}
	else // Bob on water surface.
	{
		pml.velocity[2] = (pm->waterheight - pml.desired_water_height) / pml.frametime;

		//mxd. Replicate R_EmitWaterPolys() logic, so player's vertical offset is synched with water polys movement...
		//TODO: using cl.refdef.time here probably breaks client/server logic separation...
		pml.velocity[2] += (turbsin[TURBSIN_V0(pml.origin[0], pml.origin[1], cl.refdef.time)] * 0.25f +
							turbsin[TURBSIN_V1(pml.origin[0], pml.origin[1], cl.refdef.time)] * 0.125f) * 12.0f;
	}

	PM_StepSlideMove();
}

static void PM_CatagorizePosition(void)
{
	if (pml.velocity[2] > 100.0f || pm->waterlevel > 1) // H2
	{
		pm->s.pm_flags &= ~PMF_ON_GROUND;
		pm->groundentity = NULL;

		return;
	}

	// See if standing on something solid.
	vec3_t point = VEC3_SET(pml.origin[0], pml.origin[1], pml.origin[2] - 1.0f);
	vec3_t mins = VEC3_INIT(pm->mins);
	vec3_t maxs = VEC3_INIT(pm->maxs);

	const qboolean big_maxs = (maxs[0] >= 2.0f);

	if (big_maxs)
	{
		mins[0] += 1.0f;
		mins[1] += 1.0f;
		maxs[0] -= 1.0f;
		maxs[1] -= 1.0f;
		point[2] -= 1.0f;
	}

	// If the player hull point one unit down is solid, the player is on ground.
	trace_t trace;
	pm->trace(pml.origin, mins, maxs, point, &trace);

	pml.groundplane = trace.plane;
	pml.groundcontents = trace.contents;
	pml.groundsurface = trace.surface;

	if (trace.ent == NULL || (trace.plane.normal[2] < MIN_STEP_NORMAL && !trace.startsolid))
	{
		if (pml.server)
		{
			if (big_maxs)
			{
				point[2] += 1.0f;

				pm->trace(pml.origin, pm->mins, pm->maxs, point, &trace);

				if ((trace.ent != NULL && trace.plane.normal[2] >= MIN_STEP_NORMAL) || trace.startsolid) // TODO: this check is grouped differently. A bug?
				{
					VectorCopy(mins, pm->mins);
					VectorCopy(maxs, pm->maxs);
				}
			}

			trace.ent = NULL; //mxd. Explicitly unset ent to clear pm->groundentity and PMF_ON_GROUND flag.
		}
		else
		{
			const vec3_t start = VEC3_SET(pml.origin[0], pml.origin[1], pml.origin[2] + 0.14f);
			point[2] += 0.14f;

			pm->trace(start, mins, maxs, point, &trace);

			pml.groundplane = trace.plane;
			pml.groundsurface = trace.surface;
			pml.groundcontents = trace.contents;
		}
	}

	if (trace.ent != NULL && (trace.plane.normal[2] >= MIN_STEP_NORMAL || trace.startsolid))
	{
		pm->groundentity = trace.ent;

		if (!(pm->s.pm_flags & PMF_ON_GROUND))
		{
			// Just hit the ground.
			pm->s.pm_flags |= PMF_ON_GROUND;

			// Don't do landing time if we were just going down a slope.
			if (pml.velocity[2] < -200.0f)
			{
				pm->s.pm_flags |= PMF_TIME_LAND;

				// Don't allow another jump for a little while.
				if (pml.velocity[2] >= -400.0f) //TODO: this check is inverted in Q2. Bug?
					pm->s.pm_time = 25;
				else
					pm->s.pm_time = 18;
			}
		}
	}
	else
	{
		pm->groundentity = NULL;
		pm->s.pm_flags &= ~PMF_ON_GROUND;
	}

	// Save entity for contact.
	if (pm->numtouch < MAXTOUCH && trace.ent != NULL)
	{
		pm->touchents[pm->numtouch] = trace.ent;
		pm->numtouch++;
	}
}

static void PM_CheckJump(void)
{
	if (!(pm->s.pm_flags & PMF_TIME_LAND) && pm->cmd.upmove > 9)
	{
		pm->groundentity = NULL;
		pml.velocity[2] = 280.0f;
	}
}

static qboolean PM_GoodPosition(void)
{
	static trace_t trace;
	vec3_t origin;

	for (int i = 0; i < 3; i++)
		origin[i] = (float)pml.snapped_origin[i] * 0.125f;

	pm->trace(origin, pm->mins, pm->maxs, origin, &trace);

	if (trace.startsolid)
		return false;

	if (trace.allsolid)
		Com_Printf("Bad assumption in PM_GoodPosition\n"); //mxd. 'assumptoin' in original version.

	return true;
}

// On exit, the origin will have a value that is pre-quantized to the 0.125 precision of the network channel and in a valid position.
static void PM_SnapPosition(void)
{
	// Try all single bits first.
	static const uint jitterbits[] = { 0, 4, 1, 2, 3, 5, 6, 7 };
	short offset[3] = { 0 };

	// Snap velocity to eights.
	for (int i = 0; i < 3; i++)
		pm->s.velocity[i] = (short)(pml.velocity[i] * 8.0f);

	for (int i = 0; i < 3; i++)
	{
		pml.snapped_origin[i] = (short)(pml.origin[i] * 8.0f);

		if (!FloatIsZeroEpsilon((float)pml.snapped_origin[i] * 0.125f - pml.origin[i])) // H2: FloatIsZeroEpsilon() instead of direct comparison.
			offset[i] = (short)(Q_signf(pml.origin[i]));
	}

	short base[3];
	VectorCopy_Macro(pml.snapped_origin, base); // Q2: pm->s.origin (here and below).

	// Try all combinations.
	for (int i = 0; i < 8; i++)
	{
		VectorCopy_Macro(base, pml.snapped_origin);

		for (int c = 0; c < 3; c++)
			if (jitterbits[i] & (1 << c))
				pml.snapped_origin[c] += offset[c];

		if (PM_GoodPosition())
		{
			VectorCopy_Macro(pml.snapped_origin, pm->s.origin); // H2
			return;
		}
	}

	// Go back to the last position.
	VectorCopy_Macro(pml.previous_origin, pm->s.origin);
}

static void PM_InitialSnapPosition(void)
{
	static short offset[3] = { 0, 1, -1 }; // Q2: { 0, -1, 1 }
	short base[3];

	VectorCopy_Macro(pml.snapped_origin, base); // Q2: pm->s.origin (here and below).

	for (int z = 0; z < 3; z++)
	{
		pml.snapped_origin[2] = (short)(base[2] + offset[z]);

		for (int y = 0; y < 3; y++)
		{
			pml.snapped_origin[1] = (short)(base[1] + offset[y]);

			for (int x = 0; x < 3; x++)
			{
				pml.snapped_origin[0] = (short)(base[0] + offset[x]);

				if (PM_GoodPosition())
				{
					for (int i = 0; i < 3; i++)
					{
						pml.origin[i] = (float)pml.snapped_origin[i] * 0.125f;
						pml.previous_origin[i] = pml.snapped_origin[i];
					}

					return;
				}
			}
		}
	}
}

static void PM_ClampAngles(void)
{
	if (pm->s.pm_flags & PMF_TIME_TELEPORT)
	{
		pm->viewangles[YAW] = SHORT2ANGLE(pm->cmd.angles[YAW] + pm->s.delta_angles[YAW]);
		pm->viewangles[PITCH] = 0.0f;
		pm->viewangles[ROLL] = 0.0f;
	}
	else
	{
		// Circularly clamp the angles with deltas.
		for (int i = 0; i < 3; i++)
			pm->viewangles[i] = SHORT2ANGLE(pm->s.delta_angles[i] + pm->cmd.angles[i]);

		// Don't let the player look up or down more than 90 degrees. //TODO: up/down is YAW (which is clamped somewhere else)!
		if (pm->viewangles[PITCH] > 89.0f && pm->viewangles[PITCH] < 180.0f)
			pm->viewangles[PITCH] = 89.0f;
		else if (pm->viewangles[PITCH] >= 180.0f && pm->viewangles[PITCH] < 271.0f)
			pm->viewangles[PITCH] = 271.0f;
	}
}

static void PM_SpectatorMove(void) // H2
{
	const float upmove = (float)pm->cmd.upmove * 4.0f;
	const float forwardmove = (float)pm->cmd.forwardmove * 4.0f;
	const float sidemove = (float)pm->cmd.sidemove * 4.0f;

	VectorNormalize(pml.forward);
	VectorNormalize(pml.right);

	vec3_t vel;
	vel[0] = pml.forward[0] * forwardmove + pml.right[0] * sidemove;
	vel[1] = pml.forward[1] * forwardmove + pml.right[1] * sidemove;
	vel[2] = pml.forward[2] * forwardmove + upmove;

	ClampVelocity(vel, PM_MAX_SPEED, pm->run_shrine, false);
	VectorCopy(vel, pml.velocity);
	VectorMA(pml.origin, pml.frametime, vel, pml.origin);
}

static void PM_UpdateWaterLevel(void) // H2. Part of PM_CatagorizePosition() logic in Q2.
{
	const vec3_t bottom_pos = VEC3_INITA(pml.origin, 0.0f, 0.0f, pm->mins[2] + 1.0f);

	int contents = pm->pointcontents(bottom_pos);
	if (!(contents & MASK_WATER)) // Not submerged.
	{
		pm->watertype = 0;
		pm->waterlevel = 0;
		pm->waterheight = pm->mins[2];

		return;
	}

	pm->watertype = contents;
	pm->waterlevel = 1;

	const vec3_t top_pos = VEC3_INITA(pml.origin, 0.0f, 0.0f, pm->maxs[2]);

	contents = pm->pointcontents(top_pos);
	if (!(contents & MASK_WATER)) // Partially submerged.
	{
		trace_t trace;
		pm->trace(top_pos, NULL, NULL, bottom_pos, &trace);
		pm->waterheight = trace.endpos[2] - pml.origin[2];

		if (trace.fraction < 1.0f && pml.desired_water_height < pm->waterheight)
			pm->waterlevel = 2;
	}
	else // Fully submerged.
	{
		pm->waterlevel = 3;
		pm->waterheight = pm->maxs[2];
	}
}

// Can be called by either the server (at every packetframe) or the client (at every renderframe).
void Pmove(pmove_t* pmove, const qboolean server)
{
	pm = pmove;

	// Clear results.
	pm->s.c_flags &= ~(PC_COLLISION | PC_SLIDING); // H2
	pm->numtouch = 0;
	VectorClear(pm->viewangles);

	// Clear all pmove local vars.
	memset(&pml, 0, sizeof(pml));
	pml.server = server;

	if (pml.server) // H2
	{
		VectorCopy(pm->origin, pml.origin);
		VectorCopy(pm->velocity, pml.velocity);
	}
	else
	{
		// Convert origin and velocity to float values.
		for (int i = 0; i < 3; i++)
		{
			pml.origin[i] = (float)pm->s.origin[i] * 0.125f;
			pml.velocity[i] = (float)pm->s.velocity[i] * 0.125f;
			pml.previous_origin[i] = pm->s.origin[i]; // H2
			pml.snapped_origin[i] = pm->s.origin[i]; // H2
		}
	}

	pml.desired_water_height = pm->desiredWaterHeight;
	pml.groundsurface = pm->GroundSurface;
	pml.groundplane = pm->GroundPlane;
	pml.groundcontents = pm->GroundContents;
	pml.max_velocity = (pm->waterlevel < 2 ? 1600.0f : 200.0f);
	pml.knockbackfactor = pm->knockbackfactor;
	pml.frametime = (float)pm->cmd.msec / 1000.0f;

	PM_ClampAngles();

	if (pm->s.pm_type == PM_SPECTATOR)
	{
		vec3_t aimangles;

		for (int i = 0; i < 3; i++)
			aimangles[i] = (float)pm->cmd.aimangles[i] * SHORT_TO_ANGLE;

		AngleVectors(aimangles, pml.forward, pml.right, pml.up);
		PM_SpectatorMove();

		for (int i = 0; i < 3; i++)
		{
			pml.snapped_origin[i] = (short)(pml.origin[i] * 8.0f);
			pm->s.origin[i] = pml.snapped_origin[i];
			pm->origin[i] = pml.origin[i];
		}

		return;
	}

	AngleVectors(pm->viewangles, pml.forward, pml.right, pml.up);

	if (pm->s.pm_type >= PM_DEAD)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.sidemove = 0;
		pm->cmd.upmove = 0;
	}

	if (pm->s.pm_type == PM_FREEZE || pm->s.pm_type == PM_INTERMISSION) // H2: extra PM_INTERMISSION check.
		return; // No movement at all.

	if (!pml.server)
		PM_InitialSnapPosition();

	// Drop timing counter.
	if (pm->s.pm_time > 0)
	{
		const byte msec = max(1, pm->cmd.msec >> 3); // Actually msec / 8 --mxd.

		if (msec >= pm->s.pm_time)
		{
			pm->s.pm_flags &= ~(PMF_TIME_LAND | PMF_TIME_TELEPORT);
			pm->s.pm_time = 0;
		}
		else
		{
			pm->s.pm_time -= msec;
		}
	}

	if (!(pm->s.pm_flags & PMF_TIME_TELEPORT)) // Teleport pause stays exactly in place.
	{
		PM_CheckJump();

		pm->s.w_flags &= ~WF_SWIMFREE;

		if (pm->watertype & CONTENTS_SLIME)
		{
			PM_SlimeMove();
		}
		else if (pm->watertype & CONTENTS_LAVA)
		{
			PM_LavaMove();
		}
		else if (pm->waterlevel == 0 || (pm->waterlevel == 1 && (!(pm->s.w_flags & WF_SURFACE) || (pm->s.pm_flags & PMF_ON_GROUND))))
		{
			pm->s.w_flags = 0;
			PM_AirMove();
		}
		else if (pm->waterlevel == 1)
		{
			PM_WaterMove();
		}
		else if (pm->waterlevel == 2)
		{
			if (pm->viewangles[0] > 40.0f)
			{
				if (!(pm->s.w_flags & WF_DIVING))
					pm->s.w_flags |= WF_DIVE;

				PM_UnderwaterMove();
			}
			else
			{
				pm->s.w_flags |= WF_SURFACE;
				pm->s.w_flags &= ~WF_DIVE;
				PM_WaterMove();
			}
		}
		else // m->waterlevel == 3
		{
			pm->s.w_flags = WF_SWIMFREE;
			PM_UnderwaterMove();
		}
	}

	vec3_t mins_diff = { 0 };
	vec3_t maxs_diff = { 0 };

	if (pm->intentMins != NULL)
		VectorSubtract(pm->mins, pm->intentMins, mins_diff);

	if (pm->intentMaxs != NULL)
		VectorSubtract(pm->maxs, pm->intentMaxs, maxs_diff);

	if (!Vec3IsZeroEpsilon(mins_diff) || !Vec3IsZeroEpsilon(maxs_diff))
		PM_OnBboxSizeChanged();

	// Set groundentity, watertype, and waterlevel for final spot.
	PM_CatagorizePosition();
	PM_UpdateWaterLevel();

	pm->GroundSurface = pml.groundsurface;
	pm->GroundPlane = pml.groundplane;
	pm->GroundContents = pml.groundcontents;

	if (pml.server)
	{
		VectorCopy(pml.origin, pm->origin);
		VectorCopy(pml.velocity, pm->velocity);

		for (int i = 0; i < 3; i++)
		{
			pm->s.velocity[i] = (short)(pml.velocity[i] * 8.0f);
			pm->s.origin[i] = (short)(pml.origin[i] * 8.0f);
		}
	}
	else
	{
		PM_SnapPosition();
	}
}
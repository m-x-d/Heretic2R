//
// pmove.c
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"
#include "game.h"
#include "q_Physics.h"
#include "Vector.h"

// All of the locals will be zeroed before each pmove, just to make sure
// we don't have any differences when running on client or server.

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
#define PM_STOPSPEED		100.0f
#define PM_MAXSPEED			300.0f
#define PM_FRICTION			6.0f
#define PM_WATERACCELERATE	10.0f
#define PM_WATERSPEED		400.0f

#define MIN_STEP_NORMAL	0.7f // Can't step up onto very steep slopes.
#define MAX_CLIP_PLANES	5

static float ClampVelocity(vec3_t vel, vec3_t vel_normal, const qboolean run_shrine, const qboolean high_max)
{
	float max_speed = PM_MAXSPEED;
	if (high_max || run_shrine)
		max_speed *= 2.0f;

	const float speed = VectorNormalize2(vel, vel_normal);

	if (speed > max_speed)
	{
		VectorScale(vel_normal, max_speed, vel);
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

static qboolean PM_CanMoveToPos(const float offset_z, const float frametime, trace_t* tr) // H2
{
	vec3_t start;
	vec3_t end;
	trace_t trace;

	vec3_t vel;
	VectorScale(pml.velocity, frametime, vel);

	if (vhlen(vel, vec3_origin) < 0.01f)
		return false;

	VectorCopy(pml.origin, start);
	VectorCopy(pml.origin, end);
	end[2] += offset_z + max(0.0f, vel[2]);

	pm->trace(start, pm->mins, pm->maxs, end, &trace);

	if (trace.startsolid || trace.allsolid || trace.endpos[2] < pml.origin[2] + 1.0f)
		return false;

	VectorCopy(trace.endpos, start);
	VectorCopy(trace.endpos, end);
	end[0] += vel[0];
	end[1] += vel[1];

	pm->trace(start, pm->mins, pm->maxs, end, &trace);

	if (trace.startsolid || trace.allsolid || trace.fraction == 0.0f)
		return false;

	const float prev_fraction = trace.fraction;

	VectorCopy(trace.endpos, start);
	VectorCopy(trace.endpos, end);
	end[2] = pml.origin[2] + min(0.0f, vel[2]);

	pm->trace(start, pm->mins, pm->maxs, end, &trace);

	if (trace.startsolid || trace.allsolid)
		return false;

	if ((prev_fraction < 1.0f && trace.endpos[2] <= pml.origin[2] + 0.01f) || 
		(trace.fraction < 1.0f && trace.plane.normal[2] <= MIN_STEP_NORMAL))
		return false;

	trace.fraction = prev_fraction;
	memcpy(tr, &trace, sizeof(trace_t));
	VectorCopy(trace.endpos, pml.origin);

	return true;
}

// Each intersection will try to step over the obstruction instead of sliding along it.
static void PM_StepSlideMove(void)
{
	static vec3_t planes[MAX_CLIP_PLANES];

	vec3_t dir;
	vec3_t cross;
	vec3_t vel;
	vec3_t bounce_vel;
	trace_t trace;

	float time_left = pml.frametime;

	float scaled_vel = 0.0f;
	float inv_scaled_vel = 0.0f;

	if (pm->groundentity == NULL)
		pml.groundplane.normal[2] = 0.0f;

	vec3_t primal_velocity = VEC3_INIT(pml.velocity);
	vec3_t plane_normal = VEC3_INIT(pml.groundplane.normal);

	int numplanes = 0;
	int cur_plane = 0;
	int prev_plane = -1;

	for (int bumpcount = 0; bumpcount < 4; bumpcount++)
	{
		qboolean do_bounce = false;
		qboolean is_bouncing = false;

		VectorScale(pml.velocity, time_left, vel);

		float time_left_sq = time_left * time_left;

		qboolean skip_groundentity_check = false; //TODO: better name

		if (pml.groundplane.normal[2] != 0.0f)
		{
			float vel_amount;
			qboolean check_bounce = true;

			if (Vec3IsZero(pml.velocity))
			{
				if (pml.groundplane.normal[2] >= MIN_STEP_NORMAL && pml.groundplane.normal[2] >= pml.gravity / (pml.max_velocity + pml.gravity) && bumpcount > 0)
					return;

				//mxd. Get plane_normal perpendicular. 'cross' is parallel to plane(?).
				cross[0] = plane_normal[0] * plane_normal[2];
				cross[1] = plane_normal[1] * plane_normal[2];
				cross[2] = -(plane_normal[0] * plane_normal[0]) - (plane_normal[1] * plane_normal[1]);
				VectorNormalize(cross);

				if (DotProduct(cross, plane_normal) < -FLOAT_ZERO_EPSILON)
				{
					VectorMA(pml.origin, 0.5f, planes[cur_plane], pml.origin);
					prev_plane = cur_plane;
				}

				vel_amount = 0.0f;
			}
			else
			{
				vel_amount = VectorNormalize2(vel, dir);

				const float d = DotProduct(dir, plane_normal);
				if (d < -0.05f || d >= 0.05f)
					check_bounce = false;
			}

			if (check_bounce)
			{
				do_bounce = true; //mxd. Change movement direction because of bouncing this frame(?)
				is_bouncing = true; //mxd. Currently bouncing upwards(?)

				//mxd. Get plane_normal perpendicular. 'cross' is parallel to plane(?).
				cross[0] = plane_normal[0] * plane_normal[2];
				cross[1] = plane_normal[1] * plane_normal[2];
				cross[2] = -(plane_normal[0] * plane_normal[0]) - plane_normal[1] * plane_normal[1];
				VectorNormalize(cross);

				scaled_vel = pml.groundplane.normal[2] * pml.max_velocity;

				if (pml.groundplane.normal[2] >= MIN_STEP_NORMAL || pm->waterlevel > 0)
				{
					inv_scaled_vel = -scaled_vel;
				}
				else
				{
					inv_scaled_vel = -scaled_vel * 0.1f;

					pm->s.c_flags |= PC_SLIDING;
					pml.velocity[2] = min(-40.0f, pml.velocity[2]);
				}

				float dist = inv_scaled_vel * time_left_sq * 0.5f + vel_amount;

				if (dist < 0.0f)
				{
					dist = 0.0f;
					inv_scaled_vel = 0.0f;
				}

				if (DotProduct(cross, plane_normal) < -FLOAT_ZERO_EPSILON)
				{
					VectorMA(pml.origin, 0.5f, planes[cur_plane], pml.origin);
					prev_plane = cur_plane;
				}

				if (pml.groundplane.normal[2] >= MIN_STEP_NORMAL || pm->waterlevel != 0)
				{
					scaled_vel = (1.0f - pml.groundplane.normal[2]) * pml.gravity - scaled_vel;
				}
				else
				{
					scaled_vel = (1.0f - pml.groundplane.normal[2]) * pml.gravity - pml.groundplane.normal[2] * pml.max_velocity * 0.1f;
					pm->s.c_flags |= PC_SLIDING;
				}

				scaled_vel = max(0.0f, scaled_vel);
				time_left_sq *= scaled_vel * 0.5f;

				for (int i = 0; i < 3; i++)
					vel[i] = cross[i] * time_left_sq + dir[i] * dist;

				if (scaled_vel + inv_scaled_vel + dist != 0.0f)
				{
					skip_groundentity_check = true;
				}
				else
				{
					trace.fraction = 0.0f;
					break;
				}
			}
		}

		if (!skip_groundentity_check && pm->groundentity == NULL)
			vel[2] -= time_left_sq * pml.gravity * 0.5f;

		vec3_t end;
		VectorAdd(pml.origin, vel, end);
		pm->trace(pml.origin, pm->mins, pm->maxs, end, &trace);

		if (!trace.startsolid)
		{
LAB_NotSolid:
			if (trace.allsolid)
			{
				// Entity is trapped in another solid.
				VectorClear(pml.velocity);
				pml.origin[2] += 20.0f;

				return;
			}

			const float time_step = time_left * trace.fraction;

			if (trace.fraction > 0.0f)
			{
				// Actually covered some distance.
				VectorCopy(trace.endpos, pml.origin);

				if (do_bounce)
				{
					float dist = VectorNormalize2(pml.velocity, dir);
					dist += time_step * inv_scaled_vel;

					float scaler = time_step * scaled_vel;

					for (int i = 0; i < 3; i++)
						pml.velocity[i] = cross[i] * scaler + dir[i] * dist;
				}
				else
				{
					pml.velocity[2] -= time_step * pml.gravity;
				}

				if (trace.fraction == 1.0f)
					break; // Moved the entire distance.

				numplanes = 0;
				prev_plane = -1;

				VectorCopy(pml.velocity, primal_velocity);
			}
			else if (Vec3IsZero(pml.velocity) && pml.groundplane.normal[2] >= pml.gravity / (pml.max_velocity + pml.gravity))
			{
				break;
			}

			if (pml.server && trace.ent != NULL && trace.ent->isBlocking != NULL)
			{
				trace_t tr = trace;

				tr.ent = pm->self;
				VectorInverse(tr.plane.normal);
				trace.ent->isBlocking(trace.ent, &tr);
			}

			// Save entity for contact.
			if (pm->numtouch < MAXTOUCH && trace.ent != NULL)
			{
				pm->touchents[pm->numtouch] = trace.ent;
				pm->numtouch++;
			}

			time_left -= time_step;

			if (trace.plane.normal[2] <= MIN_STEP_NORMAL && trace.ent != NULL)
			{
				const qboolean have_trace_ent = (!pml.server || (trace.ent->solid == SOLID_BSP || trace.ent->solid == SOLID_BBOX)); //mxd
				if (have_trace_ent && PM_CanMoveToPos(STEP_SIZE, time_left, &trace))
					break;
			}

			if (pm->groundentity == NULL || pml.groundplane.normal[2] <= MIN_STEP_NORMAL || trace.plane.normal[2] > MIN_STEP_NORMAL)
			{
				VectorCopy(trace.plane.normal, pml.groundplane.normal);
				VectorCopy(trace.plane.normal, plane_normal);

				qboolean have_trace_ent = (pml.server && (trace.ent->solid == SOLID_BSP || trace.ent->solid == SOLID_BBOX)) || (!pml.server && trace.ent != NULL); //mxd
				if (have_trace_ent && trace.plane.normal[2] > 0.0f)
				{
					pm->groundentity = trace.ent;
				}
				else
				{
					pm->groundentity = NULL;

					if (trace.plane.normal[2] < 0.0f)
						pml.groundplane.normal[2] = 0.0f;
				}

				if (numplanes > 4)
					break;

				if (numplanes == 0 || !VectorCompare(trace.plane.normal, planes[numplanes - 1]))
				{
					VectorCopy(trace.plane.normal, planes[numplanes]);
					numplanes++;
				}
				else
				{
					prev_plane = numplanes - 1;
					VectorMA(pml.origin, 0.5f, planes[prev_plane], pml.origin);
				}
			}
			else
			{
				vec3_t v = VEC3_SET(trace.plane.normal[0], trace.plane.normal[1], 0.0f);
				VectorNormalize(v);

				if (numplanes == 0 || !VectorCompare(v, planes[numplanes - 1]))
				{
					VectorCopy(v, planes[numplanes]);
					numplanes++;
				}
			}

			// Modify original_velocity so it parallels all of the clip planes.
			for (cur_plane = 0; cur_plane < numplanes; cur_plane++)
			{
				BounceVelocity(primal_velocity, planes[cur_plane], bounce_vel, ELASTICITY_SLIDE);

				const float bounce_amount = VectorNormalize2(bounce_vel, dir);

				if (fabsf(bounce_amount) < FLOAT_ZERO_EPSILON)
				{
					if (planes[cur_plane][2] > 0.0f)
					{
						//mxd. Get plane perpendicular. 'vel_normal' is parallel to plane(?). Opposite direction to previous 2 cases!
						dir[0] = -(planes[cur_plane][2] * planes[cur_plane][0]);
						dir[1] = -(planes[cur_plane][1] * planes[cur_plane][2]);
						dir[2] = planes[cur_plane][1] * planes[cur_plane][1] + planes[cur_plane][0] * planes[cur_plane][0];

						VectorNormalize(dir);
					}
					else
					{
						dir[2] = -1.0f;
					}
				}

				const float d = DotProduct(dir, planes[cur_plane]);

				if (d < -FLOAT_ZERO_EPSILON)
				{
					VectorMA(pml.origin, 0.5f, planes[cur_plane], pml.origin);
					prev_plane = cur_plane;
				}

				is_bouncing = (planes[cur_plane][2] > 0.0f && d >= -0.01f && fabsf(d) < 0.01f);

				int j;
				for (j = 0; j < numplanes; j++)
					if (j != cur_plane && DotProduct(bounce_vel, planes[j]) <= 0.0f)
						break; // Not ok.

				if (j == numplanes)
					break;
			}

			// Go along the crease.
			if (numplanes != 2)
			{
				if (cur_plane == numplanes)
				{
					VectorClear(pml.velocity);
					return;
				}

				VectorCopy(bounce_vel, pml.velocity);
			}
			else
			{
				CrossProduct(planes[0], planes[1], dir);

				if (DotProduct(dir, pml.velocity) < 0.0f)
					VectorInverse(dir);

				dir[2] += 0.1f;
				VectorNormalize(dir);

				VectorAdd(planes[0], planes[1], plane_normal);
				VectorNormalize(plane_normal);

				if (pm->groundentity != NULL && dir[2] > MIN_STEP_NORMAL)
				{
					trace.fraction = 0.0f;
					break;
				}

				float d = DotProduct(dir, pml.velocity);
				d = max(0.0f, d);

				if (dir[2] <= 0.0f)
					is_bouncing = true;

				VectorScale(dir, d, pml.velocity);
			}

			float dist = VectorNormalize2(pml.velocity, dir);

			if (is_bouncing)
			{
				dist += (-((1.0f - dir[2]) * pml.max_velocity) - dir[2] * pml.gravity) * time_left;
				bounce_vel[2] = dir[2] * dist;
			}
			else
			{
				bounce_vel[2] = dir[2] * dist - pml.gravity * time_left;
			}

			bounce_vel[1] = dir[1] * dist;
			bounce_vel[0] = dir[0] * dist;
		}
		else
		{
			if (prev_plane == -1)
			{
				vec3_t mins = VEC3_INIT(pm->mins);
				vec3_t maxs = VEC3_INIT(pm->maxs);

				int offset = 1;
				while (maxs[0] - (float)offset >= mins[0] + (float)offset)
				{
					VectorInc(mins);
					VectorDec(maxs);

					pm->trace(pml.origin, mins, maxs, pml.origin, &trace);

					if (!trace.startsolid)
					{
						VectorCopy(pm->mins, pm->intentMins);
						VectorCopy(pm->maxs, pm->intentMaxs);
						VectorCopy(mins, pm->mins);
						VectorCopy(maxs, pm->maxs);

						goto LAB_NotSolid;
					}

					offset++;
				}

				VectorClear(pml.velocity);
				return;
			}

			for (int i = 0; i < 3; i++)
				pml.origin[i] -= planes[prev_plane][i] * 0.5f;
		}
	}

	if (trace.fraction < 1.0f)
		VectorClear(pml.velocity);

	CheckCollision((float)pm->cmd.aimangles[YAW] * SHORT_TO_ANGLE * ANGLE_TO_RAD);
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

		float s = PM_WATERSPEED;
		if (pm->waterlevel == 1 && pm->groundentity != NULL)
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
			Com_Printf("CONTENTS_CURRENT_UP or CONTENTS_CURRENT_DOWN not supported on groundcontents (conveyor belts)\n");

		VectorMA(wishvel, 100.0f, v, wishvel);
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
	qboolean run_shrine = false; // H2
	float fmove = pm->cmd.forwardmove;
	float smove = pm->cmd.sidemove;

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

	vec3_t unused;
	const float maxspeed = ClampVelocity(wishvel, unused, run_shrine, pm->high_max);

	if (pm->groundentity != NULL)
	{
		// Standing on ground.
		if (maxspeed == 0.0f && pml.groundplane.normal[2] >= MIN_STEP_NORMAL && pml.groundplane.normal[2] >= pml.gravity / (pml.max_velocity + pml.gravity))
		{
			VectorClear(pml.velocity);
			CheckCollision((float)pm->cmd.aimangles[YAW] * SHORT_TO_ANGLE * ANGLE_TO_RAD);

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
		drop = max(PM_STOPSPEED, speed) * pml.frametime * PM_FRICTION;

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
	vec3_t wishvel;
	vec3_t wishdir;

	qboolean run_shrine = false;
	pml.gravity = 0.0f;

	PM_Friction();

	float forwardmove = pm->cmd.forwardmove;
	const float sidemove = pm->cmd.sidemove;

	if (pm->run_shrine && forwardmove > 0.0f)
	{
		forwardmove *= 1.65f;
		run_shrine = true;
	}

	// User intentions.
	for (int i = 0; i < 3; i++)
		wishvel[i] = pml.forward[i] * forwardmove + pml.right[i] * sidemove;

	PM_AddCurrents(wishvel);

	VectorScale(wishvel, scaler, wishvel);
	ClampVelocity(wishvel, wishdir, run_shrine, false);

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
static void PM_WaterAccelerate(vec3_t wishdir, const float wishspeed, const float accel) //mxd. Used only by PM_WaterMove() in H2.
{
	const float currentspeed = DotProduct(pml.velocity, wishdir);
	const float addspeed = wishspeed - currentspeed;

	if (addspeed <= 0.0f)
		return;

	float accelspeed = accel * pml.frametime * wishspeed;
	accelspeed = min(addspeed, accelspeed);

	for (int i = 0; i < 3; i++)
		pml.velocity[i] += accelspeed * wishdir[i];
}

static qboolean PM_WaterMove(const float scaler)
{
	vec3_t wishvel;
	vec3_t wishdir;

	qboolean run_shrine = false;
	pml.gravity = 0.0f;

	PM_Friction();

	float forwardmove = pm->cmd.forwardmove;
	const float sidemove = pm->cmd.sidemove;

	if (pm->run_shrine && forwardmove > 0.0f)
	{
		forwardmove *= 1.65f;
		run_shrine = true;
	}

	// User intentions.
	for (int i = 0; i < 3; i++)
		wishvel[i] = pml.forward[i] * forwardmove + pml.right[i] * sidemove;

	PM_AddCurrents(wishvel);

	const float wishspeed = ClampVelocity(wishvel, wishdir, run_shrine, false);
	PM_WaterAccelerate(wishdir, wishspeed * scaler, PM_WATERACCELERATE);

	if (pm->groundentity == NULL)
		return false;

	if (wishspeed * scaler == 0.0f && pml.groundplane.normal[2] >= MIN_STEP_NORMAL && pml.groundplane.normal[2] >= pml.gravity / (pml.max_velocity + pml.gravity))
	{
		VectorClear(pml.velocity);
		return true;
	}
	else
	{
		VectorCopy(wishvel, pml.velocity);
		return false;
	}
}

static void PM_TryUnderwaterMove(void) // H2
{
	if (PM_WaterMove(0.5f))
		return;

	PM_StepSlideMove();
}

// Swim on water surface or walk while in water.
static void PM_TryWaterMove(void) // H2
{
	if (PM_WaterMove(0.5f))
		return;

	if (pm->s.w_flags & WF_SINK)
	{
		pm->s.w_flags &= ~WF_SINK;
	}
	else
	{
		pml.velocity[2] = (pm->waterheight - pml.desired_water_height) / pml.frametime;
		pml.velocity[2] += sinf((float)Sys_Milliseconds() / 150.0f) * 8.0f;
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

	vec3_t unused;
	ClampVelocity(vel, unused, pm->run_shrine, false);

	VectorCopy(vel, pml.velocity);
	VectorMA(pml.origin, pml.frametime, vel, pml.origin);
}

static void PM_UpdateWaterLevel(void) // H2. Part of PM_CatagorizePosition() logic in Q2.
{
	trace_t trace;

	vec3_t bottom_pos;
	bottom_pos[0] = pml.origin[0];
	bottom_pos[1] = pml.origin[1];
	bottom_pos[2] = pml.origin[2] + pm->mins[2] + 1.0f;

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

	vec3_t top_pos;
	top_pos[0] = bottom_pos[0];
	top_pos[1] = bottom_pos[1];
	top_pos[2] = pml.origin[2] + pm->maxs[2];

	contents = pm->pointcontents(top_pos);
	if (!(contents & MASK_WATER)) // Partially submerged.
	{
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
	pm->s.c_flags &= ~(PC_COLLISION | PC_SLIDING); // H2

	// Clear results.
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
			PM_TryWaterMove();
		}
		else if (pm->waterlevel == 2)
		{
			if (pm->viewangles[0] > 40.0f)
			{
				if (!(pm->s.w_flags & WF_DIVING))
					pm->s.w_flags |= WF_DIVE;

				PM_TryUnderwaterMove();
			}
			else
			{
				pm->s.w_flags |= WF_SURFACE;
				pm->s.w_flags &= ~WF_DIVE;
				PM_TryWaterMove();
			}
		}
		else // m->waterlevel == 3
		{
			pm->s.w_flags = WF_SWIMFREE;
			PM_TryUnderwaterMove();
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
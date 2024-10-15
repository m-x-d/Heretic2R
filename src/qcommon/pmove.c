//
// pmove.c
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"
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

	vec3_t previous_origin;
	short short_origin[3]; // H2 //TODO: better name?
} pml_t;

static pmove_t* pm;
static pml_t pml;

static void PM_TryMove(void) // H2
{
	NOT_IMPLEMENTED
}

static void PM_AirMove(void)
{
	NOT_IMPLEMENTED
}

static void PM_TryWaterMove(void) // H2
{
	NOT_IMPLEMENTED
}

static void PM_WaterSink(void) // H2
{
	NOT_IMPLEMENTED
}

static void PM_SlimeMove(void) // H2
{
	NOT_IMPLEMENTED
}

static void PM_LavaMove(void) // H2
{
	NOT_IMPLEMENTED
}

static void PM_CatagorizePosition(void)
{
	NOT_IMPLEMENTED
}

static void PM_CheckJump(void)
{
	NOT_IMPLEMENTED
}

static void PM_SnapPosition(void)
{
	NOT_IMPLEMENTED
}

static void PM_InitialSnapPosition(void)
{
	NOT_IMPLEMENTED
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

		// Don't let the player look up or down more than 90 degrees.
		if (pm->viewangles[PITCH] > 89.0f && pm->viewangles[PITCH] < 180.0f)
			pm->viewangles[PITCH] = 89.0f;
		else if (pm->viewangles[PITCH] < 271.0f && pm->viewangles[PITCH] >= 180.0f)
			pm->viewangles[PITCH] = 271.0f;
	}
}

static void PM_UpdateOriginAndVelocity(void)
{
	NOT_IMPLEMENTED
}

static void PM_UpdateWaterLevel(void)
{
	NOT_IMPLEMENTED
}

// Can be called by either the server or the client.
void Pmove(pmove_t* pmove, const qboolean server)
{
	pm = pmove;
	pmove->s.c_flags &= ~(PC_COLLISION | PC_SLIDING); // H2

	// Clear results.
	pm->numtouch = 0;
	VectorClear(pm->viewangles);

	// Clear all pmove local vars.
	memset(&pml, 0, sizeof(pml));
	pml.server = server;

	if (server) // H2
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
			pml.previous_origin[i] = pm->s.origin[i];
			pml.short_origin[i] = pm->s.origin[i];
		}
	}

	pml.desired_water_height = pm->desiredWaterHeight;
	pml.groundsurface = pm->GroundSurface;
	pml.groundplane = pm->GroundPlane;
	pml.groundcontents = pm->GroundContents;
	pml.max_velocity = (pm->waterlevel < 2 ? 1600.0f : 200.0f);
	pml.knockbackfactor = pm->knockbackfactor;
	pml.frametime = (float)pm->cmd.msec * 0.001f;

	PM_ClampAngles();

	if (pm->s.pm_type == PM_SPECTATOR)
	{
		vec3_t aimangles;

		for (int i = 0; i < 3; i++)
			aimangles[i] = (float)pm->cmd.aimangles[i] * SHORT_TO_ANGLE;

		AngleVectors(aimangles, pml.forward, pml.right, pml.up);
		PM_UpdateOriginAndVelocity();

		for (int i = 0; i < 3; i++)
		{
			pml.short_origin[i] = (short)(pml.origin[i] * 8.0f);
			pm->s.origin[i] = pml.short_origin[i];
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
		byte msec = pm->cmd.msec >> 3;

		if (msec == 0)
			msec = 1;

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
		else if (pm->waterlevel == 0 || (pm->waterlevel == 1 && ((pm->s.w_flags & WF_SURFACE) == 0 || (pm->s.pm_flags & PMF_ON_GROUND) != 0)))
		{
			pm->s.w_flags = 0;
			PM_AirMove();
		}
		else if (pm->waterlevel == 1)
		{
			PM_WaterSink();
		}
		else if (pm->waterlevel == 2)
		{
			if (pm->viewangles[0] > 40.0f)
			{
				if (!(pm->s.w_flags & WF_DIVING))
					pm->s.w_flags |= WF_DIVE;

				PM_TryWaterMove();
			}
			else
			{
				pm->s.w_flags |= WF_SURFACE;
				pm->s.w_flags &= ~WF_DIVE;
				PM_WaterSink();
			}
		}
		else // m->waterlevel == 3
		{
			pm->s.w_flags = WF_SWIMFREE;
			PM_TryWaterMove();
		}
	}

	vec3_t mins_diff = { 0 };
	vec3_t maxs_diff = { 0 };

	if (pm->intentMins != NULL)
		VectorSubtract(pm->mins, pm->intentMins, mins_diff);

	if (pm->intentMaxs != NULL)
		VectorSubtract(pm->maxs, pm->intentMaxs, maxs_diff);

	for (int i = 0; i < 3; i++)
	{
		if (fabsf(mins_diff[i]) >= 0.0005f || fabsf(maxs_diff[i]) >= 0.0005f)
		{
			PM_TryMove();
			break;
		}
	}

	// Set groundentity, watertype, and waterlevel for final spot.
	PM_CatagorizePosition();
	PM_UpdateWaterLevel();

	pm->GroundSurface = pml.groundsurface;
	pm->GroundPlane = pml.groundplane;
	pm->GroundContents = pml.groundcontents;

	if (!pml.server)
	{
		PM_SnapPosition();
		return;
	}

	VectorCopy(pml.origin, pm->origin);
	VectorCopy(pml.velocity, pm->velocity);

	for (int i = 0; i < 3; i++)
	{
		pm->s.velocity[i] = (short)(pml.velocity[i] * 8.0f);
		pm->s.origin[i] = (short)(pml.origin[i] * 8.0f);
	}
}
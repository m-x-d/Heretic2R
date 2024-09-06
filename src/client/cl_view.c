//
// cl_view.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "Vector.h"

cvar_t* crosshair;
static cvar_t* cl_stats;

int frame_index; // H2

// H2 screen shake
static float screen_shake_duration;
static float screen_shake_intensity_max;
static float screen_shake_endtime;
static int screen_shake_flags;

static float CalcFov(float fov_x, float width, float height)
{
	NOT_IMPLEMENTED
	return 0;
}

//mxd. Defined in cl_scrn.c in Q2.
static int entitycmpfnc(const entity_t** a, const entity_t** b)
{
	NOT_IMPLEMENTED
	return 0;
}

static int alphaentitycmpfnc(const entity_t** a, const entity_t** b)
{
	NOT_IMPLEMENTED
	return 0;
}

static void ClearRenderStats(void)
{
	cls.r_numdlights = 0;
	cls.r_numentities = 0;
	cls.r_num_alpha_entities = 0;
	cls.r_numparticles = 0;
	cls.r_anumparticles = 0;
}

void V_RenderView(const float stereo_separation)
{
	// Inactive or still loading.
	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	if ((int)cl_timedemo->value)
	{
		if (cl.timedemo_start == 0)
			cl.timedemo_start = Sys_Milliseconds();

		cl.timedemo_frames++;
	}

	cl.force_refdef = false;

	// Build a refresh entity list and calc cl.sim* this also calls CL_CalcViewValues which loads v_forward, etc.
	CL_AddEntities();

	// Offset vieworg appropriately if we're doing stereo separation
	if (stereo_separation != 0.0f)
	{
		cl.refdef.vieworg[0] += cl.v_right[0] * stereo_separation;
		cl.refdef.vieworg[1] += cl.v_right[1] * stereo_separation;
		cl.refdef.vieworg[2] += cl.v_right[2] * stereo_separation;
	}

	// Never let it sit exactly on a node line, because a water plane can disappear when viewed with the eye exactly on it.
	// The server protocol only specifies to 1/8 pixel, so add 1/16 in each axis.
	cl.refdef.vieworg[0] += 1.0f / 16.0f;
	cl.refdef.vieworg[1] += 1.0f / 16.0f;
	cl.refdef.vieworg[2] += 1.0f / 16.0f;

	cl.refdef.x = scr_vrect.x;
	cl.refdef.y = scr_vrect.y;

	cl.refdef.width = scr_vrect.width;
	cl.refdef.height = scr_vrect.height;
	
	cl.refdef.fov_y = CalcFov(cl.refdef.fov_x, (float)scr_vrect.width, (float)scr_vrect.height);

	if (!(int)cl_paused->value)
		cl.refdef.time = (float)cl.time * 0.001f;

	cl.refdef.areabits = cl.frame.areabits;

	if (!(int)cl_add_entities->value)
	{
		cls.r_num_alpha_entities = 0; // H2
		cls.r_numentities = 0;
	}

	if (!(int)cl_add_particles->value)
		cls.r_numparticles = 0;

	if (!(int)cl_add_lights->value)
		cls.r_numdlights = 0;

	if (!(int)cl_add_blend->value)
		VectorClear(cl.refdef.blend);

	cl.refdef.num_entities = cls.r_numentities;
	cl.refdef.entities = cls.r_entities;

	cl.refdef.num_alpha_entities = cls.r_num_alpha_entities; // H2
	cl.refdef.alpha_entities = cls.r_alpha_entities; // H2

	cl.refdef.num_particles = cls.r_numparticles;
	cl.refdef.particles = cls.r_particles;

	cl.refdef.anum_particles = cls.r_anumparticles; // H2
	cl.refdef.aparticles = cls.r_aparticles; // H2

	cl.refdef.num_dlights = cls.r_numdlights;
	cl.refdef.dlights = cls.r_dlights;
	cl.refdef.lightstyles = cls.r_lightstyles;

	cl.refdef.rdflags = cl.frame.playerstate.rdflags;
	
	// Sort entities for better cache locality
	qsort(cls.r_entities, cls.r_numentities, sizeof(cls.r_entities[0]), entitycmpfnc);
	qsort(cl.refdef.alpha_entities, cl.refdef.num_alpha_entities, sizeof(cl.refdef.alpha_entities[0]), alphaentitycmpfnc); // H2

	frame_index = re.RenderFrame(&cl.refdef);

	if ((int)cl_stats->value)
		Com_Printf("ent:%i  lt:%i  part:%i  ", cls.r_numentities, cls.r_numdlights, cls.r_numparticles);

	if ((int)log_stats->value && log_stats_file != NULL)
		fprintf(log_stats_file, "%i,%i,%i,", cls.r_numentities, cls.r_numdlights, cls.r_numparticles);

	SCR_AddDirtyPoint(scr_vrect.x, scr_vrect.y);
	SCR_AddDirtyPoint(scr_vrect.width + scr_vrect.x - 1, scr_vrect.height + scr_vrect.y - 1);

	ClearRenderStats(); // H2
}

//mxd. Very stripped down compared to Q2 version.
void V_Init(void)
{
	cl_stats = Cvar_Get("cl_stats", "0", 0);
	crosshair = Cvar_Get("crosshair", "0", CVAR_ARCHIVE);
}

void CL_ClearScreenShake(void) // H2
{
	screen_shake_duration = 0.0f;
	screen_shake_intensity_max = 0.0f;
	screen_shake_endtime = -1.0f;
	screen_shake_flags = 0;
}

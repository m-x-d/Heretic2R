//
// cl_view.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_effects.h"
#include "cmodel.h"
#include "g_playstats.h"
#include "Random.h"
#include "Vector.h"

cvar_t* crosshair;
static cvar_t* cl_stats;

int frame_index; // H2

// H2 screen flash
static int screen_flash_color;

// H2 screen shake
static float screen_shake_duration;
static float screen_shake_intensity_min;
static float screen_shake_intensity_max;
static float screen_shake_endtime;
static int screen_shake_flags;

static void RegisterModels(void) // H2
{
	precache_models = true;
	fxe.RegisterModels();
	precache_models = false;
}

// Call before entering a new level, or after changing dlls.
void CL_PrepRefresh(void)
{
	char mapname[32];
	char name[MAX_QPATH];
	vec3_t axis;

	if (cl.configstrings[CS_MODELS + 1][0] == 0)
		return; // No map loaded.

	cls.disable_screen = false; // H2

	SCR_AddDirtyPoint(0, 0);
	SCR_AddDirtyPoint(viddef.width - 1, viddef.height - 1);

	SCR_UpdateProgressbar(0, 1); // H2
	strcpy_s(mapname, sizeof(mapname), cl.configstrings[CS_MODELS + 1] + 5); // Skip "maps/". //mxd. strcpy -> strcpy_s
	mapname[strlen(mapname) - 4] = 0; // Cut off ".bsp".

	// Register models, pics, and skins.
	Com_Printf("Map: %s\r", mapname);
	re.BeginRegistration(mapname);
	Com_Printf("                                     \r");
	Com_Printf("pics\r");

	SCR_UpdateProgressbar(0, 2); // H2
	Com_Printf("                                     \r");
	RegisterModels(); // H2

	SCR_UpdateProgressbar(0, 3); // H2

	for (int i = 1; i < MAX_MODELS && cl.configstrings[CS_MODELS + i][0] != 0; i++)
	{
		strcpy_s(name, sizeof(name), cl.configstrings[CS_MODELS + i]); //mxd. strcpy -> strcpy_s
		name[37] = 0; // Never go beyond one line.

		if (name[0] != '*')
			Com_Printf("%s\r", name);

		SCR_UpdateScreen();
		IN_Update(); // H2. Pump message loop.

		cl.model_draw[i] = re.RegisterModel(cl.configstrings[CS_MODELS + i]);
		if (name[0] == '*')
			cl.model_clip[i] = CM_InlineModel(cl.configstrings[CS_MODELS + i]);
		else
			cl.model_clip[i] = NULL;

		if (name[0] != '*')
			Com_Printf("                                     \r");
	}

	Com_Printf("images\r");
	SCR_UpdateProgressbar(0, 4); // H2

	for (int i = 1; i < MAX_IMAGES && cl.configstrings[CS_IMAGES + i][0]; i++)
	{
		cl.image_precache[i] = re.RegisterPic(cl.configstrings[CS_IMAGES + i]);
		IN_Update(); // H2. Pump message loop.
	}

	SCR_UpdateProgressbar(0, 5); // H2
	Com_Printf("                                     \r");

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!cl.configstrings[CS_PLAYERSKINS + i][0])
			continue;

		Com_Printf("client %i\r", i);
		SCR_UpdateScreen();
		IN_Update(); // H2. Pump message loop.
		CL_ParseClientinfo(i);
		Com_Printf("                                     \r");
	}

	SCR_UpdateScreen();
	CL_LoadClientinfo(&cl.baseclientinfo, "unnamed\\male/Corvus", MAX_PLAYER_MODELS);

	// Set sky textures and speed.
	Com_Printf("sky\r");
	SCR_UpdateScreen();

	const float rotate = (float)strtod(cl.configstrings[CS_SKYROTATE], NULL); //mxd. atof -> strtod
	sscanf_s(cl.configstrings[CS_SKYAXIS], "%f %f %f", &axis[0], &axis[1], &axis[2]); //mxd. sscanf -> sscanf_s
	re.SetSky(cl.configstrings[CS_SKY], rotate, axis);
	Com_Printf("                                     \r");

	// The renderer can now free unneeded stuff.
	re.EndRegistration();

	// Clear any lines of console text.
	Con_ClearNotify();

	SCR_UpdateScreen();
	cl.refresh_prepped = true;
	cl.force_refdef = true; // Make sure we have a valid refdef.

	IN_Update(); // H2. Pump message loop.
	cls.key_dest = 0; // H2

	// Start the cd track.
	se.MusicPlay(Q_atoi(cl.configstrings[CS_CDTRACK]), true); //mxd. CDAudio_Play() in original logic.
}

static void CalcFov(const float width, const float height) //mxd. Added Hor+ widescreen fov calculation.
{
	if (width * 0.75f > height) //mxd. Widescreen aspect ratio.
	{
		cl.refdef.fov_y = cl.refdef.fov_x * 0.82f;
		cl.refdef.fov_x = atanf(width / (height / tanf(cl.refdef.fov_y * M_PI / 360.0f))) * 360.0f / M_PI;
	}
	else // 4x3 (or less) aspect ratio.
	{
		cl.refdef.fov_y = atanf(height / (width / tanf(cl.refdef.fov_x * M_PI / 360.0f))) * 360.0f / M_PI;
	}
}

//mxd. Defined in cl_scrn.c in Q2.
static int entitycmpfnc(const void* a, const void* b)
{
	entity_t** e1 = (entity_t**)a;
	entity_t** e2 = (entity_t**)b;

	return (int)roundf((*e1)->depth - (*e2)->depth); //mxd. Q_ftol() instead of roundf() in original logic.
}

static int alphaentitycmpfnc(const void* a, const void* b) // H2
{
	return entitycmpfnc(b, a); // Alpha surfaces are drawn back to front.
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
	
	CalcFov((float)scr_vrect.width, (float)scr_vrect.height);

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

	frame_index = re.RenderFrame(&cl.refdef); //TODO: re.RenderFrame() always returns 0!

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

#pragma region ========================== H2 SCREEN FLASH ==========================

void Activate_Screen_Flash(const int color)
{
	if ((int)Cvar_VariableValue("flash_screen"))
		screen_flash_color = color;
}

// Screen flash unset
void Deactivate_Screen_Flash(void)
{
	screen_flash_color = 0;
}

// Return screen flash value
int Is_Screen_Flashing(void)
{
	return screen_flash_color;
}

#pragma endregion

#pragma region ========================== H2 SCREEN SHAKE ==========================

void Activate_Screen_Shake(const float intensity, const float duration, const float current_time, const int flags)
{
	screen_shake_intensity_min = intensity;
	screen_shake_intensity_max = intensity;
	screen_shake_duration = duration;
	screen_shake_flags = flags;
	screen_shake_endtime = duration + current_time;
}

void Perform_Screen_Shake(vec3_t out, const float current_time)
{
	VectorClear(out);

	if (current_time > screen_shake_endtime)
	{
		screen_shake_intensity_max = 0.0f;
		return;
	}

	if (screen_shake_intensity_max == 0.0f)
		return;

	screen_shake_intensity_max = (screen_shake_endtime - current_time) / screen_shake_duration * screen_shake_intensity_min;

	if (screen_shake_flags & SHAKE_LATERAL)
		out[PITCH] = (float)irand((int)-screen_shake_intensity_max, (int)screen_shake_intensity_max);

	if (screen_shake_flags & SHAKE_VERTICAL)
		out[ROLL] = (float)irand((int)-screen_shake_intensity_max, (int)screen_shake_intensity_max);

	if (screen_shake_flags & SHAKE_DEPTH)
		out[YAW] = (float)irand((int)-screen_shake_intensity_max, (int)screen_shake_intensity_max);
}

void Reset_Screen_Shake(void)
{
	screen_shake_duration = 0.0f;
	screen_shake_intensity_max = 0.0f;
	screen_shake_endtime = -1.0f;
	screen_shake_flags = 0;
}

#pragma endregion
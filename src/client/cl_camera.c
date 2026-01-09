//
// cl_camera.c -- Third person camera update logic.
//
// Copyright 1998 Raven Software
//

#include "cl_camera.h"
#include "cl_effects.h"
#include "client.h"
#include "menu.h"
#include "Vector.h"

int camera_timer; // H2
qboolean offsetangles_changed; // H2

// Q2 counterpart
static void vectoangles2(const vec3_t value1, vec3_t angles)
{
	if (value1[0] != 0.0f || value1[1] != 0.0f)
	{
		float yaw = atan2f(value1[1], value1[0]) * RAD_TO_ANGLE;
		if (yaw < 0.0f)
			yaw += 360.0f;

		const float forward = sqrtf(value1[0] * value1[0] + value1[1] * value1[1]);
		float pitch = atan2f(value1[2], forward) * RAD_TO_ANGLE;
		if (pitch < 0.0f)
			pitch += 360.0f;

		VectorSet(angles, -pitch, yaw, 0.0f);
	}
	else if (value1[2] > 0.0f)
	{
		VectorSet(angles, -90.0f, 0.0f, 0.0f);
	}
	else
	{
		VectorSet(angles, -270.0f, 0.0f, 0.0f);
	}
}

static void CL_UpdateWallDistances(void) // H2
{
	static int eax_presets[7][4] = // [world preset][sound preset]
	{
		{ EAX_ENVIRONMENT_GENERIC,		EAX_ENVIRONMENT_ROOM,		EAX_ENVIRONMENT_HALLWAY,		EAX_ENVIRONMENT_ALLEY }, // EAX_GENERIC
		{ EAX_ENVIRONMENT_QUARRY,		EAX_ENVIRONMENT_CAVE,		EAX_ENVIRONMENT_STONECORRIDOR,	EAX_ENVIRONMENT_ALLEY }, // EAX_ALL_STONE
		{ EAX_ENVIRONMENT_ARENA,		EAX_ENVIRONMENT_LIVINGROOM,	EAX_ENVIRONMENT_HALLWAY,		EAX_ENVIRONMENT_ALLEY }, // EAX_ARENA
		{ EAX_ENVIRONMENT_CITY,			EAX_ENVIRONMENT_ROOM,		EAX_ENVIRONMENT_SEWERPIPE,		EAX_ENVIRONMENT_ALLEY }, // EAX_CITY_AND_SEWERS
		{ EAX_ENVIRONMENT_CITY,			EAX_ENVIRONMENT_STONEROOM,	EAX_ENVIRONMENT_STONECORRIDOR,	EAX_ENVIRONMENT_ALLEY }, // EAX_CITY_AND_ALLEYS
		{ EAX_ENVIRONMENT_FOREST,		EAX_ENVIRONMENT_ROOM,		EAX_ENVIRONMENT_HALLWAY,		EAX_ENVIRONMENT_ALLEY }, // EAX_FOREST
		{ EAX_ENVIRONMENT_PSYCHOTIC,	EAX_ENVIRONMENT_PSYCHOTIC,	EAX_ENVIRONMENT_PSYCHOTIC,		EAX_ENVIRONMENT_PSYCHOTIC }, // EAX_PSYCHOTIC
	};

	const vec3_t start = VEC3_INIT(PlayerEntPtr->origin);
	vec3_t end = VEC3_INIT(PlayerEntPtr->origin);
	
	switch (cl.wall_check)
	{
		case 0:
			end[2] += 800.0f;
			break;

		case 1:
			end[0] += 800.0f;
			end[2] += 100.0f;
			break;

		case 2:
			end[1] -= 800.0f;
			end[2] += 100.0f;
			break;

		case 3:
			end[1] += 800.0f;
			end[2] += 100.0f;
			break;

		case 4:
			end[0] -= 800.0f;
			end[2] += 100.0f;
			break;

		default:
			return;
	}

	trace_t trace;
	CL_Trace(start, NULL, NULL, end, MASK_PLAYERSOLID | CONTENTS_WORLD_ONLY, CONTENTS_DETAIL, &trace);
	Vec3SubtractAssign(start, trace.endpos);
	cl.wall_dist[cl.wall_check] = VectorLength(trace.endpos);

	int sound_preset;
	if (cl.wall_dist[1] + cl.wall_dist[3] < 250.0f || cl.wall_dist[2] + cl.wall_dist[4] < 250.0f)
		sound_preset = 1;
	else
		sound_preset = 0;

	if (cl.wall_dist[0] <= 790.0f)
		sound_preset += 2;

	const int world_preset = (int)EAX_default->value;

	Cvar_SetValue("EAX_preset", (float)eax_presets[world_preset][sound_preset]);
	cl.wall_check = (cl.wall_check + 1) % (int)ARRAY_SIZE(cl.wall_dist);
}

static void CL_UpdateCameraOrientation(const vec3_t look_angles, float viewheight, const qboolean interpolate, const qboolean noclip_mode) // H2 //mxd. Add 'look_angles' arg, flip 'interpolate' arg logic, add 'noclip_mode' arg.
{
#define MAX_CAMERA_TIMER	500
#define MASK_CAMERA			(CONTENTS_SOLID | CONTENTS_ILLUSIONARY | CONTENTS_CAMERABLOCK)

	typedef enum
	{
		CM_DEFAULT,			// When on land.
		CM_DIVE,			// When swimming underwater.
		CM_SWIM,			// When swimming on water surface.
		CM_LIQUID_DEATH,	// When died in lava/slime (but not in water).
	} cam_mode_e;

	static cam_mode_e cam_mode;
	static qboolean cam_timer_reset;
	static vec3_t old_vieworg;
	static vec3_t old_viewangles;
	static vec3_t prev_start;
	static vec3_t prev_prev_start;
	static vec3_t prev_end;
	static vec3_t prev_prev_end;

	if (cls.state != ca_active)
		return;

	const vec3_t mins = { -1.0f, -1.0f, -1.0f };
	const vec3_t maxs = {  1.0f,  1.0f,  1.0f };
	const vec3_t mins_2 = { -3.0f, -3.0f, -3.0f };
	const vec3_t maxs_2 = {  3.0f,  3.0f,  3.0f };

	const int water_flags = ((int)cl_predict->value ? cl.playerinfo.pm_w_flags : cl.frame.playerstate.pmove.w_flags);
	const int waterlevel = ((int)cl_predict->value ? cl.playerinfo.waterlevel : cl.frame.playerstate.waterlevel);

	if ((int)cl_camera_fpmode->value)
	{
		viewheight += cl_camera_fpheight->value;
		cam_transparency = cl_camera_fptrans->value;
	}
	else
	{
		viewheight += cl_camera_fpoffs->value + 16.0f;
		cam_transparency = cl_playertrans->value;
	}

	vec3_t forward;
	vec3_t up;
	AngleVectors(look_angles, forward, NULL, up);

	const cam_mode_e prev_cam_mode = cam_mode;

	vec3_t start;
	vec3_t end;
	VectorCopy(PlayerEntPtr->origin, start);

	if (water_flags != 0)
	{
		if (water_flags & (WF_SURFACE | WF_DIVE | WF_DIVING))
		{
			VectorCopy(start, end);

			start[2] += 100.0f;
			end[2] -= 100.0f;

			trace_t tr;
			CL_Trace(start, mins, maxs, end, MASK_WATER | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &tr);

			if (tr.fraction < 1.0f)
			{
				start[2] = tr.endpos[2];
				VectorMA(start, viewheight - 9.5f, up, end);

				cam_mode = CM_SWIM;
			}
		}
		else // WF_SWIMFREE | WF_SINK
		{
			VectorMA(start, viewheight, forward, end);
			cam_mode = CM_DIVE;
		}
	}
	else
	{
		VectorMA(start, viewheight, up, end);

		trace_t tr;
		CL_Trace(start, mins_2, maxs_2, end, MASK_CAMERA, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &tr);

		if (!noclip_mode && tr.fraction != 1.0f)
			VectorCopy(tr.endpos, end);

		end[2] -= 4.0f;

		if (!noclip_mode && (CL_PMpointcontents(end) & MASK_WATER))
		{
			vec3_t tmp_end;
			VectorCopy(start, tmp_end);

			start[2] += 100.0f;
			tmp_end[2] -= 100.0f;

			CL_Trace(start, mins, maxs, tmp_end, MASK_WATER | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &tr);

			if (tr.fraction != 1.0f)
			{
				start[2] = tr.endpos[2] * 2.0f - end[2] + 4.0f;
				VectorMA(start, viewheight, up, end);

				cam_mode = CM_LIQUID_DEATH;
			}
		}
		else
		{
			VectorMA(start, viewheight, up, end);
			cam_mode = CM_DEFAULT;
		}
	}

	// Interpolate position when switching camera mode.
	if (prev_cam_mode != cam_mode)
	{
		if (!cam_timer_reset)
		{
			camera_timer = 0;
			cam_timer_reset = true;
		}

		VectorCopy(prev_start, prev_prev_start);
		VectorCopy(prev_end, prev_prev_end);
	}

	if (cam_timer_reset)
	{
		if (camera_timer >= MAX_CAMERA_TIMER)
		{
			cam_timer_reset = false;
		}
		else
		{
			VectorCopy(start, prev_start);
			VectorCopy(end, prev_end);

			const float lerp = (float)camera_timer / (float)MAX_CAMERA_TIMER;
			VectorLerp(prev_prev_start, lerp, prev_start, start);
			VectorLerp(prev_prev_end, lerp, prev_end, end);
		}
	}
	else
	{
		VectorCopy(start, prev_start);
		VectorCopy(end, prev_end);
	}

	trace_t trace;
	CL_Trace(start, mins_2, maxs_2, end, MASK_CAMERA, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

	if (!noclip_mode && trace.fraction != 1.0f)
		VectorCopy(trace.endpos, end);

	float viewdist;
	if ((int)cl_camera_fpmode->value)
		viewdist = -cl_camera_fpdist->value;
	else
		viewdist = -cl_camera_viewdist->value;

	vec3_t end_2;
	VectorMA(end, viewdist, forward, end_2);

	if ((water_flags & WF_SWIMFREE) && (CL_PMpointcontents(end) & MASK_WATER))
	{
		CL_Trace(end_2, mins, maxs, end, MASK_WATER | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

		if (!trace.startsolid && trace.fraction != 1.0f)
			VectorLerp(end, 0.9f, trace.endpos, end_2);
	}

	vec3_t end_3;
	if (!(int)cl_camera_clipdamp->value)
		VectorCopy(end_2, end_3);

	// Interpolate camera position when desired, not in fpmode and cl_camera_dampfactor vaues are sane.
	if (interpolate && !(int)cl_camera_fpmode->value && cl_camera_dampfactor->value > 0.0f && cl_camera_dampfactor->value < 1.0f)
	{
		float damp_factor = fabsf(look_angles[PITCH]);
		damp_factor = min(1.0f, damp_factor / 89.0f);
		damp_factor = (1.0f - cl_camera_dampfactor->value) * damp_factor * damp_factor * damp_factor + cl_camera_dampfactor->value;

		VectorLerp(old_vieworg, damp_factor, end_2, end_2);
	}

	// Check against world --mxd.
	CL_Trace(end, mins, maxs, end_2, MASK_CAMERA, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

	if (!noclip_mode && trace.fraction != 1.0f)
	{
		if ((int)cl_camera_clipdamp->value)
		{
			VectorCopy(trace.endpos, end_2);
		}
		else
		{
			VectorCopy(end_3, end_2);

			CL_Trace(end, mins, maxs, end_2, MASK_CAMERA, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

			if (trace.fraction != 1.0f)
				VectorCopy(trace.endpos, end_2);
		}
	}

	if (waterlevel == 0 || (water_flags & ~WF_SWIMFREE) || (water_flags == 0 && in_down.state == KS_NONE))
	{
		const float roll_scaler = 1.0f - fabsf(look_angles[PITCH] / 89.0f);
		const vec3_t v = { mins[0], mins[1], -1.0f - roll_scaler * 2.0f };

		// Check against bmodels / solid entities --mxd.
		CL_Trace(end, v, maxs, end_2, MASK_WATER | CONTENTS_CAMERABLOCK, CONTENTS_DETAIL | CONTENTS_TRANSLUCENT, &trace);

		if (!noclip_mode && trace.fraction != 1.0f)
			VectorCopy(trace.endpos, end_2);
	}

	vec3_t diff;
	VectorSubtract(end, end_2, diff);

	if (cl_camera_viewmax->value < VectorLength(diff))
	{
		VectorNormalize(diff);
		VectorMA(end, -cl_camera_viewmax->value, diff, end_2);
	}

	// Copy calculated angles and vieworg to refdef.
	vec3_t viewangles;
	VectorSubtract(end, end_2, viewangles);
	VectorNormalize(viewangles);

	vectoangles2(viewangles, cl.refdef.viewangles);
	VectorCopy(end_2, cl.refdef.vieworg);

	if ((int)cl_camera_freeze->value)
	{
		VectorCopy(old_viewangles, cl.refdef.viewangles);
		VectorCopy(old_vieworg, cl.refdef.vieworg);
	}
	else
	{
		VectorCopy(cl.refdef.viewangles, old_viewangles);
		VectorCopy(cl.refdef.vieworg, old_vieworg);
	}

	//mxd. Skip Perform_Screen_Shake() logic. Already done in CL_CalcViewValues().

	VectorCopy(PlayerEntPtr->origin, cl.refdef.clientmodelorg);
}

// Sets cl.refdef view values.
void CL_CalcViewValues(void)
{
	const float lerp = cl.lerpfrac;
	const player_state_t* ps = &cl.frame.playerstate;

	const int frame = (cl.frame.serverframe - 1) & UPDATE_MASK;
	frame_t* oldframe = &cl.frames[frame];
	qboolean player_teleported = false;

	if (oldframe->serverframe != cl.frame.serverframe - 1 || !oldframe->valid)
		oldframe = &cl.frame; // Previous frame was dropped or invalid.

	// See if the player entity was teleported this frame. //mxd. Original H2 logic doesn't seem to do abs() here. A bug? //TODO: value doesn't match MIN_TELEPORT_DISTANCE used in cl_prediction.c...
	if (abs(oldframe->playerstate.pmove.origin[0] - ps->pmove.origin[0]) > 256 * 8 ||
		abs(oldframe->playerstate.pmove.origin[1] - ps->pmove.origin[1]) > 256 * 8 ||
		abs(oldframe->playerstate.pmove.origin[2] - ps->pmove.origin[2]) > 256 * 8)
	{
		// Don't interpolate.
		oldframe = &cl.frame;
		player_teleported = true;
	}

	player_state_t* ops = &oldframe->playerstate;

	// Calculate the origin.
	if (ps->remote_id < 0 && ps->pmove.pm_type != PM_INTERMISSION)
	{
		if ((int)cl_predict->value)
		{
			VectorCopy(cl.predicted_origin, PlayerEntPtr->origin);
		}
		else
		{
			for (int c = 0; c < 3; c++)
				PlayerEntPtr->origin[c] = ((float)ops->pmove.origin[c] + (float)(ps->pmove.origin[c] - ops->pmove.origin[c]) * lerp) * 0.125f;
		}

		VectorCopy(PlayerEntPtr->origin, PlayerEntPtr->oldorigin);
	}

	if (offsetangles_changed)
	{
		static vec3_t old_offsetangles;
		vec3_t offsetangles;

		if ((int)cl_predict->value)
			VectorSubtract(cl.playerinfo.offsetangles, ps->offsetangles, offsetangles);
		else
			VectorSubtract(ps->offsetangles, ops->offsetangles, offsetangles);

		for (int i = 0; i < 3; i++)
		{
			if (offsetangles[i] != old_offsetangles[i])
			{
				cl.inputangles[i] += offsetangles[i];
				cl.viewangles[i] += offsetangles[i];
			}
		}

		VectorCopy(offsetangles, old_offsetangles);
		offsetangles_changed = false;
	}

	if (ps->pmove.pm_type == PM_INTERMISSION)
	{
		for (int i = 0; i < 3; i++)
		{
			cl.refdef.vieworg[i] = (float)ps->pmove.origin[i] * 0.125f;
			cl.refdef.viewangles[i] = ps->viewangles[i];
		}
	}
	else
	{
		vec3_t look_angles; //mxd. Made local.

		if (ps->pmove.pm_type == PM_FREEZE)
		{
			for (int i = 0; i < 3; i++)
				cl.predicted_angles[i] = LerpAngle(ops->viewangles[i], ps->viewangles[i], lerp);

			VectorCopy(cl.predicted_angles, look_angles);
		}
		else if ((in_lookaround.state & KS_DOWN))
		{
			for (int i = 0; i < 3; i++)
				look_angles[i] = cl.lookangles[i] + (float)ps->pmove.delta_angles[i] * SHORT_TO_ANGLE;
		}
		else
		{
			VectorCopy(cl.predicted_angles, look_angles);
		}

		if (ps->remote_id < 0) // When not looking through a remote camera.
		{
			const float viewheight = (float)ops->viewheight + (float)(ps->viewheight - ops->viewheight) * lerp;
			const qboolean noclip_mode = (ps->pmove.pm_type == PM_SPECTATOR); //mxd

			if (player_teleported)
			{
				CL_UpdateCameraOrientation(look_angles, viewheight, false, noclip_mode);
			}
			else
			{
				static float frame_delta;

				frame_delta += cls.rframetime * vid_maxfps->value; //mxd. cls.frametime * cl_maxfps->value in original logic.
				const int num_frames = (int)frame_delta;

				for (int i = 0; i < num_frames; i++)
					CL_UpdateCameraOrientation(look_angles, viewheight, true, noclip_mode);

				frame_delta -= (float)num_frames;
			}
		}
		else
		{
			if (ps->remote_id != ops->remote_id)
			{
				ops->remote_id = ps->remote_id;

				VectorCopy(ps->remote_vieworigin, ops->remote_vieworigin);
				VectorCopy(ps->remote_viewangles, ops->remote_viewangles);
			}

			// Just use interpolated values.
			for (int i = 0; i < 3; i++)
			{
				cl.refdef.vieworg[i] = (ops->remote_vieworigin[i] + (ps->remote_vieworigin[i] - ops->remote_vieworigin[i]) * lerp) * 0.125f;
				cl.refdef.viewangles[i] = LerpAngle(ops->remote_viewangles[i], ps->remote_viewangles[i], lerp);
			}
		}
	}

	AngleVectors(cl.refdef.viewangles, cl.v_forward, cl.v_right, cl.v_up);

	VectorCopy(cl.refdef.viewangles, cl.camera_viewangles);
	VectorCopy(cl.refdef.vieworg, cl.camera_vieworigin);

	// H2: Update EAX preset.
	if (CL_PMpointcontents(cl.camera_vieworigin) & CONTENTS_WATER)
	{
		if (!(int)cl_camera_under_surface->value && !(int)menus_active->value && se.SetEaxEnvironment != NULL)
		{
			Cvar_SetValue("EAX_preset", EAX_ENVIRONMENT_UNDERWATER); // H2_1.07: 22 -> 44.
			se.SetEaxEnvironment(EAX_ENVIRONMENT_UNDERWATER);
		}

		cl_camera_under_surface->value = 1.0f;
	}
	else
	{
		if (!(int)menus_active->value && se.SetEaxEnvironment != NULL)
		{
			CL_UpdateWallDistances();
			se.SetEaxEnvironment((int)EAX_preset->value);
		}

		cl_camera_under_surface->value = 0.0f;
	}

	// H2: Apply screen shake.
	vec3_t shake_amount;
	Perform_Screen_Shake(shake_amount, (float)cl.time);
	Vec3AddAssign(shake_amount, cl.refdef.vieworg);

	// Interpolate field of view.
	cl.refdef.fov_x = ops->fov + (ps->fov - ops->fov) * lerp;
}
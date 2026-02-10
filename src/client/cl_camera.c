//
// cl_camera.c -- Third person camera update logic.
//
// Copyright 1998 Raven Software
//

#include "cl_camera.h"
#include "cl_effects.h"
#include "client.h"
#include "EffectFlags.h"
#include "menu.h"
#include "Vector.h"

#define PLAYER_MIN_TELEPORT_DISTANCE	256.0f //mxd

int camera_timer; // H2
qboolean offsetangles_changed; // H2

typedef enum
{
	CMODE_NONE,			//mxd. Initial state.
	CMODE_DEFAULT,		// When on land.
	CMODE_DIVE,			// When swimming underwater.
	CMODE_SWIM,			// When swimming on water surface.
	CMODE_LIQUID_DEATH,	// When died in lava/slime (but not in water).
} cam_mode_e;

static cam_mode_e cam_mode;

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
	CL_Trace(start, NULL, NULL, end, MASK_PLAYERSOLID | CONTENTS_WORLD_ONLY, CTF_CLIP_TO_WORLD, &trace);
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

void CL_ResetCamera(void) //mxd
{
	cam_mode = CMODE_NONE;
}

static void CL_InterpolateCameraViewAngles(const vec3_t look_angles, const vec3_t old_viewangles, vec3_t viewangles) //mxd
{
	// Lerp direction vectors instead of angles to avoid angle wrapping issues...
	vec3_t forward;
	AngleVectors(look_angles, forward, NULL, NULL);

	vec3_t old_forward;
	AngleVectors(old_viewangles, old_forward, NULL, NULL);

	vec3_t lerped_dir;
	VectorLerp(old_forward, 0.5f, forward, lerped_dir);

	vectoangles2(lerped_dir, viewangles);
}

//mxd. Interpolate camera origin.
static void CL_InterpolateCameraOrigin(vec3_t cam_lerp_origin, const float yaw, vec3_t cam_origin) //mxd
{
#define CAM_MAX_LERP_DISTANCE	64.0f

	if (cl_camera_position_lerp->modified)
	{
		cl_camera_position_lerp->value = Clamp(cl_camera_position_lerp->value, 0.0f, 0.9f);
		cl_camera_position_lerp->modified = false;
	}

	vec3_t cam_lerp_dir;
	VectorSubtract(cam_lerp_origin, PlayerEntPtr->origin, cam_lerp_dir);
	float lerp_dist = VectorNormalize(cam_lerp_dir);

	if (lerp_dist > 0.0f)
	{
		lerp_dist = min(CAM_MAX_LERP_DISTANCE, lerp_dist);
		const vec3_t cam_angles = VEC3_SET(0.0f, yaw, 0.0f);

		vec3_t cam_forward;
		vec3_t cam_right;
		AngleVectors(cam_angles, cam_forward, cam_right, NULL);

		VectorMA(PlayerEntPtr->origin, lerp_dist * DotProduct(cam_forward, cam_lerp_dir), cam_forward, cam_origin);
		VectorMA(cam_origin, lerp_dist * 0.5f * DotProduct(cam_right, cam_lerp_dir), cam_right, cam_origin);

		// Z-axis requires special handling (mainly because we want smoother camera movement on z-axis when walking up the stairs)...
		const float frac_z = ((cam_lerp_origin[2] < PlayerEntPtr->origin[2]) ? 0.1f : 0.75f * cl_camera_position_lerp->value);
		cam_origin[2] = LerpFloat(cam_lerp_origin[2], PlayerEntPtr->origin[2], frac_z);

		VectorLerp(cam_lerp_origin, 1.0f - cl_camera_position_lerp->value, PlayerEntPtr->origin, cam_lerp_origin);
	}
	else
	{
		VectorCopy(PlayerEntPtr->origin, cam_origin);
	}
}

static void CL_UpdateCameraOrientation(const vec3_t look_angles, float viewheight, const qboolean interpolate, const qboolean noclip_mode) // H2 //mxd. Add 'look_angles' arg, flip 'interpolate' arg logic, add 'noclip_mode' arg.
{
#define MAX_CAMERA_TIMER	500
#define MASK_CAMERA			(CONTENTS_SOLID | CONTENTS_ILLUSIONARY | CONTENTS_CAMERABLOCK)

	static const vec3_t mins = { -1.0f, -1.0f, -1.0f };
	static const vec3_t maxs = {  1.0f,  1.0f,  1.0f };
	static const vec3_t mins_2 = { -3.0f, -3.0f, -3.0f };
	static const vec3_t maxs_2 = {  3.0f,  3.0f,  3.0f };

	static qboolean cam_timer_reset;
	static vec3_t old_vieworg;
	static vec3_t old_viewangles;
	static vec3_t prev_start;
	static vec3_t prev_prev_start;
	static vec3_t prev_end;
	static vec3_t prev_prev_end;
	static vec3_t cam_lerp_origin; //mxd

	if (cls.state != ca_active)
		return;

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

	vec3_t viewangles;

	//mxd. Interpolate camera viewangles (to smooth sudden camera angle changes when camera is re-oriented during lockmove/PLAYER_FLAG_TURNLOCK player animations).
	if (interpolate)
		CL_InterpolateCameraViewAngles(look_angles, old_viewangles, viewangles);
	else
		VectorCopy(look_angles, viewangles);

	vec3_t forward;
	vec3_t up;
	AngleVectors(viewangles, forward, NULL, up);

	const cam_mode_e prev_cam_mode = cam_mode;

	if (cam_mode == CMODE_NONE || !interpolate) //mxd
		VectorCopy(PlayerEntPtr->origin, cam_lerp_origin);

	if (cam_mode == CMODE_NONE) //mxd
		cam_mode = CMODE_DEFAULT;

	vec3_t start;
	vec3_t end;

	//mxd. Interpolate camera origin.
	if (interpolate)
		CL_InterpolateCameraOrigin(cam_lerp_origin, look_angles[YAW], start);
	else
		VectorCopy(PlayerEntPtr->origin, start);

	if (water_flags != 0)
	{
		if (water_flags & (WF_SURFACE | WF_DIVE | WF_DIVING))
		{
			VectorCopy(start, end);

			start[2] += 100.0f;
			end[2] -= 100.0f;

			trace_t tr;
			CL_Trace(start, mins, maxs, end, MASK_WATER | CONTENTS_CAMERABLOCK, CTF_CLIP_TO_ALL, &tr);

			if (tr.fraction < 1.0f)
			{
				start[2] = tr.endpos[2];
				VectorMA(start, viewheight - 9.5f, up, end);

				cam_mode = CMODE_SWIM;
			}
		}
		else // WF_SWIMFREE | WF_SINK
		{
			VectorMA(start, viewheight, forward, end);
			cam_mode = CMODE_DIVE;
		}
	}
	else
	{
		VectorMA(start, viewheight, up, end);

		trace_t tr;
		CL_Trace(start, mins_2, maxs_2, end, MASK_CAMERA, CTF_CLIP_TO_ALL, &tr);

		if (!noclip_mode && tr.fraction != 1.0f)
			VectorCopy(tr.endpos, end);

		end[2] -= 4.0f;

		if (!noclip_mode && (CL_PMpointcontents(end) & MASK_WATER))
		{
			vec3_t tmp_end = VEC3_INIT(start);

			start[2] += 100.0f;
			tmp_end[2] -= 100.0f;

			CL_Trace(start, mins, maxs, tmp_end, MASK_WATER | CONTENTS_CAMERABLOCK, CTF_CLIP_TO_ALL, &tr);

			if (tr.fraction != 1.0f)
			{
				start[2] = tr.endpos[2] * 2.0f - end[2] + 4.0f;
				VectorMA(start, viewheight, up, end);

				cam_mode = CMODE_LIQUID_DEATH;
			}
		}
		else
		{
			VectorMA(start, viewheight, up, end);
			cam_mode = CMODE_DEFAULT;
		}
	}

	// Interpolate position when switching camera mode.
	//H2_BUGFIX: mxd. Don't interpolate when switching from CM_NONE (state not present in original logic)
	// - disables camera interpolation when camera mode was switched because of map switching / game loading.
	if (prev_cam_mode != cam_mode && prev_cam_mode != CMODE_NONE)
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
	CL_Trace(start, mins_2, maxs_2, end, MASK_CAMERA, CTF_CLIP_TO_ALL, &trace);

	if (!noclip_mode && trace.fraction != 1.0f)
		VectorCopy(trace.endpos, end);

	vec3_t end_2;
	const float viewdist = (((int)cl_camera_fpmode->value) ? cl_camera_fpdist->value : cl_camera_viewdist->value);
	VectorMA(end, -viewdist, forward, end_2);

	// If player is in "swim underwater" mode, block camera by water surface.
	if ((water_flags & WF_SWIMFREE) && (CL_PMpointcontents(end) & MASK_WATER))
	{
		//mxd. We are potentially tracing from air into water contents (to avoid startsolid trace), so use mins/maxs shifted opposite to view direction (so trace.endpos ends up inside water brush).
		const vec3_t uw_mins = VEC3_INITS(forward, -8.0f);
		const vec3_t uw_maxs = VEC3_INITS(forward, -4.0f);
		CL_Trace(end_2, uw_mins, uw_maxs, end, MASK_WATER | CONTENTS_CAMERABLOCK, CTF_CLIP_TO_ALL, &trace);

		// When both 'end' and 'end_2' are underwater, we end up with startsolid/allsolid/fraction:0 trace --mxd.
		if (!trace.startsolid && trace.fraction < 1.0f)
			VectorCopy(trace.endpos, end_2); //mxd. H2: VectorLerp(end, 0.9f, trace.endpos, end_2). Removed to avoid noticeable camera position change when hitting water surface.
	}

	vec3_t end_3;
	if (!(int)cl_camera_clipdamp->value)
		VectorCopy(end_2, end_3);

	// Interpolate camera position when desired, not in fpmode and cl_camera_dampfactor vaues are sane.
	if (interpolate && !(int)cl_camera_fpmode->value && cl_camera_dampfactor->value > 0.0f && cl_camera_dampfactor->value < 1.0f)
	{
		float damp_factor = fabsf(viewangles[PITCH]);
		damp_factor = min(1.0f, damp_factor / 89.0f);
		damp_factor = (1.0f - cl_camera_dampfactor->value) * damp_factor * damp_factor * damp_factor + cl_camera_dampfactor->value;

		VectorLerp(old_vieworg, damp_factor, end_2, end_2);
	}

	// Check against world --mxd.
	CL_Trace(end, mins, maxs, end_2, MASK_CAMERA, CTF_CLIP_TO_ALL, &trace);

	if (!noclip_mode && trace.fraction != 1.0f)
	{
		if ((int)cl_camera_clipdamp->value)
		{
			VectorCopy(trace.endpos, end_2);
		}
		else
		{
			VectorCopy(end_3, end_2);

			CL_Trace(end, mins, maxs, end_2, MASK_CAMERA, CTF_CLIP_TO_ALL, &trace);

			if (trace.fraction != 1.0f)
				VectorCopy(trace.endpos, end_2);
		}
	}

	//TODO: check if 'water_flags & ~WF_SWIMFREE' (e.g. true when water_flags has any flags other than WF_SWIMFREE) check is correct.
	if (waterlevel == 0 || (water_flags & ~WF_SWIMFREE) || (water_flags == 0 && in_down.state == KS_NONE))
	{
		const float roll_scaler = 1.0f - fabsf(viewangles[PITCH] / 89.0f);
		const vec3_t v = VEC3_SET(mins[0], mins[1], -1.0f - roll_scaler * 2.0f);

		// Check against bmodels / solid entities --mxd.
		CL_Trace(end, v, maxs, end_2, MASK_WATER | CONTENTS_CAMERABLOCK, CTF_CLIP_TO_ALL, &trace);

		if (!noclip_mode && !trace.startsolid && trace.fraction != 1.0f) //mxd. Added trace.startsolid check (because now startsolid trace has fraction 0 instead of 1 in original logic).
			VectorCopy(trace.endpos, end_2);
	}

	vec3_t view_dir;
	VectorSubtract(end, end_2, view_dir);
	const float view_dist = VectorNormalize(view_dir);

	// Clamp to maximum camera distance?
	if (cl_camera_viewmax->value < view_dist)
		VectorMA(end, -cl_camera_viewmax->value, view_dir, end_2);

	// Copy calculated angles and vieworg to refdef.
	vectoangles2(view_dir, cl.refdef.viewangles);
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
	if (abs(oldframe->playerstate.pmove.origin[0] - ps->pmove.origin[0]) > POS2SHORT(PLAYER_MIN_TELEPORT_DISTANCE) ||
		abs(oldframe->playerstate.pmove.origin[1] - ps->pmove.origin[1]) > POS2SHORT(PLAYER_MIN_TELEPORT_DISTANCE) ||
		abs(oldframe->playerstate.pmove.origin[2] - ps->pmove.origin[2]) > POS2SHORT(PLAYER_MIN_TELEPORT_DISTANCE))
	{
		// Don't interpolate.
		oldframe = &cl.frame;
		player_teleported = true;
	}

	player_state_t* ops = &oldframe->playerstate;

	// Calculate the origin.
	if (ps->remote_id == REMOTE_ID_NONE && ps->pmove.pm_type != PM_INTERMISSION) //mxd. Use REMOTE_ID_NONE define.
	{
		if ((int)cl_predict->value)
		{
			VectorCopy(cl.predicted_origin, PlayerEntPtr->origin);
		}
		else
		{
			for (int i = 0; i < 3; i++)
				PlayerEntPtr->origin[i] = ((float)ops->pmove.origin[i] + (float)(ps->pmove.origin[i] - ops->pmove.origin[i]) * lerp) * 0.125f;
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
			cl.refdef.vieworg[i] = SHORT2POS(ps->pmove.origin[i]); //mxd. Use define.
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
				look_angles[i] = cl.lookangles[i] + SHORT2ANGLE(ps->pmove.delta_angles[i]);
		}
		else
		{
			VectorCopy(cl.predicted_angles, look_angles);
		}

		if (ps->remote_id == REMOTE_ID_NONE) // When not looking through a remote camera.
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
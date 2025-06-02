//
// cl_prediction.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cmodel.h"
#include "EffectFlags.h"
#include "p_main.h"
#include "Vector.h"

int pred_pm_flags;
int pred_pm_w_flags;
qboolean trace_ignore_player;
qboolean trace_ignore_camera;

static int pred_effects = 0;
static int pred_clientnum = 0;
static int pred_renderfx = 0;
static int pred_skinnum = 0;
static float pred_playerLerp = 0.0f;

static int pred_currFrame = 0;
static int pred_prevFrame = 0;

static int pred_currSwapFrame = 0;
static int pred_prevSwapFrame = 0;

static vec3_t pred_currAngles = { 0 };
static vec3_t pred_prevAngles = { 0 };

void CL_CheckPredictionError(void)
{
	int delta[3];

	if ((int)cl_predict->value == 0) // H2: no PMF_NO_PREDICTION check.
		return;

	// Calculate the last usercmd_t we sent that the server has processed.
	const int frame = cls.netchan.incoming_acknowledged & (CMD_BACKUP - 1);

	if (cl.frame.playerstate.frame != pred_currFrame) // H2
		pred_prevFrame = pred_currFrame;

	if (cl.frame.playerstate.swapFrame != pred_currSwapFrame) // H2
		pred_prevSwapFrame = pred_currSwapFrame;

	// Compare what the server returned with what we had predicted it to be.
	for (int i = 0; i < 3; i++)
		delta[i] = cl.frame.playerstate.pmove.origin[i] - cl.predicted_origins[frame][i];

	// Save the prediction error for interpolation.
	const int len = abs(delta[0]) + abs(delta[1]) + abs(delta[2]);
	if (len > 640) // 80 world units
	{
		// A teleport or something.
		VectorClear(cl.prediction_error);
	}
	else
	{
		if ((int)cl_showmiss->value && (delta[0] != 0 || delta[1] != 0 || delta[2] != 0))
			Com_Printf("prediction miss on %i: %i\n", cl.frame.serverframe, delta[0] + delta[1] + delta[2]);

		// Save for error interpolation.
		for (int i = 0; i < 3; i++)
			cl.prediction_error[i] = (float)delta[i] * 0.125f;
	}
}

void CL_ClipMoveToEntities(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t* tr)
{
	int headnode;
	const float* angles;
	vec3_t mb_mins;
	vec3_t mb_maxs;
	
	if (mins == NULL) // H2
		mins = vec3_origin;

	if (maxs == NULL) // H2
		maxs = vec3_origin;

	// H2. Setup movebox.
	for (int i = 0; i < 3; i++)
	{
		mb_mins[i] = mins[i] + min(start[i], end[i]) - 1.0f;
		mb_maxs[i] = maxs[i] + max(start[i], end[i]) + 1.0f;
	}

	for (int i = 0; i < cl.frame.num_entities; i++)
	{
		const int num = (cl.frame.parse_entities + i) & (MAX_PARSE_ENTITIES - 1);
		entity_state_t* ent = &cl_parse_entities[num];

		if (!ent->solid)
			continue;

		if (trace_ignore_player && (ent->number == cl.playernum + 1)) // H2: extra trace_ignore_player check.
			continue;

		if (trace_ignore_camera && (ent->effects & EF_CAMERA_NO_CLIP)) // H2
			continue;

		if (ent->solid == 31)
		{
			// Special value for bmodel.
			const cmodel_t* cmodel = cl.model_clip[ent->modelindex];
			if (cmodel == NULL)
				continue;

			// H2: check if inside move box...
			float max_size = 0.0f;
			for (int c = 0; c < 3; c++)
				max_size = max(cmodel->maxs[c] - cmodel->mins[c], max_size);

			max_size *= 1.75f;

			qboolean in_movebox = true;
			for (int c = 0; c < 3; c++)
			{
				if (cmodel->mins[c] + ent->origin[c] - max_size > mb_maxs[c] || cmodel->maxs[c] + ent->origin[c] + max_size < mb_mins[c])
				{
					in_movebox = false;
					break;
				}
			}

			if (!in_movebox)
				continue;

			headnode = cmodel->headnode;
			angles = ent->angles;
		}
		else
		{
			// Encoded bbox.
			const float x =  8.0f * (float)(ent->solid & 31);
			const float zd = 8.0f * (float)((ent->solid >> 5) & 31);
			const float zu = 8.0f * (float)((ent->solid >> 10) & 63) - 32;

			const vec3_t bmins = { -x, -x, -zd };
			const vec3_t bmaxs = {  x,  x,  zu };

			// H2: check if inside move box...
			qboolean in_movebox = true;
			for (int c = 0; c < 3; c++)
			{
				if (ent->origin[c] + bmins[c] > mb_maxs[c] || ent->origin[c] + bmaxs[c] < mb_mins[c])
				{
					in_movebox = false;
					break;
				}
			}
			
			if (!in_movebox)
				continue;

			headnode = CM_HeadnodeForBox(bmins, bmaxs);
			angles = vec3_origin; // Boxes don't rotate.
		}

		int brushmask = MASK_PLAYERSOLID;
		if (trace_check_water)
			brushmask |= CONTENTS_WATER;

		trace_t trace;
		CM_TransformedBoxTrace(start, end, mins, maxs, headnode, brushmask, ent->origin, angles, &trace);

		if (trace.fraction <= tr->fraction)
		{
			*tr = trace;
			tr->ent = (struct edict_s*)ent;
		}
	}
}

//mxd. Sets tr->ent to -1 when the world was hit.
static void CL_PMTrace(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t* tr)
{
	// Check against world.
	if (mins == NULL && maxs == NULL)
	{
		mins = vec3_origin;
		maxs = vec3_origin;

		CM_BoxTrace(start, end, mins, maxs, 0, MASK_PLAYERSOLID | MASK_WATER, tr);
		trace_check_water = true;
	}
	else
	{
		if (mins == NULL)
			mins = vec3_origin;

		if (maxs == NULL)
			maxs = vec3_origin;

		CM_BoxTrace(start, end, mins, maxs, 0, MASK_PLAYERSOLID, tr);
		trace_check_water = false;
	}

	if (tr->fraction < 1.0f)
		tr->ent = (struct edict_s*)(-1); // Hit the world. // Q2: 1

	if (!tr->startsolid && !tr->allsolid)
	{
		// Check all other solid models.
		trace_ignore_player = true;
		CL_ClipMoveToEntities(start, mins, maxs, end, tr);
		trace_ignore_player = false;

		trace_check_water = false;
	}
}

// Q2 counterpart
int CL_PMpointcontents(const vec3_t point)
{
	int contents = CM_PointContents(point, 0);

	for (int i = 0; i < cl.frame.num_entities; i++)
	{
		const int num = (cl.frame.parse_entities + i) & (MAX_PARSE_ENTITIES - 1);
		const entity_state_t* ent = &cl_parse_entities[num];

		if (ent->solid != 31) // Special value for bmodel.
			continue;

		const cmodel_t* cmodel = cl.model_clip[ent->modelindex];

		if (cmodel != NULL)
			contents |= CM_TransformedPointContents(point, cmodel->headnode, ent->origin, ent->angles);
	}

	return contents;
}

// Sets cl.predicted_origin and cl.predicted_angles.
void CL_PredictMovement(void) //mxd. Surprisingly, NOT the biggest H2 function...
{
	static short old_cmd_angles[3] = { 0, 0, 0 };
	static csurface_t ground_surf; //mxd. Made static.

	paceldata_t* player_anim;
	short tmp_origin[3];
	short tmp_vel[3];
	vec3_t mins;
	vec3_t maxs;
	vec3_t origin;
	pmove_t pm;

	if ((int)cl_paused->value || (int)cl_freezeworld->value || cl.cinematictime > 0 || cl.frame.playerstate.cinematicfreeze)
		return;

	// Set angles.
	for (int i = 0; i < 3; i++)
		cl.predicted_angles[i] = cl.viewangles[i] + SHORT2ANGLE(cl.frame.playerstate.pmove.delta_angles[i]);

	if (cl_predict->value == 0.0f || cls.state != ca_active)
		return;

	int ack = cls.netchan.incoming_acknowledged;
	const int current = cls.netchan.outgoing_sequence;

	// If we are too far out of date, just freeze.
	if (current - ack >= CMD_BACKUP)
	{
		if ((int)cl_showmiss->value)
			Com_Printf("Exceeded CMD_BACKUP!\n");

		return;
	}

	// Copy current state to pmove.
	memset(&pm, 0, sizeof(pm));
	pm.trace = CL_PMTrace;
	pm.pointcontents = CL_PMpointcontents;
	pm.s = cl.frame.playerstate.pmove;
	pm.origin = origin; // H2

	for (int i = 0; i < 3; i++)
	{
		cl.playerinfo.origin[i] = (float)pm.s.origin[i] / 8.0f;
		cl.playerinfo.velocity[i] = (float)pm.s.velocity[i] / 8.0f;
	}

	VectorCopy(cl.frame.playerstate.mins, pm.mins);
	VectorCopy(cl.frame.playerstate.maxs, pm.maxs);

	VectorCopy(cl.frame.playerstate.mins, mins);
	pm.intentMins = mins;

	VectorCopy(cl.frame.playerstate.maxs, maxs);
	pm.intentMaxs = maxs;

	cl.playerinfo.sv_gravity = cl.frame.playerstate.pmove.gravity;
	cl.playerinfo.self = &cl_entities[cl.playernum + 1];
	cl.playerinfo.sv_cinematicfreeze = cl.frame.playerstate.cinematicfreeze;
	cl.playerinfo.deadflag = cl.frame.playerstate.deadflag;
	cl.playerinfo.viewheight = cl.frame.playerstate.viewheight;
	cl.playerinfo.target = NULL;
	cl.playerinfo.target_ent = NULL;

	if (cl.frame.playerstate.AutotargetEntityNum > 0)
		cl.playerinfo.enemystate = &cl_entities[cl.frame.playerstate.AutotargetEntityNum].current;
	else
		cl.playerinfo.enemystate = NULL;

	cl.playerinfo.flags = cl.frame.playerstate.flags;
	cl.playerinfo.fwdvel = cl.frame.playerstate.fwdvel;
	cl.playerinfo.edictflags = cl.frame.playerstate.edictflags;
	cl.playerinfo.sidevel = cl.frame.playerstate.sidevel;
	cl.playerinfo.upvel = cl.frame.playerstate.upvel;
	cl.playerinfo.upperseq = cl.frame.playerstate.upperseq;
	cl.playerinfo.lowerseq = cl.frame.playerstate.lowerseq;
	cl.playerinfo.targetEnt = NULL;

	if (cl.frame.playerstate.edictflags & (FL_AVERAGE_CHICKEN | FL_SUPER_CHICKEN))
		player_anim = playerExport.PlayerChickenData;
	else
		player_anim = playerExport.PlayerSeqData;

	cl.playerinfo.uppermove = player_anim[cl.frame.playerstate.uppermove_index].move;
	cl.playerinfo.lowermove = player_anim[cl.frame.playerstate.lowermove_index].move;
	cl.playerinfo.upperframe = cl.frame.playerstate.upperframe;
	cl.playerinfo.lowerframe = cl.frame.playerstate.lowerframe;
	cl.playerinfo.upperframeptr = &cl.playerinfo.uppermove->frame[cl.frame.playerstate.upperframe];
	cl.playerinfo.lowerframeptr = &cl.playerinfo.lowermove->frame[cl.frame.playerstate.lowerframe];
	cl.playerinfo.upperidle = (qboolean)(cl.frame.playerstate.upperidle != 0);
	cl.playerinfo.loweridle = (qboolean)(cl.frame.playerstate.loweridle != 0);

	cl.playerinfo.grabangle = cl.frame.playerstate.grabangle;
	VectorCopy(cl.frame.playerstate.grabloc, cl.playerinfo.grabloc);
	VectorCopy(cl.frame.playerstate.offsetangles, cl.playerinfo.offsetangles);

	cl.playerinfo.effects = cl.frame.playerstate.effects;
	cl.playerinfo.clientnum = cl.frame.playerstate.clientnum;
	cl.playerinfo.renderfx = cl.frame.playerstate.renderfx;
	cl.playerinfo.skinnum = cl.frame.playerstate.skinnum;

	memcpy(cl.playerinfo.fmnodeinfo, cl.frame.playerstate.fmnodeinfo, sizeof(cl.playerinfo.fmnodeinfo));

	cl.playerinfo.leveltime = cl.frame.playerstate.leveltime;
	cl.playerinfo.idletime = cl.frame.playerstate.idletime;
	VectorCopy(cl.frame.playerstate.angles, cl.playerinfo.angles);

	cl.playerinfo.frame = cl.frame.playerstate.frame;
	pred_currFrame = cl.frame.playerstate.frame;

	cl.playerinfo.swapFrame = cl.frame.playerstate.swapFrame;
	pred_currSwapFrame = cl.frame.playerstate.swapFrame;

	cl.playerinfo.powerup_timer = cl.frame.playerstate.powerup_timer;
	cl.playerinfo.weaponcharge = cl.frame.playerstate.weaponcharge;
	cl.playerinfo.advancedstaff = cl.frame.playerstate.advancedstaff;
	cl.playerinfo.ideal_yaw = cl.frame.playerstate.ideal_yaw;
	cl.playerinfo.dmflags = cl.frame.playerstate.dmflags;

	cl.playerinfo.pers.weapon = P_GetItemByIndex(cl.frame.playerstate.weapon);
	cl.playerinfo.pers.lastweapon = P_GetItemByIndex(cl.frame.playerstate.lastweapon);
	cl.playerinfo.pers.newweapon = P_GetItemByIndex(cl.frame.playerstate.newweapon);
	cl.playerinfo.pers.defence = P_GetItemByIndex(cl.frame.playerstate.defense);
	cl.playerinfo.pers.lastdefence = P_GetItemByIndex(cl.frame.playerstate.lastdefense);
	cl.playerinfo.pers.weaponready = cl.frame.playerstate.weaponready;
	cl.playerinfo.pers.armortype = (byte)cl.frame.playerstate.armortype;
	cl.playerinfo.pers.stafflevel = cl.frame.playerstate.stafflevel;
	cl.playerinfo.pers.bowtype = cl.frame.playerstate.bowtype;
	cl.playerinfo.pers.helltype = cl.frame.playerstate.helltype;
	cl.playerinfo.pers.handfxtype = cl.frame.playerstate.handfxtype;
	cl.playerinfo.pers.skintype = cl.frame.playerstate.skintype;
	cl.playerinfo.pers.altparts = cl.frame.playerstate.altparts;
	cl.playerinfo.pers.health = PLAYER_HEALTH; //mxd. Use define.
	cl.playerinfo.pers.max_health = PLAYER_HEALTH; //mxd. Use define.

	cl.playerinfo.weap_ammo_index = cl.frame.playerstate.weap_ammo_index;
	cl.playerinfo.def_ammo_index = cl.frame.playerstate.def_ammo_index;
	cl.playerinfo.meteor_count = cl.frame.playerstate.meteor_count;
	cl.playerinfo.switchtoweapon = cl.frame.playerstate.switchtoweapon;
	cl.playerinfo.plaguelevel = cl.frame.playerstate.plaguelevel;
	cl.playerinfo.groundentity = (void*)(cl.frame.playerstate.NonNullgroundentity ? -1 : 0);

	if (cl.frame.playerstate.GroundSurface.flags != 0)
	{
		memset(&ground_surf, 0, sizeof(ground_surf));
		ground_surf.flags = cl.frame.playerstate.GroundSurface.flags;
		cl.playerinfo.GroundSurface = &ground_surf;
	}
	else
	{
		cl.playerinfo.GroundSurface = NULL;
	}

	cl.playerinfo.GroundPlane = cl.frame.playerstate.GroundPlane;
	cl.playerinfo.GroundContents = cl.frame.playerstate.GroundContents;

	cl.playerinfo.watertype = cl.frame.playerstate.watertype;
	cl.playerinfo.waterheight = cl.frame.playerstate.waterheight;
	cl.playerinfo.waterlevel = cl.frame.playerstate.waterlevel;

	VectorSet(cl.playerinfo.oldvelocity, 0.0f, 0.0f, cl.frame.playerstate.oldvelocity_z);

	int cmd_time_delta = 0;
	int frame = min(ack + 1, current); // Initialize, in case we have no frames to run.

	// Run frames.
	while (++ack < current)
	{
		frame = ack & (CMD_BACKUP - 1);
		pm.cmd = cl.cmds[frame];

		if ((cl.playerinfo.flags & PLAYER_FLAG_TURNLOCK) && pm.s.pm_type == PM_NORMAL)
		{
			pm.s.pm_flags |= PMF_LOCKTURN;
		}
		else
		{
			pm.s.pm_flags &= ~PMF_LOCKTURN;
			cl.playerinfo.turncmd += SHORT2ANGLE(pm.cmd.angles[1] - old_cmd_angles[1]);
		}

		for (int i = 0; i < 3; i++)
			old_cmd_angles[i] = pm.cmd.angles[i];

		pm.cmd.forwardmove = (short)Q_ftol(cl.playerinfo.fwdvel);
		pm.cmd.sidemove = (short)Q_ftol(cl.playerinfo.sidevel);
		pm.cmd.upmove = (short)Q_ftol(cl.playerinfo.upvel);

		pm.knockbackfactor = Clamp(cl.playerinfo.knockbacktime - cl.playerinfo.leveltime, 0.0f, 1.0f);
		pm.groundentity = (struct edict_s*)cl.playerinfo.groundentity;
		pm.GroundPlane = cl.playerinfo.GroundPlane;
		pm.GroundContents = cl.playerinfo.GroundContents;
		pm.GroundSurface = cl.playerinfo.GroundSurface;
		pm.waterheight = cl.playerinfo.waterheight;
		pm.watertype = cl.playerinfo.watertype;
		pm.desiredWaterHeight = 15.0f;
		pm.waterlevel = cl.playerinfo.waterlevel;
		pm.high_max = (cl.playerinfo.effects & EF_HIGH_MAX) != 0;
		pm.run_shrine = (cl.playerinfo.effects & EF_SPEED_ACTIVE) != 0;

		if (pm.s.pm_flags & PMF_LOCKMOVE)
		{
			for (int i = 0; i < 3; i++)
			{
				tmp_origin[i] = pm.s.origin[i];
				tmp_vel[i] = pm.s.velocity[i];
			}
		}

		Pmove(&pm, false);
		
		if (pm.s.pm_flags & PMF_LOCKMOVE)
		{
			for (int i = 0; i < 3; i++)
			{
				pm.s.origin[i] = tmp_origin[i];
				pm.s.velocity[i] = tmp_vel[i];
			}
		}

		if (pm.s.w_flags & (WF_DIVING | WF_SWIMFREE))
		{
			if (pm.viewangles[0] > 180.0f)
				cl.playerinfo.angles[0] = -(pm.viewangles[0] - 360.0f);
			else
				cl.playerinfo.angles[0] = -pm.viewangles[0];
		}
		else
		{
			cl.playerinfo.angles[0] = 0.0f;
		}

		cl.playerinfo.angles[1] = pm.viewangles[1];
		cl.playerinfo.angles[2] = pm.viewangles[2];

		VectorCopy(pred_currAngles, pred_prevAngles);
		VectorCopy(cl.playerinfo.angles, pred_currAngles);

		for (int i = 0; i < 3; i++)
		{
			cl.playerinfo.origin[i] = (float)pm.s.origin[i] / 8.0f;
			cl.playerinfo.velocity[i] = (float)pm.s.velocity[i] / 8.0f;
		}

		VectorCopy(pm.intentMins, cl.playerinfo.mins);
		VectorCopy(pm.intentMaxs, cl.playerinfo.maxs);

		cl.playerinfo.watertype = pm.watertype;
		cl.playerinfo.movetype = pm.s.pm_type;
		cl.playerinfo.pm_flags = pm.s.pm_flags;
		cl.playerinfo.pm_w_flags = pm.s.w_flags;
		cl.playerinfo.waterheight = pm.waterheight;
		cl.playerinfo.groundentity = pm.groundentity;
		cl.playerinfo.waterlevel = pm.waterlevel;
		cl.playerinfo.GroundSurface = pm.GroundSurface;
		cl.playerinfo.GroundPlane = pm.GroundPlane;
		cl.playerinfo.GroundContents = pm.GroundContents;

		if (pm.waterlevel > 0)
			cl.playerinfo.flags |= PLAYER_FLAG_BOWDRAWN;
		else
			cl.playerinfo.flags &= ~PLAYER_FLAG_BOWDRAWN;

		cl.playerinfo.flags &= ~(PLAYER_FLAG_COLLISION | PLAYER_FLAG_SLIDE);

		if (pm.s.c_flags & PC_COLLISION)
			cl.playerinfo.flags |= PLAYER_FLAG_COLLISION;

		if (pm.s.c_flags & PC_SLIDING)
		{
			cl.playerinfo.flags |= PLAYER_FLAG_SLIDE;

			if (Vec3NotZero(pm.GroundPlane.normal))
			{
				vec3_t angles;
				vectoangles(pm.GroundPlane.normal, angles);
				cl.playerinfo.ideal_yaw = angles[1];
			}
		}
		else if (pm.s.w_flags & WF_DIVE)
		{
			cl.playerinfo.flags |= PLAYER_FLAG_DIVE;
		}

		cl.playerinfo.oldbuttons = cl.playerinfo.buttons;
		cl.playerinfo.buttons = pm.cmd.buttons;
		cl.playerinfo.remember_buttons |= pm.cmd.buttons;
		cl.playerinfo.latched_buttons |= ~cl.playerinfo.buttons & pm.cmd.buttons;

		int oldframe = (frame - 1) & (CMD_BACKUP - 1);
		cmd_time_delta += (cl.cmd_time[frame] - cl.cmd_time[oldframe]);

		if (cmd_time_delta > 100)
		{
			const int steps = cmd_time_delta / 100;
			cmd_time_delta %= 100;

			pm.cmd.forwardmove = cl.cmds[frame].forwardmove;
			pm.cmd.sidemove = cl.cmds[frame].sidemove;
			pm.cmd.upmove = cl.cmds[frame].upmove;

			cl.playerinfo.pcmd = pm.cmd;

			for (int i = 0; i < 3; i++)
				cl.playerinfo.aimangles[i] = SHORT2ANGLE(pm.cmd.aimangles[i]);

			for (int i = 0; i < steps; i++)
			{
				vec3_t old_origin;
				VectorCopy(cl.playerinfo.origin, old_origin);

				cl.playerinfo.ishistory = (cl.playerinfo.leveltime <= cl.playerinfo.Highestleveltime);
				if (!cl.playerinfo.ishistory)
					cl.playerinfo.Highestleveltime = cl.playerinfo.leveltime;

				pred_prevFrame = pred_currFrame;
				pred_prevSwapFrame = pred_currSwapFrame;

				P_PlayerFallingDamage(&cl.playerinfo);
				P_PlayerUpdateCmdFlags(&cl.playerinfo);
				P_PlayerUpdate(&cl.playerinfo);
				P_AnimUpdateFrame(&cl.playerinfo);

				cl.playerinfo.leveltime += 0.1f;

				for (int c = 0; c < 3; c++)
					cl.prediction_error[c] = cl.playerinfo.origin[c] - old_origin[c];

				pred_currFrame = cl.playerinfo.frame;
				pred_currSwapFrame = cl.playerinfo.swapFrame;

				VectorCopy(cl.playerinfo.angles, pred_currAngles);
				VectorCopy(cl.playerinfo.velocity, cl.playerinfo.oldvelocity);
			}

			for (int i = 0; i < 3; i++)
			{
				pm.s.origin[i] = (short)(cl.playerinfo.origin[i] * 8.0f);
				pm.s.velocity[i] = (short)(cl.playerinfo.velocity[i] * 8.0f);
			}

			pm.s.pm_type = cl.playerinfo.movetype;
			pm.s.pm_flags = (byte)cl.playerinfo.pm_flags;
			pm.s.w_flags = (byte)cl.playerinfo.pm_w_flags;
		}
	}

	pred_playerLerp = (float)cmd_time_delta / 100.0f;
	const float backlerp = 1.0f - pred_playerLerp;

	for (int i = 0; i < 3; i++)
	{
		float val = cl.playerinfo.origin[i] - cl.prediction_error[i] * backlerp;
		cl.predicted_origins[frame][i] = (short)(val * 8.0f);
	}

	float dist = fabsf((float)cl.predicted_origins[frame][0] / 8.0f - cl.predicted_origin[0]) +
				 fabsf((float)cl.predicted_origins[frame][1] / 8.0f - cl.predicted_origin[1]) +
				 fabsf((float)cl.predicted_origins[frame][2] / 8.0f - cl.predicted_origin[2]);

	if (dist < 640.0f) // When dist > 80, assume player teleported.
	{
		for (int i = 0; i < 3; i++)
			cl.predicted_origin[i] = ((float)cl.predicted_origins[frame][i] / 8.0f + cl.predicted_origin[i]) * 0.5f;
	}
	else
	{
		VectorCopy(cl.playerinfo.origin, cl.predicted_origin);
	}

	pred_effects = cl.playerinfo.effects;
	pred_clientnum = cl.playerinfo.clientnum;
	pred_renderfx = cl.playerinfo.renderfx;
	pred_skinnum = cl.playerinfo.skinnum;

	if (VectorCompare(cl.playerinfo.offsetangles, cl.frame.playerstate.offsetangles))
	{
		VectorClear(cl.playerinfo.offsetangles);
		viewoffset_changed = false;
	}
	else
	{
		viewoffset_changed = true;
	}
}

void CL_StorePredictInfo(void) // H2
{
	VectorCopy(cl.predicted_origin, cl_entities[cl.playernum + 1].origin);

	cl.predictinfo.currFrame = pred_currFrame;
	cl.predictinfo.currSwapFrame = pred_currSwapFrame;
	cl.predictinfo.prevFrame = pred_prevFrame;
	cl.predictinfo.prevSwapFrame = pred_prevSwapFrame;

	if (cl.playerinfo.deadflag == 0)
	{
		VectorCopy(pred_currAngles, cl.predictinfo.currAngles);
		VectorCopy(pred_prevAngles, cl.predictinfo.prevAngles);
	}

	cl.predictinfo.effects = pred_effects;
	cl.predictinfo.playerLerp = pred_playerLerp;
	cl.predictinfo.clientnum = pred_clientnum;
	cl.predictinfo.renderfx = pred_renderfx;
	cl.predictinfo.skinnum = pred_skinnum;

	memcpy(cl.predictinfo.fmnodeinfo, cl.playerinfo.fmnodeinfo, sizeof(cl.predictinfo.fmnodeinfo));

	pred_pm_flags = cl.playerinfo.pm_flags;
	pred_pm_w_flags = cl.playerinfo.pm_w_flags;
}
//
// cl_prediction.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "EffectFlags.h"
#include "p_main.h"
#include "Vector.h"

static int pred_effects = 0;
static int pred_clientnum = 0;
static int pred_renderfx = 0;
static int pred_skinnum = 0;
static float pred_playerLerp = 0.0f;

static int pred_pm_flags;
static int pred_pm_w_flags;

static int pred_currFrame = 0;
static int pred_prevFrame = 0;

static int pred_currSwapFrame = 0;
static int pred_prevSwapFrame = 0;

static vec3_t pred_currAngles = { 0 };
static vec3_t pred_prevAngles = { 0 };

void CL_CheckPredictionError(void)
{
	NOT_IMPLEMENTED
}

static void CL_PMTrace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, trace_t* tr)
{
	NOT_IMPLEMENTED
}

int CL_PMpointcontents(vec3_t point)
{
	NOT_IMPLEMENTED
	return 0;
}

// Sets cl.predicted_origin and cl.predicted_angles
void CL_PredictMovement(void) //mxd. Surprisingly, NOT the biggest H2 function...
{
	static int cmd_time_delta = 0;
	static int old_cmd_time_delta = 0;
	static pmove_state_t old_pmove_state = { 0 };
	static short old_cmd_angles[3] = { 0, 0, 0 };

	paceldata_t* player_anim;
	short tmp_origin[3];
	short tmp_vel[3];
	vec3_t mins;
	vec3_t maxs;
	vec3_t origin;
	pmove_t pm;
	csurface_t ground_surf;

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
		cl.playerinfo.origin[i] = (float)pm.s.origin[i] * 0.125f;
		cl.playerinfo.velocity[i] = (float)pm.s.velocity[i] * 0.125f;
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
	cl.playerinfo.pers.health = 100;
	cl.playerinfo.pers.max_health = 100;

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

	VectorSet(cl.playerinfo.oldvelocity, 0, 0, cl.frame.playerstate.oldvelocity_z);

	old_pmove_state = pm.s;
	old_cmd_time_delta = 0;
	cmd_time_delta = 0;

	int frame = 0;

	// Run frames.
	while (++ack < current)
	{
		frame = ack & (CMD_BACKUP - 1);
		pm.cmd = cl.cmds[frame];

		if (cl.playerinfo.flags & PLAYER_FLAG_TURNLOCK && pm.s.pm_type == PM_NORMAL)
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

		if (memcmp(&pm.s, &old_pmove_state, sizeof(pmove_state_t)) != 0)
			pm.snapinitial = true;

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
			cl.playerinfo.origin[i] = (float)pm.s.origin[i] * 0.125f;
			cl.playerinfo.velocity[i] = (float)pm.s.velocity[i] * 0.125f;
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

		old_pmove_state = pm.s;

		if (pm.waterlevel)
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
		cl.playerinfo.remember_buttons |= pm.cmd.buttons;
		cl.playerinfo.latched_buttons |= ~cl.playerinfo.buttons & pm.cmd.buttons;

		int oldframe = (frame - 1) & (CMD_BACKUP - 1);
		cmd_time_delta += (cl.cmd_time[frame] - cl.cmd_time[oldframe]);
		old_cmd_time_delta = cmd_time_delta;

		if (cmd_time_delta > 100)
		{
			const int steps = cmd_time_delta / 100;
			cmd_time_delta %= 100;

			pm.cmd.forwardmove = cl.cmds[frame].forwardmove;
			pm.cmd.sidemove = cl.cmds[frame].sidemove;
			pm.cmd.upmove = cl.cmds[frame].upmove;

			old_cmd_time_delta = cmd_time_delta;
			cl.playerinfo.buttons = pm.cmd.buttons;
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
				pm.s.origin[i] = (short)Q_ftol(cl.playerinfo.origin[i] * 8);
				pm.s.velocity[i] = (short)Q_ftol(cl.playerinfo.velocity[i] * 8);
			}

			pm.s.pm_type = cl.playerinfo.movetype;
			pm.s.pm_flags = (byte)cl.playerinfo.pm_flags;
			pm.s.w_flags = (byte)cl.playerinfo.pm_w_flags;
		}
	}

	pred_playerLerp = (float)old_cmd_time_delta * 0.01f;
	float backlerp = 1.0f - pred_playerLerp;
	vec3_t delta;

	for (int i = 0; i < 3; i++)
	{
		delta[i] = cl.playerinfo.origin[i] - cl.prediction_error[i] * backlerp;
		cl.predicted_origins[frame][i] = (short)Q_ftol(delta[i] * 8.0f);
	}

	float dist = fabsf(delta[0] - cl.predicted_origin[0]) +
				 fabsf(delta[1] - cl.predicted_origin[1]) +
				 fabsf(delta[2] - cl.predicted_origin[2]);

	if (dist < 640.0f)
	{
		for (int i = 0; i < 3; i++)
			cl.predicted_origin[i] = (delta[i] + cl.predicted_origin[i]) * 0.5f;
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
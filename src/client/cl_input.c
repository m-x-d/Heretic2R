//
// cl_input.c -- builds an intended movement command to send to the server
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "EffectFlags.h"
#include "Vector.h"

static cvar_t* cl_nodelta;

static uint frame_msec;
static uint old_sys_frame_time;

int pred_pm_flags; // H2
int pred_pm_w_flags; // H2

qboolean command_down; // H2

// KEY BUTTONS

// Continuous button event tracking is complicated by the fact that two different input sources
// (say, mouse button 1 and the control key) can both press the same button,
// but the button should only be released when both of the pressing key have been released.

// When a key event issues a button command (+forward, +attack, etc), it appends its key number
// as a parameter to the command so it can be matched up with the release.

// state bit 0 is the current state of the key
// state bit 1 is edge triggered on the up to down transition
// state bit 2 is edge triggered on the down to up transition

#define MOVE_SCALER	64.0f //mxd

qboolean in_do_autoaim;

kbutton_t in_klook;
kbutton_t in_strafe;
kbutton_t in_speed;
kbutton_t in_lookaround;

static kbutton_t in_left;
static kbutton_t in_right;
static kbutton_t in_forward;
static kbutton_t in_back;

static kbutton_t in_lookup;
static kbutton_t in_lookdown;
static kbutton_t in_moveleft;
static kbutton_t in_moveright;

static kbutton_t in_up;
kbutton_t in_down;

static kbutton_t in_attack;
static kbutton_t in_defend; // H2
static kbutton_t in_action; // H2

static kbutton_t in_creep; // H2
static kbutton_t in_inventory; // H2
static kbutton_t in_quickturn; // H2
static kbutton_t in_autoaim; // H2

// Q2 counterpart
static void KeyUp(kbutton_t* b)
{
	const char* c = Cmd_Argv(1);

	if (c[0] == 0)
	{
		// Typed manually at the console, assume for unsticking, so clear all.
		b->down[0] = 0;
		b->down[1] = 0;
		b->state = 4; // Impulse up

		return;
	}

	const int k = Q_atoi(c);

	if (b->down[0] == k)
		b->down[0] = 0;
	else if (b->down[1] == k)
		b->down[1] = 0;
	else
		return; // Key up without corresponding down (menu pass through).

	if (b->down[0] || b->down[1] || !(b->state & 1))
		return; // Some other key is still holding it down or still up (this should not happen).

	// Save timestamp.
	const uint uptime = Q_atoi(Cmd_Argv(2));
	if (uptime > 0)
		b->msec += uptime - b->downtime;
	else
		b->msec += 10;

	b->state &= ~1; // Now up.
	b->state |= 4; // Impulse up.
}

// Q2 counterpart
static void KeyDown(kbutton_t* b)
{
	int k;

	const char* c = Cmd_Argv(1);
	if (c[0])
		k = Q_atoi(c);
	else
		k = -1; // Typed manually at the console for continuous down.

	if (k == b->down[0] || k == b->down[1])
		return; // Repeating key.

	if (!b->down[0])
	{
		b->down[0] = k;
	}
	else if (!b->down[1])
	{
		b->down[1] = k;
	}
	else
	{
		Com_Printf("Three keys down for a button!\n");
		return;
	}

	if (b->state & 1)
		return; // Still down.

	// Save timestamp.
	c = Cmd_Argv(2);
	b->downtime = Q_atoi(c);

	if (b->downtime == 0)
		b->downtime = sys_frame_time - 100;

	b->state |= 1 + 2; // Down + impulse down.
}

// Q2 counterpart
static void IN_UpDown(void)
{
	KeyDown(&in_up);
}

// Q2 counterpart
static void IN_UpUp(void)
{
	KeyUp(&in_up);
}

// Q2 counterpart
static void IN_DownDown(void)
{
	KeyDown(&in_down);
}

// Q2 counterpart
static void IN_DownUp(void)
{
	KeyUp(&in_down);
}

// Q2 counterpart
static void IN_LeftDown(void)
{
	KeyDown(&in_left);
}

// Q2 counterpart
static void IN_LeftUp(void)
{
	KeyUp(&in_left);
}

// Q2 counterpart
static void IN_RightDown(void)
{
	KeyDown(&in_right);
}

// Q2 counterpart
static void IN_RightUp(void)
{
	KeyUp(&in_right);
}

// Q2 counterpart
static void IN_ForwardDown(void)
{
	KeyDown(&in_forward);
}

// Q2 counterpart
static void IN_ForwardUp(void)
{
	KeyUp(&in_forward);
}

// Q2 counterpart
static void IN_BackDown(void)
{
	KeyDown(&in_back);
}

// Q2 counterpart
static void IN_BackUp(void)
{
	KeyUp(&in_back);
}

// Q2 counterpart
static void IN_LookupDown(void)
{
	KeyDown(&in_lookup);
}

// Q2 counterpart
static void IN_LookupUp(void)
{
	KeyUp(&in_lookup);
}

// Q2 counterpart
static void IN_LookdownDown(void)
{
	KeyDown(&in_lookdown);
}

// Q2 counterpart
static void IN_LookdownUp(void)
{
	KeyUp(&in_lookdown);
}

// Q2 counterpart
static void IN_StrafeDown(void)
{
	KeyDown(&in_strafe);
}

// Q2 counterpart
static void IN_StrafeUp(void)
{
	KeyUp(&in_strafe);
}

// Q2 counterpart
static void IN_MoveleftDown(void)
{
	KeyDown(&in_moveleft);
}

// Q2 counterpart
static void IN_MoveleftUp(void)
{
	KeyUp(&in_moveleft);
}

// Q2 counterpart
static void IN_MoverightDown(void)
{
	KeyDown(&in_moveright);
}

// Q2 counterpart
static void IN_MoverightUp(void)
{
	KeyUp(&in_moveright);
}

// Q2 counterpart
static void IN_AttackDown(void)
{
	KeyDown(&in_attack);
}

// Q2 counterpart
static void IN_AttackUp(void)
{
	KeyUp(&in_attack);
}

static void IN_DefendDown(void) // H2
{
	KeyDown(&in_defend);
}

static void IN_DefendUp(void) // H2
{
	KeyUp(&in_defend);
}

static void IN_ActionDown(void) // H2
{
	KeyDown(&in_action);
}

static void IN_ActionUp(void) // H2
{
	KeyUp(&in_action);
}

static void IN_CreepDown(void) // H2
{
	KeyDown(&in_creep);
}

static void IN_CreepUp(void) // H2
{
	KeyUp(&in_creep);
}

static void IN_AutoaimDown(void) // H2
{
	KeyDown(&in_autoaim);
}

static void IN_AutoaimUp(void) // H2
{
	KeyUp(&in_autoaim);
}

static void IN_KLookDown(void)
{
	KeyDown(&in_klook);
}

static void IN_KLookUp(void)
{
	if (!(int)freelook->value && (int)lookspring->value) // H2
		IN_CenterView();

	KeyUp(&in_klook);
}

static void IN_SpeedDown(void) // H2
{
	KeyDown(&in_speed);
}

static void IN_SpeedUp(void) // H2
{
	KeyUp(&in_speed);
}

static void IN_LookaroundDown(void) // H2
{
	KeyDown(&in_lookaround);
}

static void IN_LookaroundUp(void) // H2
{
	KeyUp(&in_lookaround);
}

static void IN_CommandDown(void) // H2
{
	command_down = true;
}

static void IN_CommandUp(void) // H2
{
	command_down = false;
}

static void IN_QuickturnDown(void) // H2
{
	KeyDown(&in_quickturn);
}

static void IN_QuickturnUp(void) // H2
{
	KeyUp(&in_quickturn);
}

static void IN_InventoryDown(void) // H2
{
	KeyDown(&in_inventory);
}

static void IN_InventoryUp(void) // H2
{
	KeyUp(&in_inventory);
}

void IN_CenterView(void)
{
	const float angle = -SHORT2ANGLE(cl.frame.playerstate.pmove.delta_angles[0]);
	cl.inputangles[0] = angle;
	cl.viewangles[0] = angle;
}

void CL_InitInput(void)
{
	Cmd_AddCommand("centerview", IN_CenterView);

	Cmd_AddCommand("+moveup", IN_UpDown);
	Cmd_AddCommand("-moveup", IN_UpUp);
	Cmd_AddCommand("+movedown", IN_DownDown);
	Cmd_AddCommand("-movedown", IN_DownUp);
	Cmd_AddCommand("+left", IN_LeftDown);
	Cmd_AddCommand("-left", IN_LeftUp);
	Cmd_AddCommand("+right", IN_RightDown);
	Cmd_AddCommand("-right", IN_RightUp);
	Cmd_AddCommand("+forward", IN_ForwardDown);
	Cmd_AddCommand("-forward", IN_ForwardUp);
	Cmd_AddCommand("+back", IN_BackDown);
	Cmd_AddCommand("-back", IN_BackUp);
	Cmd_AddCommand("+lookup", IN_LookupDown);
	Cmd_AddCommand("-lookup", IN_LookupUp);
	Cmd_AddCommand("+lookdown", IN_LookdownDown);
	Cmd_AddCommand("-lookdown", IN_LookdownUp);
	Cmd_AddCommand("+strafe", IN_StrafeDown);
	Cmd_AddCommand("-strafe", IN_StrafeUp);
	Cmd_AddCommand("+moveleft", IN_MoveleftDown);
	Cmd_AddCommand("-moveleft", IN_MoveleftUp);
	Cmd_AddCommand("+moveright", IN_MoverightDown);
	Cmd_AddCommand("-moveright", IN_MoverightUp);
	Cmd_AddCommand("+attack", IN_AttackDown);
	Cmd_AddCommand("-attack", IN_AttackUp);
	Cmd_AddCommand("+defend", IN_DefendDown);
	Cmd_AddCommand("-defend", IN_DefendUp);
	Cmd_AddCommand("+action", IN_ActionDown);
	Cmd_AddCommand("-action", IN_ActionUp);
	Cmd_AddCommand("+creep", IN_CreepDown);
	Cmd_AddCommand("-creep", IN_CreepUp);
	Cmd_AddCommand("+autoaim", IN_AutoaimDown);
	Cmd_AddCommand("-autoaim", IN_AutoaimUp);
	Cmd_AddCommand("+klook", IN_KLookDown);
	Cmd_AddCommand("-klook", IN_KLookUp);
	Cmd_AddCommand("+speed", IN_SpeedDown);
	Cmd_AddCommand("-speed", IN_SpeedUp);
	Cmd_AddCommand("+lookaround", IN_LookaroundDown);
	Cmd_AddCommand("-lookaround", IN_LookaroundUp);
	Cmd_AddCommand("+command", IN_CommandDown);
	Cmd_AddCommand("-command", IN_CommandUp);
	Cmd_AddCommand("+quickturn", IN_QuickturnDown);
	Cmd_AddCommand("-quickturn", IN_QuickturnUp);
	Cmd_AddCommand("+inventory", IN_InventoryDown);
	Cmd_AddCommand("-inventory", IN_InventoryUp);

	cl_nodelta = Cvar_Get("cl_nodelta", "0", 0);
}

// Q2 counterpart
// Returns the fraction of the frame that the key was down.
static float CL_KeyState(kbutton_t* key)
{
	key->state &= 1; // Clear impulses.

	uint msec = key->msec;
	key->msec = 0;

	if (key->state) // Still down.
	{
		msec += sys_frame_time - key->downtime;
		key->downtime = sys_frame_time;
	}

	return Clamp((float)msec / (float)frame_msec, 0.0f, 1.0f);
}

// Moves the local angle positions.
static void CL_AdjustAngles(void)
{
	float speed;
	float scaler;

	if (in_speed.state & 1)
		speed = cls.frametime * cl_anglespeedkey->value;
	else
		speed = cls.frametime;

	if (!(in_strafe.state & 1))
	{
		scaler = speed * cl_yawspeed->value;

		cl.delta_inputangles[YAW] -= CL_KeyState(&in_right) * scaler;
		cl.delta_inputangles[YAW] += CL_KeyState(&in_left) * scaler;
	}

	scaler = speed * cl_pitchspeed->value;

	if (in_klook.state & 1)
	{
		cl.delta_inputangles[PITCH] -= CL_KeyState(&in_forward) * scaler;
		cl.delta_inputangles[PITCH] += CL_KeyState(&in_back) * scaler;
	}

	cl.delta_inputangles[PITCH] -= CL_KeyState(&in_lookup) * scaler;
	cl.delta_inputangles[PITCH] += CL_KeyState(&in_lookdown) * scaler;
}

// Send the intended movement message to the server.
void CL_BaseMove(usercmd_t* cmd)
{
	memset(cmd, 0, sizeof(usercmd_t));

	VectorCopy(cl.delta_inputangles, cl.old_delta_inputangles);
	VectorClear(cl.delta_inputangles);

	for (int i = 0; i < 3; i++)
		cmd->angles[i] = ANGLE2SHORT(cl.inputangles[i]);

	CL_AdjustAngles();

	if (in_strafe.state & 1)
	{
		cmd->sidemove += (short)(CL_KeyState(&in_right) * MOVE_SCALER);
		cmd->sidemove -= (short)(CL_KeyState(&in_left) * MOVE_SCALER);
	}

	cmd->sidemove += (short)(CL_KeyState(&in_moveright) * MOVE_SCALER);
	cmd->sidemove -= (short)(CL_KeyState(&in_moveleft) * MOVE_SCALER);

	cmd->upmove += (short)(CL_KeyState(&in_up) * MOVE_SCALER);
	cmd->upmove -= (short)(CL_KeyState(&in_down) * MOVE_SCALER);

	if (!(in_klook.state & 1))
	{
		cmd->forwardmove += (short)(CL_KeyState(&in_forward) * MOVE_SCALER);
		cmd->forwardmove -= (short)(CL_KeyState(&in_back) * MOVE_SCALER);
	}
}

static void CL_ClampPitch(void)
{
	float pitch = SHORT2ANGLE(cl.frame.playerstate.pmove.delta_angles[PITCH]);

	if (pitch > 180.0f)
		pitch -= 360.0f;

	// H2. Clamp input angles.
	if (cl.inputangles[PITCH] + pitch > 89.0f)
		cl.inputangles[PITCH] = 89.0f - pitch;

	if (cl.inputangles[PITCH] + pitch < -89.0f)
		cl.inputangles[PITCH] = -89.0f - pitch;

	// Clamp view angles.
	if (cl.viewangles[PITCH] + pitch > 89.0f)
		cl.viewangles[PITCH] = 89.0f - pitch;

	if (cl.viewangles[PITCH] + pitch < -89.0f)
		cl.viewangles[PITCH] = -89.0f - pitch;
}

//TODO: rename 'st_unknown' cvars, check conditions & annotate of all angles calculation branches when we get to playable state.
static void CL_FinishMove(usercmd_t* cmd)
{
	static float quickturn_rate_scaler;
	static float quickturn_rate;
	static qboolean st_unknown1;
	static qboolean st_unknown2;
	static qboolean st_unknown3;
	static qboolean st_unknown4;
	static qboolean st_unknown5;
	static qboolean st_unknown6;

	// Figure button bits

	// He attac
	if (in_attack.state & 3)
		cmd->buttons |= BUTTON_ATTACK;
	in_attack.state &= ~2;

	// But also protec
	if (in_defend.state & 2)
		cmd->buttons |= BUTTON_DEFEND;
	in_defend.state &= ~2;

	// Action
	if (in_action.state & 3 && !cl.frame.playerstate.cinematicfreeze)
		cmd->buttons |= BUTTON_ACTION;
	in_action.state &= ~2;

	// Run
	const qboolean speed_state = (in_speed.state & 3);
	const qboolean run = (int)cl_run->value;
	if ((speed_state || run) && ((speed_state && !run) || speed_state == (cmd->forwardmove < -10)))
		cmd->buttons |= BUTTON_RUN;
	in_speed.state &= ~2;

	// Crouch
	if (in_creep.state & 3)
		cmd->buttons |= BUTTON_CREEP;
	in_creep.state &= ~2;

	// Autoaim
	if ((in_autoaim.state & 3) != (int)cl_doautoaim->value)
		cmd->buttons |= BUTTON_AUTOAIM;
	in_autoaim.state &= ~2;

	in_do_autoaim = (cmd->buttons & BUTTON_AUTOAIM);

	// TR-style look around
	if (in_lookaround.state & 3)
		cmd->buttons |= BUTTON_LOOKAROUND;
	in_lookaround.state &= ~2;

	// TR-style quick-turn
	if (in_quickturn.state & 3)
	{
		cmd->buttons |= BUTTON_QUICKTURN;
		if (quickturn_rate_scaler == 0.0f)
			quickturn_rate_scaler = 0.5f;
	}
	in_quickturn.state &= ~2;

	// Open inventory
	if (in_inventory.state & 3)
		cmd->buttons |= BUTTON_INVENTORY;
	in_inventory.state &= ~2;

	if (anykeydown > 0 && cls.key_dest == key_game)
		cmd->buttons |= BUTTON_ANY;

	// Look around key pressed?
	const qboolean do_lookaround = (in_lookaround.state & 1);
	if (do_lookaround)
	{
		cl.lookangles[PITCH] += cl.delta_inputangles[PITCH];
		cl.lookangles[YAW] += cl.delta_inputangles[YAW];
	}
	else
	{
		cl.lookangles[PITCH] = cl.viewangles[PITCH];
		cl.lookangles[YAW] = cl.viewangles[YAW];
	}

	// Can't do much when chicken...
	if (cl_entities[cl.playernum + 1].current.effects & EF_CHICKEN)
	{
		if (!do_lookaround)
		{
			cl.viewangles[PITCH] += cl.delta_inputangles[PITCH];
			cl.inputangles[YAW] = cl.viewangles[YAW] + cl.delta_inputangles[YAW];
			cl.viewangles[YAW] = cl.inputangles[YAW];
		}

		goto LABEL_END;
	}

	// When in water
	if (pred_pm_w_flags != 0)
	{
		if (pred_pm_w_flags & (WF_SURFACE | WF_DIVE))
		{
			st_unknown3 = true;

			if (st_unknown1)
			{
				st_unknown1 = false;
				cl.viewangles[YAW] = cl.inputangles[YAW];
				cl.delta_inputangles[YAW] = 0.0f;
			}

			if (st_unknown2)
			{
				st_unknown2 = false;
				cl.delta_inputangles[PITCH] = 0.0f;
				cl.inputangles[PITCH] = -((float)cl.frame.playerstate.pmove.delta_angles[PITCH] * SHORT_TO_ANGLE);
				cl.viewangles[PITCH] = cl.inputangles[PITCH];
			}
		}
		else
		{
			st_unknown2 = true;

			if (st_unknown3 && (pred_pm_w_flags & WF_SWIMFREE))
			{
				st_unknown3 = false;
				cl.viewangles[PITCH] = cl.inputangles[PITCH];
			}

			if (st_unknown1 && (pred_pm_w_flags & WF_SWIMFREE))
			{
				st_unknown1 = false;
				cl.inputangles[PITCH] = cl.viewangles[PITCH];
				cl.delta_inputangles[PITCH] = 0.0f;
			}
		}

		if (!do_lookaround)
		{
			cl.inputangles[PITCH] = cl.inputangles[PITCH] + cl.delta_inputangles[PITCH];
			cl.inputangles[YAW] = cl.inputangles[YAW] + cl.delta_inputangles[YAW];
			cl.viewangles[PITCH] = cl.viewangles[PITCH] + cl.delta_inputangles[PITCH];
			cl.viewangles[YAW] = cl.viewangles[YAW] + cl.delta_inputangles[YAW];
		}

		goto LABEL_END;
	}

	// When on land
	st_unknown1 = true;

	// When not frozen in place.
	if (!(pred_pm_flags & PMF_STANDSTILL))
	{
		st_unknown5 = true;

		if (st_unknown4)
		{
			st_unknown4 = false;
			cl.inputangles[YAW] = cl.viewangles[YAW];
		}

		if (!do_lookaround)
		{
			if (!(pred_pm_flags & PMF_LOCKTURN))
			{
				cl.inputangles[YAW] += cl.delta_inputangles[YAW];
				cl.viewangles[YAW] += cl.delta_inputangles[YAW];
			}

			cl.viewangles[PITCH] += cl.delta_inputangles[PITCH];
		}

		goto LABEL_END;
	}

	// When executing quickturn
	if ((int)cl_predict->value)
		quickturn_rate = cl.playerinfo.quickturn_rate;
	else
		quickturn_rate = cl.frame.playerstate.quickturn_rate;

	if (quickturn_rate_scaler > 0.0f && quickturn_rate != 0.0f)
	{
		quickturn_rate_scaler -= cls.frametime;

		float scaled_rate = 0.0f;
		if (quickturn_rate_scaler < 0.0f)
		{
			scaled_rate = quickturn_rate * quickturn_rate_scaler;
			quickturn_rate_scaler = 0.0f;
		}

		const float offset = cls.frametime * quickturn_rate + scaled_rate;
		cl.inputangles[YAW] += offset;
		cl.viewangles[YAW] += offset;

		goto LABEL_END;
	}

	// Default camera movement logic (???)
	st_unknown4 = true;

	if (st_unknown5)
	{
		st_unknown5 = false;
		cl.inputangles[YAW] = cl.viewangles[YAW];
		cl.inputangles[PITCH] = -((float)cl.frame.playerstate.pmove.delta_angles[PITCH] * SHORT_TO_ANGLE);
	}

	if (!do_lookaround)
	{
		cl.viewangles[PITCH] += cl.delta_inputangles[PITCH];
		cl.viewangles[YAW] += cl.delta_inputangles[YAW];
	}

	const float delta_yaw = cl.viewangles[YAW] - cl.inputangles[YAW];

	if (delta_yaw > 66.0f || delta_yaw < -66.0f)
		st_unknown6 = true;
	else if (!st_unknown6)
		goto LABEL_END;

	if (cl.delta_inputangles[YAW] != 0.0f)
	{
		if ((cl.delta_inputangles[YAW] > 0.0f && cl.old_delta_inputangles[YAW] >= 0.0f) ||
			(cl.delta_inputangles[YAW] < 0.0f && cl.old_delta_inputangles[YAW] <= 0.0f))
		{
			cl.inputangles[YAW] += cl.delta_inputangles[YAW];
			goto LABEL_END;
		}

		st_unknown6 = false;
		goto LABEL_END;
	}

	if (delta_yaw >= 0.0f)
	{
		if (delta_yaw > 0.0f)
		{
			if (cl.inputangles[YAW] > cl.viewangles[YAW])
			{
				st_unknown6 = false;
				goto LABEL_END;
			}

			cl.inputangles[YAW] += delta_yaw * 0.25f;

			if (Q_fabs(cl.viewangles[YAW] - cl.inputangles[YAW]) >= 5.0f)
				goto LABEL_END;

			cl.inputangles[YAW] = cl.viewangles[YAW];
		}

		st_unknown6 = false;
		goto LABEL_END;
	}

	if (cl.inputangles[YAW] >= cl.viewangles[YAW])
	{
		cl.inputangles[YAW] += delta_yaw * 0.25f;

		if (Q_fabs(cl.viewangles[YAW] - cl.inputangles[YAW]) < 5.0f)
		{
			st_unknown6 = false;
			cl.inputangles[YAW] = cl.viewangles[YAW];
		}
	}
	else
	{
		st_unknown6 = false;
	}

LABEL_END:
	CL_ClampPitch();

	// Store angles in usercmd.
	for (int i = 0; i < 3; i++)
	{
		cmd->angles[i] = ANGLE2SHORT(cl.inputangles[i]);
		cmd->aimangles[i] = (short)(cl.frame.playerstate.pmove.delta_angles[i] - ANGLE2SHORT(-cl.viewangles[i]));
		cmd->camera_vieworigin[i] = (short)(cl.camera_vieworigin[i] * 8.0f);
		cmd->camera_viewangles[i] = ANGLE2SHORT(cl.camera_viewangles[i]);
	}

	// Send the ambient light level at the player's current position
	cmd->lightlevel = (byte)cl_lightlevel->value;

	// Send milliseconds of time to apply the move
	int ms = (int)(cls.frametime * 1000.0f);
	if (ms > 250)
		ms = 100; // Time was unreasonable

	cmd->msec = (byte)ms;
}

static usercmd_t CL_CreateCmd(void)
{
	static qboolean reset_input_angles = false;
	usercmd_t cmd;

	frame_msec = ClampI((int)(sys_frame_time - old_sys_frame_time), 1, 200);

	if (cl.frame.playerstate.cinematicfreeze) // H2
	{
		memset(&cmd, 0, sizeof(cmd));

		if (cls.esc_cinematic)
		{
			cmd.buttons |= BUTTON_ACTION;
			cls.esc_cinematic = false;
		}

		reset_input_angles = true;
	}
	else
	{
		// Get basic movement from keyboard.
		CL_BaseMove(&cmd);

		// Allow mice or other external controllers to add to the move.
		IN_Move(&cmd);

		if (reset_input_angles) // H2
		{
			reset_input_angles = false;

			VectorClear(cl.inputangles);
			VectorClear(cl.delta_inputangles);
			VectorClear(cl.viewangles);
		}
	}

	CL_FinishMove(&cmd);

	old_sys_frame_time = sys_frame_time;

	return cmd;
}

// Builds a command even if not connected.
void CL_SendCmd(void)
{
	// Save this command off for prediction.
	const int index = cls.netchan.outgoing_sequence & (CMD_BACKUP - 1);
	cl.cmd_time[index] = cls.realtime; // For netgraph ping calculation.

	usercmd_t* cmd = &cl.cmds[index];
	*cmd = CL_CreateCmd();
	cl.cmd = *cmd;

	if (cls.state == ca_disconnected || cls.state == ca_connecting)
		return;

	// H2. Skip the rest of the cinematic?
	if (anykeydown > 0 && cl.cinematictime > 0 && !cl.attractloop && cls.realtime - cl.cinematictime > 1000)
		SCR_FinishCinematic();

	if (cls.state == ca_connected)
	{
		if (cls.netchan.message.cursize > 0 || curtime - cls.netchan.last_sent > 1000)
			Netchan_Transmit(&cls.netchan, 0, NULL); // Last arg is buf.data in Q2

		return;
	}

	// Send a userinfo update if needed.
	if (userinfo_modified)
	{
		userinfo_modified = false;
		MSG_WriteByte(&cls.netchan.message, clc_userinfo);
		MSG_WriteString(&cls.netchan.message, Cvar_Userinfo());
	}

	// Init sizebuffer.
	sizebuf_t buf;
	byte data[128];
	SZ_Init(&buf, data, sizeof(data));

	// Begin a client move command
	MSG_WriteByte(&buf, clc_move);

	// Save the position for a checksum byte.
	const int checksumIndex = buf.cursize;
	MSG_WriteByte(&buf, 0);

	// Let the server know what the last frame we got was, so the next message can be delta compressed.
	if ((int)cl_nodelta->value || !cl.frame.valid || cls.demowaiting)
		MSG_WriteLong(&buf, -1); // No compression
	else
		MSG_WriteLong(&buf, cl.frame.serverframe);

	const usercmd_t nullcmd = { 0 };

	// Send this and the previous cmds in the message, so if the last packet was dropped, it can be recovered.
	cmd = &cl.cmds[(cls.netchan.outgoing_sequence - 2) & (CMD_BACKUP - 1)];
	MSG_WriteDeltaUsercmd(&buf, &nullcmd, cmd);
	const usercmd_t* oldcmd = cmd;

	cmd = &cl.cmds[(cls.netchan.outgoing_sequence - 1) & (CMD_BACKUP - 1)];
	MSG_WriteDeltaUsercmd(&buf, oldcmd, cmd);
	oldcmd = cmd;

	cmd = &cl.cmds[(cls.netchan.outgoing_sequence - 0) & (CMD_BACKUP - 1)];
	MSG_WriteDeltaUsercmd(&buf, oldcmd, cmd);

	// Calculate a checksum over the move commands.
	buf.data[checksumIndex] = COM_BlockSequenceCheckByte(&buf.data[checksumIndex + 1], buf.cursize - checksumIndex - 1, cls.netchan.outgoing_sequence);

	// Deliver the message.
	Netchan_Transmit(&cls.netchan, buf.cursize, buf.data);
}

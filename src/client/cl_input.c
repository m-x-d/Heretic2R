//
// cl_input.c -- builds an intended movement command to send to the server
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "Vector.h"

static cvar_t* cl_nodelta;

static uint frame_msec;
static uint old_sys_frame_time;

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

kbutton_t in_klook;
kbutton_t in_strafe;
kbutton_t in_speed;

static kbutton_t in_left;
static kbutton_t in_right;
static kbutton_t in_forward;
static kbutton_t in_back;

static kbutton_t in_lookup;
static kbutton_t in_lookdown;
static kbutton_t in_moveleft;
static kbutton_t in_moveright;

static kbutton_t in_up;
static kbutton_t in_down;

static void IN_UpDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_UpUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_DownDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_DownUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_LeftDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_LeftUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_RightDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_RightUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_ForwardDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_ForwardUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_BackDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_BackUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_LookupDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_LookupUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_LookdownDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_LookdownUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_StrafeDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_StrafeUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_MoveleftDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_MoveleftUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_MoverightDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_MoverightUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_AttackDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_AttackUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_DefendDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_DefendUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_ActionDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_ActionUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_CreepDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_CreepUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_AutoaimDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_AutoaimUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_KLookDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_KLookUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_SpeedDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_SpeedUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_LookaroundDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_LookaroundUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_CommandDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_CommandUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_QuickturnDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_QuickturnUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_InventoryDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_InventoryUp(void)
{
	NOT_IMPLEMENTED
}

void IN_CenterView(void)
{
	NOT_IMPLEMENTED
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

static float CL_KeyState(kbutton_t* key)
{
	NOT_IMPLEMENTED
	return 0;
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
		cmd->angles[i] = (short)(cl.inputangles[i] * 182.0444f);

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

static void CL_FinishMove(usercmd_t* cmd)
{
	NOT_IMPLEMENTED
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
	if (anykeydown && cl.cinematictime > 0 && !cl.attractloop && cls.realtime - cl.cinematictime > 1000)
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

	usercmd_t nullcmd = { 0 };

	// Send this and the previous cmds in the message, so if the last packet was dropped, it can be recovered.
	cmd = &cl.cmds[(cls.netchan.outgoing_sequence - 2) & (CMD_BACKUP - 1)];
	MSG_WriteDeltaUsercmd(&buf, &nullcmd, cmd);
	usercmd_t* oldcmd = cmd;

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

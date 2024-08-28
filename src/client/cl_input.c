//
// cl_input.c -- builds an intended movement command to send to the server
//
// Copyright 1998 Raven Software
//

#include "client.h"

static cvar_t* cl_nodelta;

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

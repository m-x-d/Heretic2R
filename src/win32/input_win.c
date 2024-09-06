//
// input_win.c -- Windows mouse and joystick code
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "sys_win.h"

static qboolean in_appactive;

//TODO: used only in IN_Init(). Can be removed?
static cvar_t* v_centermove;
static cvar_t* v_centerspeed;

cvar_t* in_mouse;
cvar_t* in_joystick;

#pragma region ========================== MOUSE CONTROL ==========================

// Mouse variables
cvar_t * m_filter;

static int mouse_buttons;
static int mouse_oldbuttonstate;

static qboolean mouseactive; // False when not focus app

static qboolean mouseinitialized;
static int originalmouseparms[3] = { 0, 0, 1 };
static qboolean mouseparmsvalid;

static void IN_MLookDown(void)
{
	NOT_IMPLEMENTED
}

static void IN_MLookUp(void)
{
	NOT_IMPLEMENTED
}

static void IN_InitMouse(void)
{
	m_filter = Cvar_Get("m_filter", "0", 0);
	in_mouse = Cvar_Get("in_mouse", "1", CVAR_ARCHIVE);

	Cmd_AddCommand("+mlook", IN_MLookDown);
	Cmd_AddCommand("-mlook", IN_MLookUp);
}

void IN_DeactivateMouse(void)
{
	NOT_IMPLEMENTED
}

static void IN_StartupMouse(void)
{
	if (COM_CheckParm("-nomouse")) // H2
		return;

	const cvar_t* cv = Cvar_Get("in_initmouse", "1", CVAR_NOSET);
	if ((int)cv->value)
	{
		mouseinitialized = true;
		mouseparmsvalid = SystemParametersInfo(SPI_GETMOUSE, 0, originalmouseparms, 0);
		mouse_buttons = 3;
	}
}

// Q2 counterpart.
void IN_MouseEvent(const int mstate)
{
	if (!mouseinitialized)
		return;

	// Perform button actions.
	for (int i = 0; i < mouse_buttons; i++)
	{
		const int flag = (1 << i);
		const qboolean is_pressed = (mstate & flag);
		const qboolean was_pressed = (mouse_oldbuttonstate & flag);

		if (is_pressed != was_pressed)
			Key_Event(K_MOUSE1 + i, is_pressed, sys_msg_time);
	}

	mouse_oldbuttonstate = mstate;
}

#pragma endregion

#pragma region ========================== JOYSTICK CONTROL ==========================

static void IN_InitJoystick(void)
{
	in_joystick = Cvar_Get("in_joystick", "0", CVAR_ARCHIVE);

	//TODO: skipped for now. Add gamepad support later.
}

void IN_StartupJoystick(void)
{
	//TODO: skipped for now. Add gamepad support later.
}

#pragma endregion

void IN_Init(void)
{
	IN_InitMouse();
	IN_InitJoystick();

	// Centering
	v_centermove = Cvar_Get("v_centermove", "0.15", 0);
	v_centerspeed = Cvar_Get("v_centerspeed", "500", 0);

	IN_StartupMouse();
	IN_StartupJoystick();
}

// Called when the main window gains or loses focus.
// The window may have been destroyed and recreated between a deactivate and an activate.
void IN_Activate(const qboolean active)
{
	in_appactive = active;

	// H2: new checks, no longer sets 'mouseactive' to true
	if (active || (mouseactive && !mouseinitialized)) 
		mouseactive = false;
}

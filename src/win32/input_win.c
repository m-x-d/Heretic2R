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

static cvar_t* in_mouse;
cvar_t* in_joystick;

#pragma region ========================== MOUSE CONTROL ==========================

// Mouse variables
static cvar_t * m_filter;
static qboolean mlooking;

static int mouse_buttons;
static int mouse_oldbuttonstate;
static POINT current_pos;
static int mouse_x;
static int mouse_y;
static int old_mouse_x;
static int old_mouse_y;

static int old_x;
static int old_y;

static qboolean mouseactive; // False when not focus app

static qboolean restore_spi;
static qboolean mouseinitialized;
static int originalmouseparms[3] = { 0, 0, 1 };
static int newmouseparms[3] = { 0, 0, 1 };
static qboolean mouseparmsvalid;

static int window_center_x;
static int window_center_y;

// Q2 counterpart
static void IN_MLookDown(void)
{
	mlooking = true;
}

// Q2 counterpart
static void IN_MLookUp(void)
{
	mlooking = false;

	if (!(int)freelook->value && (int)lookspring->value)
		IN_CenterView();
}

static void IN_InitMouse(void)
{
	m_filter = Cvar_Get("m_filter", "0", 0);
	in_mouse = Cvar_Get("in_mouse", "1", CVAR_ARCHIVE);

	Cmd_AddCommand("+mlook", IN_MLookDown);
	Cmd_AddCommand("-mlook", IN_MLookUp);
}

// Q2 counterpart
// Called when the window gains focus or changes in some way.
static void IN_ActivateMouse(void)
{
	if (!mouseinitialized)
		return;

	if ((int)in_mouse->value == 0)
	{
		mouseactive = false;
		return;
	}

	if (mouseactive)
		return;

	mouseactive = true;

	if (mouseparmsvalid)
		restore_spi = SystemParametersInfo(SPI_SETMOUSE, 0, newmouseparms, 0);

	const int width = GetSystemMetrics(SM_CXSCREEN);
	const int height = GetSystemMetrics(SM_CYSCREEN);

	RECT window_rect; //mxd. Made local.
	GetWindowRect(cl_hwnd, &window_rect);

	window_rect.left = max(0, window_rect.left);
	window_rect.top = max(0, window_rect.top);
	window_rect.right = min(width - 1, window_rect.right);
	window_rect.bottom = min(height - 1, window_rect.bottom);

	window_center_x = (window_rect.right + window_rect.left) / 2;
	window_center_y = (window_rect.top + window_rect.bottom) / 2;

	SetCursorPos(window_center_x, window_center_y);

	old_x = window_center_x;
	old_y = window_center_y;

	SetCapture(cl_hwnd);
	ClipCursor(&window_rect);

	while (ShowCursor(FALSE) > -1) { }
}

// Called when the window loses focus.
void IN_DeactivateMouse(void)
{
	if (mouseinitialized && mouseactive)
	{
		if (restore_spi)
			SystemParametersInfo(SPI_SETMOUSE, 0, originalmouseparms, 0);

		mouseactive = false;

		ClipCursor(NULL);
		ReleaseCapture();

		while (ShowCursor(TRUE) < 0) { }
	}
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

static void IN_MouseMove(usercmd_t* cmd)
{
	if (!mouseactive)
		return;

	// Find mouse movement
	GetCursorPos(&current_pos);

	const int mx = current_pos.x - window_center_x;
	const int my = current_pos.y - window_center_y;

	// Check #if 0-ed in Q2
	if (mx == 0 && my == 0)
		return;

	if ((int)m_filter->value)
	{
		mouse_x = (mx + old_mouse_x) / 2;
		mouse_y = (my + old_mouse_y) / 2;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_y = my;
	old_mouse_x = mx;

	mouse_x = (int)((float)mouse_x * mouse_sensitivity_x->value); // 'sensitivity' cvar in Q2
	mouse_y = (int)((float)mouse_y * mouse_sensitivity_y->value); // 'sensitivity' cvar in Q2

	// Add mouse X/Y movement to cmd.
	if ((in_strafe.state & 1) || ((int)lookstrafe->value && mlooking)) //TODO: remove 'lookstrafe' cvar, always freelook
		cmd->sidemove += (short)((float)mouse_x * m_side->value);
	else
		cl.delta_inputangles[YAW] -= (float)mouse_x * m_yaw->value;

	if (!(in_strafe.state & 1) || ((int)freelook->value && mlooking)) // H2: no 'else' case //TODO: remove freelook cvar, always freelook
		cl.delta_inputangles[PITCH] += (float)mouse_y * m_pitch->value;

	// Force the mouse to the center, so there's room to move.
	SetCursorPos(window_center_x, window_center_y);
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

void IN_Commands(void) //TODO: rename to IN_JoystickCommands?
{
	//TODO: skipped for now. Add gamepad support later.
}

static void IN_JoyMove(usercmd_t* cmd)
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

// Called every frame, even if not generating commands.
void IN_Frame(void)
{
	if (!mouseinitialized)
		return;

	if (in_mouse == NULL || !in_appactive)
	{
		IN_DeactivateMouse();
		return;
	}

	if (!cl.refresh_prepped || cls.key_dest == key_console || cls.key_dest == key_menu)
	{
		// Temporarily deactivate if in fullscreen.
		if (Cvar_VariableValue("vid_fullscreen") == 0.0f)
		{
			IN_DeactivateMouse();
			return;
		}
	}

	IN_ActivateMouse();
}

// Q2 counterpart
void IN_Move(usercmd_t* cmd)
{
	IN_MouseMove(cmd);
	if (ActiveApp)
		IN_JoyMove(cmd);
}

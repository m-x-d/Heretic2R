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

#pragma region ========================== MOUSE CONTROL ==========================

static qboolean mouseactive; // False when not focus app
static qboolean mouseinitialized;

static void IN_InitMouse(void)
{
	NOT_IMPLEMENTED
}

void IN_DeactivateMouse(void)
{
	NOT_IMPLEMENTED
}

static void IN_StartupMouse(void)
{
	NOT_IMPLEMENTED
}

void IN_MouseEvent(int mstate)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== JOYSTICK CONTROL ==========================

static void IN_InitJoystic(void)
{
	NOT_IMPLEMENTED
}

void IN_StartupJoystick(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

void IN_Init(void)
{
	IN_InitMouse();
	IN_InitJoystic();

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

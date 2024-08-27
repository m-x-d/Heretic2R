//
// input_win.c -- Windows mouse and joystick code
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "sys_win.h"

static qboolean in_appactive;

#pragma region ========================== MOUSE CONTROL ==========================

static qboolean mouseactive; // False when not focus app
static qboolean mouseinitialized;

void IN_MouseEvent(int mstate)
{
	NOT_IMPLEMENTED
}

#pragma endregion

void IN_Init(void)
{
	NOT_IMPLEMENTED
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

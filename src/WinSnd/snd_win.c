//
// snd_win.c
//
// Copyright 1998 Raven Software
//

#include <dsound.h>
#include "snd_win.h"
#include "sys_win.h"

static qboolean snd_isdirect;

static LPDIRECTSOUND pDS;

static qboolean DS_CreateBuffers(void)
{
	NOT_IMPLEMENTED
	return false;
}

static void DS_DestroyBuffers(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart.
// Called when the main window gains or loses focus.
// The window have been destroyed and recreated between a deactivate and an activate.
void S_Activate(const qboolean active)
{
	if (pDS != NULL && cl_hwnd != NULL && snd_isdirect)
	{
		if (active)
			DS_CreateBuffers();
		else
			DS_DestroyBuffers();
	}
}
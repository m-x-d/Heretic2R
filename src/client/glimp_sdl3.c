//
// glimp_sdl3.c --	This is the client side of the render backend, implemented trough SDL.
//					The SDL window and related functions (mouse grab, fullscreen switch) are implemented here,
//					everything else is in the renderers.
//
// Copyright 2025 mxd
//

#include "qcommon.h"
#include "ref.h"
#include <SDL3/SDL.h>

// Initializes the SDL video subsystem. Must be called before anything else.
qboolean GLimp_Init(void)
{
	NOT_IMPLEMENTED
	return false;
}

// Shuts the SDL video subsystem down. Must be called after everything's finished and cleaned up.
void GLimp_Shutdown(void)
{
	NOT_IMPLEMENTED
}

// Shuts the window down.
void GLimp_ShutdownGraphics(void)
{
	NOT_IMPLEMENTED
}
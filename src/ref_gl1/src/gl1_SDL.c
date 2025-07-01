//
// gl1_SDL.c
//
// Copyright 1998 Raven Software
//

#include "gl1_SDL.h"
#include "gl1_Local.h"
#include <SDL3/SDL.h>

static SDL_Window* window = NULL;
static SDL_GLContext context = NULL;

void R_EndFrame(void) //mxd. GLimp_EndFrame in original logic.
{
	NOT_IMPLEMENTED
}

// Returns the address of an OpenGL function.
void* R_GetProcAddress(const char* proc)
{
	return (void(*)(void))SDL_GL_GetProcAddress(proc);
}

int R_PrepareForWindow(void)
{
	NOT_IMPLEMENTED
	return -1;
}

qboolean R_InitContext(void* win)
{
	NOT_IMPLEMENTED
	return false;
}

// Shuts the GL context down.
void R_ShutdownContext(void)
{
	if (window != NULL && context != NULL)
	{
		SDL_GL_DestroyContext(context);
		context = NULL;
	}
}
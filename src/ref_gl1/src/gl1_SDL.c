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

// This function returns the flags used at the SDL window creation by GLimp_InitGraphics().
// In case of error -1 is returned.
int R_PrepareForWindow(void)
{
	// Set GL context attributes bound to the window.
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

	return SDL_WINDOW_OPENGL;
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
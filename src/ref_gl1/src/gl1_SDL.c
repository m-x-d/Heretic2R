//
// gl1_SDL.c
//
// Copyright 1998 Raven Software
//

#include "gl1_SDL.h"
#include "gl1_Local.h"
#include <SDL3/SDL.h>

void R_EndFrame(void) //mxd. GLimp_EndFrame in original logic.
{
	NOT_IMPLEMENTED
}

// Returns the address of an OpenGL function.
void* R_GetProcAddress(const char* proc)
{
	return (void(*)(void))SDL_GL_GetProcAddress(proc);
}

void R_ShutdownContext(void)
{
	NOT_IMPLEMENTED
}
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

// Swaps the buffers and shows the next frame.
void RI_EndFrame(void) //mxd. GLimp_EndFrame in original logic.
{
	SDL_GL_SwapWindow(window);
}

// Returns the address of an OpenGL function.
void* R_GetProcAddress(const char* proc)
{
	return (void(*)(void))SDL_GL_GetProcAddress(proc);
}

// This function returns the flags used at the SDL window creation by GLimp_InitGraphics().
// In case of error -1 is returned.
int RI_PrepareForWindow(void)
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

// Enables or disables the vsync.
void R_SetVsync(void)
{
	// Make sure that the user given value is SDL compatible...
	int vsync = 0;

	if (r_vsync->value == 1.0f)
		vsync = 1;
	else if (r_vsync->value == 2.0f)
		vsync = -1;

	if (!SDL_GL_SetSwapInterval(vsync) && vsync == -1)
	{
		// Not every system supports adaptive VSync, fallback to normal VSync.
		ri.Con_Printf(PRINT_ALL, "Failed to set adaptive VSync, reverting to normal VSync.\n");
		SDL_GL_SetSwapInterval(1);
	}

	if (!SDL_GL_GetSwapInterval(&vsync))
		ri.Con_Printf(PRINT_ALL, "Failed to get VSync state, assuming no VSync.\n");

	//TODO: update r_vsync cvar?
}

// Initializes the OpenGL context.
qboolean RI_InitContext(void* win)
{
	if (win == NULL)
	{
		ri.Sys_Error(ERR_FATAL, "RI_InitContext() called with NULL argument!");
		return false;
	}

	window = (SDL_Window*)win;

	// Initialize GL context.
	context = SDL_GL_CreateContext(window);

	if (context == NULL)
	{
		ri.Con_Printf(PRINT_ALL, "RI_InitContext(): failed to create OpenGL context: %s\n", SDL_GetError());
		window = NULL;

		return false;
	}

	//mxd. Load OpenGL function pointers through GLAD. Must be called after GLimp_Init().
	if (!gladLoadGLLoader(R_GetProcAddress))
	{
		ri.Con_Printf(PRINT_ALL, "RI_InitContext(): failed to initialize OpenGL\n");
		return false;
	}

	//mxd. Check OpenGL version.
	if (!GLAD_GL_VERSION_1_3)
	{
		ri.Con_Printf(PRINT_ALL, "RI_InitContext(): unsupported OpenGL version. Expected 1.3, got %i.%i!\n", GLVersion.major, GLVersion.minor);
		return false;
	}

	R_SetVsync();
	vid_gamma->modified = true; // Force R_UpdateGamma() call in R_BeginFrame().

	return true;
}

// Shuts the GL context down.
void RI_ShutdownContext(void)
{
	if (window != NULL && context != NULL)
	{
		SDL_GL_DestroyContext(context);
		context = NULL;
	}
}
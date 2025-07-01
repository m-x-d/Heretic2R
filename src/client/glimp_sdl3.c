//
// glimp_sdl3.c --	This is the client side of the render backend, implemented trough SDL.
//					The SDL window and related functions (mouse grab, fullscreen switch) are implemented here,
//					everything else is in the renderers.
//
// Copyright 2025 mxd
//

#include "qcommon.h"
#include "ref.h"
#include "vid.h" //mxd
#include <SDL3/SDL.h>

static SDL_Window* window = NULL;

static qboolean InitDisplayModes(void) //mxd
{
	uint cur_display;

	if (window == NULL)
	{
		// Called without a window, list modes from the first display.
		// This is the primary display and likely the one the game will run on.
		cur_display = SDL_GetPrimaryDisplay();
	}
	else
	{
		// Otherwise use the display were the window is displayed.
		// There are some obscure setups were this can fail (one X11 server with several screen is one of these), so add a fallback to the first display.
		cur_display = SDL_GetDisplayForWindow(window);
		if (cur_display == 0)
			cur_display = SDL_GetPrimaryDisplay();
	}

	int num_modes = 0;
	SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(cur_display, &num_modes);

	if (modes != NULL)
	{
		//mxd. Collect available display resolutions, in descending order, starting at desktop resolution and stopping at 640x480.
		// Good thing:	all display modes are guaranteed to be available on this display.
		// Bad thing:	same vid_mode cvar value will result in different resolutions on different PCs / displays...
		//TODO: mode 0 needs to be re-initialized when switching target display.
		viddef_t* valid_modes = malloc(sizeof(viddef_t) * num_modes);
		
		// Mode 0 is desktop resolution.
		const SDL_DisplayMode* desktop_mode = SDL_GetDesktopDisplayMode(cur_display);
		valid_modes->width = desktop_mode->w;
		valid_modes->height = desktop_mode->h;
		int num_valid_modes = 1;

		SDL_Rect safe_bounds;
		if (!SDL_GetDisplayUsableBounds(cur_display, &safe_bounds))
		{
			Com_Printf("Failed get usable display bounds: %s\n", SDL_GetError());
			return false;
		}

		// Add resolutions for windowed modes.
		for (int i = 1; i < num_modes; i++)
		{
			const SDL_DisplayMode* mode = modes[i];

			if (mode->w > safe_bounds.w || mode->h > safe_bounds.h || mode->displayID != cur_display)
				continue;

			if (mode->w < DEF_WIDTH && mode->h < DEF_HEIGHT)
				break;

			// Check if already added. Modes can differ by pixel format, pixel density or refresh rate only...
			qboolean skip_mode = false;
			for (int c = 0; c < num_valid_modes; c++)
			{
				if (valid_modes[c].width == mode->w && valid_modes[c].height == mode->h)
				{
					skip_mode = true; // Already added...
					break;
				}
			}

			if (!skip_mode)
			{
				valid_modes[num_valid_modes].width = mode->w;
				valid_modes[num_valid_modes].height = mode->h;

				num_valid_modes++;
			}
		}

		SDL_free(modes);

		// Need at least 2 modes, one for fullscreen, one for windowed mode...
		if (num_valid_modes < 2)
		{
			Com_Printf("Failed to initialize display modes!\n");
			free(valid_modes);

			return false;
		}

		VID_InitModes(valid_modes, num_valid_modes); // Store in SDL-independent fashion...
		free(valid_modes);

		// List detected modes.
		Com_DPrintf("SDL display modes:\n");
		for (int i = 0; i < num_valid_modes; i++)
			Com_DPrintf(" - Mode %2i: %ix%i\n", i, &valid_modes[i].width, &valid_modes[i].height);

		return true;
	}

	Com_Printf("Couldn't get display modes: %s\n", SDL_GetError());
	return false;
}

// Initializes the SDL video subsystem. Must be called before anything else.
qboolean GLimp_Init(void)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			Com_Printf("Couldn't init SDL video: %s.\n", SDL_GetError());
			return false;
		}

		const int version = SDL_GetVersion();

		Com_Printf("-------- vid initialization --------\n");

		Com_Printf("SDL version is: %i.%i.%i\n", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
		Com_Printf("SDL video driver is \"%s\".\n", SDL_GetCurrentVideoDriver());

		if (!InitDisplayModes()) //mxd
			return false;

		Com_Printf("------------------------------------\n\n");
	}

	return true;
}

// Shuts the SDL video subsystem down. Must be called after everything's finished and cleaned up.
void GLimp_Shutdown(void)
{
	NOT_IMPLEMENTED
}

// (Re)initializes the actual window.
qboolean GLimp_InitGraphics(int* pwidth, int* pheight, qboolean fullscreen)
{
	NOT_IMPLEMENTED
	return false;
}

// Shuts the window down.
void GLimp_ShutdownGraphics(void)
{
	NOT_IMPLEMENTED
}
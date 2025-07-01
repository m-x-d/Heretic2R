//
// glimp_sdl3.c --	This is the client side of the render backend, implemented trough SDL.
//					The SDL window and related functions (mouse grab, fullscreen switch) are implemented here,
//					everything else is in the renderers.
//
// Copyright 2025 mxd
//

#include "qcommon.h"
#include "client.h"
#include <SDL3/SDL.h>


static int last_flags = 0;
static SDL_Window* window = NULL;
static qboolean init_successful = false;

//mxd. Let's not rely on 640x480 video mode availability...
static int safe_mode = -1;
static int safe_width;
static int safe_height;

static qboolean CreateSDLWindow(SDL_WindowFlags flags, qboolean fullscreen, int w, int h)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean IsFullscreen(void)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean GetWindowSize(int* w, int* h)
{
	NOT_IMPLEMENTED
	return false;
}

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

		// List detected modes.
		Com_DPrintf("SDL display modes:\n");
		for (int i = 0; i < num_valid_modes; i++)
			Com_DPrintf(" - Mode %2i: %ix%i\n", i, valid_modes[i].width, valid_modes[i].height);

		// Store our 'safe' video mode...
		safe_mode = num_valid_modes - 1;
		safe_width = valid_modes[safe_mode].width;
		safe_height = valid_modes[safe_mode].height;

		free(valid_modes);

		return true;
	}

	Com_Printf("Couldn't get display modes: %s\n", SDL_GetError());
	return false;
}

// Sets the window icon.
static void SetSDLIcon(void)
{
	NOT_IMPLEMENTED
}

// Shuts the SDL render backend down.
static void ShutdownGraphics(void)
{
	NOT_IMPLEMENTED
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
	int cur_width;
	int cur_height;
	int width = *pwidth;
	int height = *pheight;

	assert(safe_mode > 0);

	const SDL_WindowFlags fs_flag = (fullscreen ? SDL_WINDOW_FULLSCREEN : 0);

	// Only do this if we already have a working window and a fully initialized rendering backend.
	// GLimp_InitGraphics() is also	called when recovering if creating GL context fails or the one we got is unusable.
	if (init_successful && GetWindowSize(&cur_width, &cur_height) && cur_width == width && cur_height == height)
	{
		// If we want fullscreen, but aren't.
		if (IsFullscreen())
		{
			if (!SDL_SetWindowFullscreenMode(window, NULL))
			{
				Com_Printf("Couldn't set fullscreen mode: %s\n", SDL_GetError());
				Cvar_SetValue("vid_fullscreen", 0);
			}
			else if (!SDL_SyncWindow(window))
			{
				Com_Printf("Couldn't synchronize window state: %s\n", SDL_GetError());
				Cvar_SetValue("vid_fullscreen", 0);
			}

			Cvar_SetValue("vid_fullscreen", fullscreen);
		}

		// Are we now?
		if (IsFullscreen())
			return true;
	}

	// Is the surface used?
	if (window != NULL)
	{
		re.ShutdownContext();
		ShutdownGraphics();

		window = NULL;
	}

	if (last_flags != -1 && (last_flags & SDL_WINDOW_OPENGL))
		SDL_GL_ResetAttributes(); // Reset SDL.

	// Let renderer prepare things (set OpenGL attributes).
	// FIXME: This is no longer necessary, the renderer could and should pass the flags when calling this function.
	SDL_WindowFlags flags = re.PrepareForWindow();

	if ((int)flags == -1)
		return false; // It's PrepareForWindow() job to log an error.

	flags |= fs_flag;

	// Now the hard work. Let's create the window.
	while (!CreateSDLWindow(flags, fullscreen, width, height))
	{
		if (width != safe_width || height != safe_height || (flags & fs_flag))
		{
			Com_Printf("SDL SetVideoMode failed: %s\n", SDL_GetError());
			Com_Printf("Reverting to windowed mode %i (%ix%i).\n", safe_mode, safe_width, safe_height);

			// Try to recover.
			Cvar_SetValue("vid_mode", (float)safe_mode);
			Cvar_SetValue("vid_fullscreen", 0);

			width = safe_width;
			height = safe_height;
			*pwidth = width;
			*pheight = height;

			fullscreen = false;
			flags &= ~fs_flag;
		}
		else
		{
			Com_Printf("Failed to revert to windowed mode. Will try another render backend...\n");
			return false;
		}
	}

	last_flags = (int)flags;

	// Initialize rendering context.
	if (!re.InitContext(window))
		return false; // InitContext() should have logged an error.

	// Another bug or design failure in SDL: when we are not high dpi aware, the drawable size returned by SDL may be too small.
	// It seems like the window decoration are taken into account when they shouldn't. It can be seen when creating a fullscreen window.
	// Work around that by always using the resolution and not the drawable size when we are not high dpi aware.
	viddef.width = *pwidth;
	viddef.height = *pheight;

	SetSDLIcon();
	SDL_ShowCursor();
	init_successful = true;

	return true;
}

// Shuts the window down.
void GLimp_ShutdownGraphics(void)
{
	NOT_IMPLEMENTED
}
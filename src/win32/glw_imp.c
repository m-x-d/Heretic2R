//
// glw_imp.c
//
// Copyright 1998 Raven Software
//

#include "../ref_gl/gl_local.h"
#include "glw_win.h"

glwstate_t glw_state;

qboolean VID_CreateWindow(int width, int height, qboolean fullscreen)
{
	NOT_IMPLEMENTED
	return false;
}

// H2: new 'create_window' arg //TODO: 'create_window' is always true. Remove?
rserr_t GLimp_SetMode(int* pwidth, int* pheight, const int mode, const qboolean fullscreen, const qboolean create_window)
{
	int width;
	int height;
	const char* win_fs[] = { "W", "FS" };

	ri.Con_Printf(PRINT_ALL, "Initializing OpenGL display\n");
	ri.Con_Printf(PRINT_ALL, "...setting mode %d:", mode);

	if (!ri.Vid_GetModeInfo(&width, &height, mode))
	{
		ri.Con_Printf(PRINT_ALL, " invalid mode\n");
		return rserr_invalid_mode;
	}

	ri.Con_Printf(PRINT_ALL, " %d %d %s\n", width, height, win_fs[fullscreen]);

	// Destroy the existing window
	if (create_window && glw_state.hWnd)
		GLimp_Shutdown();

	//mxd. Set once instead of setting in every case...
	*pwidth = width;
	*pheight = height;

	// Do a CDS if needed
	if (fullscreen)
	{
		ri.Con_Printf(PRINT_ALL, "...attempting fullscreen\n");

		DEVMODE dm = { 0 };
		dm.dmSize = sizeof(dm);
		dm.dmPelsWidth = width;
		dm.dmPelsHeight = height;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		//mxd. Ignore gl_bitdepth cvar logic (Win7+ can't into 8 and 16-bit color modes)
		const HDC hdc = GetDC(NULL);
		ri.Con_Printf(0, "...using desktop display depth of %d\n", GetDeviceCaps(hdc, BITSPIXEL));
		ReleaseDC(0, hdc);

		ri.Con_Printf(0, "...calling CDS: ");
		if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			gl_state.fullscreen = true;
			ri.Con_Printf(PRINT_ALL, "ok\n");

			if (create_window && !VID_CreateWindow(width, height, true))
				return rserr_invalid_mode;

			return rserr_ok;
		}

		// First CDS failed, so maybe we're running on some weird dual monitor system
		ri.Con_Printf(0, "failed\n");
		ri.Con_Printf(0, "...calling CDS assuming dual monitors:");

		dm.dmPelsWidth = width * 2;
		dm.dmPelsHeight = height;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			ri.Con_Printf(PRINT_ALL, " ok\n");
			if (create_window && !VID_CreateWindow(width, height, true))
				return rserr_invalid_mode;

			gl_state.fullscreen = true;

			return rserr_ok;
		}

		// Second CDS failed, try windowed mode...
		ri.Con_Printf(PRINT_ALL, " failed\n");
		ri.Con_Printf(PRINT_ALL, "...setting windowed mode\n");

		ChangeDisplaySettings(NULL, 0);
		gl_state.fullscreen = false;

		if (create_window && !VID_CreateWindow(width, height, false))
			return rserr_invalid_mode;

		return rserr_invalid_fullscreen;
	}

	// Windowed mode desired
	ri.Con_Printf(PRINT_ALL, "...setting windowed mode\n");

	ChangeDisplaySettings(NULL, 0);
	gl_state.fullscreen = false;

	if (create_window && !VID_CreateWindow(width, height, false))
		return rserr_invalid_mode;

	return rserr_ok;
}

void GLimp_Shutdown(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart (in original .dll). We'll just ASSUME we are running on Win95 OSR2 or better
qboolean GLimp_Init(void* hinstance, void* wndproc)
{
	glw_state.hInstance = (HINSTANCE)hinstance;
	glw_state.wndproc = wndproc;
	glw_state.allowdisplaydepthchange = true; // False on pre-OSR2 Win95

	return true;
}

void GLimp_EndFrame(void)
{
	NOT_IMPLEMENTED
}

void GLimp_AppActivate(qboolean active)
{
	NOT_IMPLEMENTED
}
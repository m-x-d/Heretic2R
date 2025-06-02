//
// glw_imp.c
//
// Copyright 1998 Raven Software
//

#include "../ref_gl/gl_local.h"
#include "../launcher/resource.h"
#include "glw_win.h"

#define WINDOW_CLASS_NAME "Heretic II R" // H2: 'Heretic 2'

glwstate_t glw_state;

static qboolean GLimp_InitGL(void); // VID_CreateWindow forward declaration.

static qboolean VID_CreateWindow(const int width, const int height, const qboolean fullscreen)
{
	WNDCLASS wc;
	RECT r;
	int stylebits;
	int exstyle;
	int x;
	int y;

	// Register the frame class.
	wc.style = CS_OWNDC; // 0 in Q2
	wc.lpfnWndProc = (WNDPROC)glw_state.wndproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = glw_state.hInstance;
	wc.hIcon = LoadIcon(glw_state.hInstance, MAKEINTRESOURCE(IDI_ICON1)); // Q2: 0
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_GRAYTEXT); //mxd. Use GetSysColorBrush().
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS_NAME;

	if (!RegisterClass(&wc))
		ri.Sys_Error(ERR_FATAL, "Couldn't register window class");

	if (fullscreen)
		stylebits = WS_POPUP | WS_VISIBLE | WS_SYSMENU; // Q2: WS_POPUP | WS_VISIBLE
	else
		stylebits = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_GROUP; // Q2: WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE

	// Based on 'fullscreen' var in Q2
	vid_fullscreen = ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	if ((int)vid_fullscreen->value)
		exstyle = WS_EX_TOPMOST | WS_EX_APPWINDOW; // Q2: WS_EX_TOPMOST
	else
		exstyle = WS_EX_APPWINDOW; // Q2: 0

	r.left = 0;
	r.top = 0;
	r.right = width;
	r.bottom = height;

	AdjustWindowRect(&r, stylebits, FALSE);

	const int w = r.right - r.left;
	const int h = r.bottom - r.top;

	if (fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
	{
		const cvar_t* vid_xpos = ri.Cvar_Get("vid_xpos", "0", 0);
		const cvar_t* vid_ypos = ri.Cvar_Get("vid_ypos", "0", 0);
		x = (int)vid_xpos->value;
		y = (int)vid_ypos->value;
	}

	glw_state.hWnd = CreateWindowEx(exstyle, WINDOW_CLASS_NAME, WINDOW_CLASS_NAME, stylebits, x, y, w, h, 0, 0, glw_state.hInstance, 0);
	if (!glw_state.hWnd)
		ri.Sys_Error(ERR_FATAL, "Couldn't create window");

	SetForegroundWindow(glw_state.hWnd);
	SetFocus(glw_state.hWnd);
	Sleep(500);

	// Init all the gl stuff for the window
	if (GLimp_InitGL())
	{
		// Let the sound and input subsystems know about the new window
		ri.Vid_NewWindow(width, height);
		return true;
	}

	ri.Con_Printf(PRINT_ALL, "VID_CreateWindow() - GLimp_InitGL failed\n");
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
		ri.Con_Printf(PRINT_ALL, "...using desktop display depth of %d\n", GetDeviceCaps(hdc, BITSPIXEL));
		ReleaseDC(NULL, hdc);

		ri.Con_Printf(PRINT_ALL, "...calling CDS: ");
		if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			gl_state.fullscreen = true;
			ri.Con_Printf(PRINT_ALL, "ok\n");

			if (create_window && !VID_CreateWindow(width, height, true))
				return rserr_invalid_mode;

			return rserr_ok;
		}

		// First CDS failed, so maybe we're running on some weird dual monitor system
		ri.Con_Printf(PRINT_ALL, "failed\n");
		ri.Con_Printf(PRINT_ALL, "...calling CDS assuming dual monitors:");

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
	if (qwglMakeCurrent != NULL && !qwglMakeCurrent(NULL, NULL))
		ri.Con_Printf(PRINT_ALL, "ref_gl::GLimp_Shutdown() - wglMakeCurrent failed\n");

	if (glw_state.hGLRC != NULL)
	{
		if (qwglDeleteContext != NULL && !qwglDeleteContext(glw_state.hGLRC))
			ri.Con_Printf(PRINT_ALL, "ref_gl::GLimp_Shutdown() - wglDeleteContext failed\n");

		glw_state.hGLRC = NULL;
	}

	if (glw_state.hDC != NULL)
	{
		if (!ReleaseDC(glw_state.hWnd, glw_state.hDC))
			ri.Con_Printf(PRINT_ALL, "ref_gl::GLimp_Shutdown() - ReleaseDC failed\n");

		glw_state.hDC = NULL;
	}

	if (glw_state.hWnd != NULL)
	{
		DestroyWindow(glw_state.hWnd);
		glw_state.hWnd = NULL;
	}

	if (glw_state.log_fp != NULL)
	{
		fclose(glw_state.log_fp);
		glw_state.log_fp = NULL;
	}

	UnregisterClass(WINDOW_CLASS_NAME, glw_state.hInstance);

	if (gl_state.fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);
		gl_state.fullscreen = false;
	}
}

// Q2 counterpart (in original .dll). We'll just ASSUME we are running on Win95 OSR2 or better
qboolean GLimp_Init(void* hinstance, void* wndproc)
{
	glw_state.hInstance = (HINSTANCE)hinstance;
	glw_state.wndproc = wndproc;
	glw_state.allowdisplaydepthchange = true; // False on pre-OSR2 Win95

	return true;
}

//mxd. To avoid label jumps...
static void ReleaseContexts(void)
{
	if (glw_state.hGLRC)
	{
		qwglDeleteContext(glw_state.hGLRC);
		glw_state.hGLRC = NULL;
	}

	if (glw_state.hDC)
	{
		ReleaseDC(glw_state.hWnd, glw_state.hDC);
		glw_state.hDC = NULL;
	}
}

//mxd. Original H2 logic does extra strlwr on gl_driver->string before calling strstr.
static qboolean GLimp_InitGL(void)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size of this pfd.
		1,								// Version number.
		PFD_DRAW_TO_WINDOW |			// Support window.
		PFD_SUPPORT_OPENGL |			// Support OpenGL.
		PFD_DOUBLEBUFFER,				// Double buffered.
		PFD_TYPE_RGBA,					// RGBA type.
		24,								// 24-bit color depth.
		0, 0, 0, 0, 0, 0,				// Color bits ignored.
		0,								// No alpha buffer.
		0,								// Shift bit ignored.
		0,								// No accumulation buffer.
		0, 0, 0, 0, 					// Accum bits ignored.
		32,								// 32-bit z-buffer.
		0,								// No stencil buffer.
		0,								// No auxiliary buffer.
		PFD_MAIN_PLANE,					// Main layer.
		0,								// Reserved.
		0, 0, 0							// Layer masks ignored.
	};

	//mxd. Don't set PFD_STEREO.
	gl_state.stereo_enabled = false;

	//mxd. We are not running on a minidriver.
	glw_state.minidriver = false;

	// Get a DC for the specified window.
	if (glw_state.hDC != NULL)
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - non-NULL DC exists\n");

	glw_state.hDC = GetDC(glw_state.hWnd);
	if (glw_state.hDC == NULL)
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - GetDC failed\n");
		return false;
	}

	//mxd. Ignore minidriver logic.
	const int pixelformat = ChoosePixelFormat(glw_state.hDC, &pfd);
	if (pixelformat == 0)
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - ChoosePixelFormat failed\n");
		return false;
	}

	if (!SetPixelFormat(glw_state.hDC, pixelformat, &pfd))
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - SetPixelFormat failed\n");
		return false;
	}

	DescribePixelFormat(glw_state.hDC, pixelformat, sizeof(pfd), &pfd);

	//mxd. Ignore gl_allow_software logic. Add PFD_GENERIC_FORMAT flag check.
	//mxd. These 2 flag checks should be enough to determine whether pixelformat is accelerated.
	if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT))
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - no hardware acceleration detected\n");
		return false;
	}

	// Startup the OpenGL subsystem by creating a context and making it current.
	glw_state.hGLRC = qwglCreateContext(glw_state.hDC);

	if (glw_state.hGLRC == NULL)
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - qwglCreateContext failed\n");
		ReleaseContexts();
		return false;
	}

	if (!qwglMakeCurrent(glw_state.hDC, glw_state.hGLRC))
	{
		ri.Con_Printf(PRINT_ALL, "GLimp_Init() - qwglMakeCurrent failed\n");
		ReleaseContexts();
		return false;
	}

	//mxd. Skip VerifyDriver() logic.

	// Print out PFD specifics.
	ri.Con_Printf(PRINT_ALL, "GL PFD: color(%d-bits) Z(%d-bit)\n", pfd.cColorBits, pfd.cDepthBits);
	return true;
}

// Q2 counterpart (in original H2 .dll)
void GLimp_BeginFrame(const float camera_separation)
{
	//mxd. Skip gl_bitdepth / allowdisplaydepthchange logic.
	//mxd. Skip stereo rendering logic.
	qglDrawBuffer(GL_BACK);
}

void GLimp_EndFrame(void)
{
	//mxd. Missing: qglGetError() logic.

	if (!_stricmp(gl_drawbuffer->string, "GL_BACK") && !qwglSwapBuffers(glw_state.hDC))
		ri.Sys_Error(ERR_FATAL, "GLimp_EndFrame() - SwapBuffers() failed!\n");
}

void GLimp_AppActivate(const qboolean active)
{
	if (active)
	{
		SetForegroundWindow(glw_state.hWnd);
		ShowWindow(glw_state.hWnd, SW_RESTORE);
	}
	else if ((int)vid_fullscreen->value)
	{
		ShowWindow(glw_state.hWnd, SW_MINIMIZE);
	}
	
	disablerendering = !active; // H2
}
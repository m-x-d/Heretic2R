//
// qgl_imp.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

// H2: new 'create_window' arg //TODO: 'create_window' is always true. Remove?
rserr_t GLimp_SetMode(int* pwidth, int* pheight, int mode, qboolean fullscreen, qboolean create_window)
{
	NOT_IMPLEMENTED
	return 0;
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
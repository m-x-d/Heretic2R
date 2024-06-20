//
// glw_win.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include <windows.h>

typedef struct
{
	HINSTANCE hInstance;
	void* wndproc;

	HDC hDC;		// Handle to device context
	HWND hWnd;		// Handle to window
	HGLRC hGLRC;	// Handle to GL rendering context

	HINSTANCE hinstOpenGL;	// HINSTANCE for the OpenGL library

	qboolean minidriver; //TODO: ignored. Remove?
	qboolean allowdisplaydepthchange; //TODO: always true on Win95 OSR2+. Remove?
	qboolean mcd_accelerated; //TODO: ignored. Remove?

	FILE* log_fp;
	qboolean minimized; // NEW!
} glwstate_t;

extern glwstate_t glw_state;
//
// vid_dll.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "vid_dll.h"
#include "menu.h"

// Structure containing functions exported from refresh DLL
refexport_t	re;

static cvar_t* win_noalttab;

// Console variables that we need to access from this module
cvar_t* vid_gamma;
cvar_t* vid_brightness; // New in H2
cvar_t* vid_contrast; // New in H2
cvar_t* vid_ref;	// Name of Refresh DLL loaded
cvar_t* vid_xpos;	// X coordinate of window position
cvar_t* vid_ypos;	// Y coordinate of window position
cvar_t* vid_fullscreen;
cvar_t* vid_mode;

// Global variables used internally by this module
viddef_t viddef; // Global video state; used by other modules

HWND cl_hwnd; // Main window handle for life of program

qboolean vid_restart_required; // New in H2

static void VID_Restart_f(void)
{
	NOT_IMPLEMENTED
}

static void VID_Front_f(void)
{
	NOT_IMPLEMENTED
}

static void VID_ShowModes_f(void)
{
	NOT_IMPLEMENTED
}

void VID_CheckChanges(void)
{
	NOT_IMPLEMENTED
}

void VID_Init(void)
{
	vid_restart_required = true; // New in H2

	// Create the video variables so we know how to start the graphics drivers
	vid_ref = Cvar_Get("vid_ref", "gl", CVAR_ARCHIVE);
	vid_xpos = Cvar_Get("vid_xpos", "0", CVAR_ARCHIVE);
	vid_ypos = Cvar_Get("vid_ypos", "0", CVAR_ARCHIVE);
	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get("vid_gamma", "0.5", CVAR_ARCHIVE);
	vid_brightness = Cvar_Get("vid_brightness", "0.5", CVAR_ARCHIVE); // New in H2
	vid_contrast = Cvar_Get("vid_contrast", "0.5", CVAR_ARCHIVE); // New in H2
	win_noalttab = Cvar_Get("win_noalttab", "0", CVAR_ARCHIVE);

	// Add some console commands that we want to handle
	Cmd_AddCommand("vid_restart", VID_Restart_f);
	Cmd_AddCommand("vid_front", VID_Front_f);
	Cmd_AddCommand("vid_showmodes", VID_ShowModes_f); // New in H2

	//mxd. Skip 'Disable the 3Dfx splash screen' logic.

	// New in H2
	VID_PreMenuInit();

	// Start the graphics mode and load refresh DLL
	VID_CheckChanges();
}
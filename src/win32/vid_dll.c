//
// vid_dll.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_skeletons.h"
#include "vid_dll.h"
#include "clfx_dll.h"
#include "snd_dll.h"
#include "sound.h"
#include "menu.h"

// Structure containing functions exported from refresh DLL
refexport_t re;

static cvar_t* win_noalttab;
static qboolean s_alttab_disabled;

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
static HINSTANCE reflib_library; // Handle to refresh DLL 
static qboolean reflib_active = false;

HWND cl_hwnd; // Main window handle for life of program

qboolean vid_restart_required; // New in H2

// Q2 counterpart
static void WIN_DisableAltTab(void)
{
	if (!s_alttab_disabled)
	{
		//mxd. Skip s_win95 logic
		RegisterHotKey(NULL, 0, MOD_ALT, VK_TAB);
		RegisterHotKey(NULL, 1, MOD_ALT, VK_RETURN);
		s_alttab_disabled = true;
	}
}

// Q2 counterpart
static void WIN_EnableAltTab(void)
{
	if (s_alttab_disabled)
	{
		//mxd. Skip s_win95 logic
		UnregisterHotKey(NULL, 0);
		UnregisterHotKey(NULL, 1);
		s_alttab_disabled = false;
	}
}

static LONG WINAPI MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NOT_IMPLEMENTED
	return 0;
}

#pragma region ========================== DLL GLUE ==========================

static void VID_Printf(int print_level, char* fmt, ...)
{
	NOT_IMPLEMENTED
}

static void VID_Error(int err_level, char* fmt, ...)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== H2 SCREEN FLASH ==========================

void Activate_Screen_Flash(int color)
{
	NOT_IMPLEMENTED
}

// Screen flash unset
void Deactivate_Screen_Flash(void)
{
	NOT_IMPLEMENTED
}

// Return screen flash value
int Is_Screen_Flashing(void)
{
	NOT_IMPLEMENTED
	return 0;
}

#pragma endregion

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

static qboolean VID_GetModeInfo(int* width, int* height, int mode)
{
	NOT_IMPLEMENTED
	return false;
}

static void VID_UpdateWindowPosAndSize(int x, int y, int width, int height)
{
	NOT_IMPLEMENTED
}

static void VID_NewWindow(int width, int height)
{
	NOT_IMPLEMENTED
}

static void VID_FreeReflib(void)
{
	NOT_IMPLEMENTED
}

static qboolean VID_LoadRefresh(const char* name)
{
	refimport_t ri;

	if (reflib_active)
	{
		re.Shutdown();
		VID_FreeReflib();
	}

	Com_ColourPrintf(P_HEADER, "------- Loading %s -------\n", name); // Q2: Com_Printf

	reflib_library = LoadLibrary(name);
	if (reflib_library == NULL)
	{
		Com_Printf("LoadLibrary(\"%s\") failed\n", name);
		return false;
	}

	ri.Sys_Error = VID_Error;
	ri.Com_Error = Com_Error;
	ri.Con_Printf = VID_Printf;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_FullSet = Cvar_FullSet;
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ExecuteText = Cbuf_ExecuteText;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_Gamedir = FS_Gamedir;
	ri.FS_Userdir = FS_Userdir;
	ri.FS_CreatePath = FS_CreatePath;
	ri.Vid_GetModeInfo = VID_GetModeInfo;
	ri.Vid_MenuInit = VID_MenuInit;
	ri.Vid_NewWindow = VID_NewWindow;
	ri.Is_Screen_Flashing = Is_Screen_Flashing;
	ri.Deactivate_Screen_Flash = Deactivate_Screen_Flash;
	ri.skeletalJoints = skeletal_joints;
	ri.jointNodes = joint_nodes;

	GetRefAPI_t GetRefAPI = (void*)GetProcAddress(reflib_library, "GetRefAPI");
	if (GetRefAPI == NULL)
		Com_Error(ERR_FATAL, "GetProcAddress failed on %s", name);

	re = GetRefAPI(ri);

	if (re.api_version != API_VERSION)
	{
		VID_FreeReflib();
		Com_Error(ERR_FATAL, "%s has incompatible api_version", name);
	}

	if (re.Init(global_hInstance, (void*)MainWndProc) == -1)
	{
		re.Shutdown();
		VID_FreeReflib();

		return false;
	}

#ifdef __A3D_GEOM
	if (A3D_ExportRenderGeom != NULL)
		A3D_ExportRenderGeom(&re);
#endif

	Com_ColourPrintf(P_HEADER, "------------------------------------\n"); // Q2: Com_Printf
	reflib_active = true;

	// Missing: PGM vidref_val logic

	return true;
}

// This function gets called once just before drawing each frame, and it's sole purpose is to check to see
// if any of the video mode parameters have changed, and if they have to update the rendering DLL and/or video mode to match.
void VID_CheckChanges(void)
{
	int height;
	int width;
	char name[100];

	if (win_noalttab->modified)
	{
		if ((int)win_noalttab->value)
			WIN_DisableAltTab();
		else
			WIN_EnableAltTab();

		win_noalttab->modified = false;
	}

	while (vid_restart_required || vid_ref->modified || vid_fullscreen->modified)
	{
		// Refresh has changed
		vid_restart_required = false; // H2

		vid_ref->modified = false;
		vid_fullscreen->modified = true;
		cl.force_refdef = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		if (sound_library != NULL) // H2
			S_StopAllSounds();

		Cvar_SetValue("win_ignore_destroy", true); // H2

		if (Q_stricmp(vid_ref->string, "gl") == 0 && (int)vid_fullscreen->value) // H2
			Cvar_SetValue("win_noalttab", false);

		Com_sprintf(name, sizeof(name), "ref_%s.dll", vid_ref->string);
		if (!VID_LoadRefresh(name))
		{
			if (strcmp(vid_ref->string, "gl") == 0) // Q2: strcmp(vid_ref->string, "soft") //TODO: H2 never switches to software mode?
				Com_Error(ERR_FATAL, "Couldn't fall back to software refresh!");

			Cvar_Set("vid_ref", "gl");

			vid_restart_required = true; // H2

			// Drop the console if we fail to load a refresh
			if (cls.key_dest != key_console)
				Con_ToggleConsole_f();
		}

		Cvar_SetValue("win_ignore_destroy", false); // H2

		if (cl.configstrings[CS_MODELS + 1][0]) // H2
			CLFX_Init();

		cls.disable_screen = false;
	}

	// Update our window position
	if (vid_xpos->modified || vid_ypos->modified)
	{
		VID_GetModeInfo(&width, &height, (int)vid_mode->value); // H2
		VID_UpdateWindowPosAndSize((int)vid_xpos->value, (int)vid_ypos->value, width, height);

		vid_xpos->modified = false;
		vid_ypos->modified = false;
	}
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
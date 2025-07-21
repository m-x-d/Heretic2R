//
// vid_dll.c
//
// Copyright 1998 Raven Software
//

#include "vid_dll.h"
#include "client.h"
#include "cl_skeletons.h"
#include "clfx_dll.h"
#include "glimp_sdl3.h" // YQ2
#include "menus/menu_video.h"

// Structure containing functions exported from refresh DLL.
refexport_t re;

// Console variables that we need to access from this module.
cvar_t* vid_gamma;
cvar_t* vid_brightness; // H2
cvar_t* vid_contrast; // H2
cvar_t* vid_ref; // Name of Refresh DLL loaded.
cvar_t* vid_fullscreen;
cvar_t* vid_mode;

// Global variables used internally by this module.
viddef_t viddef; // Global video state; used by other modules.
static HINSTANCE reflib_library; // Handle to refresh DLL.
static qboolean reflib_active = false;

Q2DLL_DECLSPEC HWND cl_hwnd; // Main window handle for life of program (unused, but still needs to be exported for vanilla mods to work... -- mxd).

qboolean vid_restart_required; // H2

typedef struct vidmode_s
{
	char description[32]; // Q2: char*
	int width;
	int height;
	int mode;
} vidmode_t;

static vidmode_t* vid_modes; //mxd. Static array in Q2 / H2. H2 has no mode 10.
static int num_vid_modes = 0; //mxd

#pragma region ========================== DLL GLUE ==========================

#define MAXPRINTMSG	4096

// Q2 counterpart
void VID_Printf(const int print_level, const char* fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	switch (print_level)
	{
		case PRINT_ALL:
		default: //mxd. Added 'default' case 
			Com_Printf("%s", msg);
			break;

		case PRINT_DEVELOPER:
			Com_DPrintf("%s", msg);
			break;

		case PRINT_ALERT:
			MessageBox(NULL, msg, "PRINT_ALERT", MB_ICONWARNING);
			OutputDebugString(msg);
			break;
	}
}

// Q2 counterpart
H2R_NORETURN void VID_Error(const int err_level, const char* fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	Com_Error(err_level, "%s", msg);
}

#pragma endregion

// Console command to restart the video mode and refresh DLL.
static void VID_Restart_f(void)
{
	vid_restart_required = true; // H2
}

static void VID_ShowModes_f(void) // H2
{
	Com_Printf("-------- Video Modes --------\n");

	for (int i = 0; i < num_vid_modes; i++)
		Com_Printf("%s\n", vid_modes[i].description);

	Com_Printf("-----------------------------\n");
	Com_Printf("Gamma      : %f\n", (double)vid_gamma->value);
	Com_Printf("Brightness : %f\n", (double)vid_brightness->value);
	Com_Printf("Contrast   : %f\n", (double)vid_contrast->value);
	Com_Printf("-----------------------------\n");
}

//mxd
static void VID_ShutdownModes(void)
{
	if (vid_modes != NULL)
	{
		free(vid_modes);
		vid_modes = NULL;
		num_vid_modes = 0;
	}
}

//mxd
void VID_InitModes(viddef_t* modes, const int num_modes)
{
	if (num_vid_modes > 0)
		VID_ShutdownModes();

	vid_modes = malloc(sizeof(vidmode_t) * num_modes);
	num_vid_modes = num_modes;
	
	viddef_t* src_mode = &modes[0];
	vidmode_t* dst_mode = &vid_modes[0];

	// Mode 0 is desktop resolution.
	for (int i = 0; i < num_modes; i++, src_mode++, dst_mode++)
	{
		dst_mode->width = src_mode->width;
		dst_mode->height = src_mode->height;
		dst_mode->mode = i;

		sprintf_s(dst_mode->description, sizeof(dst_mode->description), "Mode %i: %ix%i", i, dst_mode->width, dst_mode->height);
	}
}

static qboolean VID_GetModeInfo(int* width, int* height, const int mode)
{
	if (num_vid_modes < 1) //mxd. Add sanity check.
		Com_Error(ERR_FATAL, "VID_GetModeInfo() called before VID_InitModes()!");

	//TODO: we can now get a mode outside of expected range when using .cfg from different PC or when switching to different display.
	//TODO: clamp mode, return it instead of qboolean?..
	if (mode >= 0 && mode < num_vid_modes)
	{
		*width =  vid_modes[mode].width;
		*height = vid_modes[mode].height;

		return true;
	}

	return false;
}

// Q2 counterpart
static void VID_NewWindow(const int width, const int height)
{
	viddef.width = width;
	viddef.height = height;
	cl.force_refdef = true; // Can't use a paused refdef
}

// Shuts the renderer down and unloads it.
static void VID_ShutdownRenderer(void) // YQ2
{
	if (reflib_active)
	{
		re.Shutdown();
		GLimp_ShutdownGraphics();

		if (!FreeLibrary(reflib_library)) //TODO: replace with YQ2 Sys_FreeLibrary()?
			Com_Error(ERR_FATAL, "Reflib FreeLibrary failed");

		reflib_library = NULL;
		memset(&re, 0, sizeof(re));

		reflib_active = false;
	}
}

static qboolean VID_LoadRefresh(const char* name)
{
	refimport_t ri;

	// If the refresher is already active, we need to shut it down before loading a new one.
	VID_ShutdownRenderer(); // YQ2

	Com_ColourPrintf(P_HEADER, "------- Loading %s -------\n", name); // Q2: Com_Printf

	reflib_library = LoadLibrary(name); //TODO: replace with YQ2 Sys_LoadLibrary()?
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
	ri.Cmd_ExecuteText = Cbuf_ExecuteText; //TODO: unused?
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_Gamedir = FS_Gamedir;
	ri.FS_Userdir = FS_Userdir;
	ri.FS_CreatePath = FS_CreatePath;
	ri.Vid_GetModeInfo = VID_GetModeInfo;
	ri.Vid_MenuInit = VID_MenuInit;
	ri.Vid_NewWindow = VID_NewWindow;
	ri.GLimp_InitGraphics = GLimp_InitGraphics; // YQ2
	ri.Is_Screen_Flashing = Is_Screen_Flashing;
	ri.Deactivate_Screen_Flash = Deactivate_Screen_Flash;
	ri.skeletalJoints = skeletal_joints;
	ri.jointNodes = joint_nodes;

#ifdef _DEBUG
	ri.pv = pv;
	ri.psv = psv;

	ri.DBG_IDEPrint = DBG_IDEPrint;
	ri.DBG_HudPrint = DBG_HudPrint;
#endif

	const GetRefAPI_t GetRefAPI = (void*)GetProcAddress(reflib_library, "GetRefAPI");
	if (GetRefAPI == NULL)
		Com_Error(ERR_FATAL, "GetProcAddress failed on '%s'", name);

	re = GetRefAPI(ri);

	if (re.api_version != REF_API_VERSION)
	{
		Com_Printf("'%s' has incompatible api_version %i!\n", name, re.api_version); // H2 uses Com_Error() here.
		VID_ShutdownRenderer(); // YQ2

		return false;
	}

	if (!re.Init())
	{
		Com_Printf("Failed to initialize '%s' as rendering backend!\n", name);
		VID_ShutdownRenderer(); // YQ2

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
void VID_CheckChanges(void) //TODO: check YQ2 logic.
{
	while (vid_restart_required || vid_ref->modified || vid_fullscreen->modified)
	{
		// Refresh has changed.
		vid_restart_required = false; // H2

		vid_ref->modified = false;
		vid_fullscreen->modified = true;
		cl.force_refdef = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		se.StopAllSounds();

		char ref_name[100];
		Com_sprintf(ref_name, sizeof(ref_name), "ref_%s.dll", vid_ref->string);

		if (!VID_LoadRefresh(ref_name))
		{
			if (strcmp(vid_ref->string, "soft") == 0) // H2_1.07: "soft" -> "gl"
				Com_Error(ERR_FATAL, "Couldn't fall back to software refresh!");

			Cvar_Set("vid_ref", "soft"); // H2_1.07: "soft" -> "gl"

			vid_restart_required = true; // H2

			// Drop the console if we fail to load a refresh
			if (cls.key_dest != key_console)
				Con_ToggleConsole_f();
		}

		if (cl.configstrings[CS_MODELS + 1][0]) // H2
			CLFX_Init();

		cls.disable_screen = false;
	}
}

void VID_Init(void)
{
	vid_restart_required = true; // H2

	// Create the video variables so we know how to start the graphics drivers.
	vid_ref = Cvar_Get("vid_ref", "gl1", CVAR_ARCHIVE); // H2_1.07: "soft" -> "gl"
	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get("vid_gamma", "0.5", CVAR_ARCHIVE);
	vid_brightness = Cvar_Get("vid_brightness", "0.5", CVAR_ARCHIVE); // H2
	vid_contrast = Cvar_Get("vid_contrast", "0.5", CVAR_ARCHIVE); // H2

	// Add some console commands that we want to handle.
	Cmd_AddCommand("vid_restart", VID_Restart_f);
	Cmd_AddCommand("vid_showmodes", VID_ShowModes_f); // H2

	// YQ2. Initializes the video backend. This is NOT the renderer itself, just the client side support stuff!
	if (!GLimp_Init())
		Com_Error(ERR_FATAL, "Couldn't initialize the graphics subsystem!\n");

	VID_PreMenuInit(); // H2

	// Start the graphics mode and load refresh DLL.
	VID_CheckChanges();
}

void VID_Shutdown(void)
{
	VID_ShutdownRenderer(); // YQ2
	GLimp_Shutdown(); // YQ2
	VID_ShutdownModes(); //mxd
}
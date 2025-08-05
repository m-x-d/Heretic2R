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

//mxd. Compatible ref_xxx.dlls info. For use in menu_video.c.
reflib_info_t reflib_infos[MAX_REFLIBS];
int num_reflib_infos = 0;

// Console variables that we need to access from this module.
cvar_t* vid_gamma;
cvar_t* vid_brightness; // H2
cvar_t* vid_contrast; // H2
cvar_t* vid_textures_refresh_required; //mxd

cvar_t* vid_ref; // Name of Refresh DLL loaded.
cvar_t* vid_mode;

// Global variables used internally by this module.
viddef_t viddef; // Global video state; used by other modules.
static HINSTANCE reflib_library; // Handle to refresh DLL.
static qboolean reflib_active = false;

Q2DLL_DECLSPEC HWND cl_hwnd; // Main window handle for life of program (unused, but still needs to be exported for vanilla mods to work... -- mxd).

qboolean vid_restart_required; // H2

vidmode_t* vid_modes; //mxd. Static array in Q2 / H2. H2 has no mode 10.
int num_vid_modes = 0; //mxd

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
		Com_Printf("Mode %*i: %ix%i\n", 2, vid_modes[i].mode, vid_modes[i].width, vid_modes[i].height);

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

		if (dst_mode->mode == 0)
			sprintf_s(dst_mode->description, sizeof(dst_mode->description), "Desktop");
		else
			sprintf_s(dst_mode->description, sizeof(dst_mode->description), "%ix%i", dst_mode->width, dst_mode->height);
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
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.Vid_GetModeInfo = VID_GetModeInfo;
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

static qboolean VID_StroreReflibInfo(const char* ref_path) //mxd
{
	// Try loading it...
	const HINSTANCE reflib = LoadLibrary(ref_path); //TODO: replace with YQ2 Sys_LoadLibrary()?
	if (reflib == NULL)
		return false;

	const GetRefAPI_t GetRefAPI = (void*)GetProcAddress(reflib, "GetRefAPI");
	if (GetRefAPI == NULL)
	{
		FreeLibrary(reflib);
		return false;
	}

	const refimport_t ref_import = { 0 }; // Assume GetRefAPI() doesn't use ref_import function pointers... 
	const refexport_t ref_export = GetRefAPI(ref_import);

	if (ref_export.api_version != REF_API_VERSION || ref_export.title == NULL)
	{
		FreeLibrary(reflib);
		return false;
	}

	const char* start = strchr(ref_path, '_');
	const char* end = strrchr(ref_path, '.');
	const qboolean is_valid = (start != NULL && end != NULL);

	if (is_valid)
	{
		// Seems valid. Store info...
		reflib_info_t* info = &reflib_infos[num_reflib_infos];

		strcpy_s(info->title, sizeof(info->title), ref_export.title);
		strncpy_s(info->id, sizeof(info->id), start + 1, end - start - 1); // Strip "ref_" and ".dll" parts...
	}

	FreeLibrary(reflib);

	return is_valid;
}

static void VID_InitReflibInfos(void) //mxd
{
	num_reflib_infos = 0;

	// Find all compatible ref_xxx.dll libraries.
	char mask[MAX_QPATH];
	Com_sprintf(mask, sizeof(mask), "ref_*.dll");

	const char* ref_path = Sys_FindFirst(mask, 0, 0);
	
	while (ref_path != NULL && num_reflib_infos < MAX_REFLIBS)
	{
		const char* path = strchr(ref_path, '/') + 1; // Skip starting '/'...
		if (VID_StroreReflibInfo(path != NULL ? path : ref_path))
			num_reflib_infos++;

		ref_path = Sys_FindNext(0, 0);
	}

	Sys_FindClose();
}

// This function gets called once just before drawing each frame, and it's sole purpose is to check to see
// if any of the video mode parameters have changed, and if they have to update the rendering DLL and/or video mode to match.
void VID_CheckChanges(void) //TODO: check YQ2 logic.
{
	while (vid_restart_required || vid_ref->modified)
	{
		// Refresh has changed.
		vid_restart_required = false; // H2
		vid_ref->modified = false;

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
		SCR_UpdateUIScale(); //mxd
	}
}

void VID_Init(void)
{
	vid_restart_required = true; // H2

	// Create the video variables so we know how to start the graphics drivers.
	vid_ref = Cvar_Get("vid_ref", "gl1", CVAR_ARCHIVE); // H2_1.07: "soft" -> "gl"
	vid_gamma = Cvar_Get("vid_gamma", "0.5", CVAR_ARCHIVE);
	vid_brightness = Cvar_Get("vid_brightness", "0.5", CVAR_ARCHIVE); // H2
	vid_contrast = Cvar_Get("vid_contrast", "0.5", CVAR_ARCHIVE); // H2
	vid_textures_refresh_required = Cvar_Get("vid_textures_refresh_required", "0", 0); //mxd

	// Add some console commands that we want to handle.
	Cmd_AddCommand("vid_restart", VID_Restart_f);
	Cmd_AddCommand("vid_showmodes", VID_ShowModes_f); // H2

	// YQ2. Initializes the video backend. This is NOT the renderer itself, just the client side support stuff!
	if (!GLimp_Init())
		Com_Error(ERR_FATAL, "Couldn't initialize the graphics subsystem!\n");

	VID_InitReflibInfos(); //mxd
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
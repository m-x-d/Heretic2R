//
// vid_dll.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_skeletons.h"
#include "clfx_dll.h"
#include "vid_dll.h"
#include "sys_win.h"
#include "menus/menu_video.h"

// Structure containing functions exported from refresh DLL
refexport_t re;

cvar_t* win_noalttab;
static qboolean s_alttab_disabled;

// Console variables that we need to access from this module
cvar_t* vid_gamma;
cvar_t* vid_brightness; // H2
cvar_t* vid_contrast; // H2
cvar_t* vid_ref;			// Name of Refresh DLL loaded
static cvar_t* vid_xpos;	// X coordinate of window position
static cvar_t* vid_ypos;	// Y coordinate of window position
cvar_t* vid_fullscreen;
cvar_t* vid_mode;

// Global variables used internally by this module
viddef_t viddef; // Global video state; used by other modules
static HINSTANCE reflib_library; // Handle to refresh DLL 
static qboolean reflib_active = false;

HWND cl_hwnd; // Main window handle for life of program

qboolean vid_restart_required; // H2

#define VID_NUM_MODES	(sizeof(vid_modes) / sizeof(vid_modes[0]))

typedef struct vidmode_s
{
	const char* description;
	int width;
	int height;
	int mode;
} vidmode_t;

static vidmode_t vid_modes[] =
{
	{ "Mode 0: 320x240",	320,	240,	0 },
	{ "Mode 1: 400x300",	400,	300,	1 },
	{ "Mode 2: 512x384",	512,	384,	2 },
	{ "Mode 3: 640x480",	640,	480,	3 },
	{ "Mode 4: 800x600",	800,	600,	4 },
	{ "Mode 5: 960x720",	960,	720,	5 },
	{ "Mode 6: 1024x768",	1024,	768,	6 },
	{ "Mode 7: 1152x864",	1152,	864,	7 },
	{ "Mode 8: 1280x960",	1280,	960,	8 },
	{ "Mode 9: 1600x1200",	1600,	1200,	9 }
}; //mxd. H2 has no mode 10

static byte scantokey[128] =
{
//	0				1		2			3				4		5				6				7 
//	8				9		A			B				C		D				E				F 
	0,				27,		'1',		'2',			'3',	'4',			'5',			'6',
	'7',			'8',	'9',		'0',			'-',	'=',			K_BACKSPACE,	9,		// 0 
	'q',			'w',	'e',		'r',			't',	'y',			'u',			'i',
	'o',			'p',	'[',		']',			13 ,	K_CTRL,			'a',			's',	// 1 
	'd',			'f',	'g',		'h',			'j',	'k',			'l',			';',
	'\'',			'`',	K_SHIFT,	'\\',			'z',	'x',			'c',			'v',	// 2 
	'b',			'n',	'm',		',',			'.',	'/',			K_SHIFT,		'*',
	K_ALT,			' ',	0,			K_F1,			K_F2,	K_F3,			K_F4,			K_F5,	// 3 
	K_F6,			K_F7,	K_F8,		K_F9,			K_F10,	K_PAUSE,		0,				K_HOME,
	K_UPARROW,		K_PGUP,	K_KP_MINUS,	K_LEFTARROW,	K_KP_5,	K_RIGHTARROW,	K_KP_PLUS,		K_END,	// 4 
	K_DOWNARROW,	K_PGDN,	K_INS,		K_DEL,			0,		0,				0,				K_F11,
	K_F12,			0,		0,			0,				0,		0,				0,				0,		// 5
	0,				0,		0,			0,				0,		0,				0,				0,
	0,				0,		0,			0,				0,		0,				0,				0,		// 6 
	0,				0,		0,			0,				0,		0,				0,				0,
	0,				0,		0,			0,				0,		0,				0,				0		// 7 
};

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

//mxd
static void WIN_SetAltTabState(const qboolean disable)
{
	if (disable)
		WIN_DisableAltTab();
	else
		WIN_EnableAltTab();
}

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

#pragma region ========================== WND PROC ==========================

// Map from windows to quake keynums.
static int MapKey(const int key)
{
	const int modified = (key >> 16) & 255;
	if (modified > 127)
		return 0;

	const qboolean is_extended = key & (1 << 24);
	const int result = scantokey[modified];

	if (!is_extended)
	{
		switch (result)
		{
			case K_UPARROW:
				return K_KP_UPARROW;

			case K_DOWNARROW:
				return K_KP_DOWNARROW;

			case K_LEFTARROW:
				return K_KP_LEFTARROW;

			case K_RIGHTARROW:
				return K_KP_RIGHTARROW;

			case K_INS:
				return K_KP_INS;

			case K_DEL:
				return K_KP_DEL;

			case K_PGDN:
				return K_KP_PGDN;

			case K_PGUP:
				return K_KP_PGUP;

			case K_HOME:
				return K_KP_HOME;

			case K_END:
				return K_KP_END;

			default:
				return result;
		}
	}
	else
	{
		switch (result)
		{
			case K_ENTER:
				return K_KP_ENTER;

			case K_SLASH:
				return K_KP_SLASH;

			case K_KP_NUMLOCK:
				return K_KP_PLUS;

			case K_PAUSE: // H2
				return K_KP_NUMLOCK;

			default:
				return result;
		}
	}
}

static void AppActivate(const BOOL fActive, const BOOL minimize)
{
	Minimized = minimize;

	Key_ClearStates();

	// We don't want to act like we're active if we're minimized.
	ActiveApp = (fActive && !Minimized);

	// Minimize/restore mouse-capture on demand.
	IN_Activate(ActiveApp);
	se.Activate(ActiveApp); //mxd. Also activates music backend.

	if ((int)win_noalttab->value)
		WIN_SetAltTabState(ActiveApp); //mxd
}

// Main window procedure
static LONG WINAPI MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static uint MSH_MOUSEWHEEL;

	if (uMsg == MSH_MOUSEWHEEL) //TODO: do we still need this logic? Should be handled by WM_MOUSEWHEEL on Win98+
	{
		const int key = ((int)wParam > 0 ? K_MWHEELUP : K_MWHEELDOWN);
		Key_Event(key, true, sys_msg_time);
		Key_Event(key, false, sys_msg_time);

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
		// This chunk of code theoretically only works under NT4 and Win98 since this message doesn't exist under Win95.
		case WM_MOUSEWHEEL:
		{
			const int key = ((short)HIWORD(wParam) > 0 ? K_MWHEELUP : K_MWHEELDOWN);
			Key_Event(key, true, sys_msg_time);
			Key_Event(key, false, sys_msg_time);
		} break;

		case WM_HOTKEY:
			return 0;

		case WM_CREATE:
			cl_hwnd = hWnd;
			MSH_MOUSEWHEEL = RegisterWindowMessage("MSWHEEL_ROLLMSG");
			break;

		case WM_PAINT:
			SCR_DirtyScreen(); // Force entire screen to update next frame
			break;

		case WM_DESTROY:
			// Let sound and input know about this?
			if (cl_hwnd != NULL && Cvar_VariableValue("win_ignore_destroy") == 0.0f) // Changed in H2
			{
				cl_hwnd = NULL;
				Com_Quit();
			}
			break;

		case WM_ACTIVATE:
		{
			// KJB: Watch this for problems in fullscreen modes with Alt-tabbing.
			const int fActive = LOWORD(wParam);
			const int fMinimized = HIWORD(wParam);

			AppActivate(fActive != WA_INACTIVE, fMinimized);

			//if (reflib_active)
				//re.AppActivate(fActive != WA_INACTIVE);

			cls.disable_screen = (fActive == WA_INACTIVE); // H2
		} break;

		case WM_MOVE:
			if ((int)vid_fullscreen->value == 0)
			{
				const int xPos = (short)LOWORD(lParam); // Horizontal position 
				const int yPos = (short)HIWORD(lParam); // Vertical position 

				RECT r = { 0, 0, 1, 1 };
				const int style = GetWindowLong(hWnd, GWL_STYLE);
				AdjustWindowRect(&r, style, FALSE);

				Cvar_SetValue("vid_xpos", (float)(xPos + r.left));
				Cvar_SetValue("vid_ypos", (float)(yPos + r.top));
				vid_xpos->modified = false;
				vid_ypos->modified = false;

				if (ActiveApp)
					IN_Activate(true);
			}
			break;

		// This is complicated because Win32 seems to pack multiple mouse events into one update sometimes,
		// so we always check all states and look for events.
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
		{
			int temp = 0;

			if (wParam & MK_LBUTTON)
				temp |= 1;

			if (wParam & MK_RBUTTON)
				temp |= 2;

			if (wParam & MK_MBUTTON)
				temp |= 4;

			IN_MouseEvent(temp);
		} break;

		case WM_SYSCOMMAND:
			if (wParam == SC_SCREENSAVE)
				return 0;
			break;

		case WM_SYSKEYDOWN:
			if (wParam == 13)
			{
				if (vid_fullscreen != NULL)
					Cvar_SetValue("vid_fullscreen", (vid_fullscreen->value == 0.0f ? 1.0f : 0.0f));
				return 0;
			}
		// Intentional fallthrough
		case WM_KEYDOWN:
			Key_Event(MapKey(lParam), true, sys_msg_time);
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			Key_Event(MapKey(lParam), false, sys_msg_time);
			break;

		//mxd. Skip MM_MCINOTIFY / CDAudio_MessageHandler logic.

		default:
			break;
	}

	// Pass all unhandled messages to DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#pragma endregion

// Console command to restart the video mode and refresh DLL.
static void VID_Restart_f(void)
{
	vid_restart_required = true; // H2
}

// Q2 counterpart
static void VID_Front_f(void)
{
	SetWindowLong(cl_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
	SetForegroundWindow(cl_hwnd);
}

static void VID_ShowModes_f(void) // H2
{
	Com_Printf("-------- Video Modes --------\n");

	for (uint i = 0; i < VID_NUM_MODES; i++)
		Com_Printf("%s\n", vid_modes[i].description);

	Com_Printf("-----------------------------\n");
	Com_Printf("Gamma      : %f\n", (double)vid_gamma->value);
	Com_Printf("Brightness : %f\n", (double)vid_brightness->value);
	Com_Printf("Contrast   : %f\n", (double)vid_contrast->value);
	Com_Printf("-----------------------------\n");
}

// Q2 counterpart
static qboolean VID_GetModeInfo(int* width, int* height, const int mode)
{
	if (mode >= 0 && mode < (int)VID_NUM_MODES)
	{
		*width =  vid_modes[mode].width;
		*height = vid_modes[mode].height;

		return true;
	}

	return false;
}

static void VID_UpdateWindowPosAndSize(const int x, const int y, const int width, const int height) // H2: extra 'width' and 'height' args. //mxd. Actually use 'x' and 'y' args.
{
	RECT r = { 0, 0, width, height };

	const int style = GetWindowLong(cl_hwnd, GWL_STYLE);
	AdjustWindowRect(&r, style, FALSE);

	const int w = r.right - r.left;
	const int h = r.bottom - r.top;

	MoveWindow(cl_hwnd, x, y, w, h, TRUE);
}

// Q2 counterpart
static void VID_NewWindow(const int width, const int height)
{
	viddef.width = width;
	viddef.height = height;
	cl.force_refdef = true; // Can't use a paused refdef
}

// Q2 counterpart
static void VID_FreeReflib(void)
{
	if (!FreeLibrary(reflib_library))
		Com_Error(ERR_FATAL, "Reflib FreeLibrary failed");

	memset(&re, 0, sizeof(re));
	reflib_library = NULL;
	reflib_active = false;
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
	ri.Cmd_ExecuteText = Cbuf_ExecuteText; //TODO: unused?
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

#ifdef _DEBUG
	ri.pv = pv;
	ri.psv = psv;

	ri.DBG_IDEPrint = DBG_IDEPrint;
	ri.DBG_HudPrint = DBG_HudPrint;
#endif

	const GetRefAPI_t GetRefAPI = (void*)GetProcAddress(reflib_library, "GetRefAPI");
	if (GetRefAPI == NULL)
		Com_Error(ERR_FATAL, "GetProcAddress failed on %s", name);

	re = GetRefAPI(ri);

	if (re.api_version != REF_API_VERSION)
	{
		VID_FreeReflib();
		Com_Error(ERR_FATAL, "%s has incompatible api_version", name);
	}

	if (!re.Init())
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

	if (win_noalttab->modified)
	{
		WIN_SetAltTabState((int)win_noalttab->value);
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

		se.StopAllSounds();

		Cvar_SetValue("win_ignore_destroy", true); // H2

		if (Q_stricmp(vid_ref->string, "soft") == 0 && (int)vid_fullscreen->value) // H2_1.07: "soft" -> "gl"
			Cvar_SetValue("win_noalttab", false);

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
	vid_restart_required = true; // H2

	// Create the video variables so we know how to start the graphics drivers
	vid_ref = Cvar_Get("vid_ref", "gl1", CVAR_ARCHIVE); // H2_1.07: "soft" -> "gl"
	vid_xpos = Cvar_Get("vid_xpos", "0", CVAR_ARCHIVE);
	vid_ypos = Cvar_Get("vid_ypos", "0", CVAR_ARCHIVE);
	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get("vid_gamma", "0.5", CVAR_ARCHIVE);
	vid_brightness = Cvar_Get("vid_brightness", "0.5", CVAR_ARCHIVE); // H2
	vid_contrast = Cvar_Get("vid_contrast", "0.5", CVAR_ARCHIVE); // H2
	win_noalttab = Cvar_Get("win_noalttab", "0", CVAR_ARCHIVE);

	// Add some console commands that we want to handle
	Cmd_AddCommand("vid_restart", VID_Restart_f);
	Cmd_AddCommand("vid_front", VID_Front_f);
	Cmd_AddCommand("vid_showmodes", VID_ShowModes_f); // H2

	//mxd. Skip 'Disable the 3Dfx splash screen' logic.

	VID_PreMenuInit(); // H2

	// Start the graphics mode and load refresh DLL
	VID_CheckChanges();
}

// Q2 counterpart
void VID_Shutdown(void)
{
	if (reflib_active)
	{
		re.Shutdown();
		VID_FreeReflib();
	}
}
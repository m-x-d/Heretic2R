//
// sys_win.c
//
// Copyright 1998 Raven Software
//

#include "Quake2Main.h"
#include <VersionHelpers.h> //mxd
#include "qcommon.h"
#include "q_shared.h"
#include "conproc.h"
#include "sys_win.h" //mxd

qboolean ActiveApp;
qboolean Minimized;

static HANDLE hinput;
static HANDLE houtput;

uint sys_msg_time;
uint sys_frame_time;

#define MAX_NUM_ARGVS	128
int argc;
char* argv[MAX_NUM_ARGVS];

HINSTANCE global_hInstance;

#pragma region ========================== SYSTEM IO ==========================

void Sys_Error(const char* error, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, error);
	vsprintf_s(text, sizeof(text), error, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	CL_Shutdown();

	if (dedicated != NULL && (int)dedicated->value) // H2
		FreeConsole();

	MessageBox(NULL, text, "Error", MB_OK);

	// Shut down QHOST hooks if necessary
	DeinitConProc();

	exit(1);
}

void Sys_Quit(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== DLL HANDLING ==========================

void Sys_LoadGameDll(const char* dll_name, HINSTANCE* hinst, DWORD* checksum)
{
	IMAGE_DOS_HEADER dos_header;
	IMAGE_OPTIONAL_HEADER win_header;
	char name[MAX_OSPATH];
	char dll_path[MAX_OSPATH];
	FILE* f;

	// Run through the search paths
	char* path = NULL;
	while (true)
	{
		path = FS_NextPath(path);
		if (path == NULL)
			break; // Couldn't find one anywhere

		Com_sprintf(name, sizeof(name), "%s/%s", path, dll_name);
		*hinst = LoadLibrary(name);
		if (*hinst != NULL)
		{
			sprintf_s(dll_path, sizeof(dll_path), "%s.dll", name);
			break;
		}
	}

	if (*hinst == NULL)
	{
		Sys_Error("Failed to load %s", dll_name);
		return;
	}

	// Read file checksum //TODO: remove? result never used?
	if (fopen_s(&f, dll_path, "r") == 0)
	{
		fread(&dos_header, sizeof(dos_header), 1, f);
		fseek(f, dos_header.e_lfanew + 24, SEEK_SET); //mxd. Skip PE signature and IMAGE_FILE_HEADER struct.
		fread(&win_header, sizeof(win_header), 1, f);
		fclose(f);

		*checksum = win_header.CheckSum;
	}
	else
	{
		*checksum = 0;
	}

	Com_DPrintf("LoadLibrary (%s)\n", dll_path);
}

void Sys_UnloadGameDll(const char* name, HINSTANCE* hinst)
{
	if (!FreeLibrary(*hinst))
		Sys_Error("Failed to unload %s", name);

	*hinst = NULL;
}

#pragma endregion

void Sys_Init(void)
{
	Set_Com_Printf(Com_Printf); // New in H2
	timeBeginPeriod(1);

	//mxd. Skip ancient OS versions checks.
	if (!IsWindows7OrGreater())
		Sys_Error("Heretic 2 R requires Windows 7 or greater");

	if ((int)dedicated->value)
	{
		if (!AllocConsole())
			Sys_Error("Couldn\'t create dedicated server console");

		hinput = GetStdHandle(STD_INPUT_HANDLE);
		houtput = GetStdHandle(STD_OUTPUT_HANDLE);

		// Let QHOST hook in
		InitConProc(argc, argv);
	}
}

static char console_text[256];
static int console_textlen;

// Q2 counterpart
char* Sys_ConsoleInput(void)
{
	INPUT_RECORD recs[1024];
	DWORD dummy;
	DWORD numread;
	DWORD numevents;

	if (dedicated == NULL || !(int)dedicated->value)
		return NULL;

	while (true)
	{
		if (!GetNumberOfConsoleInputEvents(hinput, &numevents))
			Sys_Error("Error getting # of console events");

		if (numevents < 1)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error("Error reading console input");

		if (numread != 1)
			Sys_Error("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT && !recs[0].Event.KeyEvent.bKeyDown)
		{
			char ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

			switch (ch)
			{
				case '\r':
					WriteFile(houtput, "\r\n", 2, &dummy, NULL);

					if (console_textlen > 0)
					{
						console_text[console_textlen] = 0;
						console_textlen = 0;
						return console_text;
					}
					break;

				case '\b':
					if (console_textlen > 0)
					{
						console_textlen--;
						WriteFile(houtput, "\b \b", 3, &dummy, NULL);
					}
					break;

				default:
					if (ch >= ' ' && console_textlen < (int)sizeof(console_text) - 2)
					{
						WriteFile(houtput, &ch, 1, &dummy, NULL);
						console_text[console_textlen] = ch;
						console_textlen++;
					}
					break;
			}
		}
	}

	return NULL;
}

// Q2 counterpart
void Sys_ConsoleOutput(const char* string)
{
	DWORD dummy;
	char text[256];

	if (!dedicated || !(int)dedicated->value)
		return;

	if (console_textlen > 0)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen + 1] = '\r';
		text[console_textlen + 2] = 0;
		WriteFile(houtput, text, console_textlen + 2, &dummy, NULL);
	}

	WriteFile(houtput, string, strlen(string), &dummy, NULL);

	if (console_textlen > 0)
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
}

// Q2 counterpart
// Send Key_Event calls.
void Sys_SendKeyEvents(void)
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, 0, 0))
			Sys_Quit();

		sys_msg_time = msg.time;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Grab frame time.
	sys_frame_time = timeGetTime();	// FIXME: should this be at start?
}

char* Sys_GetClipboardData(void)
{
	NOT_IMPLEMENTED
	return NULL;
}

void Sys_AppActivate(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
static void ParseCommandLine(LPSTR lpCmdLine)
{
	argc = 1;
	argv[0] = "exe";

	while (*lpCmdLine && argc < MAX_NUM_ARGVS)
	{
		while (*lpCmdLine && (*lpCmdLine <= ' ' || *lpCmdLine > '~'))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && (*lpCmdLine > ' ' && *lpCmdLine <= '~'))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
		}
	}
}

GAME_DECLSPEC int Quake2Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int time;
	int newtime;
	MSG msg;

	// Previous instances do not exist in Win32
	if (hPrevInstance != NULL)
		return 0;

	global_hInstance = hInstance;

	ParseCommandLine(lpCmdLine);

	//mxd. Skip "if we find the CD, add a +set cddir xxx command line" logic
	Qcommon_Init(argc, argv);
	int oldtime = Sys_Milliseconds();

	// Main window message loop
	while (true)
	{
		// If at a full screen console, don't update unless needed
		if (Minimized || (dedicated != NULL && (int)dedicated->value))
			Sleep(1);

		while (PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessageA(&msg, NULL, 0, 0))
				 Com_Quit();

			sys_msg_time = msg.time;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		do
		{
			newtime = Sys_Milliseconds();
			time = newtime - oldtime;
		} while (time < 1);

		// Missing in H2: _controlfp( _PC_24, _MCW_PC );
		Qcommon_Frame(time);
		oldtime = newtime;
	}

	// never gets here
}

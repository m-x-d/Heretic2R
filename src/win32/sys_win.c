//
// sys_win.c
//
// Copyright 1998 Raven Software
//

#include "Quake2Main.h"
#include "qcommon.h"
#include "input.h"

#include <VersionHelpers.h> //mxd

static HANDLE hinput;
static HANDLE houtput;

#define MAX_NUM_ARGVS	128
static int argc;
static char* argv[MAX_NUM_ARGVS];

static char console_text[256];
static int console_textlen;

#pragma region ========================== SYSTEM IO ==========================

H2R_NORETURN void Sys_Error(const char* error, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, error);
	vsprintf_s(text, sizeof(text), error, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	//BUGFIX: mxd. CL_Shutdown() calls Z_FreeTags(0), which frees all cvars and dereferences all pointers to them...
	const qboolean is_dedicated = (dedicated != NULL && (int)dedicated->value);

	CL_Shutdown();

	if (is_dedicated) // H2
		FreeConsole();

	MessageBox(NULL, text, "Error", MB_OK);

	//mxd. Skip QHOST logic.
	exit(1);
}

H2R_NORETURN void Sys_Quit(void)
{
	timeEndPeriod(1);

	//BUGFIX: mxd. CL_Shutdown() calls Z_FreeTags(0), which frees all cvars and dereferences all pointers to them...
	const qboolean is_dedicated = (dedicated != NULL && (int)dedicated->value);

	CL_Shutdown();
	// Missing: CloseHandle (qwclsemaphore);

	if (is_dedicated)
		FreeConsole();

	//mxd. Skip QHOST logic.
	exit(0);
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
		Sys_Error("Failed to load %s", dll_name);

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

	Com_DDPrintf(2, "LoadLibrary (%s)\n", dll_path); //mxd. Com_DPrintf() -> Com_DDPrintf(), to reduce console spam when developer 1.
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
	Set_Com_Printf(Com_Printf); // H2
	timeBeginPeriod(1);

	//mxd. Skip ancient OS versions checks.
	if (!IsWindows7OrGreater())
		Sys_Error(GAME_NAME" requires Windows 7 or greater");

	if ((int)dedicated->value)
	{
		if (!AllocConsole())
			Sys_Error("Couldn't create dedicated server console");

		hinput = GetStdHandle(STD_INPUT_HANDLE);
		houtput = GetStdHandle(STD_OUTPUT_HANDLE);

		//mxd. Skip QHOST logic.
	}
}

// Q2 counterpart
char* Sys_ConsoleInput(void)
{
	INPUT_RECORD rec; //mxd. Was recs[1024]
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

		if (!ReadConsoleInput(hinput, &rec, 1, &numread))
			Sys_Error("Error reading console input");

		if (numread != 1)
			Sys_Error("Couldn't read console input");

		if (rec.EventType == KEY_EVENT && !rec.Event.KeyEvent.bKeyDown)
		{
			char ch = rec.Event.KeyEvent.uChar.AsciiChar;

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

Q2DLL_DECLSPEC int Quake2Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	ParseCommandLine(lpCmdLine);
	Qcommon_Init(argc, argv);

	long long oldtime = Sys_Microseconds();

	// The main game loop.
	while (true)
	{
		//TODO: use Sys_Nanosleep(5000) instead of busywait logic when game window is unfocused or minimized?
		const long long spintime = Sys_Microseconds();

		// YQ2 busywait logic.
		while (Sys_Microseconds() - spintime < 5)
			Sys_CpuPause(); // Give the CPU a hint that this is a very tight spinloop.

		const long long newtime = Sys_Microseconds();
		curtime = (int)(newtime / 1000ll); // Save global time for network and input code.

		// Missing in H2: _controlfp( _PC_24, _MCW_PC );
		Qcommon_Frame((int)(newtime - oldtime));
		oldtime = newtime;
	}

	// Never gets here.
}
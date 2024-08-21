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

qboolean Minimized;

static HANDLE hinput;
static HANDLE houtput;

uint sys_msg_time;

#define MAX_NUM_ARGVS	128
int argc;
char* argv[MAX_NUM_ARGVS];

HINSTANCE global_hInstance;

#pragma region ========================== SYSTEM IO ==========================

void Sys_Error(const char* error, ...)
{
	NOT_IMPLEMENTED
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

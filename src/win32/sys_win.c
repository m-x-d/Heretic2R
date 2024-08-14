//
// sys_win.c
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"
#include "Quake2Main.h"
#include "q_shared.h"

qboolean Minimized;

uint sys_msg_time;

#define MAX_NUM_ARGVS	128
int argc;
char* argv[MAX_NUM_ARGVS];

HINSTANCE global_hInstance;

#pragma region ========================== SYSTEM IO ==========================

void Sys_Error(char* error, ...)
{
	NOT_IMPLEMENTED
}

#pragma endregion

void Sys_Init(void)
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

int GAME_DECLSPEC Quake2Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
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

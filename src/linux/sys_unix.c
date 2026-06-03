//
// sys_unix.c -- Linux system layer + main loop + DLL loading, ported from win32/sys_win.c.
//
// Copyright 1998 Raven Software
//

#include <dlfcn.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include "sys_unix.h"
#include "qcommon.h"
#include "client/input.h"

static char console_text[256];
static int console_textlen;

#pragma region ========================== SYSTEM IO ==========================

H2R_NORETURN void Sys_Error(const char* error, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, error);
	vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	CL_Shutdown();

	fprintf(stderr, "Error: %s\n", text);

	exit(1);
}

H2R_NORETURN void Sys_Quit(void)
{
	CL_Shutdown();
	exit(0);
}

#pragma endregion

#pragma region ========================== DLL HANDLING ==========================

// Translates a Windows-style module name to a Linux .so and dlopen()s it.
// (Backs the LoadLibrary() macro in linux_compat.h.)
void* Sys_dlopen(const char* name)
{
	char path[MAX_OSPATH];
	strcpy_s(path, sizeof(path), name);

	// Translate a trailing ".dll" to ".so", or append ".so" if the basename has no extension.
	const size_t len = strlen(path);
	if (len > 4 && Q_stricmp(path + len - 4, ".dll") == 0)
	{
		strcpy(path + len - 4, ".so");
	}
	else
	{
		char* base = strrchr(path, '/');
		base = (base != NULL) ? base + 1 : path;
		if (strchr(base, '.') == NULL)
			strcat_s(path, sizeof(path), ".so");
	}

	// RTLD_NODELETE: keep the object mapped after dlclose(). Several modules
	// (Client Effects, renderer/sound probes) follow a "load, fetch an API entry
	// point, unload, then call that entry point later" pattern. On Windows the
	// freed DLL's pages stay valid by luck; on Linux dlclose() unmaps the .so and
	// the address gets reused (e.g. by SDL's libdbus), so the stale pointer jumps
	// into unrelated code and crashes. NODELETE preserves the Windows behaviour.
	const int flags = RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE;

	void* handle = dlopen(path, flags);

	// If a bare name didn't resolve via the loader search path, try the cwd.
	if (handle == NULL && strchr(path, '/') == NULL)
	{
		char local[MAX_OSPATH];
		Com_sprintf(local, sizeof(local), "./%s", path);
		handle = dlopen(local, flags);
	}

	if (handle == NULL)
		Com_DDPrintf(2, "dlopen(%s) failed: %s\n", path, dlerror());

	return handle;
}

// Backs the FreeLibrary() macro: returns non-zero on success (opposite of dlclose).
int Sys_dlclose(void* handle)
{
	return (dlclose(handle) == 0);
}

void Sys_LoadGameDll(const char* dll_name, HINSTANCE* hinst, DWORD* checksum)
{
	char name[MAX_OSPATH];

	// Run through the search paths.
	char* path = NULL;
	*hinst = NULL;

	while (true)
	{
		path = FS_NextPath(path);
		if (path == NULL)
			break; // Couldn't find one anywhere.

		Com_sprintf(name, sizeof(name), "%s/%s", path, dll_name);
		*hinst = Sys_dlopen(name);
		if (*hinst != NULL)
			break;
	}

	if (*hinst == NULL)
		Sys_Error("Failed to load %s", dll_name);

	// PE checksum is meaningless on Linux; result is unused anyway.
	*checksum = 0;

	Com_DDPrintf(2, "dlopen (%s)\n", name);
}

void Sys_UnloadGameDll(const char* name, HINSTANCE* hinst)
{
	if (!Sys_dlclose(*hinst))
		Sys_Error("Failed to unload %s", name);

	*hinst = NULL;
}

#pragma endregion

void Sys_Init(void)
{
	Set_Com_Printf(Com_Printf); // H2
}

// Q2 counterpart. Non-blocking dedicated-server console input.
char* Sys_ConsoleInput(void)
{
	if (dedicated == NULL || !(int)dedicated->value)
		return NULL;

	fd_set fdset;
	struct timeval timeout = { 0, 0 };

	FD_ZERO(&fdset);
	FD_SET(STDIN_FILENO, &fdset);

	if (select(STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(STDIN_FILENO, &fdset))
		return NULL;

	const ssize_t len = read(STDIN_FILENO, console_text, sizeof(console_text) - 1);
	if (len < 1)
		return NULL;

	console_text[len] = '\0';
	if (len > 0 && console_text[len - 1] == '\n')
		console_text[len - 1] = '\0';

	console_textlen = 0;

	return console_text;
}

// Q2 counterpart
void Sys_ConsoleOutput(const char* string)
{
	if (dedicated == NULL || !(int)dedicated->value)
		return;

	fputs(string, stdout);
	fflush(stdout);
}

int Quake2Main_Unix(int argc, char** argv)
{
	Qcommon_Init(argc, argv);

	long long oldtime = Sys_Microseconds();

	// The main game loop.
	while (true)
	{
		const long long spintime = Sys_Microseconds();

		// YQ2 busywait logic.
		while (Sys_Microseconds() - spintime < 5)
			Sys_CpuPause(); // Give the CPU a hint that this is a very tight spinloop.

		const long long newtime = Sys_Microseconds();
		curtime = (int)(newtime / 1000ll); // Save global time for network and input code.

		Qcommon_Frame((int)(newtime - oldtime));
		oldtime = newtime;
	}

	// Never gets here.
}

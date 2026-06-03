//
// q_shunix.c -- Linux filesystem/time helpers, ported from win32/q_shwin.c.
//
// Copyright 1998 Raven Software
//

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <glob.h>
#include "qcommon.h"

int curtime;

long long Sys_Microseconds(void) // YQ2
{
	static long long base = 0;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	long long now = (long long)ts.tv_sec * 1000000ll + ts.tv_nsec / 1000ll;

	if (base == 0)
		base = now - 1001;

	return now - base;
}

// Windows multimedia timer replacement: milliseconds since startup.
unsigned int timeGetTime(void)
{
	return (unsigned int)(Sys_Microseconds() / 1000ll);
}

void Sys_Nanosleep(const int nanosec)
{
	struct timespec t = { .tv_sec = nanosec / 1000000000, .tv_nsec = nanosec % 1000000000 };
	nanosleep(&t, NULL);
}

// Q2 counterpart
void Sys_Mkdir(const char* path)
{
	mkdir(path, 0755);
}

qboolean Sys_IsDir(const char* path) // YQ2
{
	struct stat st;
	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

qboolean Sys_IsFile(const char* path) // YQ2
{
	struct stat st;
	return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

#pragma region ========================== FIND FILES ==========================

static char findpath[MAX_OSPATH];
static glob_t find_glob;
static size_t find_index;
static qboolean find_active;

static qboolean Sys_FindAttrMatch(const char* path, const uint musthave, const uint canthave)
{
	struct stat st;
	if (stat(path, &st) != 0)
		return false;

	const qboolean is_dir = S_ISDIR(st.st_mode);

	if (is_dir && (canthave & SFF_SUBDIR))
		return false;

	if ((musthave & SFF_SUBDIR) && !is_dir)
		return false;

	return true;
}

// Q2 counterpart
char* Sys_FindFirst(const char* path, const uint musthave, const uint canthave)
{
	if (find_active)
		Sys_Error("Sys_FindFirst called without close");

	if (glob(path, GLOB_NOSORT, NULL, &find_glob) != 0)
	{
		globfree(&find_glob);
		memset(&find_glob, 0, sizeof(find_glob));
		return NULL;
	}

	find_active = true;
	find_index = 0;

	return Sys_FindNext(musthave, canthave);
}

// Q2 counterpart
char* Sys_FindNext(const uint musthave, const uint canthave)
{
	if (!find_active)
		return NULL;

	while (find_index < find_glob.gl_pathc)
	{
		const char* match = find_glob.gl_pathv[find_index++];

		// Skip "." and ".." entries.
		const char* name = strrchr(match, '/');
		name = (name != NULL) ? name + 1 : match;
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;

		if (!Sys_FindAttrMatch(match, musthave, canthave))
			continue;

		strcpy_s(findpath, sizeof(findpath), match);
		return findpath;
	}

	return NULL;
}

// Q2 counterpart
void Sys_FindClose(void)
{
	if (find_active)
	{
		globfree(&find_glob);
		memset(&find_glob, 0, sizeof(find_glob));
		find_active = false;
	}
}

#pragma endregion

qboolean Sys_GetWorkingDir(char* buffer, const size_t len) // YQ2
{
	if (getcwd(buffer, len) != NULL)
		return true;

	buffer[0] = '\0';
	return false;
}

//mxd. Get OS-specific path to create the Heretic2R userdir in.
qboolean Sys_GetOSUserDir(char* buffer, const size_t len)
{
	const char* xdg = getenv("XDG_DATA_HOME");
	const char* home = getenv("HOME");

	if (xdg != NULL && xdg[0] != 0)
		Com_sprintf(buffer, len, "%s", xdg);
	else if (home != NULL && home[0] != 0)
		Com_sprintf(buffer, len, "%s/.local/share", home);
	else
		buffer[0] = '\0';

	return (buffer[0] != 0);
}

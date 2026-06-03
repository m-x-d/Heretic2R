//
// q_shunix.c -- Linux filesystem/time helpers, ported from win32/q_shwin.c.
//
// Copyright 1998 Raven Software
//

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <fnmatch.h>
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
static char find_dir[MAX_OSPATH];
static char find_pattern[MAX_OSPATH];
static DIR* find_dirp;

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

// Q2 counterpart. Uses opendir()/fnmatch() rather than glob() so that matching is
// case-insensitive (FNM_CASEFOLD): Windows game data filenames often differ in case
// from what the engine/config requests, which would otherwise fail on case-sensitive
// Linux filesystems.
char* Sys_FindFirst(const char* path, const uint musthave, const uint canthave)
{
	if (find_dirp != NULL)
		Sys_Error("Sys_FindFirst called without close");

	// Split the search pattern into a directory and a filename glob.
	const char* slash = strrchr(path, '/');
	if (slash != NULL)
	{
		size_t dir_len = (size_t)(slash - path);
		if (dir_len >= sizeof(find_dir))
			dir_len = sizeof(find_dir) - 1;
		memcpy(find_dir, path, dir_len);
		find_dir[dir_len] = '\0';
		strcpy_s(find_pattern, sizeof(find_pattern), slash + 1);
	}
	else
	{
		strcpy_s(find_dir, sizeof(find_dir), ".");
		strcpy_s(find_pattern, sizeof(find_pattern), path);
	}

	find_dirp = opendir(find_dir);
	if (find_dirp == NULL)
		return NULL;

	return Sys_FindNext(musthave, canthave);
}

// Q2 counterpart
char* Sys_FindNext(const uint musthave, const uint canthave)
{
	if (find_dirp == NULL)
		return NULL;

	const struct dirent* entry;
	while ((entry = readdir(find_dirp)) != NULL)
	{
		// Skip "." and ".." entries.
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		if (fnmatch(find_pattern, entry->d_name, FNM_CASEFOLD) != 0)
			continue;

		Com_sprintf(findpath, sizeof(findpath), "%s/%s", find_dir, entry->d_name);

		if (!Sys_FindAttrMatch(findpath, musthave, canthave))
			continue;

		return findpath;
	}

	return NULL;
}

// Q2 counterpart
void Sys_FindClose(void)
{
	if (find_dirp != NULL)
	{
		closedir(find_dirp);
		find_dirp = NULL;
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

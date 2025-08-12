//
// q_shwin.c
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include <io.h>
#include "qcommon.h"

int curtime;

static char findbase[MAX_OSPATH];
static char findpath[MAX_OSPATH];
static int findhandle;

long long Sys_Microseconds(void) // YQ2
{
	static LARGE_INTEGER freq = { 0 };
	static LARGE_INTEGER base = { 0 };

	if (!freq.QuadPart)
		QueryPerformanceFrequency(&freq);

	if (!base.QuadPart)
	{
		QueryPerformanceCounter(&base);
		base.QuadPart -= 1001;
	}

	LARGE_INTEGER cur;
	QueryPerformanceCounter(&cur);

	return (cur.QuadPart - base.QuadPart) * 1000000 / freq.QuadPart;
}

int Sys_Milliseconds(void) // YQ2. No longer sets 'curtime' global var.
{
	return (int)(Sys_Microseconds() / 1000ll);
}

void Sys_Nanosleep(const int nanosec)
{
	const HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);

	// Windows has a max. resolution of 100ns.
	const LARGE_INTEGER li = { .QuadPart = -nanosec / 100 };
	SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
	WaitForSingleObject(timer, INFINITE);

	CloseHandle(timer);
}

// Q2 counterpart
void Sys_Mkdir(const char* path)
{
	_mkdir(path);
}

// Q2 counterpart
static qboolean CompareAttributes(const uint found, const uint musthave, const uint canthave)
{
	if ((found & _A_RDONLY) && (canthave & SFF_RDONLY))
		return false;
	if ((found & _A_HIDDEN) && (canthave & SFF_HIDDEN))
		return false;
	if ((found & _A_SYSTEM) && (canthave & SFF_SYSTEM))
		return false;
	if ((found & _A_SUBDIR) && (canthave & SFF_SUBDIR))
		return false;
	if ((found & _A_ARCH) && (canthave & SFF_ARCH))
		return false;

	if ((musthave & SFF_RDONLY) && !(found & _A_RDONLY))
		return false;
	if ((musthave & SFF_HIDDEN) && !(found & _A_HIDDEN))
		return false;
	if ((musthave & SFF_SYSTEM) && !(found & _A_SYSTEM))
		return false;
	if ((musthave & SFF_SUBDIR) && !(found & _A_SUBDIR))
		return false;
	if ((musthave & SFF_ARCH) && !(found & _A_ARCH))
		return false;

	return true;
}

// Q2 counterpart
char* Sys_FindFirst(const char* path, const uint musthave, const uint canthave)
{
	struct _finddata_t findinfo;

	if (findhandle != 0)
		Sys_Error("Sys_FindFirst called without close");

	COM_FilePath(path, findbase);
	findhandle = _findfirst(path, &findinfo);

	if (findhandle == -1)
		return NULL;

	if (!CompareAttributes(findinfo.attrib, musthave, canthave))
		return NULL;

	Com_sprintf(findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
	return findpath;
}

// Q2 counterpart
char* Sys_FindNext(const uint musthave, const uint canthave)
{
	struct _finddata_t findinfo;

	if (findhandle == -1)
		return NULL;

	if (_findnext(findhandle, &findinfo) == -1)
		return NULL;

	if (!CompareAttributes(findinfo.attrib, musthave, canthave))
		return NULL;

	Com_sprintf(findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
	return findpath;
}

// Q2 counterpart
void Sys_FindClose(void)
{
	if (findhandle != -1)
		_findclose(findhandle);
	
	findhandle = 0;
}
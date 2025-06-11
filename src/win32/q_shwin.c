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

// Q2 counterpart
int Sys_Milliseconds(void)
{
	static int base;
	static qboolean	initialized = false;

	if (!initialized)
	{
		// Let base retain 16 bits of effectively random data.
		base = (int)(timeGetTime() & 0xffff0000);
		initialized = true;
	}
	curtime = (int)timeGetTime() - base;

	return curtime;
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
		Sys_Error("Sys_BeginFind without close");
	//findhandle = 0; //mxd. Not needed.

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
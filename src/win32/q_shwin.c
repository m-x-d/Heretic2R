//
// q_shwin.c
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include <io.h>
#include "qcommon.h"

static byte* membase;

static int hunkcount;
static uint hunkmaxsize; //mxd. int -> uint
static uint cursize; //mxd. int -> uint

int curtime;

static char findbase[MAX_OSPATH];
static char findpath[MAX_OSPATH];
static int findhandle;

// Q2 counterpart
void* Hunk_Begin(const int maxsize)
{
	// Reserve a huge chunk of memory, but don't commit any yet
	// YQ2: plus 32 bytes for cacheline
	hunkmaxsize = maxsize + sizeof(uint) + 32;
	cursize = 0;

	membase = VirtualAlloc(NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS);
	if (membase == NULL)
		Sys_Error("VirtualAlloc reserve failed");

	return membase;
}

// Q2 counterpart
void* Hunk_Alloc(int size)
{
	// Round to cacheline
	size = (size + 31) & ~31;

	// Commit pages as needed
	void* buf = VirtualAlloc(membase, cursize + size, MEM_COMMIT, PAGE_READWRITE);

	if (buf == NULL)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buf, 0, NULL);
		Sys_Error("VirtualAlloc commit failed.\n%s", buf);
	}

	cursize += size;

	if (cursize > hunkmaxsize)
		Sys_Error("Hunk_Alloc overflow");

	return membase + cursize - size;
}

// Q2 counterpart
int Hunk_End(void)
{
	hunkcount++;
	return (int)cursize;
}

// Q2 counterpart
void Hunk_Free(void* buf)
{
	if (buf)
		VirtualFree(buf, 0, MEM_RELEASE);

	hunkcount--;
}

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
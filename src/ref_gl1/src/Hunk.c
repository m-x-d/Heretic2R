//
// Hunk.c
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include "Hunk.h"
#include "gl1_Local.h"

static byte* membase;

static int hunkcount;
static uint hunkmaxsize; //mxd. int -> uint
static uint cursize; //mxd. int -> uint

// Q2 counterpart
void* Hunk_Begin(const int maxsize)
{
	// Reserve a huge chunk of memory, but don't commit any yet
	// YQ2: plus 32 bytes for cacheline
	hunkmaxsize = maxsize + sizeof(uint) + 32;
	cursize = 0;

	membase = VirtualAlloc(NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS);
	if (membase == NULL)
		ri.Sys_Error(ERR_DROP, "VirtualAlloc reserve failed"); //mxd. Sys_Error() -> ri.Sys_Error().

	return membase;
}

// Q2 counterpart
void* Hunk_Alloc(int size)
{
	// Round to cacheline.
	size = (size + 31) & ~31;

	// Commit pages as needed.
	void* buf = VirtualAlloc(membase, cursize + size, MEM_COMMIT, PAGE_READWRITE);

	if (buf == NULL)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buf, 0, NULL);
		ri.Sys_Error(ERR_DROP, "VirtualAlloc commit failed.\n%s", buf); //mxd. Sys_Error() -> ri.Sys_Error().
	}

	cursize += size;

	if (cursize > hunkmaxsize)
		ri.Sys_Error(ERR_DROP, "Hunk_Alloc overflow"); //mxd. Sys_Error() -> ri.Sys_Error().

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
	if (buf != NULL)
		VirtualFree(buf, 0, MEM_RELEASE);

	hunkcount--;
}
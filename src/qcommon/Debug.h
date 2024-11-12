//
// Debug.h -- Development aids
//
// Copyright 2024 mxd
//

#pragma once

#if _DEBUG
	#include <windows.h>
#endif

#define NOT_IMPLEMENTED		assert(!("Not implemented!"));

// Print to Visual Studio console.
__inline void __cdecl IDEPrintf(const char* fmt, ...)
{
#if _DEBUG
	va_list argptr;
	char msg[1024];

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	OutputDebugString(msg);
#endif
}
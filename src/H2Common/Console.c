//
// Console.c
//
// Copyright 1998 Raven Software
//

#include "q_shared.h"

void (*com_printf)(const char* fmt, ...);

H2COMMON_API void Set_Com_Printf(void (*toSet)(const char* fmt, ...))
{
	com_printf = toSet;
}
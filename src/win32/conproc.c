//
// conproc.c -- support for qhost
//
// Copyright 1998 Raven Software
//

#include <windows.h>
#include "conproc.h"
#include "q_shared.h" //TODO: Added only for NOT_IMPLEMENTED macro!

static HANDLE heventDone;

void InitConProc(int argc, char** argv)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void DeinitConProc(void)
{
	if (heventDone != NULL)
		SetEvent(heventDone);
}
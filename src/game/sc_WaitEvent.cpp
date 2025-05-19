//
// sc_WaitEvent.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_WaitEvent.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

WaitEvent::WaitEvent(float NewTime)
	:Event(NewTime, EVENT_SCRIPT_WAIT)
{
}

WaitEvent::WaitEvent(FILE* FH, CScript* Script)
	:Event(FH, Script)
{
}

void WaitEvent::Write(FILE* FH, CScript* Script, int ID)
{
	Event::Write(FH, Script, RLID_WAITEVENT);
}

bool WaitEvent::Process(CScript* Script)
{
	if (level.time < time)
	{
		return FALSE;
	}

	Script->ClearTimeWait();

	if (Script->CheckWait())
	{
		Script->Execute(NULL, NULL);
	}

	return TRUE;
}
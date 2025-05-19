//
// sc_WaitEvent.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_WaitEvent.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

WaitEvent::WaitEvent(const float new_time) : Event(new_time, EVENT_SCRIPT_WAIT)
{
}

WaitEvent::WaitEvent(FILE* f, CScript* script) : Event(f, script)
{
}

void WaitEvent::Write(FILE* f, CScript* script, int id)
{
	Event::Write(f, script, RLID_WAITEVENT);
}

bool WaitEvent::Process(CScript* script)
{
	if (level.time < time)
		return FALSE;

	script->ClearTimeWait();

	if (script->CheckWait())
		script->Execute(nullptr, nullptr);

	return TRUE;
}
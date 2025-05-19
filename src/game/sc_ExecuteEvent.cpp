//
// sc_ExecuteEvent.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_ExecuteEvent.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

ExecuteEvent::ExecuteEvent(const float new_time, edict_t* new_other, edict_t* new_activator) : Event(new_time, EVENT_SCRIPT_EXECUTE)
{
	other = new_other;
	activator = new_activator;
}

ExecuteEvent::ExecuteEvent(FILE* f, CScript* script) : Event(f, script)
{
	ReadEnt(&other, f);
	ReadEnt(&activator, f);
}

void ExecuteEvent::Write(FILE* f, CScript* script, int id)
{
	Event::Write(f, script, RLID_EXECUTEEVENT);
	WriteEnt(&other, f);
	WriteEnt(&activator, f);
}

bool ExecuteEvent::Process(CScript* script)
{
	if (level.time < time)
		return FALSE;

	if (script->CheckWait())
		script->Execute(other, activator);

	return TRUE;
}
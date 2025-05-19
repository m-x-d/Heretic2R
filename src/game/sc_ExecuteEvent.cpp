//
// sc_ExecuteEvent.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_ExecuteEvent.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

ExecuteEvent::ExecuteEvent(float NewTime, edict_t* NewOther, edict_t* NewActivator)
	:Event(NewTime, EVENT_SCRIPT_EXECUTE)
{
	Other = NewOther;
	Activator = NewActivator;
}

ExecuteEvent::ExecuteEvent(FILE* FH, CScript* Script)
	:Event(FH, Script)
{
	ReadEnt(&Other, FH);
	ReadEnt(&Activator, FH);
}

void ExecuteEvent::Write(FILE* FH, CScript* Script, int ID)
{
	Event::Write(FH, Script, RLID_EXECUTEEVENT);
	WriteEnt(&Other, FH);
	WriteEnt(&Activator, FH);
}

bool ExecuteEvent::Process(CScript* Script)
{
	if (level.time < time)
	{
		return FALSE;
	}

	if (Script->CheckWait())
	{
		Script->Execute(Other, Activator);
	}

	return TRUE;
}
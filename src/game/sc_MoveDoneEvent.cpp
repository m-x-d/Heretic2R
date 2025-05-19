//
// sc_MoveDoneEvent.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_MoveDoneEvent.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

MoveDoneEvent::MoveDoneEvent(float NewTime, edict_t* NewEnt)
	:Event(NewTime, EVENT_MOVE_DONE)
{
	Ent = NewEnt;
}

MoveDoneEvent::MoveDoneEvent(FILE* FH, CScript* Script)
	:Event(FH, Script)
{
	ReadEnt(&Ent, FH);
}

void MoveDoneEvent::Write(FILE* FH, CScript* Script, int ID)
{
	Event::Write(FH, Script, RLID_MOVEDONEEVENT);
	WriteEnt(&Ent, FH);
}

bool MoveDoneEvent::Process(CScript* Script)
{
	if (level.time < time)
	{
		return FALSE;
	}

	Script->Move_Done(Ent);
	script_signaler(Ent, SIGNAL_MOVE); //mxd. Inline move_signaler().

	return TRUE;
}
//
// sc_RotateDoneEvent.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_RotateDoneEvent.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

RotateDoneEvent::RotateDoneEvent(float NewTime, edict_t* NewEnt)
	:Event(NewTime, EVENT_ROTATE_DONE)
{
	Ent = NewEnt;

	Priority = 10;
}

RotateDoneEvent::RotateDoneEvent(FILE* FH, CScript* Script)
	:Event(FH, Script)
{
	ReadEnt(&Ent, FH);
}

void RotateDoneEvent::Write(FILE* FH, CScript* Script, int ID)
{
	Event::Write(FH, Script, RLID_ROTATEDONEEVENT);
	WriteEnt(&Ent, FH);
}

bool RotateDoneEvent::Process(CScript* Script)
{
	if (level.time < Time)
	{
		return FALSE;
	}

	Script->Rotate_Done(Ent);
	script_signaler(Ent, SIGNAL_ROTATE); //mxd. Inline rotate_signaler().

	return TRUE;
}
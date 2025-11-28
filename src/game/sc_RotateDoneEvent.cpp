//
// sc_RotateDoneEvent.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_RotateDoneEvent.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

RotateDoneEvent::RotateDoneEvent(const float new_time, edict_t* new_ent) : Event(new_time, EVENT_ROTATE_DONE)
{
	ent = new_ent;
}

RotateDoneEvent::RotateDoneEvent(FILE* f, CScript* script) : Event(f, script)
{
	ReadEnt(&ent, f);
}

void RotateDoneEvent::Write(FILE* f, CScript* script, RestoreListID_t id)
{
	Event::Write(f, script, RLID_ROTATEDONEEVENT);
	WriteEnt(&ent, f);
}

bool RotateDoneEvent::Process(CScript* script)
{
	if (level.time < time)
		return false;

	script->Rotate_Done(ent);
	script_signaler(ent, SIGNAL_ROTATE); //mxd. Inline rotate_signaler().

	return true;
}
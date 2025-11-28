//
// sc_MoveDoneEvent.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_MoveDoneEvent.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

MoveDoneEvent::MoveDoneEvent(const float new_time, edict_t* new_ent) : Event(new_time, EVENT_MOVE_DONE)
{
	ent = new_ent;
}

MoveDoneEvent::MoveDoneEvent(FILE* f, CScript* script) : Event(f, script)
{
	ReadEnt(&ent, f);
}

void MoveDoneEvent::Write(FILE* f, CScript* script, RestoreListID_t id)
{
	Event::Write(f, script, RLID_MOVEDONEEVENT);
	WriteEnt(&ent, f);
}

bool MoveDoneEvent::Process(CScript* script)
{
	if (level.time < time)
		return false;

	script->Move_Done(ent);
	script_signaler(ent, SIGNAL_MOVE); //mxd. Inline move_signaler().

	return true;
}
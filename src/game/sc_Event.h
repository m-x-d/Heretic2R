//
// sc_Event.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Pcode.h" //mxd
#include "g_local.h"

enum EventT
{
	EVENT_MOVE_DONE,
	EVENT_ROTATE_DONE,
	EVENT_SCRIPT_WAIT,
	EVENT_SCRIPT_EXECUTE,
};

class CScript;

class Event
{
protected:
	float time = 0.0f;
	EventT type = EVENT_MOVE_DONE;

public:
	Event(float new_time, EventT new_type);
	Event(FILE* f, CScript* script);
	virtual ~Event() = default; //mxd. Added to avoid compiler warning...

	virtual void Write(FILE* f, CScript* script, RestoreListID_t id = RLID_UNDEFINED);

	float GetTime() const
	{
		return time;
	}

	EventT GetType() const
	{
		return type;
	}

	virtual bool Process(CScript* script);
};
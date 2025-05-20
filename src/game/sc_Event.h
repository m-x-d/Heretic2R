//
// sc_Event.h
//
// Copyright 1998 Raven Software
//

#pragma once

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

	virtual void Write(FILE* f, CScript* script, int id = -1);

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
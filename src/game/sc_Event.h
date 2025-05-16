//
// sc_Event.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_CScript.h"

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
	float		Time;
	EventT		Type;
	int			Priority;

public:
	Event(float NewTime, EventT NewType);
	Event(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	float		GetTime(void) { return Time; }
	EventT		GetType(void) { return Type; }
	int			GetPriority(void) { return Priority; }
	virtual bool		Process(CScript* Script);
};
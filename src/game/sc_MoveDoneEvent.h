//
// sc_MoveDoneEvent.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Event.h"

class MoveDoneEvent : public Event
{
private:
	edict_t* Ent;

public:
	MoveDoneEvent(float NewTime, edict_t* NewEnt);
	MoveDoneEvent(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual bool		Process(CScript* Script);
};
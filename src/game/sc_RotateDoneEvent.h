//
// sc_MoveDoneEvent.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Event.h"

class RotateDoneEvent : public Event
{
private:
	edict_t* Ent;

public:
	RotateDoneEvent(float NewTime, edict_t* NewEnt);
	RotateDoneEvent(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual bool		Process(CScript* Script);
};
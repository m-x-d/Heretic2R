//
// sc_ExecuteEvent.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Event.h"

class ExecuteEvent : public Event
{
private:
	edict_t* Other;
	edict_t* Activator;

public:
	ExecuteEvent(float NewTime, edict_t* NewOther = NULL, edict_t* NewActivator = NULL);
	ExecuteEvent(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual bool		Process(CScript* Script);
};
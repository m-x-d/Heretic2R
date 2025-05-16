//
// sc_WaitEvent.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Event.h"

class WaitEvent : public Event
{
public:
	WaitEvent(float NewTime);
	WaitEvent(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual bool		Process(CScript* Script);
};
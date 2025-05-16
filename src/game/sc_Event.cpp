//
// sc_Event.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Event.h"
#include "sc_Utility.h"
#include "g_local.h"

Event::Event(float NewTime, EventT NewType)
{
	Time = floor((NewTime + 0.05) * 10) / 10;		// avoids stupid math rounding errors
	Type = NewType;
}

Event::Event(FILE* FH, CScript* Script)
{
	tRead(&Time, FH);
	tRead(&Type, FH);
	tRead(&Priority, FH);
}

void Event::Write(FILE* FH, CScript* Script, int ID)
{
	fwrite(&ID, 1, sizeof(ID), FH);
	tWrite(&Time, FH);
	tWrite(&Type, FH);
	tWrite(&Priority, FH);
}

bool Event::Process(CScript* Script)
{
	return FALSE;
}
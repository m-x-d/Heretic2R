//
// sc_Event.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Event.h"
#include "sc_Utility.h"
#include "g_local.h"

Event::Event(const float new_time, const EventT new_type)
{
	time = floorf((new_time + 0.05f) * 10.0f) / 10.0f; // Avoids stupid math rounding errors.
	type = new_type;
}

Event::Event(FILE* f, CScript* script)
{
	tRead(&time, f);
	tRead(&type, f);

	int priority; //mxd. Preserve compatibility...
	tRead(&priority, f);
}

void Event::Write(FILE* f, CScript* script, const RestoreListID_t id)
{
	fwrite(&id, 1, sizeof(id), f);
	tWrite(&time, f);
	tWrite(&type, f);

	int priority = 0; //mxd. Preserve compatibility...
	tWrite(&priority, f);
}

bool Event::Process(CScript* script)
{
	return false;
}
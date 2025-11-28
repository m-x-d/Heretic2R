//
// sc_Event.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Event.h"
#include "g_local.h"

Event::Event(const float new_time, const EventT new_type)
{
	time = floorf((new_time + 0.05f) * 10.0f) / 10.0f; // Avoids stupid math rounding errors.
	type = new_type;
}

Event::Event(FILE* f, CScript* script)
{
	fread(&time, 1, sizeof(time), f);
	fread(&type, 1, sizeof(type), f);

	int priority; //mxd. Preserve compatibility...
	fread(&priority, 1, sizeof(priority), f);
}

void Event::Write(FILE* f, CScript* script, const RestoreListID_t id)
{
	fwrite(&id, 1, sizeof(id), f);
	fwrite(&time, 1, sizeof(time), f);
	fwrite(&type, 1, sizeof(type), f);

	constexpr int priority = 0; //mxd. Preserve compatibility...
	fwrite(&priority, 1, sizeof(priority), f);
}

bool Event::Process(CScript* script)
{
	return false;
}
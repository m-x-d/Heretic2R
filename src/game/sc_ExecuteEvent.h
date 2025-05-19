//
// sc_ExecuteEvent.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Event.h"

class ExecuteEvent : public Event
{
	edict_t* other = nullptr;
	edict_t* activator = nullptr;

public:
	ExecuteEvent(float new_time, edict_t* new_other = nullptr, edict_t* new_activator = nullptr);
	ExecuteEvent(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, int id = -1) override;
	bool Process(CScript* script) override;
};
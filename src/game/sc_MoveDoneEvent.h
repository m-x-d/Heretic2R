//
// sc_MoveDoneEvent.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Event.h"

class MoveDoneEvent : public Event
{
	edict_t* ent = nullptr;

public:
	MoveDoneEvent(float new_time, edict_t* new_ent);
	MoveDoneEvent(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, int id = -1) override;
	bool Process(CScript* script) override;
};
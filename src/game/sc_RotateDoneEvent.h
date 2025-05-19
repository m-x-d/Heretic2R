//
// sc_MoveDoneEvent.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Event.h"

class RotateDoneEvent : public Event
{
	edict_t* ent = nullptr;

public:
	RotateDoneEvent(float new_time, edict_t* new_ent);
	RotateDoneEvent(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, int id = -1) override;
	bool Process(CScript* script) override;
};
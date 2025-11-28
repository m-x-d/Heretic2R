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
	WaitEvent(float new_time);
	WaitEvent(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, RestoreListID_t id = RLID_UNDEFINED) override;
	bool Process(CScript* script) override;
};
//
// sc_EntityVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class EntityVar : public Variable
{
protected:
	edict_t* value = nullptr;

public:
	EntityVar(const char* new_name = "", int ent_index = 0);
	EntityVar(edict_t* which);
	EntityVar(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, RestoreListID_t id = RLID_UNDEFINED) override;
	int GetIntValue() const override;

	edict_t* GetEdictValue() const override
	{
		return value;
	}

	void ReadValue(CScript* script) override;
	void Debug(CScript* script) override;

	void operator =(Variable* v) override;
	bool operator ==(Variable* v) override;
	bool operator !=(Variable* v) override;
};
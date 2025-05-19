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

	void Write(FILE* f, CScript* script, int id = -1) override;
	int GetIntValue() override;

	edict_t* GetEdictValue() override
	{
		return value;
	}

	void ReadValue(CScript* script) override;
	void Debug(CScript* script) override;

	void operator =(Variable* v) override;
	bool operator ==(Variable* v) override;
	bool operator !=(Variable* v) override;
};
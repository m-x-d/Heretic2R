//
// sc_StringVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class StringVar : public Variable
{
protected:
	char value[VAR_LENGTH] = { 0 };

public:
	StringVar(const char* new_name = "", const char* new_value = "");
	StringVar(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, int id = -1) override;

	const char* GetStringValue() const override
	{
		return value;
	}

	void ReadValue(CScript* script) override;
};

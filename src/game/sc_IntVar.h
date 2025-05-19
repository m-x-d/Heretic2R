//
// sc_IntVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class IntVar : public Variable
{
protected:
	int	value = 0;

public:
	IntVar(const char* new_name = "", int new_value = 0);
	IntVar(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, int id = -1) override;

	int GetIntValue() const override
	{
		return value;
	}

	float GetFloatValue() const override
	{
		return static_cast<float>(value);
	}

	void ReadValue(CScript* script) override;
	void Debug(CScript* script) override;
	void Signal(edict_t* which) override;
	void ClearSignal() override;

	Variable* operator +(Variable* v) override;
	Variable* operator -(Variable* v) override;
	Variable* operator *(Variable* v) override;
	Variable* operator /(Variable* v) override;
	void operator =(Variable* v) override;

	bool operator ==(Variable* v) override
	{
		return value == v->GetIntValue();
	}

	bool operator !=(Variable* v) override
	{
		return value != v->GetIntValue();
	}

	bool operator <(Variable* v) override
	{
		return value < v->GetIntValue();
	}

	bool operator <=(Variable* v) override
	{
		return value <= v->GetIntValue();
	}

	bool operator >(Variable* v) override
	{
		return value > v->GetIntValue();
	}

	bool operator >=(Variable* v) override
	{
		return value >= v->GetIntValue();
	}
};
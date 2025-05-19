//
// sc_FloatVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class FloatVar : public Variable
{
protected:
	float value = 0.0f;

public:
	FloatVar(const char* new_name = "", float new_value = 0.0f);
	FloatVar(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, int id = -1) override;

	int GetIntValue() const override
	{
		return static_cast<int>(value);
	}

	float GetFloatValue() const override
	{
		return value;
	}

	void ReadValue(CScript* script) override;
	void Debug(CScript* script) override;

	Variable* operator +(Variable* v) override;
	Variable* operator -(Variable* v) override;
	Variable* operator *(Variable* v) override;
	Variable* operator /(Variable* v) override;
	void operator =(Variable* v) override;

	bool operator ==(Variable* VI) override
	{
		return value == VI->GetFloatValue();
	}

	bool operator !=(Variable* VI) override
	{
		return value != VI->GetFloatValue();
	}

	bool operator <(Variable* VI) override
	{
		return value < VI->GetFloatValue();
	}

	bool operator <=(Variable* VI) override
	{
		return value <= VI->GetFloatValue();
	}

	bool operator >(Variable* VI) override
	{
		return value > VI->GetFloatValue();
	}

	bool operator >=(Variable* VI) override
	{
		return value >= VI->GetFloatValue();
	}
};
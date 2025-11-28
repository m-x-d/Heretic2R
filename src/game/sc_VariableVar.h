//
// sc_VariableVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class VariableVar : public Variable
{
protected:
	Variable* value = nullptr;

public:
	VariableVar(const char* new_name = "");
	VariableVar(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, RestoreListID_t id = RLID_UNDEFINED) override;

	int GetIntValue() const override
	{
		return value->GetIntValue();
	}

	float GetFloatValue() const override
	{
		return value->GetFloatValue();
	}

	void GetVectorValue(vec3_t& dest_value) const override
	{
		value->GetVectorValue(dest_value);
	}

	edict_t* GetEdictValue() const override
	{
		return value->GetEdictValue();
	}

	const char* GetStringValue() const override
	{
		return value->GetStringValue();
	}

	void ReadValue(CScript* script) override;
	void Debug(CScript* script) override;

	void Signal(edict_t* which) override
	{
		value->Signal(which);
	}

	void ClearSignal() override
	{
		value->ClearSignal();
	}

	Variable* operator +(Variable* v) override
	{
		return *value + v;
	}

	Variable* operator -(Variable* v) override
	{
		return *value - v;
	}

	Variable* operator *(Variable* v) override
	{
		return *value * v;
	}

	Variable* operator /(Variable* v) override
	{
		return *value / v;
	}

	void operator =(Variable* v) override
	{
		*value = v;
	}

	bool operator ==(Variable* v) override
	{
		return *value == v;
	}

	bool operator !=(Variable* v) override
	{
		return *value != v;
	}

	bool operator <(Variable* v) override
	{
		return *value < v;
	}

	bool operator <=(Variable* v) override
	{
		return *value <= v;
	}

	bool operator >(Variable* v) override
	{
		return *value > v;
	}

	bool operator >=(Variable* v) override
	{
		return *value >= v;
	}
};
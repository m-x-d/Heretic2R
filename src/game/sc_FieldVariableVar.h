//
// sc_FieldVariableVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_FieldDef.h"

class FieldVariableVar : public Variable
{
protected:
	Variable* value;
	FieldDef* field;

public:
	FieldVariableVar(const char* new_name = "");
	FieldVariableVar(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, RestoreListID_t id = RLID_UNDEFINED) override;
	int GetIntValue() const override;
	float GetFloatValue() const override;
	void GetVectorValue(vec3_t& vec_value) const override;
	edict_t* GetEdictValue() const override;
	const char* GetStringValue() const override;
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

	Variable* operator +(Variable* v) override;
	Variable* operator -(Variable* v) override;
	Variable* operator *(Variable* v) override;
	Variable* operator /(Variable* v) override;
	void operator =(Variable* v) override;

	bool operator ==(Variable* v) override;
	bool operator !=(Variable* v) override;
	bool operator <(Variable* v) override;
	bool operator <=(Variable* v) override;
	bool operator >(Variable* v) override;
	bool operator >=(Variable* v) override;
};
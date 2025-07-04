//
// sc_VectorVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class VectorVar : public Variable
{
protected:
	vec3_t value = {};

public:
	VectorVar(const char* new_name = "", float x = 0.0f, float y = 0.0f, float z = 0.0f);
	VectorVar(vec3_t new_value);
	VectorVar(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script, int id = -1) override;
	void GetVectorValue(vec3_t& dest_value) const override;
	void ReadValue(CScript* script) override;
	void Debug(CScript* script) override;

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
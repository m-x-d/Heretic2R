//
// sc_FieldDef.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"
#include "g_save.h"

class FieldDef
{
	char name[VAR_LENGTH] = { 0 };
	VariableType type = TYPE_INT;
	int offset = -1;
	fieldtype_t field_type = F_IGNORE;

public:
	explicit FieldDef(CScript* script);
	FieldDef(FILE* f, CScript* script);

	void Write(FILE* f, CScript* script);
	byte* GetOffset(Variable* var) const;
	Variable* GetValue(Variable* var) const;
	int GetIntValue(Variable* var) const;
	float GetFloatValue(Variable* var) const;
	void GetVectorValue(Variable* var, vec3_t& value) const;
	edict_t* GetEdictValue(Variable* var) const;
	char* GetStringValue(Variable* var) const;
	void SetValue(Variable* var, Variable* value) const;
};
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
private:
	char Name[VAR_LENGTH];
	VariableType Type;
	int Offset;
	fieldtype_t	 FieldType;

public:
	FieldDef(CScript* Script);
	FieldDef(FILE* FH, CScript* Script);
	void Write(FILE* FH, CScript* Script);
	byte* GetOffset(Variable* Var);
	Variable* GetValue(Variable* Var);
	int GetIntValue(Variable* Var);
	float GetFloatValue(Variable* Var);
	void GetVectorValue(Variable* Var, vec3_t& VecValue);
	edict_t* GetEdictValue(Variable* Var);
	char* GetStringValue(Variable* Var);
	void SetValue(Variable* Var, Variable* Value);
};
//
// sc_FieldVariableVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_FieldVariableVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

FieldVariableVar::FieldVariableVar(char* Name)
	:Variable(Name, TYPE_UNKNOWN)
{
	Value = NULL;
	Field = NULL;
}

FieldVariableVar::FieldVariableVar(FILE* FH, CScript* Script)
	:Variable(FH, Script)
{
	int index;

	fread(&index, 1, sizeof(index), FH);
	Value = Script->LookupVar(index);

	fread(&index, 1, sizeof(index), FH);
	Field = Script->LookupField(index);
}

void FieldVariableVar::Write(FILE* FH, CScript* Script, int ID)
{
	int index;

	Variable::Write(FH, Script, RLID_FIELDVARIABLEVAR);

	index = Script->LookupVarIndex(Value);
	fwrite(&index, 1, sizeof(index), FH);

	index = Script->LookupFieldIndex(Field);
	fwrite(&index, 1, sizeof(index), FH);
}

void FieldVariableVar::ReadValue(CScript* Script)
{
	int Index;

	Index = Script->ReadInt();
	Value = Script->LookupVar(Index);

	Index = Script->ReadInt();
	Field = Script->LookupField(Index);
}

void FieldVariableVar::Debug(CScript* Script)
{
	Value->Debug(Script);
}

int FieldVariableVar::GetIntValue(void)
{
	return Field->GetIntValue(Value);
}

float FieldVariableVar::GetFloatValue(void)
{
	return Field->GetFloatValue(Value);
}

void FieldVariableVar::GetVectorValue(vec3_t& VecValue)
{
	Field->GetVectorValue(Value, VecValue);
}

edict_t* FieldVariableVar::GetEdictValue(void)
{
	return Field->GetEdictValue(Value);
}

char* FieldVariableVar::GetStringValue(void)
{
	return Field->GetStringValue(Value);
}

Variable* FieldVariableVar::operator +(Variable* VI)
{
	Variable* Result, * Val;

	Val = Field->GetValue(Value);

	Result = (*Val) + VI;

	delete Val;

	return Result;
}

Variable* FieldVariableVar::operator -(Variable* VI)
{
	Variable* Result, * Val;

	Val = Field->GetValue(Value);

	Result = (*Val) - VI;

	delete Val;

	return Result;
}

Variable* FieldVariableVar::operator *(Variable* VI)
{
	Variable* Result, * Val;

	Val = Field->GetValue(Value);

	Result = (*Val) * VI;

	delete Val;

	return Result;
}

Variable* FieldVariableVar::operator /(Variable* VI)
{
	Variable* Result, * Val;

	Val = Field->GetValue(Value);

	Result = (*Val) / VI;

	delete Val;

	return Result;
}

void FieldVariableVar::operator =(Variable* VI)
{
	Field->SetValue(Value, VI);
}

bool FieldVariableVar::operator ==(Variable* VI)
{
	return (*Value) == VI;
}

bool FieldVariableVar::operator !=(Variable* VI)
{
	return (*Value) != VI;
}

bool FieldVariableVar::operator <(Variable* VI)
{
	return (*Value) < VI;
}

bool FieldVariableVar::operator <=(Variable* VI)
{
	return (*Value) <= VI;
}

bool FieldVariableVar::operator >(Variable* VI)
{
	return (*Value) > VI;
}

bool FieldVariableVar::operator >=(Variable* VI)
{
	return (*Value) >= VI;
}
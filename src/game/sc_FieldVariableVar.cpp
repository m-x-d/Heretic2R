//
// sc_FieldVariableVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_FieldVariableVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"

FieldVariableVar::FieldVariableVar(const char* new_name) : Variable(new_name, TYPE_UNKNOWN)
{
	value = nullptr;
	field = nullptr;
}

FieldVariableVar::FieldVariableVar(FILE* f, CScript* script) : Variable(f, script)
{
	int var_index;
	fread(&var_index, 1, sizeof(var_index), f);
	value = script->LookupVar(var_index);

	int field_index;
	fread(&field_index, 1, sizeof(field_index), f);
	field = script->LookupField(field_index);
}

void FieldVariableVar::Write(FILE* f, CScript* script, int id)
{
	Variable::Write(f, script, RLID_FIELDVARIABLEVAR);

	const int var_index = script->LookupVarIndex(value);
	fwrite(&var_index, 1, sizeof(var_index), f);

	const int field_index = script->LookupFieldIndex(field);
	fwrite(&field_index, 1, sizeof(field_index), f);
}

void FieldVariableVar::ReadValue(CScript* script)
{
	const int var_index = script->ReadInt();
	value = script->LookupVar(var_index);

	const int field_index = script->ReadInt();
	field = script->LookupField(field_index);
}

void FieldVariableVar::Debug(CScript* script)
{
	value->Debug(script);
}

int FieldVariableVar::GetIntValue() const
{
	return field->GetIntValue(value);
}

float FieldVariableVar::GetFloatValue() const
{
	return field->GetFloatValue(value);
}

void FieldVariableVar::GetVectorValue(vec3_t& vec_value) const
{
	field->GetVectorValue(value, vec_value);
}

edict_t* FieldVariableVar::GetEdictValue() const
{
	return field->GetEdictValue(value);
}

const char* FieldVariableVar::GetStringValue() const
{
	return field->GetStringValue(value);
}

Variable* FieldVariableVar::operator +(Variable* v)
{
	Variable* val = field->GetValue(value);
	Variable* result = *val + v;
	delete val;

	return result;
}

Variable* FieldVariableVar::operator -(Variable* v)
{
	Variable* val = field->GetValue(value);
	Variable* result = *val - v;
	delete val;

	return result;
}

Variable* FieldVariableVar::operator *(Variable* v)
{
	Variable* val = field->GetValue(value);
	Variable* result = *val * v;
	delete val;

	return result;
}

Variable* FieldVariableVar::operator /(Variable* v)
{
	Variable* val = field->GetValue(value);
	Variable* result = *val / v;
	delete val;

	return result;
}

void FieldVariableVar::operator =(Variable* v)
{
	field->SetValue(value, v);
}

bool FieldVariableVar::operator ==(Variable* v)
{
	return *value == v;
}

bool FieldVariableVar::operator !=(Variable* v)
{
	return *value != v;
}

bool FieldVariableVar::operator <(Variable* v)
{
	return *value < v;
}

bool FieldVariableVar::operator <=(Variable* v)
{
	return *value <= v;
}

bool FieldVariableVar::operator >(Variable* v)
{
	return *value > v;
}

bool FieldVariableVar::operator >=(Variable* v)
{
	return *value >= v;
}
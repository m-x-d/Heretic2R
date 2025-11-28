//
// sc_FloatVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_FloatVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"

FloatVar::FloatVar(const char* new_name, const float new_value) : Variable(new_name, TYPE_FLOAT)
{
	value = new_value;
}

FloatVar::FloatVar(FILE* f, CScript* script) : Variable(f, script)
{
	fread(&value, 1, sizeof(value), f);
}

void FloatVar::Write(FILE* f, CScript* script, RestoreListID_t id)
{
	Variable::Write(f, script, RLID_FLOATVAR);
	fwrite(&value, 1, sizeof(value), f);
}

void FloatVar::ReadValue(CScript* script)
{
	value = script->ReadFloat();
}

void FloatVar::Debug(CScript* script)
{
	Variable::Debug(script);
	script->DebugLine("      Float Value: %0.f\n", value);
}

Variable* FloatVar::operator +(Variable* v)
{
	return new FloatVar("", value + v->GetFloatValue());
}

Variable* FloatVar::operator -(Variable* v)
{
	return new FloatVar("", value - v->GetFloatValue());
}

Variable* FloatVar::operator *(Variable* v)
{
	return new FloatVar("", value * v->GetFloatValue());
}

Variable* FloatVar::operator /(Variable* v)
{
	return new FloatVar("", value / v->GetFloatValue());
}

void FloatVar::operator =(Variable* v)
{
	value = v->GetFloatValue();
}
//
// sc_IntVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_IntVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"

IntVar::IntVar(const char* new_name, const int new_value) : Variable(new_name, TYPE_INT)
{
	value = new_value;
}

IntVar::IntVar(FILE* f, CScript* script) : Variable(f, script)
{
	fread(&value, 1, sizeof(value), f);
}

void IntVar::Write(FILE* f, CScript* script, int id)
{
	Variable::Write(f, script, RLID_INTVAR);
	fwrite(&value, 1, sizeof(value), f);
}

void IntVar::ReadValue(CScript* script)
{
	value = script->ReadInt();
}

void IntVar::Debug(CScript* script)
{
	Variable::Debug(script);
	script->DebugLine("      Integer Value: %d\n", value);
}

void IntVar::Signal(edict_t* which)
{
	value++;
}

void IntVar::ClearSignal()
{
	value = 0;
}

Variable* IntVar::operator +(Variable* v)
{
	return new IntVar("", value + v->GetIntValue());
}

Variable* IntVar::operator -(Variable* v)
{
	return new IntVar("", value - v->GetIntValue());
}

Variable* IntVar::operator *(Variable* v)
{
	return new IntVar("", value * v->GetIntValue());
}

Variable* IntVar::operator /(Variable* v)
{
	return new IntVar("", value / v->GetIntValue());
}

void IntVar::operator =(Variable* v)
{
	value = v->GetIntValue();
}
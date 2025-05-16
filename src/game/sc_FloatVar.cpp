//
// sc_FloatVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_FloatVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

FloatVar::FloatVar(char* Name, float InitValue)
	:Variable(Name, TYPE_FLOAT)
{
	Value = InitValue;
}

FloatVar::FloatVar(FILE* FH, CScript* Script)
	:Variable(FH, Script)
{
	fread(&Value, 1, sizeof(Value), FH);
}

void FloatVar::Write(FILE* FH, CScript* Script, int ID)
{
	Variable::Write(FH, Script, RLID_FLOATVAR);

	fwrite(&Value, 1, sizeof(Value), FH);
}

void FloatVar::ReadValue(CScript* Script)
{
	Value = Script->ReadFloat();
}

void FloatVar::Debug(CScript* Script)
{
	Variable::Debug(Script);

	Script->DebugLine("      Float Value: %0.f\n", Value);
}

Variable* FloatVar::operator +(Variable* VI)
{
	return new FloatVar("", Value + VI->GetFloatValue());
}

Variable* FloatVar::operator -(Variable* VI)
{
	return new FloatVar("", Value - VI->GetFloatValue());
}

Variable* FloatVar::operator *(Variable* VI)
{
	return new FloatVar("", Value * VI->GetFloatValue());
}

Variable* FloatVar::operator /(Variable* VI)
{
	return new FloatVar("", Value / VI->GetFloatValue());
}

void FloatVar::operator =(Variable* VI)
{
	Value = VI->GetFloatValue();
}
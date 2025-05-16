//
// sc_IntVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_IntVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

IntVar::IntVar(char* Name, int InitValue)
	:Variable(Name, TYPE_INT)
{
	Value = InitValue;
}

IntVar::IntVar(FILE* FH, CScript* Script)
	:Variable(FH, Script)
{
	fread(&Value, 1, sizeof(Value), FH);
}

void IntVar::Write(FILE* FH, CScript* Script, int ID)
{
	Variable::Write(FH, Script, RLID_INTVAR);

	fwrite(&Value, 1, sizeof(Value), FH);
}

void IntVar::ReadValue(CScript* Script)
{
	Value = Script->ReadInt();
}

void IntVar::Debug(CScript* Script)
{
	Variable::Debug(Script);

	Script->DebugLine("      Integer Value: %d\n", Value);
}

void IntVar::Signal(edict_t* Which)
{
	Value++;
}

void IntVar::ClearSignal(void)
{
	Value = 0;
}

Variable* IntVar::operator +(Variable* VI)
{
	return new IntVar("", Value + VI->GetIntValue());
}

Variable* IntVar::operator -(Variable* VI)
{
	return new IntVar("", Value - VI->GetIntValue());
}

Variable* IntVar::operator *(Variable* VI)
{
	return new IntVar("", Value * VI->GetIntValue());
}

Variable* IntVar::operator /(Variable* VI)
{
	return new IntVar("", Value / VI->GetIntValue());
}

void IntVar::operator =(Variable* VI)
{
	Value = VI->GetIntValue();
}
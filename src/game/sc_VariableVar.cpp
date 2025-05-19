//
// sc_VariableVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_VariableVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

VariableVar::VariableVar(char* Name)
	:Variable(Name, TYPE_UNKNOWN)
{
	Value = NULL;
}

VariableVar::VariableVar(FILE* FH, CScript* Script)
	:Variable(FH, Script)
{
	int index;

	fread(&index, 1, sizeof(index), FH);
	Value = Script->LookupVar(index);
}

void VariableVar::Write(FILE* FH, CScript* Script, int ID)
{
	int index;

	Variable::Write(FH, Script, RLID_VARIABLEVAR);

	index = Script->LookupVarIndex(Value);
	fwrite(&index, 1, sizeof(index), FH);
}

void VariableVar::ReadValue(CScript* Script)
{
	int Index;

	Index = Script->ReadInt();

	Value = Script->LookupVar(Index);

	if (Value)
	{
		type = Value->GetType();
	}
}

void VariableVar::Debug(CScript* Script)
{
	Value->Debug(Script);
}
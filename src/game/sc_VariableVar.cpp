//
// sc_VariableVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_VariableVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"

VariableVar::VariableVar(const char* new_name) : Variable(new_name, TYPE_UNKNOWN)
{
	value = nullptr;
}

VariableVar::VariableVar(FILE* f, CScript* script) : Variable(f, script)
{
	int index;
	fread(&index, 1, sizeof(index), f);

	value = script->LookupVar(index);
}

void VariableVar::Write(FILE* f, CScript* script, RestoreListID_t id)
{
	Variable::Write(f, script, RLID_VARIABLEVAR);

	const int index = script->LookupVarIndex(value);
	fwrite(&index, 1, sizeof(index), f);
}

void VariableVar::ReadValue(CScript* script)
{
	const int index = script->ReadInt();
	value = script->LookupVar(index);

	if (value != nullptr)
		type = value->GetType();
}

void VariableVar::Debug(CScript* script)
{
	value->Debug(script);
}
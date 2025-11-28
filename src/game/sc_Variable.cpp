//
// sc_Variable.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Variable.h"
#include "sc_CScript.h"

Variable::Variable(const char* new_name, const VariableType new_type)
{
	strcpy_s(name, new_name); //mxd. strcpy -> strcpy_s.
	type = new_type;
}

Variable::Variable(FILE* f, CScript* script)
{
	int index;

	fread(name, 1, sizeof(name), f);
	fread(&type, 1, sizeof(type), f);
	fread(&index, 1, sizeof(index), f);

	if (script != nullptr && index != -1)
		script->SetVarIndex(index, this);
}

void Variable::Write(FILE* f, CScript* script, const RestoreListID_t id)
{
	fwrite(&id, 1, sizeof(id), f);
	fwrite(name, 1, sizeof(name), f);
	fwrite(&type, 1, sizeof(type), f);

	const int index = ((script != nullptr) ? script->LookupVarIndex(this) : -1);
	fwrite(&index, 1, sizeof(index), f);
}

void Variable::Debug(CScript* script)
{
	script->DebugLine("   Name: %s\n", name);
}
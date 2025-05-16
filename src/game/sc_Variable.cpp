//
// sc_Variable.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Variable.h"
#include "sc_CScript.h"
#include "g_local.h"

Variable::Variable(char* NewName, VariableType NewType)
{
	strcpy(Name, NewName);
	Type = NewType;
}

Variable::Variable(FILE* FH, CScript* Script)
{
	int index;

	fread(Name, 1, sizeof(Name), FH);
	fread(&Type, 1, sizeof(Type), FH);
	fread(&index, 1, sizeof(index), FH);

	if (Script && index != -1)
	{
		Script->SetVarIndex(index, this);
	}
}

void Variable::Write(FILE* FH, CScript* Script, int ID)
{
	int	index = -1;

	fwrite(&ID, 1, sizeof(ID), FH);
	fwrite(Name, 1, sizeof(Name), FH);
	fwrite(&Type, 1, sizeof(Type), FH);

	if (Script)
	{
		index = Script->LookupVarIndex(this);
	}
	fwrite(&index, 1, sizeof(index), FH);
}

void Variable::Debug(CScript* Script)
{
	Script->DebugLine("   Name: %s\n", Name);
}
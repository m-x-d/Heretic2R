//
// sc_EntityVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_EntityVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

EntityVar::EntityVar(char* Name, int InitValue)
	:Variable(Name, TYPE_ENTITY)
{
	if (InitValue == -1)
	{
		Value = NULL;
	}
	else
	{
		Value = &g_edicts[InitValue];
	}
}

EntityVar::EntityVar(edict_t* Which)
	:Variable("", TYPE_ENTITY)
{
	Value = Which;
}

EntityVar::EntityVar(FILE* FH, CScript* Script)
	:Variable(FH, Script)
{
	int index;

	fread(&index, 1, sizeof(index), FH);

	if (index == -1)
	{
		Value = NULL;
	}
	else
	{
		Value = &g_edicts[index];
	}
}

void EntityVar::Write(FILE* FH, CScript* Script, int ID)
{
	int index;

	Variable::Write(FH, Script, RLID_ENTITYVAR);

	index = GetIntValue();
	fwrite(&index, 1, sizeof(index), FH);
}

void EntityVar::ReadValue(CScript* Script)
{
	int Index;

	Index = Script->ReadInt();
	if (Index == -1)
	{
		Value = NULL;
	}
	else
	{
		Value = &g_edicts[Index];
	}
}

void EntityVar::Debug(CScript* Script)
{
	Variable::Debug(Script);

	Script->DebugLine("      Entity Value: %d\n", GetIntValue());
}

int EntityVar::GetIntValue(void)
{
	if (Value)
	{
		return Value - g_edicts;
	}

	return -1;
}

void EntityVar::operator =(Variable* VI)
{
	Value = VI->GetEdictValue();
}

bool EntityVar::operator ==(Variable* VI)
{
	if (VI->GetType() == TYPE_INT)
	{
		return GetIntValue() == VI->GetIntValue();
	}
	else if (VI->GetType() == TYPE_ENTITY)
	{
		return GetEdictValue() == VI->GetEdictValue();
	}

	return false;
}

bool EntityVar::operator !=(Variable* VI)
{
	if (VI->GetType() == TYPE_INT)
	{
		return GetIntValue() != VI->GetIntValue();
	}
	else if (VI->GetType() == TYPE_ENTITY)
	{
		return GetEdictValue() != VI->GetEdictValue();
	}

	return false;
}
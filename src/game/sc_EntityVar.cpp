//
// sc_EntityVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_EntityVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

EntityVar::EntityVar(const char* new_name, const int ent_index) : Variable(new_name, TYPE_ENTITY)
{
	if (ent_index == -1)
		value = nullptr;
	else
		value = &g_edicts[ent_index]; //TODO: check if new_value >= 0 && < globals.num_edicts?
}

EntityVar::EntityVar(edict_t* which) : Variable("", TYPE_ENTITY)
{
	value = which;
}

EntityVar::EntityVar(FILE* f, CScript* script) : Variable(f, script)
{
	int index;
	fread(&index, 1, sizeof(index), f);

	if (index == -1)
		value = nullptr;
	else
		value = &g_edicts[index]; //TODO: check if index >= 0 && < globals.num_edicts?
}

void EntityVar::Write(FILE* f, CScript* script, int id)
{
	Variable::Write(f, script, RLID_ENTITYVAR);

	const int index = GetIntValue();
	fwrite(&index, 1, sizeof(index), f);
}

void EntityVar::ReadValue(CScript* script)
{
	const int index = script->ReadInt();

	if (index == -1)
		value = nullptr;
	else
		value = &g_edicts[index]; //TODO: check if index >= 0 && < globals.num_edicts?
}

void EntityVar::Debug(CScript* script)
{
	Variable::Debug(script);
	script->DebugLine("      Entity Value: %d\n", GetIntValue());
}

int EntityVar::GetIntValue()
{
	if (value != nullptr)
		return value - g_edicts;

	return -1;
}

void EntityVar::operator =(Variable* v)
{
	value = v->GetEdictValue();
}

bool EntityVar::operator ==(Variable* v)
{
	if (v->GetType() == TYPE_INT)
		return GetIntValue() == v->GetIntValue();

	if (v->GetType() == TYPE_ENTITY)
		return GetEdictValue() == v->GetEdictValue();

	return false;
}

bool EntityVar::operator !=(Variable* v)
{
	if (v->GetType() == TYPE_INT)
		return GetIntValue() != v->GetIntValue();

	if (v->GetType() == TYPE_ENTITY)
		return GetEdictValue() != v->GetEdictValue();

	return false;
}
//
// sc_Utility.cpp
//
// Copyright 1998 Raven Software
//

#include <map>
#include "sc_Utility.h"
#include "sc_CScript.h"
#include "sc_EntityVar.h"
#include "sc_ExecuteEvent.h"
#include "sc_FieldVariableVar.h"
#include "sc_FloatVar.h"
#include "sc_IntVar.h"
#include "sc_MoveDoneEvent.h"
#include "sc_RotateDoneEvent.h"
#include "sc_VariableVar.h"
#include "sc_VectorVar.h"
#include "sc_WaitEvent.h"
#include "g_local.h"

std::list<CScript*> Scripts;
List<Variable*> GlobalVariables;

void* RestoreObject(FILE* f, CScript* data)
{
	RestoreListID_t id;
	fread(&id, 1, sizeof(id), f);

	switch (id)
	{
		case RLID_INTVAR:			return new IntVar(f, data);
		case RLID_FLOATVAR:			return new FloatVar(f, data);
		case RLID_VECTORVAR:		return new VectorVar(f, data);
		case RLID_ENTITYVAR:		return new EntityVar(f, data);
		case RLID_STRINGVAR:		return new StringVar(f, data);
		case RLID_VARIABLEVAR:		return new VariableVar(f, data);
		case RLID_FIELDVARIABLEVAR:	return new FieldVariableVar(f, data);
		case RLID_SIGNALER:			return new Signaler(f, data);
		case RLID_MOVEDONEEVENT:	return new MoveDoneEvent(f, data);
		case RLID_ROTATEDONEEVENT:	return new RotateDoneEvent(f, data);
		case RLID_EXECUTEEVENT:		return new ExecuteEvent(f, data);
		case RLID_WAITEVENT:		return new WaitEvent(f, data);
		case RLID_SCRIPT:			return new CScript(f);
		case RLID_FIELDDEF:			return new FieldDef(f, data);
		default:					return nullptr;
	}
}

//==========================================================================

void ReadEnt(edict_t** to, FILE* f)
{
	int index;
	fread(&index, 1, sizeof(index), f);

	if (index < 0 || index >= globals.num_edicts)
	{
		assert(index == -1); // Else invalid edict number.
		*to = nullptr;
	}
	else
	{
		*to = g_edicts + index;
	}
}

void WriteEnt(edict_t** to, FILE* f)
{
	int index;

	if (*to != nullptr)
	{
		index = *to - g_edicts;
		assert(index >= 0 && index < globals.num_edicts); // Else invalid edict pointer.
	}
	else
	{
		index = -1;
	}

	fwrite(&index, 1, sizeof(index), f);
}

//==========================================================================

Variable* FindGlobal(const char* name)
{
	if (GlobalVariables.Size() > 0)
		for (List<Variable*>::Iter var = GlobalVariables.Begin(); var != GlobalVariables.End(); ++var)
			if (strcmp(name, (*var)->GetName()) == 0)
				return *var;

	return nullptr;
}

bool NewGlobal(Variable* var)
{
	if (FindGlobal(var->GetName()) != nullptr)
		return false; // Already exists.

	GlobalVariables.PushBack(var);
	return true;
}

//==========================================================================

void script_signaler(edict_t* which, const SignalT signal_type)
{
	for (CScript* script : Scripts)
		script->CheckSignalers(which, signal_type);
}

extern "C" void animate_signaler(edict_t* which)
{
	script_signaler(which, SIGNAL_ANIMATE);
}
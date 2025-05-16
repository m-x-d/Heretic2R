//
// sc_Utility.cpp
//
// Copyright 1998 Raven Software
//

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

List<CScript*> Scripts;
List<Variable*>	GlobalVariables;

//==========================================================================

static void* RF_IntVar(FILE* f, void* data)
{
	return new IntVar(f, static_cast<CScript*>(data));
}

static void* RF_FloatVar(FILE* f, void* data)
{
	return new FloatVar(f, static_cast<CScript*>(data));
}

static void* RF_VectorVar(FILE* f, void* data)
{
	return new VectorVar(f, static_cast<CScript*>(data));
}

static void* RF_EntityVar(FILE* f, void* data)
{
	return new EntityVar(f, static_cast<CScript*>(data));
}

static void* RF_StringVar(FILE* f, void* data)
{
	return new StringVar(f, static_cast<CScript*>(data));
}

static void* RF_VariableVar(FILE* f, void* data)
{
	return new VariableVar(f, static_cast<CScript*>(data));
}

static void* RF_FieldVariableVar(FILE* f, void* data)
{
	return new FieldVariableVar(f, static_cast<CScript*>(data));
}

static void* RF_Signaler(FILE* f, void* data)
{
	return new Signaler(f, static_cast<CScript*>(data));
}

static void* RF_MoveDoneEvent(FILE* f, void* data)
{
	return new MoveDoneEvent(f, static_cast<CScript*>(data));
}

static void* RF_RotateDoneEvent(FILE* f, void* data)
{
	return new RotateDoneEvent(f, static_cast<CScript*>(data));
}

static void* RF_ExecuteEvent(FILE* f, void* data)
{
	return new ExecuteEvent(f, static_cast<CScript*>(data));
}

static void* RF_WaitEvent(FILE* f, void* data)
{
	return new WaitEvent(f, static_cast<CScript*>(data));
}

static void* RF_Script(FILE* f, void* data)
{
	return new CScript(f);
}

static void* RF_FieldDef(FILE* f, void* data)
{
	return new FieldDef(f, static_cast<CScript*>(data));
}

RestoreList_t ScriptRL[] =
{
	{ RLID_INTVAR,				RF_IntVar },
	{ RLID_FLOATVAR,			RF_FloatVar },
	{ RLID_VECTORVAR,			RF_VectorVar },
	{ RLID_ENTITYVAR,			RF_EntityVar },
	{ RLID_STRINGVAR,			RF_StringVar },
	{ RLID_VARIABLEVAR,			RF_VariableVar },
	{ RLID_FIELDVARIABLEVAR,	RF_FieldVariableVar },
	{ RLID_SIGNALER,			RF_Signaler },
	{ RLID_MOVEDONEEVENT,		RF_MoveDoneEvent },
	{ RLID_ROTATEDONEEVENT,		RF_RotateDoneEvent },
	{ RLID_EXECUTEEVENT,		RF_ExecuteEvent },
	{ RLID_WAITEVENT,			RF_WaitEvent },
	{ RLID_SCRIPT,				RF_Script },
	{ RLID_FIELDDEF,			RF_FieldDef },

	{ 0,						nullptr },
};

//==========================================================================

void* RestoreObject(FILE* f, const RestoreList_t* list, void* data)
{
	int id;
	fread(&id, 1, sizeof(id), f);

	for (const RestoreList_t* pos = list; pos->alloc_func; pos++)
		if (pos->ID == id)
			return pos->alloc_func(f, data);

	return nullptr;
}

//==========================================================================

void ReadEnt(edict_t** to, FILE* f)
{
	int index;
	tRead(&index, f);

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

	tWrite(&index, f);
}

//==========================================================================

Variable* FindGlobal(char* Name)
{
	List<Variable*>::Iter	iv;

	if (GlobalVariables.Size())
	{
		for (iv = GlobalVariables.Begin(); iv != GlobalVariables.End(); iv++)
		{
			if (strcmp(Name, (*iv)->GetName()) == 0)
			{
				return *iv;
			}
		}
	}

	return NULL;
}

bool NewGlobal(Variable* Which)
{
	Variable* Check;

	Check = FindGlobal(Which->GetName());
	if (Check)
	{	// already exists
		return false;
	}

	GlobalVariables.PushBack(Which);

	return true;
}

//==========================================================================

void script_signaler(edict_t* which, SignalT SignalType)
{
	List<CScript*>::Iter	is;

	if (Scripts.Size())
	{
		for (is = Scripts.Begin(); is != Scripts.End(); is++)
		{
			(*is)->CheckSignalers(which, SignalType);
		}
	}
}

void animate_signaler(edict_t* which)
{
	script_signaler(which, SIGNAL_ANIMATE);
}
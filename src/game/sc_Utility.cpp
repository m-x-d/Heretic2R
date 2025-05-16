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

void* RF_IntVar(FILE* FH, void* Data)
{
	return new IntVar(FH, (CScript*)Data);
}

void* RF_FloatVar(FILE* FH, void* Data)
{
	return new FloatVar(FH, (CScript*)Data);
}

void* RF_VectorVar(FILE* FH, void* Data)
{
	return new VectorVar(FH, (CScript*)Data);
}

void* RF_EntityVar(FILE* FH, void* Data)
{
	return new EntityVar(FH, (CScript*)Data);
}

void* RF_StringVar(FILE* FH, void* Data)
{
	return new StringVar(FH, (CScript*)Data);
}

void* RF_VariableVar(FILE* FH, void* Data)
{
	return new VariableVar(FH, (CScript*)Data);
}

void* RF_FieldVariableVar(FILE* FH, void* Data)
{
	return new FieldVariableVar(FH, (CScript*)Data);
}

void* RF_Signaler(FILE* FH, void* Data)
{
	return new Signaler(FH, (CScript*)Data);
}

void* RF_MoveDoneEvent(FILE* FH, void* Data)
{
	return new MoveDoneEvent(FH, (CScript*)Data);
}

void* RF_RotateDoneEvent(FILE* FH, void* Data)
{
	return new RotateDoneEvent(FH, (CScript*)Data);
}

void* RF_ExecuteEvent(FILE* FH, void* Data)
{
	return new ExecuteEvent(FH, (CScript*)Data);
}

void* RF_WaitEvent(FILE* FH, void* Data)
{
	return new WaitEvent(FH, (CScript*)Data);
}

void* RF_Script(FILE* FH, void* Data)
{
	return new CScript(FH);
}

void* RF_FieldDef(FILE* FH, void* Data)
{
	return new FieldDef(FH, (CScript*)Data);
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

	{ 0,						NULL },
};

//==========================================================================

void* RestoreObject(FILE* FH, RestoreList_t* RestoreList, void* Data)
{
	int				ID;
	RestoreList_t* pos;

	fread(&ID, 1, sizeof(ID), FH);

	for (pos = RestoreList; pos->alloc_func; pos++)
	{
		if (pos->ID == ID)
		{
			return pos->alloc_func(FH, Data);
		}
	}

	return NULL;
}

//==========================================================================

void ReadEnt(edict_t** to, FILE* FH)
{
	int index;
	tRead(&index, FH);
	if (index < 0 || index >= globals.num_edicts)
	{
		assert(index == -1); //else invalid edict number
		*to = 0;
	}
	else
		*to = g_edicts + index;
}

void WriteEnt(edict_t** to, FILE* FH)
{
	int index;
	if (*to)
	{
		index = (*to) - g_edicts;
		assert(index >= 0 && index < globals.num_edicts); //else invalid edict pointer
	}
	else
		index = -1;
	tWrite(&index, FH);
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
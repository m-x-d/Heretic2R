//
// sc_Signaler.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Signaler.h"
#include "sc_Utility.h"

Signaler::Signaler(edict_t* NewEdict, Variable* NewVar, SignalT NewSignalType)
{
	Edict = NewEdict;
	Var = NewVar;
	SignalType = NewSignalType;
}

Signaler::Signaler(FILE* FH, CScript* Script)
{
	ReadEnt(&Edict, FH);
	tRead(&SignalType, FH);

	Var = (Variable*)RestoreObject(FH, ScriptRL, Script);
}

Signaler::~Signaler(void)
{
	if (Var)
	{
		delete Var;
	}
}

void Signaler::Write(FILE* FH, CScript* Script)
{
	int index;

	index = RLID_SIGNALER;
	fwrite(&index, 1, sizeof(index), FH);

	WriteEnt(&Edict, FH);
	tWrite(&SignalType, FH);

	Var->Write(FH, Script);
}

bool Signaler::Test(edict_t* Which, SignalT WhichType)
{
	if (WhichType != SignalType)
	{
		return false;
	}

	if (Edict != Which)
	{
		return false;
	}

	Var->Signal(Which);

	return true;
}

bool Signaler::operator ==(Signaler* SI)
{
	if (Var == SI->GetVar())
	{
		return true;
	}

	return false;
}
//
// sc_Signaler.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"
#include "g_local.h"

enum SignalT
{
	SIGNAL_MOVE,
	SIGNAL_ROTATE,
	SIGNAL_ANIMATE,
};

class Signaler
{
private:
	edict_t* Edict;
	Variable* Var;
	SignalT		SignalType;

public:
	Signaler(edict_t* NewEdict, Variable* NewVar, SignalT NewSignalType);
	Signaler(FILE* FH, CScript* Script);
	~Signaler(void);
	virtual void				Write(FILE* FH, CScript* Script);
	bool				Test(edict_t* Which, SignalT WhichType);
	edict_t* GetEdict(void) { return Edict; }
	Variable* GetVar(void) { return Var; }
	SignalT				GetType(void) { return SignalType; }
	bool	operator	==(Signaler* SI);
};
//
// sc_Signaler.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

enum SignalT
{
	SIGNAL_MOVE,
	SIGNAL_ROTATE,
	SIGNAL_ANIMATE,
};

class Signaler
{
	edict_t* edict = nullptr;
	Variable* var = nullptr;
	SignalT signal_type = SIGNAL_MOVE;

public:
	Signaler(edict_t* new_edict, Variable* new_var, SignalT new_signal_type);
	Signaler(FILE* f, CScript* script);
	~Signaler();

	void Write(FILE* f, CScript* script);
	bool Test(edict_t* which, SignalT which_type) const;

	edict_t* GetEdict() const
	{
		return edict;
	}

	Variable* GetVar() const
	{
		return var;
	}

	SignalT GetType() const
	{
		return signal_type;
	}

	bool operator ==(const Signaler* other) const;
};
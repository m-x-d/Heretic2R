//
// sc_Signaler.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_Signaler.h"
#include "sc_Utility.h"

Signaler::Signaler(edict_t* new_edict, Variable* new_var, const SignalT new_signal_type)
{
	edict = new_edict;
	var = new_var;
	signal_type = new_signal_type;
}

Signaler::Signaler(FILE* f, CScript* script)
{
	ReadEnt(&edict, f);
	fread(&signal_type, 1, sizeof(signal_type), f);

	var = static_cast<Variable*>(RestoreObject(f, script));
}

Signaler::~Signaler()
{
	delete var;
}

void Signaler::Write(FILE* f, CScript* script)
{
	constexpr int index = RLID_SIGNALER;
	fwrite(&index, 1, sizeof(index), f);

	WriteEnt(&edict, f);
	fwrite(&signal_type, 1, sizeof(signal_type), f);

	var->Write(f, script);
}

bool Signaler::Test(edict_t* which, const SignalT which_type) const
{
	if (which_type == signal_type && edict == which)
	{
		var->Signal(which);
		return true;
	}

	return false;
}

bool Signaler::operator ==(const Signaler* other) const
{
	return (var == other->GetVar());
}
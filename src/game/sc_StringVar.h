//
// sc_StringVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class StringVar : public Variable
{
protected:
	char Value[VAR_LENGTH];

public:
	StringVar(char* Name = "", char* InitValue = "");
	StringVar(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual char* GetStringValue(void) { return Value; }
	virtual void		ReadValue(CScript* Script);
};

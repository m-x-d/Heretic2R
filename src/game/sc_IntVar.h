//
// sc_IntVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class IntVar : public Variable
{
protected:
	int	Value;

public:
	IntVar(char* Name = "", int InitValue = 0);
	IntVar(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual int			GetIntValue(void) { return Value; }
	virtual float		GetFloatValue(void) { return (float)Value; }
	virtual void		ReadValue(CScript* Script);
	virtual void		Debug(CScript* Script);
	virtual void		Signal(edict_t* Which);
	virtual void		ClearSignal(void);

	virtual Variable* operator	+(Variable* VI);
	virtual Variable* operator	-(Variable* VI);
	virtual Variable* operator	*(Variable* VI);
	virtual Variable* operator	/(Variable* VI);
	virtual void	operator	=(Variable* VI);

	virtual bool	operator	==(Variable* VI) { return Value == VI->GetIntValue(); }
	virtual bool	operator	!=(Variable* VI) { return Value != VI->GetIntValue(); }
	virtual bool	operator	<(Variable* VI) { return Value < VI->GetIntValue(); }
	virtual bool	operator	<=(Variable* VI) { return Value <= VI->GetIntValue(); }
	virtual bool	operator	>(Variable* VI) { return Value > VI->GetIntValue(); }
	virtual bool	operator	>=(Variable* VI) { return Value >= VI->GetIntValue(); }
};
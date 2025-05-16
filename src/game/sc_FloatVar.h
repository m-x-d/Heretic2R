//
// sc_FloatVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class FloatVar : public Variable
{
protected:
	float	Value;

public:
	FloatVar(char* Name = "", float InitValue = 0.0);
	FloatVar(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual int			GetIntValue(void) { return (int)Value; }
	virtual float		GetFloatValue(void) { return Value; }
	virtual void		ReadValue(CScript* Script);
	virtual void		Debug(CScript* Script);

	virtual Variable* operator	+(Variable* VI);
	virtual Variable* operator	-(Variable* VI);
	virtual Variable* operator	*(Variable* VI);
	virtual Variable* operator	/(Variable* VI);
	virtual void	operator	=(Variable* VI);

	virtual bool	operator	==(Variable* VI) { return Value == VI->GetFloatValue(); }
	virtual bool	operator	!=(Variable* VI) { return Value != VI->GetFloatValue(); }
	virtual bool	operator	<(Variable* VI) { return Value < VI->GetFloatValue(); }
	virtual bool	operator	<=(Variable* VI) { return Value <= VI->GetFloatValue(); }
	virtual bool	operator	>(Variable* VI) { return Value > VI->GetFloatValue(); }
	virtual bool	operator	>=(Variable* VI) { return Value >= VI->GetFloatValue(); }
};
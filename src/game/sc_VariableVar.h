//
// sc_VariableVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class VariableVar : public Variable
{
protected:
	Variable* Value;

public:
	VariableVar(char* Name = "");
	VariableVar(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual int			GetIntValue(void) { return Value->GetIntValue(); }
	virtual float		GetFloatValue(void) { return Value->GetFloatValue(); }
	virtual void		GetVectorValue(vec3_t& VecValue) { Value->GetVectorValue(VecValue); }
	virtual edict_t* GetEdictValue(void) { return Value->GetEdictValue(); }
	virtual const char* GetStringValue(void) { return Value->GetStringValue(); }
	virtual void		ReadValue(CScript* Script);
	virtual void		Debug(CScript* Script);
	virtual void		Signal(edict_t* Which) { Value->Signal(Which); }
	virtual void		ClearSignal(void) { Value->ClearSignal(); }

	virtual Variable* operator	+(Variable* VI) { return (*Value) + VI; }
	virtual Variable* operator	-(Variable* VI) { return (*Value) - VI; }
	virtual Variable* operator	*(Variable* VI) { return (*Value) * VI; }
	virtual Variable* operator	/(Variable* VI) { return (*Value) / VI; }
	virtual void	operator	=(Variable* VI) { (*Value) = VI; }

	virtual bool	operator	==(Variable* VI) { return (*Value) == VI; }
	virtual bool	operator	!=(Variable* VI) { return (*Value) != VI; }
	virtual bool	operator	<(Variable* VI) { return (*Value) < VI; }
	virtual bool	operator	<=(Variable* VI) { return (*Value) <= VI; }
	virtual bool	operator	>(Variable* VI) { return (*Value) > VI; }
	virtual bool	operator	>=(Variable* VI) { return (*Value) >= VI; }
};
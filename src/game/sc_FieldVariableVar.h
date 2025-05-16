//
// sc_FieldVariableVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_FieldDef.h"

class FieldVariableVar : public Variable
{
protected:
	Variable* Value;
	FieldDef* Field;

public:
	FieldVariableVar(char* Name = "");
	FieldVariableVar(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual int			GetIntValue(void);
	virtual float		GetFloatValue(void);
	virtual void		GetVectorValue(vec3_t& VecValue);
	virtual edict_t* GetEdictValue(void);
	virtual char* GetStringValue(void);
	virtual void		ReadValue(CScript* Script);
	virtual void		Debug(CScript* Script);
	virtual void		Signal(edict_t* Which) { Value->Signal(Which); }
	virtual void		ClearSignal(void) { Value->ClearSignal(); }

	virtual Variable* operator	+(Variable* VI);
	virtual Variable* operator	-(Variable* VI);
	virtual Variable* operator	*(Variable* VI);
	virtual Variable* operator	/(Variable* VI);
	virtual void	operator	=(Variable* VI);

	virtual bool	operator	==(Variable* VI);
	virtual bool	operator	!=(Variable* VI);
	virtual bool	operator	<(Variable* VI);
	virtual bool	operator	<=(Variable* VI);
	virtual bool	operator	>(Variable* VI);
	virtual bool	operator	>=(Variable* VI);
};
//
// sc_Variable.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Pcode.h"
#include "g_local.h"

class Variable
{
protected:
	char		Name[VAR_LENGTH];
	VariableType	Type;

public:
	Variable(char* NewName = "", VariableType NewType = TYPE_UNKNOWN);
	Variable(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	char* GetName(void) { return Name; }
	VariableType			GetType(void) { return Type; }
	virtual int			GetIntValue(void) { return 0; }
	virtual float		GetFloatValue(void) { return 0.0; }
	virtual void		GetVectorValue(vec3_t& VecValue) { VecValue[0] = VecValue[1] = VecValue[2] = 0.0; }
	virtual edict_t* GetEdictValue(void) { return NULL; }
	virtual char* GetStringValue(void) { return ""; }
	virtual void		ReadValue(CScript* Script) {}
	virtual void		Debug(CScript* Script);
	virtual void		Signal(edict_t* Which) {}
	virtual void		ClearSignal(void) {}

	virtual Variable* operator	+(Variable* VI) { return NULL; }
	virtual Variable* operator	-(Variable* VI) { return NULL; }
	virtual Variable* operator	*(Variable* VI) { return NULL; }
	virtual Variable* operator	/(Variable* VI) { return NULL; }
	virtual void	operator	=(Variable* VI) {}
	virtual bool	operator	==(Variable* VI) { return false; }
	virtual bool	operator	!=(Variable* VI) { return false; }
	virtual bool	operator	<(Variable* VI) { return false; }
	virtual bool	operator	<=(Variable* VI) { return false; }
	virtual bool	operator	>(Variable* VI) { return false; }
	virtual bool	operator	>=(Variable* VI) { return false; }
};
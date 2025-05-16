//
// sc_EntityVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class EntityVar : public Variable
{
protected:
	edict_t* Value;

public:
	EntityVar(char* Name = "", int InitValue = 0);
	EntityVar(edict_t* Which);
	EntityVar(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual int			GetIntValue(void);
	virtual edict_t* GetEdictValue(void) { return Value; }
	virtual void		ReadValue(CScript* Script);
	virtual void		Debug(CScript* Script);

	virtual void	operator	=(Variable* VI);
	virtual bool	operator	==(Variable* VI);
	virtual bool	operator	!=(Variable* VI);
};
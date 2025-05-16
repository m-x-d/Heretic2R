//
// sc_VectorVar.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "sc_Variable.h"

class VectorVar : public Variable
{
protected:
	vec3_t	Value;

public:
	VectorVar(char* Name = "", float InitValueX = 0.0, float InitValueY = 0.0, float InitValueZ = 0.0);
	VectorVar(vec3_t NewValue);
	VectorVar(FILE* FH, CScript* Script);
	virtual void		Write(FILE* FH, CScript* Script, int ID = -1);
	virtual void		GetVectorValue(vec3_t& VecValue);
	virtual void		ReadValue(CScript* Script);
	virtual void		Debug(CScript* Script);

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
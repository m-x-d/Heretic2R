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
	char name[VAR_LENGTH] = {};
	VariableType type = TYPE_UNKNOWN;

public:
	Variable(const char* new_name = "", VariableType new_type = TYPE_UNKNOWN);
	Variable(FILE* f, CScript* script);
	virtual ~Variable() = default; //mxd. Added to avoid compiler warning...

	virtual void Write(FILE* f, CScript* script, RestoreListID_t id = RLID_UNDEFINED);

	const char* GetName() const
	{
		return name;
	}

	VariableType GetType() const
	{
		return type;
	}

	virtual int GetIntValue() const
	{
		return 0;
	}

	virtual float GetFloatValue() const
	{
		return 0.0f;
	}

	virtual void GetVectorValue(vec3_t& value) const
	{
		value[0] = 0.0f;
		value[1] = 0.0f;
		value[2] = 0.0f;
	}

	virtual edict_t* GetEdictValue() const
	{
		return nullptr;
	}

	virtual const char* GetStringValue() const
	{
		return "";
	}

	virtual void ReadValue(CScript* script) { }
	virtual void Debug(CScript* script);
	virtual void Signal(edict_t* which) { }
	virtual void ClearSignal() { }

	//TODO: add +=, -=, *=, /= operator overrides?
	virtual Variable* operator +(Variable* v)
	{
		return nullptr;
	}

	virtual Variable* operator -(Variable* v)
	{
		return nullptr;
	}

	virtual Variable* operator *(Variable* v)
	{
		return nullptr;
	}

	virtual Variable* operator /(Variable* v)
	{
		return nullptr;
	}

	virtual void operator =(Variable* v) { }

	virtual bool operator ==(Variable* v)
	{
		return false;
	}

	virtual bool operator !=(Variable* v)
	{
		return false;
	}

	virtual bool operator <(Variable* v)
	{
		return false;
	}

	virtual bool operator <=(Variable* v)
	{
		return false;
	}

	virtual bool operator >(Variable* v)
	{
		return false;
	}

	virtual bool operator >=(Variable* v)
	{
		return false;
	}
};
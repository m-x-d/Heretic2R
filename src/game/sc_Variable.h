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
	char name[VAR_LENGTH] = { 0 };
	VariableType type = TYPE_UNKNOWN;

public:
	Variable(const char* new_name = "", VariableType new_type = TYPE_UNKNOWN); //TODO: change new_name type to const char*.
	Variable(FILE* f, CScript* script);

	virtual void Write(FILE* f, CScript* script, int id = -1);

	char* GetName()
	{
		return name;
	}

	VariableType GetType() const
	{
		return type;
	}

	virtual int GetIntValue()
	{
		return 0;
	}

	virtual float GetFloatValue()
	{
		return 0.0f;
	}

	virtual void GetVectorValue(vec3_t& value)
	{
		value[0] = 0.0f;
		value[1] = 0.0f;
		value[2] = 0.0f;
	}

	virtual edict_t* GetEdictValue()
	{
		return nullptr;
	}

	virtual char* GetStringValue() //TODO: change return type to const char*.
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
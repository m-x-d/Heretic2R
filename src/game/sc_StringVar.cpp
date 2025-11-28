//
// sc_StringVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_StringVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"

StringVar::StringVar(const char* new_name, const char* new_value) : Variable(new_name, TYPE_STRING)
{
	strcpy_s(value, new_value); //mxd. strcpy -> strcpy_s.
}

StringVar::StringVar(FILE* f, CScript* script) : Variable(f, script)
{
	fread(&value, 1, sizeof(value), f);
}

void StringVar::Write(FILE* f, CScript* script, RestoreListID_t id)
{
	Variable::Write(f, script, RLID_STRINGVAR);
	fwrite(&value, 1, sizeof(value), f);
}

void StringVar::ReadValue(CScript* script)
{
	strcpy_s(value, script->ReadString()); //mxd. strcpy -> strcpy_s.
}
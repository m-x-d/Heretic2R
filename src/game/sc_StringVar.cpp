//
// sc_StringVar.cpp
//
// Copyright 1998 Raven Software
//

#include "sc_StringVar.h"
#include "sc_CScript.h"
#include "sc_Utility.h"
#include "g_local.h"

StringVar::StringVar(char* Name, char* InitValue)
	:Variable(Name, TYPE_STRING)
{
	strcpy(Value, InitValue);
}

StringVar::StringVar(FILE* FH, CScript* Script)
	:Variable(FH, Script)
{
	fread(&Value, 1, sizeof(Value), FH);
}

void StringVar::Write(FILE* FH, CScript* Script, int ID)
{
	Variable::Write(FH, Script, RLID_STRINGVAR);

	fwrite(&Value, 1, sizeof(Value), FH);
}

void StringVar::ReadValue(CScript* Script)
{
	strcpy(Value, Script->ReadString());
}
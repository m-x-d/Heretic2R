//
// cvar.c -- dynamic variable tracking
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"

static cvar_t* Cvar_FindVar(const char* var_name)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart
char* Cvar_VariableString(const char* var_name)
{
	const cvar_t* var = Cvar_FindVar(var_name);
	return (var != NULL ? var->string : "");
}

cvar_t* Cvar_Get(char* var_name, char* var_value, int flags)
{
	NOT_IMPLEMENTED
	return NULL;
}

void Cvar_Init(void)
{
	NOT_IMPLEMENTED
}
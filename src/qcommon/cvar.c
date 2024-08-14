//
// cvar.c -- dynamic variable tracking
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"

cvar_t* cvar_vars;

static cvar_t* Cvar_FindVar(const char* var_name)
{
	for (cvar_t* var = cvar_vars; var != NULL; var = var->next)
		if (strcmp(var_name, var->name) == 0)
			return var;

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
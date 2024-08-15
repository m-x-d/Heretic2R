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

cvar_t* Cvar_Set2(char* var_name, char* value, qboolean force)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart
cvar_t* Cvar_Set(char* var_name, char* value)
{
	return Cvar_Set2(var_name, value, false);
}

// Q2 counterpart
void Cvar_SetValue(char* var_name, const float value)
{
	char val[32];

	if (value == (int)value)
		Com_sprintf(val, sizeof(val), "%i", (int)value);
	else
		Com_sprintf(val, sizeof(val), "%f", (double)value);

	Cvar_Set(var_name, val);
}

static void Cvar_Set_f(void)
{
	NOT_IMPLEMENTED
}

static void Cvar_List_f(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void Cvar_Init(void)
{
	Cmd_AddCommand("set", Cvar_Set_f);
	Cmd_AddCommand("cvarlist", Cvar_List_f);
}
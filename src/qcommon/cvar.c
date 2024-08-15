//
// cvar.c -- dynamic variable tracking
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"

cvar_t* cvar_vars;
qboolean userinfo_modified;

static qboolean Cvar_InfoValidate(char* s)
{
	NOT_IMPLEMENTED
	return false;
}

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

cvar_t* Cvar_Set2(char* var_name, char* value, const qboolean force)
{
	cvar_t* var = Cvar_FindVar(var_name);
	if (var == NULL) // Create it
		return Cvar_Get(var_name, value, 0);

	if (var->flags & (CVAR_USERINFO | CVAR_SERVERINFO) && !Cvar_InfoValidate(var_name)) // Q2 validates 'value' instead of 'var_name'.
	{
		Com_Printf("invalid info cvar name\n");
		return NULL; // Q2 returns 'var' here.
	}

	if (!force)
	{
		if (var->flags & CVAR_NOSET)
		{
			Com_Printf("%s is write protected.\n", var_name);
			return var;
		}

		if (var->flags & CVAR_LATCH)
		{
			if (var->latched_string)
			{
				if (strcmp(value, var->latched_string) == 0)
					return var;

				Z_Free(var->latched_string);
			}
			else
			{
				if (strcmp(value, var->string) == 0)
					return var;
			}

			if (Com_ServerState())
			{
				Com_Printf("%s will be changed for next game.\n", var_name);
				var->latched_string = CopyString(value);
			}
			else
			{
				var->string = CopyString(value);
				var->value = (float)strtod(var->string, NULL); //mxd. atof -> strtod
				if (strcmp(var->name, "game") == 0)
				{
					FS_SetGamedir(var->string);
					FS_ExecAutoexec();
				}
			}

			return var;
		}
	}
	else
	{
		if (var->latched_string)
		{
			Z_Free(var->latched_string);
			var->latched_string = NULL;
		}
	}

	if (strcmp(value, var->string) == 0)
		return var; // Not changed

	var->modified = true;

	if (var->flags & CVAR_USERINFO)
		userinfo_modified = true; // Transmit at next opportunity

	Z_Free(var->string); // Free the old value string

	var->string = CopyString(value);
	var->value = (float)strtod(var->string, NULL); //mxd. atof -> strtod

	return var;
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
//
// cvar.c -- dynamic variable tracking
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"

cvar_t* cvar_vars;
qboolean userinfo_modified;

// Q2 counterpart
static qboolean Cvar_InfoValidate(const char* s)
{
	if (strstr(s, "\\") || strstr(s, "\"") || strstr(s, ";"))
		return false;

	return true;
}

static cvar_t* Cvar_FindVar(const char* var_name)
{
	for (cvar_t* var = &cvar_vars[0]; var != NULL; var = var->next)
		if (strcmp(var_name, var->name) == 0)
			return var;

	return NULL;
}

// Q2 counterpart
float Cvar_VariableValue(const char* var_name)
{
	const cvar_t* var = Cvar_FindVar(var_name);
	if (var != NULL)
		return (float)strtod(var->string, NULL); //mxd. atof -> strtod

	return 0.0f;
}

// Q2 counterpart
char* Cvar_VariableString(const char* var_name)
{
	const cvar_t* var = Cvar_FindVar(var_name);
	return (var != NULL ? var->string : "");
}

// Attempts to match a partial variable name for command line completion. Returns NULL if nothing fits.
const char* Cvar_CompleteVariable(const char* partial)
{
	const int len = (int)strlen(partial);

	if (len == 0)
		return NULL;

	// Check for exact match.
	for (const cvar_t* cvar = &cvar_vars[0]; cvar != NULL; cvar = cvar->next)
		if (strcmp(partial, cvar->name) == 0)
			return cvar->name;

	// Check for partial match.
	for (const cvar_t* cvar = &cvar_vars[0]; cvar != NULL; cvar = cvar->next)
		if (strncmp(partial, cvar->name, len) == 0)
			return cvar->name;

	return NULL;
}

// Similar to above, except that it goes to next match, if any.
const char* Cvar_CompleteVariableNext(const char* partial, const char* last) // H2
{
	if (last == NULL)
		return Cvar_CompleteVariable(partial);

	const int len = (int)strlen(partial);

	if (len == 0)
		return NULL;

	// Find previous match...
	const cvar_t* prev;
	for (prev = &cvar_vars[0]; prev != NULL; prev = prev->next)
		if (strcmp(last, prev->name) == 0)
			break;

	if (prev != NULL)
	{
		// Check for next exact match.
		for (const cvar_t* cvar = prev->next; cvar != NULL; cvar = cvar->next)
			if (strcmp(partial, cvar->name) == 0)
				return cvar->name;

		// Check for next partial match.
		for (const cvar_t* cvar = prev->next; cvar != NULL; cvar = cvar->next)
			if (strncmp(partial, cvar->name, len) == 0)
				return cvar->name;
	}

	return NULL;
}

// Q2 counterpart
// If the variable already exists, the value will not be set.
// The flags will be or 'ed in if the variable exists.
cvar_t* Cvar_Get(const char* var_name, const char* var_value, const int flags)
{
	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO) && !Cvar_InfoValidate(var_name))
	{
		Com_Printf("invalid info cvar name\n");
		return NULL;
	}

	cvar_t* var = Cvar_FindVar(var_name);
	if (var != NULL)
	{
		var->flags |= flags;
		return var;
	}

	if (var_value == NULL)
		return NULL;

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO) && !Cvar_InfoValidate(var_value)) //BUG: original Heretic 2 code validates 'var_name' again here (H2 bug?)
	{
		Com_Printf("invalid info cvar value\n");
		return NULL;
	}

	var = Z_Malloc(sizeof(cvar_t));
	var->name = CopyString(var_name);
	var->string = CopyString(var_value);
	var->modified = true;
	var->value = (float)strtod(var->string, NULL); //mxd. atof -> strtod
	var->flags = flags;

	// Link the variable in
	var->next = cvar_vars;
	cvar_vars = var;

	return var;
}

static cvar_t* Cvar_Set2(const char* var_name, const char* value, const qboolean force)
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
cvar_t* Cvar_ForceSet(const char* var_name, const char* value)
{
	return Cvar_Set2(var_name, value, true);
}

// Q2 counterpart
cvar_t* Cvar_Set(const char* var_name, const char* value)
{
	return Cvar_Set2(var_name, value, false);
}

// Q2 counterpart
cvar_t* Cvar_FullSet(const char* var_name, const char* value, const int flags)
{
	cvar_t* var = Cvar_FindVar(var_name);
	if (var == NULL)
		return Cvar_Get(var_name, value, flags); // Create it

	var->modified = true;

	if (var->flags & CVAR_USERINFO)
		userinfo_modified = true; // Transmit at next opportunity

	Z_Free(var->string); // Free the old value string

	var->string = CopyString(value);
	var->value = (float)strtod(var->string, NULL); //mxd. atof -> strtod
	var->flags = flags;

	return var;
}

// Q2 counterpart
void Cvar_SetValue(const char* var_name, const float value)
{
	char val[32];

	if (value == (int)value)
		Com_sprintf(val, sizeof(val), "%i", (int)value);
	else
		Com_sprintf(val, sizeof(val), "%f", (double)value);

	Cvar_Set(var_name, val);
}

// Q2 counterpart
// Any variables with latched values will now be updated
void Cvar_GetLatchedVars(void)
{
	for (cvar_t* var = &cvar_vars[0]; var != NULL; var = var->next)
	{
		if (var->latched_string == NULL)
			continue;

		Z_Free(var->string);
		var->string = var->latched_string;
		var->latched_string = NULL;
		var->value = (float)strtod(var->string, NULL); //mxd. atof -> strtod

		if (strcmp(var->name, "game") == 0)
		{
			FS_SetGamedir(var->string);
			FS_ExecAutoexec();
		}
	}
}

// Q2 counterpart
// Handles variable inspection and changing from the console
qboolean Cvar_Command(void)
{
	// Check variables
	const cvar_t* v = Cvar_FindVar(Cmd_Argv(0));
	if (v == NULL)
		return false;

	// Perform a variable print or set
	if (Cmd_Argc() == 1)
		Com_Printf("\"%s\" is \"%s\"\n", v->name, v->string);
	else
		Cvar_Set(v->name, Cmd_Argv(1));

	return true;
}

// Q2 counterpart
// Allows setting and defining of arbitrary cvars from console
static void Cvar_Set_f(void)
{
	int flags;

	const int c = Cmd_Argc();
	if (c != 3 && c != 4)
	{
		Com_Printf("usage: set <variable> <value> [u / s]\n");
		return;
	}

	if (c == 4)
	{
		if (strcmp(Cmd_Argv(3), "u") == 0)
		{
			flags = CVAR_USERINFO;
		}
		else if (strcmp(Cmd_Argv(3), "s") == 0)
		{
			flags = CVAR_SERVERINFO;
		}
		else
		{
			Com_Printf("flags can only be 'u' or 's'\n");
			return;
		}

		Cvar_FullSet(Cmd_Argv(1), Cmd_Argv(2), flags);
	}
	else
	{
		Cvar_Set(Cmd_Argv(1), Cmd_Argv(2));
	}
}

// Appends lines containing "set variable value" for all variables with the CVAR_ARCHIVE flag set to true.
void Cvar_WriteVariables(FILE* f) //mxd. const char* path -> FILE* f.
{
	for (const cvar_t* var = &cvar_vars[0]; var != NULL; var = var->next)
	{
		if (var->flags & CVAR_ARCHIVE)
		{
			char buffer[1024];
			Com_sprintf(buffer, sizeof(buffer), "set %s \"%s\"\n", var->name, var->string);

			fprintf(f, "%s", buffer);
		}
	}
}

// Q2 counterpart
static void Cvar_List_f(void)
{
	int i = 0;
	for (const cvar_t* var = &cvar_vars[0]; var != NULL; var = var->next, i++)
	{
		if (var->flags & CVAR_ARCHIVE)
			Com_Printf("*");
		else
			Com_Printf(" ");

		if (var->flags & CVAR_USERINFO)
			Com_Printf("U");
		else
			Com_Printf(" ");

		if (var->flags & CVAR_SERVERINFO)
			Com_Printf("S");
		else
			Com_Printf(" ");

		if (var->flags & CVAR_NOSET)
			Com_Printf("-");
		else if (var->flags & CVAR_LATCH)
			Com_Printf("L");
		else
			Com_Printf(" ");

		Com_Printf(" %s \"%s\"\n", var->name, var->string);
	}

	Com_Printf("%i cvars\n", i);
}

// Q2 counterpart
static char* Cvar_BitInfo(const int bit)
{
	static char	info[MAX_INFO_STRING];

	info[0] = 0;

	for (const cvar_t* var = &cvar_vars[0]; var != NULL; var = var->next)
		if (var->flags & bit)
			Info_SetValueForKey(info, var->name, var->string);

	return info;
}

// Q2 counterpart
// Returns an info string containing all the CVAR_USERINFO cvars.
char* Cvar_Userinfo(void)
{
	return Cvar_BitInfo(CVAR_USERINFO);
}

// Q2 counterpart
// Returns an info string containing all the CVAR_SERVERINFO cvars.
char* Cvar_Serverinfo(void)
{
	return Cvar_BitInfo(CVAR_SERVERINFO);
}

// Q2 counterpart
void Cvar_Init(void)
{
	Cmd_AddCommand("set", Cvar_Set_f);
	Cmd_AddCommand("cvarlist", Cvar_List_f);
}
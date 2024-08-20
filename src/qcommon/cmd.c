//
// cmd.c -- Heretic 2 script command processing module
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"

#define MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s* next;
	char name[MAX_ALIAS_NAME];
	char* value;
} cmdalias_t;

cmdalias_t* cmd_alias;

qboolean cmd_wait;

#define ALIAS_LOOP_COUNT	16
int alias_count; // For detecting runaway loops

#pragma region ========================== COMMAND BUFFER ==========================

sizebuf_t cmd_text;
byte cmd_text_buf[8192];

// Q2 counterpart
void Cbuf_Init(void)
{
	SZ_Init(&cmd_text, cmd_text_buf, sizeof(cmd_text_buf));
}

// Q2 counterpart
void Cbuf_AddText(const char* text)
{
	const int len = (int)strlen(text);

	if (cmd_text.cursize + len < cmd_text.maxsize)
		SZ_Write(&cmd_text, text, len);
	else
		Com_Printf("Cbuf_AddText: overflow\n");
}

void Cbuf_InsertText(char* text)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void Cbuf_Execute(void)
{
	int i;
	char line[1024];

	alias_count = 0; // Don't allow infinite alias loops

	while (cmd_text.cursize)
	{
		// Find a \n or ; line break
		char* text = (char*)cmd_text.data;

		int quotes = 0;
		for (i = 0; i < cmd_text.cursize; i++)
		{
			if (text[i] == '"')
				quotes++;

			if (!(quotes & 1) && text[i] == ';')
				break; // Don't break if inside a quoted string

			if (text[i] == '\n')
				break;
		}

		memcpy(line, text, i);
		line[i] = 0;

		// Delete the text from the command buffer and move remaining commands down.
		// This is necessary because commands (exec, alias) can insert data at the beginning of the text buffer.
		if (i == cmd_text.cursize)
		{
			cmd_text.cursize = 0;
		}
		else
		{
			i++;
			cmd_text.cursize -= i;
			memmove(text, text + i, cmd_text.cursize);
		}

		// Execute the command line
		Cmd_ExecuteString(line);

		if (cmd_wait)
		{
			// Skip out while text still remains in buffer, leaving it for next frame.
			cmd_wait = false;
			break;
		}
	}
}

// Q2 counterpart
void Cbuf_AddEarlyCommands(const qboolean clear)
{
	for (int i = 0; i < COM_Argc(); i++)
	{
		if (strcmp(COM_Argv(i), "+set") != 0)
			continue;

		Cbuf_AddText(va("set %s %s\n", COM_Argv(i + 1), COM_Argv(i + 2)));

		if (clear)
		{
			COM_ClearArgv(i);
			COM_ClearArgv(i + 1);
			COM_ClearArgv(i + 2);
		}

		i += 2;
	}
}

qboolean Cbuf_AddLateCommands(void)
{
	NOT_IMPLEMENTED
	return false;
}

#pragma endregion

#pragma region ========================== SCRIPT COMMANDS =========================

static void Cmd_List_f(void)
{
	NOT_IMPLEMENTED
}

static void AddTextToCommandBuffer(char* f, uint len)
{
	NOT_IMPLEMENTED
}

static qboolean Cmd_Exec(char* cmd)
{
	char filename[MAX_QPATH];
	char* buf;
	FILE* f;
	int len;

	if (strchr(cmd, ':') != NULL || strstr(cmd, "\\\\") != NULL || strstr(cmd, "//") != NULL)
		Com_sprintf(filename, sizeof(filename), "%s", cmd);
	else
		Com_sprintf(filename, sizeof(filename), "%s/%s", FS_Userdir(), cmd);

	// Try to load from OS filesystem
	if (fopen_s(&f, filename, "rb") == 0) //mxd. fopen -> fopen_s
	{
		Com_Printf("execing %s\n", cmd);

		len = FS_FileLength(f);
		buf = Z_Malloc(len);

		fread_s(buf, len, 1, len, f); //mxd. fread -> fread_s
		AddTextToCommandBuffer(buf, len);

		Z_Free(buf);
		fclose(f);

		return true;
	}

	// Load from quake filesystem
	len = FS_LoadFile(cmd, (void**)&buf);
	if (buf != NULL)
	{
		Com_Printf("execing %s\n", cmd);
		AddTextToCommandBuffer(buf, len);
		FS_FreeFile(buf);

		return true;
	}

	return false;
}

static void Cmd_Exec_f(void)
{
	char arg_list[200];

	if (Cmd_Argc() < 2) // != 2 in Q2
	{
		Com_Printf("exec <filename> : execute a script file\n");
		return;
	}

	arg_list[0] = '\0';

	for (int i = 1; i < Cmd_Argc(); i++)
	{
		strcat_s(arg_list, sizeof(arg_list), Cmd_Argv(i)); //mxd. strcat -> strcat_s
		strcat_s(arg_list, sizeof(arg_list), " "); //mxd. strcat -> strcat_s
	}

	arg_list[strlen(arg_list) - 1] = '\0';

	if (!Cmd_Exec(arg_list))
		Com_Printf("Unable to exec %s\n", Cmd_Argv(1));
}

static void Cmd_Echo_f(void)
{
	NOT_IMPLEMENTED
}

static void Cmd_Alias_f(void)
{
	NOT_IMPLEMENTED
}

static void Cmd_Wait_f(void)
{
	NOT_IMPLEMENTED
}

// New in H2
static void Cmd_CpuID_f(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== COMMAND EXECUTION =======================

typedef struct cmd_function_s
{
	struct cmd_function_s* next;
	const char* name;
	xcommand_t function;
} cmd_function_t;

static int cmd_argc;
static char* cmd_argv[MAX_STRING_TOKENS];
static char* cmd_null_string = "";
static char cmd_args[MAX_STRING_CHARS];

static cmd_function_t* cmd_functions; // Possible commands to execute

// Q2 counterpart
int Cmd_Argc(void)
{
	return cmd_argc;
}

// Q2 counterpart
char* Cmd_Argv(const int arg)
{
	if ((uint)arg >= (uint)cmd_argc)
		return cmd_null_string;

	return cmd_argv[arg];
}

// Q2 counterpart
// Cmd_MacroExpandString in Q2
static char* MacroExpandString(char* text)
{
	static char expanded[MAX_STRING_CHARS];
	char temporary[MAX_STRING_CHARS];

	qboolean inquote = false;
	char* scan = text;

	int len = (int)strlen(scan);
	if (len >= MAX_STRING_CHARS)
	{
		Com_Printf("Line exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
		return NULL;
	}

	int count = 0;

	for (int i = 0; i < len; i++)
	{
		if (scan[i] == '"')
			inquote ^= 1;

		// Don't expand inside quotes
		if (inquote)
			continue; 

		if (scan[i] != '$')
			continue;

		// Scan out the complete macro
		char* start = scan + i + 1;
		const char* token = COM_Parse(&start);

		if (start == NULL)
			continue;

		token = Cvar_VariableString(token);

		const int j = (int)strlen(token);
		len += j;
		if (len >= MAX_STRING_CHARS)
		{
			Com_Printf("Expanded line exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
			return NULL;
		}

		strncpy_s(temporary, sizeof(temporary), scan, i); //mxd. strncpy -> strncpy_s
		strcpy_s(temporary + i, sizeof(temporary) - i, token); //mxd. strcpy -> strcpy_s
		strcpy_s(temporary + i + j, sizeof(temporary) - i - j, start); //mxd. strcpy -> strcpy_s

		strcpy_s(expanded, sizeof(expanded), temporary); //mxd. strcpy -> strcpy_s
		scan = expanded;
		i--;

		if (++count == 100)
		{
			Com_Printf("Macro expansion loop, discarded.\n");
			return NULL;
		}
	}

	if (inquote)
	{
		Com_Printf("Line has unmatched quote, discarded.\n");
		return NULL;
	}

	return scan;
}

// Q2 counterpart
// Parses the given string into command line tokens.
// $Cvars will be expanded unless they are in a quoted token
void Cmd_TokenizeString(char* text, const qboolean macro_expand)
{
	// Clear the args from the last string
	for (int i = 0; i < cmd_argc; i++)
		Z_Free(cmd_argv[i]);

	cmd_argc = 0;
	cmd_args[0] = '\0';

	// Macro expand the text
	if (macro_expand)
		text = MacroExpandString(text);

	if (text == NULL)
		return;

	while (true)
	{
		// Skip whitespace up to a /n
		while (*text != '\0' && *text <= ' ' && *text != '\n')
			text++;

		if (*text == '\n')
		{
			// A newline separates commands in the buffer
			text++;
			break;
		}

		if (*text == '\0')
			return;

		// Set cmd_args to everything after the first arg
		if (cmd_argc == 1)
		{
			strcpy_s(cmd_args, sizeof(cmd_args), text); //mxd. strcpy -> strcpy_s

			// Strip off any trailing whitespaces
			for (int l = (int)strlen(cmd_args) - 1; l >= 0 && cmd_args[l] <= ' '; l--)
				cmd_args[l] = '\0';
		}

		const char* com_token = COM_Parse(&text);
		if (text != NULL && cmd_argc < MAX_STRING_TOKENS)
		{
			// Store argument
			const int arg_len = (int)strlen(com_token) + 1;
			cmd_argv[cmd_argc] = Z_Malloc(arg_len);
			strcpy_s(cmd_argv[cmd_argc], arg_len, com_token); //mxd. strcpy -> strcpy_s
			cmd_argc++;
		}
	}
}

// Q2 counterpart
void Cmd_AddCommand(const char* cmd_name, const xcommand_t function)
{
	cmd_function_t* cmd;

	// Fail if the command is a variable name
	if (Cvar_VariableString(cmd_name)[0])
	{
		Com_Printf("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
	}

	// Fail if the command already exists
	for (cmd = cmd_functions; cmd != NULL; cmd = cmd->next)
	{
		if (strcmp(cmd_name, cmd->name) == 0)
		{
			Com_Printf("Cmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}
	}

	cmd = Z_Malloc(sizeof(cmd_function_t));
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

void Cmd_RemoveCommand(char* cmd_name)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
// A complete command line has been parsed, so try to execute it.
// FIXME: lookupnoadd the token to speed search?
void Cmd_ExecuteString(char* text)
{
	Cmd_TokenizeString(text, true);

	// Execute the command line
	if (!Cmd_Argc())
		return; // No tokens

	// Check functions
	for (const cmd_function_t* cmd = cmd_functions; cmd != NULL; cmd = cmd->next)
	{
		if (Q_stricmp(cmd_argv[0], cmd->name) == 0)
		{
			if (cmd->function != NULL)
				cmd->function();
			else
				Cmd_ExecuteString(va("cmd %s", text)); // Forward to server command

			return;
		}
	}

	// Check alias
	for (const cmdalias_t* a = cmd_alias; a != NULL; a = a->next)
	{
		if (Q_stricmp(cmd_argv[0], a->name) == 0)
		{
			if (++alias_count < ALIAS_LOOP_COUNT)
				Cbuf_InsertText(a->value);
			else
				Com_Printf("ALIAS_LOOP_COUNT\n");
			
			return;
		}
	}

	// Check cvars
	if (Cvar_Command())
		return;

	// Send it as a server command if we are connected
	Cmd_ForwardToServer();
}

void Cmd_ForwardToServer(void)
{
	NOT_IMPLEMENTED
}

void Cmd_Init(void)
{
	// Register our commands
	Cmd_AddCommand("cmdlist", Cmd_List_f);
	Cmd_AddCommand("exec", Cmd_Exec_f);
	Cmd_AddCommand("echo", Cmd_Echo_f);
	Cmd_AddCommand("alias", Cmd_Alias_f);
	Cmd_AddCommand("wait", Cmd_Wait_f);
	Cmd_AddCommand("cpuid", Cmd_CpuID_f); // New in H2
}

#pragma endregion
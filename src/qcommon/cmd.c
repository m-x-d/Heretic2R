//
// cmd.c -- Heretic 2 script command processing module
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"

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

static void Cmd_Exec_f(void)
{
	NOT_IMPLEMENTED
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

void Cmd_ExecuteString(char* text)
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
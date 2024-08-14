//
// cmd.c -- Heretic 2 script command processing module
//
// Copyright 1998 Raven Software
//

#include "qcommon.h"

#pragma region ========================== COMMAND BUFFER ==========================

sizebuf_t cmd_text;
byte cmd_text_buf[8192];

// Q2 counterpart
void Cbuf_Init(void)
{
	SZ_Init(&cmd_text, cmd_text_buf, sizeof(cmd_text_buf));
}

void Cbuf_AddText(char* text)
{
	NOT_IMPLEMENTED
}

void Cbuf_Execute(void)
{
	NOT_IMPLEMENTED
}

void Cbuf_AddEarlyCommands(qboolean clear)
{
	NOT_IMPLEMENTED
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

static cmd_function_t* cmd_functions; // Possible commands to execute

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
	for (cmd = cmd_functions; cmd; cmd = cmd->next)
	{
		if (!strcmp(cmd_name, cmd->name))
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
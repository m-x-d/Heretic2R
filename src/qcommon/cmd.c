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

void Cmd_AddCommand(char* cmd_name, xcommand_t function)
{
	NOT_IMPLEMENTED
}

void Cmd_Init(void)
{
	Cmd_AddCommand("cmdlist", Cmd_List_f);
	Cmd_AddCommand("exec", Cmd_Exec_f);
	Cmd_AddCommand("echo", Cmd_Echo_f);
	Cmd_AddCommand("alias", Cmd_Alias_f);
	Cmd_AddCommand("wait", Cmd_Wait_f);
	Cmd_AddCommand("cpuid", Cmd_CpuID_f); // New in H2
}

#pragma endregion
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

#pragma endregion

#pragma region ========================== COMMAND EXECUTION =======================

void Cmd_AddCommand(char* cmd_name, xcommand_t function)
{
	NOT_IMPLEMENTED
}

void Cmd_Init(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion
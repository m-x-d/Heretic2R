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

static cmdalias_t* cmd_alias;
static qboolean cmd_wait;

#define ALIAS_LOOP_COUNT	16
static int alias_count; // For detecting runaway loops

#pragma region ========================== COMMAND BUFFER ==========================

static sizebuf_t cmd_text;
static byte cmd_text_buf[8192];
static byte defer_text_buf[8192];

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

// Adds command text immediately after the current command. Adds a \n to the text.
// FIXME: actually change the command buffer to do less copying.
void Cbuf_InsertText(const char* text)
{
	char* temp;

	// Copy off any commands still remaining in the exec buffer
	const int templen = cmd_text.cursize;

	if (templen > 0)
	{
		temp = Z_Malloc(templen);
		memcpy(temp, cmd_text.data, templen);
		SZ_Clear(&cmd_text);
	}
	else
	{
		temp = NULL; // Shut up compiler
	}

	// Add the entire text of the file
	Cbuf_AddText(text);
	Cbuf_AddText("\n"); // H2

	// Add the copied off data
	if (templen > 0)
	{
		SZ_Write(&cmd_text, temp, templen);
		Z_Free(temp);
	}
}

// Q2 counterpart
void Cbuf_CopyToDefer(void)
{
	memcpy(defer_text_buf, cmd_text_buf, cmd_text.cursize);
	defer_text_buf[cmd_text.cursize] = 0;
	cmd_text.cursize = 0;
}

// Q2 counterpart
void Cbuf_InsertFromDefer(void)
{
	Cbuf_InsertText((char*)defer_text_buf);
	defer_text_buf[0] = 0;
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

// Q2 counterpart
// Adds command line parameters as script statements.
// Commands lead with a + and continue until another + or -.
// Returns true if any late commands were added, which will keep the demoloop from immediately starting.
qboolean Cbuf_AddLateCommands(void)
{
	// Build the combined string to parse from
	int s = 0;
	const int argc = COM_Argc();
	for (int i = 1; i < argc; i++)
		s += (int)strlen(COM_Argv(i)) + 1;

	if (s == 0)
		return false;

	const int text_len = s + 1;
	char* text = Z_Malloc(text_len);
	text[0] = 0;
	for (int i = 1; i < argc; i++)
	{
		strcat_s(text, text_len, COM_Argv(i)); //mxd. strcat -> strcat_s
		if (i != argc - 1)
			strcat_s(text, text_len, " "); //mxd. strcat -> strcat_s
	}

	// Pull out the commands
	char* build = Z_Malloc(text_len);
	build[0] = 0;

	for (int i = 0; i < s - 1; i++)
	{
		if (text[i] == '+')
		{
			i++;

			int j = i;
			while (text[j] != '+' && text[j] != '-' && text[j] != 0)
				j++;

			const char c = text[j];
			text[j] = 0;

			strcat_s(build, text_len, text + i); //mxd. strcat -> strcat_s
			strcat_s(build, text_len, "\n"); //mxd. strcat -> strcat_s
			text[j] = c;
			i = j - 1;
		}
	}

	const qboolean ret = (build[0] != 0);
	if (ret)
		Cbuf_AddText(build);

	Z_Free(text);
	Z_Free(build);

	return ret;
}

#pragma endregion

#pragma region ========================== SCRIPT COMMANDS =========================

// Part of Cmd_Exec_f in Q2
static void AddTextToCommandBuffer(const char* f, const int len)
{
	// The file doesn't have a trailing 0, so we need to copy it off
	char* f2 = Z_Malloc(len + 1);
	memcpy(f2, f, len);
	f2[len] = '\0';

	Cbuf_InsertText(f2);

	Z_Free(f2);
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

// Q2 counterpart
// Just prints the rest of the line to the console.
static void Cmd_Echo_f(void)
{
	for (int i = 1; i < Cmd_Argc(); i++)
		Com_Printf("%s ", Cmd_Argv(i));

	Com_Printf("\n");
}

// Q2 counterpart
// Creates a new command that executes a command string (possibly ; seperated).
static void Cmd_Alias_f(void)
{
	cmdalias_t* a;
	char cmd[1024];

	if (Cmd_Argc() == 1)
	{
		Com_Printf("Current alias commands:\n");
		for (a = cmd_alias; a != NULL; a = a->next)
			Com_Printf("%s : %s\n", a->name, a->value);

		return;
	}

	const char* s = Cmd_Argv(1);
	if (strlen(s) >= MAX_ALIAS_NAME)
	{
		Com_Printf("Alias name is too long\n");
		return;
	}

	// If the alias already exists, reuse it.
	for (a = cmd_alias; a != NULL; a = a->next)
	{
		if (strcmp(s, a->name) == 0)
		{
			Z_Free(a->value);
			break;
		}
	}

	if (a == NULL)
	{
		a = Z_Malloc(sizeof(cmdalias_t));
		a->next = cmd_alias;
		cmd_alias = a;
	}

	strcpy_s(a->name, sizeof(a->name), s); //mxd. strcpy -> strcpy_s

	// Copy the rest of the command line.
	cmd[0] = 0; // Start out with a null string.

	const int c = Cmd_Argc();
	for (int i = 2; i < c; i++)
	{
		strcat_s(cmd, sizeof(cmd), Cmd_Argv(i)); //mxd. strcat -> strcat_s
		if (i != c - 1)
			strcat_s(cmd, sizeof(cmd), " "); //mxd. strcat -> strcat_s
	}

	strcat_s(cmd, sizeof(cmd), "\n");
	a->value = CopyString(cmd);
}

// Q2 counterpart
// Causes execution of the remainder of the command buffer to be delayed until next frame.
// This allows commands like:
// bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
static void Cmd_Wait_f(void)
{
	cmd_wait = true;
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
// Returns a single string containing argv(1) to argv(argc() - 1)
char* Cmd_Args(void)
{
	return cmd_args;
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

		if (text == NULL)
			return;

		if (cmd_argc < MAX_STRING_TOKENS)
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

// Q2 counterpart
void Cmd_RemoveCommand(const char* cmd_name)
{
	cmd_function_t** back = &cmd_functions;
	while (true)
	{
		cmd_function_t* cmd = *back;
		if (cmd == NULL)
		{
			Com_Printf("Cmd_RemoveCommand: %s not added\n", cmd_name);
			return;
		}

		if (strcmp(cmd_name, cmd->name) == 0)
		{
			*back = cmd->next;
			Z_Free(cmd);

			return;
		}

		back = &cmd->next;
	}
}

// Q2 counterpart
// Attempts to match a partial command for automatic command line completion. Returns NULL if nothing fits.
const char* Cmd_CompleteCommand(const char* partial)
{
	const int len = (int)strlen(partial);

	if (len == 0)
		return NULL;

	// Check for exact match.
	for (const cmd_function_t* cmd = cmd_functions; cmd != NULL; cmd = cmd->next)
		if (strcmp(partial, cmd->name) == 0)
			return cmd->name;

	for (const cmdalias_t* a = cmd_alias; a != NULL; a = a->next)
		if (strcmp(partial, a->name) == 0)
			return a->name;

	// Check for partial match.
	for (const cmd_function_t* cmd = cmd_functions; cmd != NULL; cmd = cmd->next)
		if (strncmp(partial, cmd->name, len) == 0)
			return cmd->name;

	for (const cmdalias_t* a = cmd_alias; a != NULL; a = a->next)
		if (strncmp(partial, a->name, len) == 0)
			return a->name;

	return NULL;
}

// Similar to above, but returns the next value after last.
const char* Cmd_CompleteCommandNext(const char* partial, const char* last) // H2
{
	if (last == NULL)
		return Cmd_CompleteCommand(partial);

	const int len = (int)strlen(partial);

	if (len == 0)
		return NULL;

	// Find previous function match...
	const cmd_function_t* prev_cmd;
	for (prev_cmd = cmd_functions; prev_cmd != NULL; prev_cmd = prev_cmd->next)
		if (strcmp(last, prev_cmd->name) == 0)
			break;

	if (prev_cmd != NULL)
	{
		// Check for next exact match.
		for (const cmd_function_t* cmd = prev_cmd->next; cmd != NULL; cmd = cmd->next)
			if (strcmp(partial, cmd->name) == 0)
				return cmd->name;

		// Check for next partial match.
		for (const cmd_function_t* cmd = prev_cmd->next; cmd != NULL; cmd = cmd->next)
			if (strncmp(partial, cmd->name, len) == 0)
				return cmd->name;
	}

	// Find previous alias match...
	const cmdalias_t* prev_a;
	for (prev_a = cmd_alias; prev_a != NULL; prev_a = prev_a->next)
		if (strcmp(last, prev_a->name) == 0)
			break;

	if (prev_a != NULL)
	{
		// Check for next exact match.
		for (const cmdalias_t* a = prev_a->next; a != NULL; a = a->next)
			if (strcmp(partial, a->name) == 0)
				return a->name;

		// Check for next partial match.
		for (const cmdalias_t* a = prev_a->next; a != NULL; a = a->next)
			if (strncmp(partial, a->name, len) == 0)
				return a->name;
	}

	return NULL;
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

// Q2 counterpart
static void Cmd_List_f(void)
{
	int cmd_count = 0;
	for (const cmd_function_t* cmd = cmd_functions; cmd != NULL; cmd = cmd->next, cmd_count++)
		Com_Printf("%s\n", cmd->name);

	Com_Printf("%i commands\n", cmd_count);
}

void Cmd_Init(void)
{
	// Register our commands
	Cmd_AddCommand("cmdlist", Cmd_List_f);
	Cmd_AddCommand("exec", Cmd_Exec_f);
	Cmd_AddCommand("echo", Cmd_Echo_f);
	Cmd_AddCommand("alias", Cmd_Alias_f);
	Cmd_AddCommand("wait", Cmd_Wait_f);
	//Cmd_AddCommand("cpuid", Cmd_CpuID_f); // H2
}

#pragma endregion
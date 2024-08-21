//
// keys.c
//
// Copyright 1998 Raven Software
//

#include "client.h"

// Key up events are sent even if in console mode

#define MAXCMDLINE		256

char key_lines[32][MAXCMDLINE];
int key_linepos;

char* keybindings[256];
char* keybindings_double[256];
qboolean consolekeys[256];	// If true, can't be rebound while in console
qboolean menubound[256];	// If true, can't be rebound while in menu
int keyshift[256];			// Key to map to if Shift held down in console
int key_repeats[256];		// if > 1, it is autorepeating

static int Key_StringToKeynum(char* str)
{
	NOT_IMPLEMENTED
	return 0;
}

char* Key_KeynumToString(int keynum)
{
	NOT_IMPLEMENTED
	return NULL;
}

void Key_SetBinding(int keynum, char* binding)
{
	NOT_IMPLEMENTED
}

static void Key_Unbind_f(void)
{
	NOT_IMPLEMENTED
}

static void Key_Unbindall_f(void)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
static void Key_Bind_f(void)
{
	char cmd[1024];

	const int c = Cmd_Argc();
	if (c < 2)
	{
		Com_Printf("bind <key> [command] : attach a command to a key\n");
		return;
	}

	const int b = Key_StringToKeynum(Cmd_Argv(1));
	if (b == -1)
	{
		Com_Printf("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b] != NULL)
			Com_Printf("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b]);
		else
			Com_Printf("\"%s\" is not bound\n", Cmd_Argv(1));

		return;
	}

	// Copy the rest of the command line
	cmd[0] = 0; // Start out with a null string

	for (int i = 2; i < c; i++)
	{
		strcat_s(cmd, sizeof(cmd), Cmd_Argv(i)); //mxd. strcat -> strcat_s
		if (i != c - 1)
			strcat_s(cmd, sizeof(cmd), " "); //mxd. strcat -> strcat_s
	}

	Key_SetBinding(b, cmd);
}

static void Key_Bindlist_f(void)
{
	NOT_IMPLEMENTED
}

static void Key_BindDouble_f(void)
{
	NOT_IMPLEMENTED
}

static void Key_UnbindDouble_f(void)
{
	NOT_IMPLEMENTED
}

static void Key_DoubleBindList_f(void)
{
	NOT_IMPLEMENTED
}

static void Key_UnbindCommand_f(void)
{
	NOT_IMPLEMENTED
}

static void Key_UnbindallCommands_f(void)
{
	NOT_IMPLEMENTED
}

static void Key_BindCommand_f(void)
{
	NOT_IMPLEMENTED
}

static void Key_CommandsList_f(void)
{
	NOT_IMPLEMENTED
}

void Key_Init(void)
{
	for (int i = 0; i < 32; i++)
	{
		key_lines[i][0] = '>'; // Q2: ']';
		key_lines[i][1] = 0;
	}
	
	key_linepos = 1;

	// Init ascii characters in console mode
	for (int i = 32; i < 128; i++)
		consolekeys[i] = true;

	consolekeys[K_ENTER] = true;
	consolekeys[K_KP_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_KP_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_KP_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_KP_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_KP_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_HOME] = true;
	consolekeys[K_KP_HOME] = true;
	consolekeys[K_END] = true;
	consolekeys[K_KP_END] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_KP_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_KP_PGDN] = true;
	consolekeys[K_SHIFT] = true;
	consolekeys[K_INS] = true;
	consolekeys[K_KP_INS] = true;
	consolekeys[K_KP_DEL] = true;
	consolekeys[K_KP_SLASH] = true;
	consolekeys[K_KP_PLUS] = true;
	consolekeys[K_KP_MINUS] = true;
	consolekeys[K_KP_5] = true;

	// Missing in H2:
	//consolekeys['`'] = false;
	//consolekeys['~'] = false;

	for (int i = 0; i < 256; i++)
		keyshift[i] = i;

	for (int i = 'a'; i <= 'z'; i++)
		keyshift[i] = i - 'a' + 'A';

	keyshift['1'] = '!';
	keyshift['2'] = '@';
	keyshift['3'] = '#';
	keyshift['4'] = '$';
	keyshift['5'] = '%';
	keyshift['6'] = '^';
	keyshift['7'] = '&';
	keyshift['8'] = '*';
	keyshift['9'] = '(';
	keyshift['0'] = ')';
	keyshift['-'] = '_';
	keyshift['='] = '+';
	keyshift[','] = '<';
	keyshift['.'] = '>';
	keyshift['/'] = '?';
	keyshift[';'] = ':';
	keyshift['\''] = '"';
	keyshift['['] = '{';
	keyshift[']'] = '}';
	keyshift['`'] = '~';
	keyshift['\\'] = '|';

	menubound[K_ESCAPE] = true;

	for (int i = 0; i < 12; i++)
		menubound[K_F1 + i] = true;

	// Register our functions
	Cmd_AddCommand("bind", Key_Bind_f);
	Cmd_AddCommand("unbind", Key_Unbind_f);
	Cmd_AddCommand("unbindall", Key_Unbindall_f);
	Cmd_AddCommand("bindlist", Key_Bindlist_f);

	// New in H2:
	Cmd_AddCommand("bind_double", Key_BindDouble_f);
	Cmd_AddCommand("unbind_double", Key_UnbindDouble_f);
	Cmd_AddCommand("bindlist_double", Key_DoubleBindList_f);
	Cmd_AddCommand("bind_command", Key_BindCommand_f);
	Cmd_AddCommand("unbind_command", Key_UnbindCommand_f);
	Cmd_AddCommand("unbindall_command", Key_UnbindallCommands_f);
	Cmd_AddCommand("bindlist_command", Key_CommandsList_f);
}

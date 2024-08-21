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
char* commandbindings[256]; //New in H2
qboolean consolekeys[256];	// If true, can't be rebound while in console
qboolean menubound[256];	// If true, can't be rebound while in menu
int keyshift[256];			// Key to map to if Shift held down in console
int key_repeats[256];		// if > 1, it is autorepeating

typedef struct
{
	char* name;
	int keynum;
} keyname_t;

keyname_t keynames[] =
{
	{ "Tab",		K_TAB },
	{ "Enter",		K_ENTER },
	{ "Escape",		K_ESCAPE },
	{ "Space",		K_SPACE },
	{ "Backspace",	K_BACKSPACE },
	{ "UpArrow",	K_UPARROW },
	{ "DownArrow",	K_DOWNARROW },
	{ "LeftArrow",	K_LEFTARROW },
	{ "RightArrow",	K_RIGHTARROW },

	{ "Alt",	K_ALT },
	{ "Ctrl",	K_CTRL },
	{ "Shift",	K_SHIFT },

	{ "F1",		K_F1 },
	{ "F2",		K_F2 },
	{ "F3",		K_F3 },
	{ "F4",		K_F4 },
	{ "F5",		K_F5 },
	{ "F6",		K_F6 },
	{ "F7",		K_F7 },
	{ "F8",		K_F8 },
	{ "F9",		K_F9 },
	{ "F10",	K_F10 },
	{ "F11",	K_F11 },
	{ "F12",	K_F12 },

	{ "Ins",	K_INS },
	{ "Del",	K_DEL },
	{ "PgDn",	K_PGDN },
	{ "PgUp",	K_PGUP },
	{ "Home",	K_HOME },
	{ "End",	K_END },

	{ "Mouse1",	K_MOUSE1 },
	{ "Mouse2",	K_MOUSE2 },
	{ "Mouse3",	K_MOUSE3 },

	{ "Joy1",	K_JOY1 },
	{ "Joy2",	K_JOY2 },
	{ "Joy3",	K_JOY3 },
	{ "Joy4",	K_JOY4 },

	{ "Aux1",	K_AUX1 },
	{ "Aux2",	K_AUX2 },
	{ "Aux3",	K_AUX3 },
	{ "Aux4",	K_AUX4 },
	{ "Aux5",	K_AUX5 },
	{ "Aux6",	K_AUX6 },
	{ "Aux7",	K_AUX7 },
	{ "Aux8",	K_AUX8 },
	{ "Aux9",	K_AUX9 },
	{ "Aux10",	K_AUX10 },
	{ "Aux11",	K_AUX11 },
	{ "Aux12",	K_AUX12 },
	{ "Aux13",	K_AUX13 },
	{ "Aux14",	K_AUX14 },
	{ "Aux15",	K_AUX15 },
	{ "Aux16",	K_AUX16 },
	{ "Aux17",	K_AUX17 },
	{ "Aux18",	K_AUX18 },
	{ "Aux19",	K_AUX19 },
	{ "Aux20",	K_AUX20 },
	{ "Aux21",	K_AUX21 },
	{ "Aux22",	K_AUX22 },
	{ "Aux23",	K_AUX23 },
	{ "Aux24",	K_AUX24 },
	{ "Aux25",	K_AUX25 },
	{ "Aux26",	K_AUX26 },
	{ "Aux27",	K_AUX27 },
	{ "Aux28",	K_AUX28 },
	{ "Aux29",	K_AUX29 },
	{ "Aux30",	K_AUX30 },
	{ "Aux31",	K_AUX31 },
	{ "Aux32",	K_AUX32 },

	{ "Kp_Home",	K_KP_HOME },
	{ "Kp_Up",		K_KP_UPARROW },
	{ "Kp_PgUp",	K_KP_PGUP },
	{ "Kp_Left",	K_KP_LEFTARROW },
	{ "Kp_5",		K_KP_5 },
	{ "Kp_Right",	K_KP_RIGHTARROW },
	{ "Kp_End",		K_KP_END },
	{ "Kp_Down",	K_KP_DOWNARROW },
	{ "Kp_PgDn",	K_KP_PGDN },
	{ "Kp_Enter",	K_KP_ENTER },
	{ "Kp_Ins",		K_KP_INS },
	{ "Kp_Del",		K_KP_DEL },
	{ "Kp_Slash",	K_KP_SLASH },
	{ "Kp_Minus",	K_KP_MINUS },
	{ "Kp_Plus",	K_KP_PLUS },
	{ "Kp_NumLock",	K_KP_NUMLOCK }, // New in H2

	{ "MWheelUp",	K_MWHEELUP },
	{ "MWheelDown",	K_MWHEELDOWN },

	{ "Pause",		K_PAUSE },

	{ "Semicolon",	';' }, // Because a raw semicolon separates commands.

	{ NULL,	0 }
};

// Q2 counterpart
// Returns a key number to be used to index keybindings[] by looking at the given string.
// Single ascii characters return themselves, while the K_* names are matched up.
static int Key_StringToKeynum(const char* str)
{
	if (str == NULL || str[0] == 0)
		return -1;

	if (str[1] == 0)
		return str[0];

	for (const keyname_t* kn = keynames; kn->name != NULL; kn++)
		if (!Q_stricmp(str, kn->name)) // Q2: Q_strcasecmp
			return kn->keynum;

	return -1;
}

char* Key_KeynumToString(int keynum)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart
void Key_SetBinding(const int keynum, const char* binding)
{
	if (keynum == -1)
		return;

	// Free old bindings
	if (keybindings[keynum])
	{
		Z_Free(keybindings[keynum]);
		keybindings[keynum] = NULL;
	}

	// Allocate memory for new binding
	const int l = (int)strlen(binding) + 1;
	char* new = Z_Malloc(l);
	strcpy_s(new, l, binding); //mxd. strcpy -> strcpy_s
	//new[l - 1] = 0; //mxd. Not needed? strlen already stops at '0'.
	keybindings[keynum] = new;
}

// Very similar to Key_SetBinding
void Key_SetDoubleBinding(const int keynum, const char* binding)
{
	NOT_IMPLEMENTED
}

// Very similar to Key_SetBinding
static void Key_SetCommandBinding(const int keynum, const char* binding)
{
	if (keynum == -1)
		return;

	// Free old bindings
	if (commandbindings[keynum])
	{
		Z_Free(commandbindings[keynum]);
		commandbindings[keynum] = NULL;
	}

	// Allocate memory for new binding
	const int l = (int)strlen(binding) + 1;
	char* new = Z_Malloc(l);
	strcpy_s(new, l, binding); //mxd. strcpy -> strcpy_s
	//new[l - 1] = 0; //mxd. Not needed? strlen already stops at '0'.
	commandbindings[keynum] = new;
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

static void Key_UnbindDouble_f(void)
{
	NOT_IMPLEMENTED
}

// Very similar to Key_Bind_f
static void Key_BindDouble_f(void)
{
	char cmd[1024];

	const int c = Cmd_Argc();
	if (c < 2)
	{
		Com_Printf("bind_double <key> [command] : attach a command to a double tapped key\n");
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
		if (keybindings_double[b] != NULL)
			Com_Printf("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings_double[b]);
		else
			Com_Printf("\"%s\" is not bound\n", Cmd_Argv(1));

		return;
	}

	// Copy the rest of the command line
	cmd[0] = 0; // Start out with a null string

	for (int i = 2; i < c; i++)
	{
		strcat_s(cmd, sizeof(cmd), Cmd_Argv(i));
		if (i != c - 1)
			strcat_s(cmd, sizeof(cmd), " ");
	}

	Key_SetDoubleBinding(b, cmd);
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

// Very similar to Key_Bind_f
static void Key_BindCommand_f(void)
{
	char cmd[1024];

	const int c = Cmd_Argc();
	if (c < 2)
	{
		Com_Printf("bind_command <key> [command] : attach a command to a command altered key\n");
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
		if (commandbindings[b] != NULL)
			Com_Printf("\"%s\" = \"%s\"\n", Cmd_Argv(1), commandbindings[b]);
		else
			Com_Printf("\"%s\" is not bound\n", Cmd_Argv(1));

		return;
	}

	// Copy the rest of the command line
	cmd[0] = 0; // Start out with a null string

	for (int i = 2; i < c; i++)
	{
		strcat_s(cmd, sizeof(cmd), Cmd_Argv(i));
		if (i != c - 1)
			strcat_s(cmd, sizeof(cmd), " ");
	}

	Key_SetCommandBinding(b, cmd);
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

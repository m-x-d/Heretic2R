//
// keys.c
//
// Copyright 1998 Raven Software
//

#include <ctype.h>
#include "client.h"

// Key up events are sent even if in console mode.

char key_lines[NUM_KEY_LINES][MAXCMDLINE];
int key_linepos;
qboolean keydown[256];
int anykeydown;

int edit_line = 0;
static int history_line = 0;

char* keybindings[256];
char* keybindings_double[256];
int key_repeats[256]; // if > 1, it is auto-repeating

char chat_buffer[MAXCMDLINE];
int chat_bufferlen;
qboolean chat_team;

static char* commandbindings[256]; // H2
static qboolean consolekeys[256];	// If true, can't be rebound while in console
static qboolean menubound[256];		// If true, can't be rebound while in menu
static int keyshift[256];			// Key to map to if Shift held down in console

static qboolean con_have_previous_command; // H2

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
	{ "Kp_NumLock",	K_KP_NUMLOCK }, // H2

	{ "MWheelUp",	K_MWHEELUP },
	{ "MWheelDown",	K_MWHEELDOWN },

	{ "Pause",		K_PAUSE },

	{ "Semicolon",	';' }, // Because a raw semicolon separates commands.

	{ NULL,	0 }
};

#pragma region ========================== LINE TYPING INTO THE CONSOLE ==========================

static void CompleteCommand(void)
{
	static qboolean is_command;
	static char buffer[256];
	static char buffer_last[256];

	const char* cmd;

	char* s = &key_lines[edit_line][1];
	if (*s == '\\' || *s == '/')
		s++;

	if (con_have_previous_command) // H2
	{
		if (is_command)
		{
			cmd = Cmd_CompleteCommandNext(buffer, buffer_last);
			if (cmd == NULL)
			{
				is_command = false;
				cmd = Cvar_CompleteVariableNext(buffer, 0);

				if (cmd == NULL)
				{
					is_command = true;
					cmd = Cmd_CompleteCommandNext(buffer, 0);
				}
			}
		}
		else
		{
			cmd = Cvar_CompleteVariableNext(buffer, buffer_last);
			if (cmd == NULL)
			{
				is_command = true;
				cmd = Cmd_CompleteCommandNext(buffer, 0);

				if (cmd == NULL)
				{
					is_command = false;
					cmd = Cvar_CompleteVariableNext(buffer, 0);
				}
			}
		}
	}
	else
	{
		strcpy_s(buffer, sizeof(buffer), s); //mxd. strcpy -> strcpy_s
		is_command = true;
		cmd = Cmd_CompleteCommand(s);

		if (cmd == NULL)
		{
			is_command = false;
			cmd = Cvar_CompleteVariable(s);
		}
	}

	if (cmd != NULL)
	{
		key_lines[edit_line][1] = '/';
		strcpy_s(key_lines[edit_line] + 2, sizeof(key_lines[edit_line]) - 2, cmd); //mxd. strcpy -> strcpy_s
		key_linepos = (int)strlen(cmd) + 2;
		key_lines[edit_line][key_linepos] = ' ';
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;

		strcpy_s(buffer_last, sizeof(buffer_last), cmd); //mxd. strcpy -> strcpy_s
		con_have_previous_command = true;
	}
}

// Interactive line editing and console scrollback.
static void Key_Console(int key)
{
	const int orig_key = key; //mxd

	switch (key)
	{
		case K_KP_HOME:
			key = '7';
			break;

		case K_KP_UPARROW:
			key = '8';
			break;

		case K_KP_PGUP:
			key = '9';
			break;

		case K_KP_LEFTARROW:
			key = '4';
			break;

		case K_KP_5:
			key = '5';
			break;

		case K_KP_RIGHTARROW:
			key = '6';
			break;

		case K_KP_END:
			key = '1';
			break;

		case K_KP_DOWNARROW:
			key = '2';
			break;

		case K_KP_PGDN:
			key = '3';
			break;

		case K_KP_INS:
			key = '0';
			break;

		case K_KP_DEL:
			key = '.';
			break;

		case K_KP_SLASH:
			key = K_SLASH;
			break;

		case K_KP_MINUS:
			key = '-';
			break;

		case K_KP_PLUS:
			key = '+';
			break;

		default:
			break;
	}

	// Command completion.
	if (key == K_TAB)
	{
		CompleteCommand();
		return;
	}

	con_have_previous_command = false; // H2

	// Support pasting from the clipboard.
	if ((toupper(key) == 'V' && keydown[K_CTRL]) || ((key == K_INS || orig_key == K_KP_INS) && keydown[K_SHIFT])) //mxd. K_KP_INS was already remapped to '0' above, so check orig_key.
	{
		char* cbd = Sys_GetClipboardData();
		if (cbd != NULL)
		{
			char* ptr = NULL; //mxd
			strtok_s(cbd, "\n\r\b", &ptr); //mxd. strtok -> strtok_s

			int len = (int)strlen(cbd);
			if (key_linepos + len >= MAXCMDLINE)
				len = MAXCMDLINE - key_linepos;

			if (len > 0)
			{
				cbd[len] = 0;
				strcat_s(key_lines[edit_line], sizeof(key_lines[edit_line]), cbd); //mxd. strcat -> strcat_s
				key_linepos += len;
			}

			free(cbd);
		}

		return;
	}

	// Ctrl-L to clear.
	if (key == 'l' && keydown[K_CTRL])
	{
		Cbuf_AddText("clear\n");
		return;
	}

	if (key == K_ENTER || key == K_KP_ENTER)
	{
		// Backslash text are commands, else chat.
		if (key_lines[edit_line][1] == '\\' || key_lines[edit_line][1] == '/')
			Cbuf_AddText(key_lines[edit_line] + 2); // Skip the '>'.
		else
			Cbuf_AddText(key_lines[edit_line] + 1); // Valid command.

		Cbuf_AddText("\n");
		Com_Printf("%s\n", key_lines[edit_line]);

		edit_line = (edit_line + 1) & (NUM_KEY_LINES - 1);
		history_line = edit_line;
		key_lines[edit_line][0] = '>'; // ']' in Q2
		key_linepos = 1;

		if (cls.state == ca_disconnected)
			SCR_UpdateScreen(); // Force an update, because the command may take some time

		return;
	}

	if (key == K_BACKSPACE || key == K_LEFTARROW || (key == 'h' && keydown[K_CTRL]))
	{
		if (key_linepos > 1)
			key_linepos--;

		return;
	}

	if (key == K_UPARROW || (key == 'p' && keydown[K_CTRL]))
	{
		do
		{
			history_line = (history_line - 1) & (NUM_KEY_LINES - 1);
		} while (history_line != edit_line && key_lines[history_line][1] == 0);

		if (history_line == edit_line)
			history_line = (edit_line + 1) & (NUM_KEY_LINES - 1);

		strcpy_s(key_lines[edit_line], sizeof(key_lines[edit_line]), key_lines[history_line]); //mxd. strcpy -> strcpy_s
		key_linepos = (int)strlen(key_lines[edit_line]);

		return;
	}

	if (key == K_DOWNARROW || (key == 'n' && keydown[K_CTRL]))
	{
		if (history_line == edit_line)
			return;

		do
		{
			history_line = (history_line + 1) & (NUM_KEY_LINES - 1);
		} while (history_line != edit_line && key_lines[history_line][1] == 0);

		if (history_line == edit_line)
		{
			key_lines[edit_line][0] = '>'; // ']' in Q2
			key_linepos = 1;
		}
		else
		{
			strcpy_s(key_lines[edit_line], sizeof(key_lines[edit_line]), key_lines[history_line]); //mxd. strcpy -> strcpy_s
			key_linepos = (int)strlen(key_lines[edit_line]);
		}

		return;
	}

	if (key == K_PGUP)
	{
		con.display -= 2;
		return;
	}

	if (key == K_PGDN)
	{
		con.display += 2;
		con.display = min(con.display, con.current);

		return;
	}

	if (key == K_HOME)
	{
		con.display = con.current - con.totallines + 10;
		return;
	}

	if (key == K_END)
	{
		con.display = con.current;
		return;
	}

	if (key < 32 || key > 127)
		return; // Non-printable char.

	if (key_linepos < MAXCMDLINE - 1)
	{
		key_lines[edit_line][key_linepos] = (char)key;
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
	}
}

#pragma endregion

// Q2 counterpart
static void Key_Message(const int key)
{
	if (key == K_ENTER || key == K_KP_ENTER)
	{
		Cbuf_AddText((chat_team ? "say_team \"" : "say \""));
		Cbuf_AddText(chat_buffer);
		Cbuf_AddText("\"\n");

		cls.key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;

		return;
	}

	if (key == K_ESCAPE)
	{
		cls.key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;

		return;
	}

	if (key < 32 || key > 127)
		return;	// Non-printable.

	if (key == K_BACKSPACE)
	{
		if (chat_bufferlen > 0)
			chat_buffer[--chat_bufferlen] = 0;

		return;
	}

	if (chat_bufferlen < (int)sizeof(chat_buffer) - 1)
	{
		chat_buffer[chat_bufferlen++] = (char)key;
		chat_buffer[chat_bufferlen] = 0;
	}
}

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

// Returns a string (either a single ascii char, or a K_* name) for the given keynum.
// FIXME: handle quote special (general escape sequence ?).
char* Key_KeynumToString(const int keynum)
{
	static char tinystr[2][2]; //mxd. Allow 2 sequential calls without overriding the first value...
	static int tinystr_index = 0; //mxd

	if (keynum == -1)
		return "<KEY NOT FOUND>";

	if (keynum > 32 && keynum < 127 && keynum != ';') // H2: extra ';' check.
	{
		tinystr_index = !tinystr_index; //mxd

		// Printable ascii.
		tinystr[tinystr_index][0] = (char)keynum;
		tinystr[tinystr_index][1] = '\0';

		return tinystr[tinystr_index];
	}

	for (const keyname_t* kn = keynames; kn->name != NULL; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
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
	if (keynum == -1)
		return;

	// Free old bindings
	if (keybindings_double[keynum])
	{
		Z_Free(keybindings_double[keynum]);
		keybindings_double[keynum] = NULL;
	}

	// Allocate memory for new binding
	const int l = (int)strlen(binding) + 1;
	char* new = Z_Malloc(l);
	strcpy_s(new, l, binding); //mxd. strcpy -> strcpy_s
	//new[l - 1] = 0; //mxd. Not needed? strlen already stops at '0'.
	keybindings_double[keynum] = new;
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

// Q2 counterpart
static void Key_Unbind_f(void)
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("unbind <key> : remove commands from a key\n");
		return;
	}

	const char* key = Cmd_Argv(1);
	const int b = Key_StringToKeynum(key);

	if (b != -1)
		Key_SetBinding(b, "");
	else
		Com_Printf("\"%s\" isn't a valid key\n", key);
}

static void Key_Unbindall_f(void)
{
	for (int i = 0; i < 256; i++)
	{
		if (keybindings[i] != NULL)
			Key_SetBinding(i, "");

		if (keybindings_double[i] != NULL) // H2
			Key_SetDoubleBinding(i, "");
	}
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

// Q2 counterpart
// Writes lines containing "bind key value".
void Key_WriteBindings(FILE* f)
{
	for (int i = 0; i < 256; i++)
		if (keybindings[i] != NULL && keybindings[i][0] != 0)
			fprintf(f, "bind %s \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}

// Writes lines containing "bind_double key value".
void Key_WriteBindings_Double(FILE* f)
{
	for (int i = 0; i < 256; i++)
		if (keybindings_double[i] != NULL && keybindings_double[i][0] != 0)
			fprintf(f, "bind_double %s \"%s\"\n", Key_KeynumToString(i), keybindings_double[i]);
}

// Q2 counterpart
static void Key_Bindlist_f(void)
{
	for (int i = 0; i < 256; i++)
		if (keybindings[i] != NULL && keybindings[i][0] != 0)
			Com_Printf("%s \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}

//mxd. Very similar to Key_Unbind_f().
static void Key_UnbindDouble_f(void) // H2
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("unbind_double <key> : remove commands from a double tapped key\n");
		return;
	}

	const char* key = Cmd_Argv(1);
	const int b = Key_StringToKeynum(key);

	if (b != -1)
		Key_SetDoubleBinding(b, "");
	else
		Com_Printf("\"%s\" isn't a valid key\n", key);
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

//mxd. Very similar to Key_BindList_f().
static void Key_DoubleBindList_f(void) // H2
{
	for (int i = 0; i < 256; i++)
		if (keybindings_double[i] != NULL && keybindings_double[i][0] != 0)
			Com_Printf("%s \"%s\"\n", Key_KeynumToString(i), keybindings_double[i]);
}

//mxd. Very similar to Key_Unbind_f().
static void Key_UnbindCommand_f(void) // H2
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("unbind_command <key> : remove commands from a command altered key\n");
		return;
	}

	const char* key = Cmd_Argv(1);
	const int b = Key_StringToKeynum(key);

	if (b != -1)
		Key_SetCommandBinding(b, "");
	else
		Com_Printf("\"%s\" isn't a valid key\n", key);
}

//mxd. Similar to Key_Unbindall_f().
static void Key_UnbindallCommands_f(void)
{
	for (int i = 0; i < 256; i++)
		if (commandbindings[i] != NULL)
			Key_SetCommandBinding(i, "");
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

//mxd. Very similar to Key_Bindlist_f().
static void Key_CommandsList_f(void)
{
	for (int i = 0; i < 256; i++)
		if (commandbindings[i] != NULL && commandbindings[i][0] != 0)
			Com_Printf("%s \"%s\"\n", Key_KeynumToString(i), commandbindings[i]);
}

void Key_Init(void)
{
	for (int i = 0; i < NUM_KEY_LINES; i++)
	{
		key_lines[i][0] = '>'; // Q2: ']';
		key_lines[i][1] = 0;
	}
	
	key_linepos = 1;

	// Init ascii characters in console mode.
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

	// H2:
	Cmd_AddCommand("bind_double", Key_BindDouble_f);
	Cmd_AddCommand("unbind_double", Key_UnbindDouble_f);
	Cmd_AddCommand("bindlist_double", Key_DoubleBindList_f);
	Cmd_AddCommand("bind_command", Key_BindCommand_f);
	Cmd_AddCommand("unbind_command", Key_UnbindCommand_f);
	Cmd_AddCommand("unbindall_command", Key_UnbindallCommands_f);
	Cmd_AddCommand("bindlist_command", Key_CommandsList_f);
}

static qboolean IsAutorepeatKey(const int key) // H2
{
	switch (key)
	{
		case K_BACKSPACE:
		case K_PAUSE:
		case K_PGUP:
		case K_KP_PGUP:
		case K_PGDN:
		case K_KP_PGDN:
			return true;

		default:
			break;
	}

	if (cls.key_dest == key_menu)
	{
		switch (key)
		{
			case K_LEFTARROW:
			case K_RIGHTARROW:
			case K_KP_LEFTARROW:
			case K_KP_RIGHTARROW:
				return true;

			default:
				break;
		}
	}

	return key_repeats[key] < 2;
}

// Called by the system between frames for both key up and key down events.
// Should NOT be called during an interrupt!
void Key_Event(int key, const qboolean down, const uint time)
{
	static qboolean key_doubletaps[256];
	static qboolean menu_keys_pressed_state[256];
	static uint key_doubletap_delays[256];
	static int key_waiting;
	static qboolean shift_down;
	char cmd[1024];

	char* kb = NULL;
	qboolean is_doubletap_key = false;

	// H2
	if (cls.key_dest != key_menu && time - key_doubletap_delays[key] < (uint)doubletap_speed->value)
	{
		key_doubletaps[key] = true;
		is_doubletap_key = true;
	}

	// Hack for modal presses.
	if (key_waiting == -1)
	{
		if (down)
			key_waiting = key;

		return;
	}

	// Update auto-repeat status.
	if (down)
	{
		key_repeats[key]++;

		if (!IsAutorepeatKey(key)) // H2
			return;

		if (key >= 200 && keybindings[key] == NULL)
			Com_Printf("%s is unbound.\n", Key_KeynumToString(key));
	}
	else
	{
		key_repeats[key] = 0;
	}

	if (key == K_SHIFT)
	{
		shift_down = down;
	}
	else if (key == '`' || key == '~') // Console key is hardcoded, so the user can never unbind it.
	{
		if (down)
			Con_ToggleConsole_f();
		
		return;
	}

	// Menu key is hardcoded, so the user can never unbind it.
	if (key == K_ESCAPE || (cl.attractloop && cls.key_dest != key_menu)) // H2: extra checks
	{
		if (!down)
			return;

		switch (cls.key_dest)
		{
			case key_game:
			case key_console:
				M_Menu_Main_f();
				return;

			case key_message:
				Key_Message(K_ESCAPE);
				return;

			case key_menu:
				M_Keydown(K_ESCAPE); // Q2: M_Keydown(key);
				return;

			default:
				Com_Error(ERR_FATAL, "Bad cls.key_dest");
				return;
		}
	}

	// Track if any key is down for BUTTON_ANY.
	keydown[key] = down;
	if (down)
	{
		if (key_repeats[key] == 1)
			anykeydown++;
	}
	else
	{
		anykeydown = max(0, anykeydown - 1);
	}

	if (down)
	{
		// If not a consolekey, send to the interpreter no matter what mode is.
		if ((cls.key_dest == key_menu && menubound[key]) ||
			(cls.key_dest == key_console && !consolekeys[key]) ||
			(cls.key_dest == key_game && (cls.state == ca_active || !consolekeys[key]))) // H2
		{
			if (!command_down || menu_keys_pressed_state[key])
			{
				if (is_doubletap_key)
					kb = keybindings_double[key];

				if (!is_doubletap_key || kb == NULL || Q_stricmp(kb, "") == 0)
					kb = keybindings[key];
			}
			else
			{
				kb = commandbindings[key];
				if (kb == NULL || Q_stricmp(kb, "") == 0)
					kb = keybindings[key];
				else
					menu_keys_pressed_state[key] = true;
			}

			if (kb != NULL)
			{
				if (*kb == '+')
				{
					Com_sprintf(cmd, sizeof(cmd), "%s %i %i\n", kb, key, time);
					Cbuf_AddText(cmd);
				}
				else
				{
					Cbuf_AddText(kb);
					Cbuf_AddText("\n");
				}
			}

			return;
		}
		
		if (shift_down)
			key = keyshift[key];

		switch (cls.key_dest)
		{
			case key_game:
			case key_console:
				Key_Console(key);
				return;

			case key_message:
				Key_Message(key);
				return;

			case key_menu:
				M_Keydown(key);
				return;

			default:
				Com_Error(ERR_FATAL, "Bad cls.key_dest");
				return;
		}
	}

	if (key_doubletaps[key])
	{
		key_doubletaps[key] = false;
		is_doubletap_key = true;
	}

	key_doubletap_delays[key] = time;

	if (command_down && keybindings[key] != NULL && Q_stricmp(keybindings[key], "+command") == 0)
		command_down = false;

	// Key up events only generate commands if the game key binding is a button command (leading + sign).
	// These will occur even in console mode, to keep the character from continuing an action started before a console switch.
	// Button commands include the keynum as a parameter, so multiple downs can be matched with ups.
	if (menu_keys_pressed_state[key])
	{
		kb = commandbindings[key];
		if (kb == NULL || Q_stricmp(kb, "") == 0)
			kb = keybindings[key];
		else
			menu_keys_pressed_state[key] = false;
	}
	else
	{
		if (is_doubletap_key)
			kb = keybindings_double[key];

		if (!is_doubletap_key || kb == NULL || Q_stricmp(kb, "") == 0)
			kb = keybindings[key];
	}

	if (kb != NULL && *kb == '+')
	{
		Com_sprintf(cmd, sizeof(cmd), "-%s %i %i\n", kb + 1, key, time);
		Cbuf_AddText(cmd);
	}

	if (keyshift[key] == key)
		return;

	key = keyshift[key];

	if (menu_keys_pressed_state[key])
	{
		kb = commandbindings[key];
		if (kb == NULL || Q_stricmp(kb, "") == 0)
		{
			key = keyshift[key];
			kb = keybindings[key];
		}
		else
		{
			menu_keys_pressed_state[keyshift[key]] = false;
		}
	}
	else if (is_doubletap_key)
	{
		kb = keybindings_double[key];

		if (kb == NULL || Q_stricmp(kb, "") == 0)
			kb = keybindings[keyshift[key]];
	}
	else
	{
		kb = keybindings[key];
	}

	if (kb != NULL && *kb == '+')
	{
		Com_sprintf(cmd, sizeof(cmd), "-%s %i %i\n", kb + 1, key, time);
		Cbuf_AddText(cmd);
	}
}

// Q2 counterpart
void Key_ClearStates(void)
{
	anykeydown = 0;

	for (int i = 0; i < 256; i++)
	{
		if (keydown[i] || key_repeats[i])
			Key_Event(i, false, 0);

		keydown[i] = 0;
		key_repeats[i] = 0;
	}
}

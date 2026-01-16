//
// console.c
//
// Copyright 1998 Raven Software
//

#include "client.h"

console_t con;

static cvar_t* con_notifytime;
static cvar_t* con_alpha; // H2
static cvar_t* nextserver; // H2

void DrawString(int x, const int y, const char* s, const paletteRGBA_t color, int maxlen) //mxd. +char scaling and shadow logic.
{
	while (*s != 0 && maxlen != 0) //NOTE: 'maxlen' can be -1!
	{
		re.DrawChar(x, y, ui_scale, *s, color, true);
		s++;
		x += ui_char_size;
		maxlen--;
	}
}

// Q2 counterpart
void Key_ClearTyping(void)
{
	key_lines[edit_line][1] = '\0'; // Clear any typing
	key_linepos = 1;
}

// Q2 counterpart
void Con_ClearNotify(void)
{
	for (int i = 0; i < NUM_CON_TIMES; i++)
		con.times[i] = 0.0f;
}

void Con_ToggleConsole_f(void)
{
	if (cl.frame.playerstate.cinematicfreeze) // H2
	{
		cls.esc_cinematic = true;
		return;
	}

	if (cl.cinematictime > 0 || scr_draw_loading_plaque) // H2
		return;

	SCR_EndLoadingPlaque(); // Get rid of loading plaque.

	if (cl.attractloop)
	{
		Cbuf_AddText("killserver\n");
		return;
	}

	if (cls.state == ca_disconnected && cls.key_dest != key_menu) // H2
	{
		Cbuf_AddText("menu_main\n");
		return;
	}

	Key_ClearTyping();
	Con_ClearNotify();

	if (cls.key_dest == key_console)
	{
		M_ForceMenuOff(); //mxd. Also un-pauses the game.
	}
	else
	{
		M_ForceMenuOff();
		cls.key_dest = key_console;

		if ((int)(Cvar_VariableValue("maxclients")) == 1 && Com_ServerState())
			Cvar_Set("paused", "1");
	}
}

static void Con_ToggleChat_f(void)
{
	Key_ClearTyping();

	if (cls.key_dest == key_console)
	{
		if (cls.state == ca_active)
		{
			M_ForceMenuOff();
			Key_ClearStates(); // H2
			cls.key_dest = key_game;
		}
	}
	else
	{
		cls.key_dest = key_console;
	}

	Con_ClearNotify();
}

// Q2 counterpart
static void Con_MessageMode_f(void)
{
	chat_team = false;
	cls.key_dest = key_message;
}

// Q2 counterpart
static void Con_MessageMode2_f(void)
{
	chat_team = true;
	cls.key_dest = key_message;
}

void Con_Clear_f(void)
{
	memset(con.text, ' ', CON_TEXTSIZE);
	memset(con.color, 0xff, sizeof(con.color)); // H2
	con.display = con.current; //mxd. Reset console scroll position.
}

// Save the console contents out to a file.
static void Con_Dump_f(void)
{
	int l;
	int x;
	FILE* f;
	char buffer[1024];
	char name[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Com_Printf("usage: condump <filename>\n");
		return;
	}

	Com_sprintf(name, sizeof(name), "%s/%s.txt", FS_Userdir(), Cmd_Argv(1)); // H2: FS_Gamedir -> FS_Userdir
	Com_Printf("Dumped console text to %s.\n", name);
	FS_CreatePath(name);

	if (fopen_s(&f, name, "w") != 0) //mxd. fopen -> fopen_s
	{
		Com_Printf("ERROR: couldn't open.\n");
		return;
	}

	// Skip empty lines.
	for (l = con.current - con.totallines + 1; l <= con.current; l++)
	{
		const char* line = con.text + (l % con.totallines) * con.linewidth;

		for (x = 0; x < con.linewidth; x++)
			if (line[x] != ' ')
				break;

		if (x != con.linewidth)
			break;
	}

	// Write the remaining lines.
	buffer[con.linewidth] = 0;
	for (; l <= con.current; l++)
	{
		const char* line = con.text + (l % con.totallines) * con.linewidth;
		strncpy_s(buffer, sizeof(buffer), line, con.linewidth); //mxd. strncpy -> strncpy_s

		for (int c = con.linewidth - 1; c >= 0 && buffer[c] == ' '; c--)
			buffer[c] = 0;

		fprintf(f, "%s\n", buffer);
	}

	fclose(f);
}

static void Con_Chars_f(void) // H2
{
	char chars[33];

	// Print 7 lines, 32 chars per line (224 chars in total).
	char c = ' ';

	for (int line = 0; line < 7; line++)
	{
		for (int i = 0; i < 32; i++, c++)
			chars[i] = c;

		chars[32] = 0;
		Com_Printf(":%s\n", chars);
	}
}

// If the line width has changed, reformat the buffer.
void Con_CheckResize(void)
{
	static char tbuf[CON_TEXTSIZE]; //mxd. Made static.

	const int width = (viddef.width / ui_char_size) - 1; //mxd. Adjust for UI scale; increase by 1, because we subtract by 1 in Con_Print().

	if (width == con.linewidth && con.linewidth != -1) //mxd. Don't skip if not initialized yet.
		return;

	if (width < 1) // Video hasn't been initialized yet.
	{
		con.linewidth = 78;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		memset(con.text, ' ', sizeof(con.text));
		memset(con.color, 0xFF, sizeof(con.color));
	}
	else
	{
		const int oldwidth = con.linewidth;
		con.linewidth = width;

		const int oldtotallines = con.totallines;
		con.totallines = CON_TEXTSIZE / con.linewidth;

		const int numlines = min(con.totallines, oldtotallines);
		const int numchars = min(con.linewidth, oldwidth);

		memcpy(tbuf, con.text, sizeof(tbuf));
		memset(con.text, ' ', sizeof(con.text));

		paletteRGBA_t cbuf[MAX_LINES];
		memcpy(cbuf, con.color, sizeof(cbuf)); // H2
		memset(con.color, 0xff, sizeof(con.color)); // H2

		for (int i = 0; i < numlines; i++)
		{
			const int con_line_index = con.totallines - 1 - i;
			const int buf_line_index = (con.current - i + oldtotallines) % oldtotallines;
			con.color[con_line_index] = cbuf[buf_line_index]; // H2

			for (int j = 0; j < numchars; j++)
				con.text[con_line_index * con.linewidth + j] = tbuf[buf_line_index * oldwidth + j];
		}

		Con_ClearNotify();
	}

	con.current = con.totallines - 1;
	con.display = con.current;

	Key_ClearTyping(); // H2
}

void Con_Init(void)
{
	con.linewidth = -1;

	Con_CheckResize();
	Com_Printf("Console initialized.\n");

	// Register our commands
	con_notifytime = Cvar_Get("con_notifytime", "3", 0);
	con_alpha = Cvar_Get("con_alpha", "0.7", CVAR_ARCHIVE); // H2_1.07: "0.7" -> "0.5".
	nextserver = Cvar_Get("nextserver", "", 0); // H2

	Cmd_AddCommand("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand("togglechat", Con_ToggleChat_f);
	Cmd_AddCommand("messagemode", Con_MessageMode_f);
	Cmd_AddCommand("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand("clear", Con_Clear_f);
	Cmd_AddCommand("condump", Con_Dump_f);
	Cmd_AddCommand("conchars", Con_Chars_f); // H2

	con.initialized = true;
}

// Q2 counterpart
static void Con_Linefeed(void)
{
	con.x = 0;

	if (con.display == con.current)
		con.display++;

	con.current++;

	const int offset = (con.current % con.totallines) * con.linewidth;
	memset(&con.text[offset], ' ', con.linewidth);
}

// Handles cursor positioning, line wrapping, etc.
// All console printing must go through this in order to be logged to disk.
// If no console is visible, the text will appear at the top of the game window.
void Con_Print(const char* txt)
{
	static qboolean newline; // Named 'cr' in Q2
	int l;

	if (!con.initialized)
		return;

	for (const char* c = txt; *c != 0; c++)
	{
		// Count word length
		for (l = 0; l < con.linewidth; l++)
			if (c[l] <= ' ')
				break;

		// Word wrap
		if (l != con.linewidth && con.x + l >= con.linewidth) //mxd. '>' -> '>=', because last line char is always 0.
			con.x = 0;

		if (newline)
		{
			con.current--;
			newline = false;
		}

		if (con.x == 0)
		{
			Con_Linefeed();

			// Mark time for transparent overlay
			if (con.current >= 0)
				con.times[con.current % NUM_CON_TIMES] = (float)cls.realtime;
		}

		switch (*c)
		{
			case '\n':
				con.x = 0;
				break;

			case '\r':
				con.x = 0;
				newline = true;
				break;

			default:
			{
				// Display character and advance
				const int y = con.current % con.totallines;
				con.text[y * con.linewidth + con.x] = *c;
				con.color[y] = con.current_color;
				con.x++;
				if (con.x >= con.linewidth)
					con.x = 0;
			} break;
		}
	}
}

#pragma region ========================== DRAWING ==========================

// The input line scrolls horizontally if typing goes beyond the right edge.
static void Con_DrawInput(void)
{
	if (cls.key_dest == key_menu || (cls.key_dest != key_console && cls.state == ca_active))
		return; // Don't draw anything (always draw if not active).

	char* text = key_lines[edit_line];

	// Add the cursor frame.
	text[key_linepos] = (char)(10 + ((cls.realtime >> 8) & 1));

	// Fill out remainder with spaces.
	for (int i = key_linepos + 1; i < con.linewidth; i++)
		text[i] = ' ';

	// Pre-step if horizontally scrolling.
	if (key_linepos >= con.linewidth)
		text += 1 + key_linepos - con.linewidth;

	// Draw it.
	DrawString(ui_char_size, con.vislines - ui_char_size * 2, text, TextPalette[P_WHITE], con.linewidth); // H2

	// Remove cursor.
	key_lines[edit_line][key_linepos] = 0;
}

// Draws the last few lines of output transparently over the game top.
void Con_DrawNotify(void)
{
	int y = 0;

	for (int i = con.current - NUM_CON_TIMES + 1; i <= con.current; i++)
	{
		if (i < 0)
			continue;

		const int time = (int)con.times[i % NUM_CON_TIMES];

		if (time > 0 && cls.realtime - time <= (int)(con_notifytime->value * 1000))
		{
			const int line_index = (i % con.totallines);
			DrawString(ui_char_size, y, &con.text[line_index * con.linewidth], con.color[line_index], con.linewidth);
			y += ui_char_size;
		}
	}

	if (cls.key_dest == key_message)
	{
		char* s = chat_buffer;
		const cvar_t* cv_colour = (chat_team ? colour_teamchat : colour_chat);
		const char* prompt = (chat_team ? "say_team:" : "say:");
		const int max_line_width = viddef.width / ui_char_size - ui_scale * (chat_team ? 12 : 6);
		const int x = ui_scale * (chat_team ? 88 : 40);

		// Draw prompt.
		const paletteRGBA_t colour = TextPalette[COLOUR(cv_colour)];
		DrawString(ui_char_size, y, prompt, colour, -1);

		if (chat_bufferlen > max_line_width)
			s = &chat_buffer[chat_bufferlen - max_line_width];

		const int len = (int)strlen(s);

		s[len] = (char)(10 + ((cls.realtime / 256) & 1)); // Alternate between conchar 10 (empty) and 11 (text input cursor).
		s[len + 1] = 0;

		// Draw chat message.
		DrawString(x, y, s, colour, -1);

		s[len] = 0;
		y += ui_char_size;
	}

	if (y > 0)
	{
		SCR_AddDirtyPoint(0, 0);
		SCR_AddDirtyPoint(viddef.width - 1, y);
	}
}

static qboolean ShouldDrawConsole(void) // H2
{
	static int console_delay;

	if ((int)developer->value)
		return true;

	if (nextserver != NULL && nextserver->string[0] != 0) //mxd. strlen(str) -> str[0] check.
	{
		console_delay = 5;
		return false;
	}

	if (console_delay > 0)
	{
		console_delay--;
		return false;
	}

	return true;
}

// Draws the console with the solid background.
void Con_DrawConsole(float frac)
{
	//TODO: why doesn't it work as a #define?
	static char* backscroll_arrows = " ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^  ";
	
	if (!ShouldDrawConsole()) // H2
	{
		re.DrawFill(0, 0, viddef.width, viddef.height, TextPalette[P_BLACK]);
		return;
	}

	const int lines_y = min((int)(frac * (float)viddef.height), viddef.height);

	if (lines_y < 1)
		return;

	frac = ((cls.state == ca_active) ? con_alpha->value : 1.0f);

	// Draw the background.
	const int conback_height = viddef.width / 4 * 3; //mxd. Keep aspect ratio in widescreen...
	re.DrawStretchPic(0, lines_y - conback_height, viddef.width, conback_height, "misc/conback.m8", frac, DSP_NONE);

	SCR_AddDirtyPoint(0, 0);
	SCR_AddDirtyPoint(viddef.width - 1, lines_y - 1);

	// Draw version.
	const int ver_x = viddef.width - ((int)strlen(GAME_FULLNAME) * ui_char_size + ui_char_size);
	const int ver_y = lines_y - ui_scale * 12;
	DrawString(ver_x, ver_y, GAME_FULLNAME, TextPalette[P_GREEN], -1); //mxd. P_VERSION in original logic.

	// Draw the text.
	con.vislines = lines_y;

	// H2 uses #if 0-ed version of Q2 logic.
	int rows = (lines_y - 8) >> 3; // Rows of text to draw.
	int y = lines_y - ui_char_size * 3;

	// Draw from the bottom up.
	if (con.display != con.current)
	{
		// Draw arrows to show the buffer is backscrolled.
		DrawString(ui_char_size, y, backscroll_arrows, TextPalette[P_WHITE], con.linewidth);

		y -= ui_char_size;
		rows--;
	}

	int row = con.display;
	for (int i = 0; i < rows; i++, y -= ui_char_size, row--)
	{
		if (row < 0 || con.current - row >= con.totallines)
			break; // Past scrollback wrap point.

		DrawString(ui_char_size, y, &con.text[(row % con.totallines) * con.linewidth], con.color[row % con.totallines], con.linewidth);
	}

	// Draw the input prompt, user text, and cursor if desired.
	Con_DrawInput();
}

#pragma endregion
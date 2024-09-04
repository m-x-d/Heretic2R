//
// console.c
//
// Copyright 1998 Raven Software
//

#include "client.h"

console_t con;

static cvar_t* con_notifytime;
static cvar_t* con_alpha; // New in H2
static cvar_t* nextserver; // New in H2

void DrawString(int x, int y, char* s, uint color, int maxlen)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
static void Key_ClearTyping(void)
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
	NOT_IMPLEMENTED
}

void Con_ToggleChat_f(void)
{
	NOT_IMPLEMENTED
}

void Con_MessageMode_f(void)
{
	NOT_IMPLEMENTED
}

void Con_MessageMode2_f(void)
{
	NOT_IMPLEMENTED
}

void Con_Clear_f(void)
{
	NOT_IMPLEMENTED
}

void Con_Dump_f(void)
{
	NOT_IMPLEMENTED
}

void Con_Chars_f(void)
{
	NOT_IMPLEMENTED
}

// If the line width has changed, reformat the buffer.
void Con_CheckResize(void)
{
	char tbuf[CON_TEXTSIZE];
	paletteRGBA_t cbuf[MAX_LINES];

	const int width = (viddef.width >> 3) - 2;

	if (width == con.linewidth)
		return;

	if (width < 1) // Video hasn't been initialized yet
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
		memcpy(cbuf, con.color, sizeof(cbuf)); // H2
		memset(con.color, 0xFF, sizeof(con.color)); // H2

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
	con_alpha = Cvar_Get("con_alpha", "0.5", CVAR_ARCHIVE); // New in H2
	nextserver = Cvar_Get("nextserver", "", 0); // New in H2

	Cmd_AddCommand("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand("togglechat", Con_ToggleChat_f);
	Cmd_AddCommand("messagemode", Con_MessageMode_f);
	Cmd_AddCommand("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand("clear", Con_Clear_f);
	Cmd_AddCommand("condump", Con_Dump_f);
	Cmd_AddCommand("conchars", Con_Chars_f); // New in H2

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
		if (l != con.linewidth && con.x + l > con.linewidth)
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
				// Display character and advance
				const int y = con.current % con.totallines;
				con.text[y * con.linewidth + con.x] = *c;
				con.color[y] = con.current_color;
				con.x++;
				if (con.x >= con.linewidth)
					con.x = 0;
				break;
		}
	}
}

#pragma region ========================== DRAWING ==========================

static void Con_DrawInput(void)
{
	NOT_IMPLEMENTED
}

void Con_DrawNotify(void)
{
	NOT_IMPLEMENTED
}

static qboolean ShouldDrawConsole(void)
{
	NOT_IMPLEMENTED
	return false;
}

// Draws the console with the solid background.
void Con_DrawConsole(float frac)
{
	//TODO: why it doesn't work as a #define?
	static char* backscroll_arrows = " ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^  ";
	
	if (!ShouldDrawConsole())
	{
		re.DrawFill(0, 0, viddef.width, viddef.height, 0, 0, 0);
		return;
	}

	int lines = min((int)(frac * (float)DEF_HEIGHT), DEF_HEIGHT);

	if (lines < 1)
		return;

	lines = (viddef.height * lines) / DEF_HEIGHT;

	if (cls.state == ca_active)
		frac = con_alpha->value;
	else
		frac = 1.0f;

	// Draw the background
	re.DrawStretchPic(0, lines - DEF_HEIGHT, DEF_WIDTH, DEF_HEIGHT, "misc/conback.m8", frac, true);
	
	SCR_AddDirtyPoint(0, 0);
	SCR_AddDirtyPoint(viddef.width - 1, lines - 1);

	// Draw version
	char version[MAX_QPATH];
	Com_sprintf(version, sizeof(version), "Heretic II: %s", VERSIONDISP);
	DrawString(viddef.width - ((int)strlen(version) * 8 + 8), lines - 12, version, TextPalette[P_VERSION].c, -1);

	// Draw the text
	con.vislines = lines;

	// H2 uses #if 0-ed version of Q2 logic.
	int rows = (lines - 8) >> 3; // Rows of text to draw
	int y = lines - 24;

	// Draw from the bottom up
	if (con.display != con.current)
	{
		// Draw arrows to show the buffer is backscrolled
		DrawString(8, y, backscroll_arrows, TextPalette[P_WHITE].c, con.linewidth);

		y -= 8;
		rows--;
	}

	int row = con.display;
	for (int i = 0; i < rows; i++, y -= 8, row--)
	{
		if (row < 0 || con.current - row >= con.totallines)
			break; // Past scrollback wrap point

		DrawString(8, y, &con.text[(row % con.totallines) * con.linewidth], con.color[row % con.totallines].c, con.linewidth);
	}

	// Draw the input prompt, user text, and cursor if desired
	Con_DrawInput();
}

#pragma endregion
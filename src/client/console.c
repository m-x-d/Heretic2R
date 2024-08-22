//
// console.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "q_shared.h" //TODO: added for NOT_IMPLEMENTED macro!

console_t con;

static cvar_t* con_notifytime;
static cvar_t* con_alpha; // New in H2
static cvar_t* nextserver; // New in H2

static void Key_ClearTyping(void)
{
	NOT_IMPLEMENTED
}

void Con_ClearNotify(void)
{
	NOT_IMPLEMENTED
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

static void Con_Linefeed(void)
{
	NOT_IMPLEMENTED
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
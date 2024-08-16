//
// console.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "q_shared.h" //TODO: added for NOT_IMPLEMENTED macro!

console_t con;

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
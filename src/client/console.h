//
// console.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

#define NUM_CON_TIMES	8 // Q2: 4
#define CON_TEXTSIZE	32768
#define MAX_LINES		((CON_TEXTSIZE / 38) + 8)

typedef struct
{
	qboolean initialized;

	char text[CON_TEXTSIZE];
	int current;	// Line where next message will be printed.
	int x;			// offset in current line for next print.
	int display;	// bottom of console displays this line.

	int linewidth;	// Characters across screen.
	int totallines;	// Total lines in console scrollback.

	paletteRGBA_t color[MAX_LINES];
	paletteRGBA_t current_color;

	float cursorspeed;
	int vislines;

	float times[NUM_CON_TIMES];	// cls.realtime time the line was generated for transparent notify lines.
} console_t;

extern console_t con;

extern void Con_CheckResize(void);
extern void Con_Init(void);
extern void Con_DrawConsole(float frac);
extern void Con_Print(const char* txt);
extern void Con_Clear_f(void);
extern void Con_DrawNotify(void);
extern void Con_ClearNotify(void);
extern void Con_ToggleConsole_f(void);
//
// Debug.c -- Development aids
//
// Copyright 2024 mxd
//

#include "Debug.h"
#include "client.h"
#include "vid.h"

#if _DEBUG
	#include <windows.h>
	#include <stdio.h>
#endif

// Print to Visual Studio console.
Q2DLL_DECLSPEC void DBG_IDEPrint(const char* fmt, ...)
{
#if _DEBUG
	va_list argptr;
	char msg[1024];

	va_start(argptr, fmt);
	vsprintf_s(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	strcat_s(msg, sizeof(msg), "\n");
	OutputDebugString(msg);
#endif
}

// Print vector.
Q2DLL_DECLSPEC char* pv(const vec3_t v)
{
	static char buf[8][128];
	static int buf_index;

	buf_index = (buf_index + 1) % 7;

	sprintf_s(buf[buf_index], sizeof(buf[buf_index]), "[%f %f %f]", v[0], v[1], v[2]);

	return buf[buf_index];
}

// Print short vector.
Q2DLL_DECLSPEC char* psv(const short* v)
{
	static char buf[8][128];
	static int buf_index;

	buf_index = (buf_index + 1) % 7;

	sprintf_s(buf[buf_index], sizeof(buf[buf_index]), "[%i %i %i]", v[0], v[1], v[2]);

	return buf[buf_index];
}

#define NUM_DEBUG_MESSAGES	16

typedef struct
{
	char title[32];
	char message[256];
} DebugHudMessage_t;

static DebugHudMessage_t dbg_messages[NUM_DEBUG_MESSAGES];

Q2DLL_DECLSPEC void DBG_HudPrint(const int slot, const char* label, const char* fmt, ...)
{
#if _DEBUG

	va_list argptr;

	if (slot < 0 || slot >= NUM_DEBUG_MESSAGES)
	{
		DBG_IDEPrint("DBG_HudPrint: invalid HUD slot %i!", slot);
		return;
	}

	DebugHudMessage_t* msg = &dbg_messages[slot];

	// Title
	strcpy_s(msg->title, sizeof(msg->title), label);

	// Message
	va_start(argptr, fmt);
	vsprintf_s(msg->message, sizeof(msg->message), fmt, argptr);
	va_end(argptr);

#endif
}

void DBG_DrawMessages(void)
{
#if _DEBUG

	int ox = 0;
	int oy = (viddef.height - ui_char_size) / 2;

	for (int i = 0; i < NUM_DEBUG_MESSAGES; i++)
	{
		const DebugHudMessage_t* msg = &dbg_messages[i];
		ox = max(ox, (int)strlen(msg->title));
	}

	// Convert to char offset, add pad...
	ox = (ox + 1) * ui_char_size;

	for (int i = 0; i < NUM_DEBUG_MESSAGES; i++)
	{
		const DebugHudMessage_t* msg = &dbg_messages[i];

		const int t_len = (int)strlen(msg->title);
		const int m_len = (int)strlen(msg->message);

		if (t_len == 0 || m_len == 0)
			continue;

		// Title
		DrawString(ox - t_len * ui_char_size, oy, msg->title, TextPalette[P_CYAN], -1);

		// Message
		DrawString(ox + ui_char_size, oy, msg->message, TextPalette[P_CYAN], -1);

		oy += ui_line_height;
	}

#endif
}

Q2DLL_DECLSPEC void DBG_AddBox(const vec3_t center, const float size, const paletteRGBA_t color, const float lifetime)
{
#if _DEBUG
	re.AddDebugBox(center, size, color, lifetime);
#endif
}

Q2DLL_DECLSPEC void DBG_AddBbox(const vec3_t mins, const vec3_t maxs, const paletteRGBA_t color, const float lifetime)
{
#if _DEBUG
	re.AddDebugBbox(mins, maxs, color, lifetime);
#endif
}

Q2DLL_DECLSPEC void DBG_AddEntityBbox(const edict_t* ent, const paletteRGBA_t color)
{
#if _DEBUG
	re.AddDebugEntityBbox(ent, color);
#endif
}

Q2DLL_DECLSPEC void DBG_AddLine(const vec3_t start, const vec3_t end, const paletteRGBA_t color, const float lifetime)
{
#if _DEBUG
	re.AddDebugLine(start, end, color, lifetime);
#endif
}

Q2DLL_DECLSPEC void DBG_AddArrow(const vec3_t start, const vec3_t end, const paletteRGBA_t color, const float lifetime)
{
#if _DEBUG
	re.AddDebugArrow(start, end, color, lifetime);
#endif
}
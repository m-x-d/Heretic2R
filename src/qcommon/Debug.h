//
// Debug.h -- Development aids
//
// Copyright 2024 mxd
//

#pragma once

#include "Heretic2.h"
#include "q_Typedef.h"

#if _DEBUG
	#define NOT_IMPLEMENTED		__asm { int 3 } // https://learn.microsoft.com/en-us/cpp/intrinsics/debugbreak
#else
	#if NDEBUG
		#define TOGGLE_NDEBUG	1
	#endif

	#if TOGGLE_NDEBUG
		#undef NDEBUG
	#endif

	#include <assert.h>
	#define NOT_IMPLEMENTED		assert(!("Not implemented!"));

	#if TOGGLE_NDEBUG
		#define NDEBUG
	#endif
#endif

// Debug colors.
Q2DLL_DECLSPEC extern paletteRGBA_t dc_white;
Q2DLL_DECLSPEC extern paletteRGBA_t dc_red;
Q2DLL_DECLSPEC extern paletteRGBA_t dc_green;
Q2DLL_DECLSPEC extern paletteRGBA_t dc_blue;
Q2DLL_DECLSPEC extern paletteRGBA_t dc_yellow;
Q2DLL_DECLSPEC extern paletteRGBA_t dc_orange;
Q2DLL_DECLSPEC extern paletteRGBA_t dc_cyan;
Q2DLL_DECLSPEC extern paletteRGBA_t dc_purple;

#define pb(v)	((v) ? "TURE " : "FALSE") // Print bool.
Q2DLL_DECLSPEC extern char* pv(const vec3_t v); // vtos() from g_utils.c, basically...
Q2DLL_DECLSPEC extern char* psv(const short* v);

#ifdef __cplusplus //mxd. Needed, so code in game/ds.cpp could build...
extern "C"
{
#endif
Q2DLL_DECLSPEC extern void DBG_IDEPrint(const char* fmt, ...);
#ifdef __cplusplus
}
#endif

Q2DLL_DECLSPEC extern void DBG_HudPrint(int slot, const char* label, const char* fmt, ...);

Q2DLL_DECLSPEC extern void DBG_AddBox(const vec3_t center, float size, paletteRGBA_t color, float lifetime);
Q2DLL_DECLSPEC extern void DBG_AddBbox(const vec3_t mins, const vec3_t maxs, paletteRGBA_t color, float lifetime);
Q2DLL_DECLSPEC extern void DBG_AddEntityBbox(const edict_t* ent, paletteRGBA_t color);

Q2DLL_DECLSPEC extern void DBG_AddLine(const vec3_t start, const vec3_t end, paletteRGBA_t color, float lifetime);
Q2DLL_DECLSPEC extern void DBG_AddArrow(const vec3_t start, const vec3_t end, paletteRGBA_t color, float lifetime);
Q2DLL_DECLSPEC extern void DBG_AddDirection(const vec3_t start, const vec3_t angles_deg, float size, paletteRGBA_t color, float lifetime);
Q2DLL_DECLSPEC extern void DBG_AddMarker(const vec3_t center, float size, paletteRGBA_t color, float lifetime);

extern void DBG_DrawMessages(void);
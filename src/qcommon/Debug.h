//
// Debug.h -- Development aids
//
// Copyright 2024 mxd
//

#pragma once

#include <assert.h>
#include "Heretic2.h"
#include "q_Typedef.h"

#define NOT_IMPLEMENTED		assert(!("Not implemented!"));

Q2DLL_DECLSPEC extern char* pv(const vec3_t v); // vtos() from g_utils.c, basically...
Q2DLL_DECLSPEC extern char* psv(const short* v);

Q2DLL_DECLSPEC extern void DBG_IDEPrint(const char* fmt, ...);
Q2DLL_DECLSPEC extern void DBG_HudPrint(int slot, const char* label, const char* fmt, ...);

Q2DLL_DECLSPEC extern void DBG_AddBox(const vec3_t center, float size, paletteRGBA_t color, float lifetime);
Q2DLL_DECLSPEC extern void DBG_AddBbox(const vec3_t mins, const vec3_t maxs, paletteRGBA_t color, float lifetime);
Q2DLL_DECLSPEC extern void DBG_AddEntityBbox(const edict_t* ent, paletteRGBA_t color);

Q2DLL_DECLSPEC extern void DBG_AddLine(const vec3_t start, const vec3_t end, paletteRGBA_t color, float lifetime);
Q2DLL_DECLSPEC extern void DBG_AddArrow(const vec3_t start, const vec3_t end, paletteRGBA_t color, float lifetime);

extern void DBG_DrawMessages(void);
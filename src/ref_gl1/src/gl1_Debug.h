//
// gl1_Debug.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

extern void R_AddDebugBox(const vec3_t center, float size, paletteRGBA_t color, float lifetime);
extern void R_AddDebugBbox(const vec3_t mins, const vec3_t maxs, paletteRGBA_t color, float lifetime);

extern void R_AddDebugEntityBbox(const struct edict_s* ent, paletteRGBA_t color);
extern void R_AddDebugLine(const vec3_t start, const vec3_t end, paletteRGBA_t color, float lifetime);
extern void R_AddDebugArrow(const vec3_t start, const vec3_t end, paletteRGBA_t color, float lifetime);

extern void R_DrawDebugPrimitives(void);
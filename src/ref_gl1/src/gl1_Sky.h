//
// gl1_Sky.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

extern void R_AddSkySurface(const msurface_t* fa);
extern void R_ClearSkyBox(void);
extern void R_DrawSkyBox(void);
extern void RI_SetSky(const char* name, float rotate, const vec3_t axis);
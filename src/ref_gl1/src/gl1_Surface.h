//
// gl1_Surface.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

extern int c_visible_lightmaps;
extern int c_visible_textures;

extern void R_SortAndDrawAlphaSurfaces(void);
extern void R_DrawWorld(void);
extern void R_MarkLeaves(void);
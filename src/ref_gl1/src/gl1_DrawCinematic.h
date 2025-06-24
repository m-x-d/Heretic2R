//
// gl1_DrawCinematic.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

extern void Draw_InitCinematic(int w, int h, char* overlay, char* backdrop);
extern void Draw_CloseCinematic(void);
extern void Draw_Cinematic(int cols, int rows, const byte* data, const paletteRGB_t* palette, float alpha);
//
// gl1_Image.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

extern int gl_filter_min;
extern int gl_filter_max;

extern void R_InitGammaTable(void);

extern void R_InitImages(void);
extern void R_ShutdownImages(void);

extern void R_ImageList_f(void);

extern void R_TexEnv(GLint mode);
extern void R_BindImage(const image_t* image);
extern void R_TextureMode(const char* string);

extern struct image_s* R_RegisterSkin(const char* name, qboolean* retval);
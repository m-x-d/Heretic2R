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
extern image_t* R_FindImage(const char* name, imagetype_t type);
extern image_t* R_GetFreeImage(void);
extern void R_BindImage(const image_t* image);
extern void R_MBind(GLenum target, int texnum);
extern void R_MBindImage(GLenum target, const image_t* image);
extern void R_UploadPaletted(int level, const byte* data, const paletteRGB_t* palette, int width, int height);
extern void R_FreeImageNoHash(image_t* image);
extern void R_FreeUnusedImages(void);

extern void R_ImageList_f(void);

extern void R_EnableMultitexture(qboolean enable);
extern void R_SelectTexture(GLenum texture);
extern void R_TexEnv(GLint mode);
extern void R_Bind(int texnum);
extern void R_TextureMode(const char* string);
extern void R_SetFilter(const image_t* image);

extern struct image_s* RI_RegisterSkin(const char* name, qboolean* retval);
extern void R_GammaAffect(void);
extern void R_DisplayHashTable(void);
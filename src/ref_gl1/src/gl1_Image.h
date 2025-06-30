//
// gl1_Image.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

extern void InitGammaTable(void);
extern void GL_ImageList_f(void);

extern struct image_s* R_RegisterSkin(const char* name, qboolean* retval);
extern void GL_InitImages(void);
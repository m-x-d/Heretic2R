//
// gl1_Misc.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Misc.h"
#include "gl1_Image.h"
#include "gl1_Local.h"
#include "Vector.h"

void R_ScreenShot_f(void) // Q2: GL_ScreenShot_f()
{
	NOT_IMPLEMENTED
}

void R_Strings_f(void) // Q2: GL_Strings_f()
{
	NOT_IMPLEMENTED
}

void R_SetDefaultState(void) // Q2: GL_SetDefaultState()
{
	glClearColor(1.0f, 0.0f, 0.5f, 0.5f);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666f);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_FLAT);

	R_TextureMode(gl_texturemode->string);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min); //mxd. Q2/H2: qglTexParameterf
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max); //mxd. Q2/H2: qglTexParameterf

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //mxd. Q2/H2: qglTexParameterf
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //mxd. Q2/H2: qglTexParameterf

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR.

	R_TexEnv(GL_REPLACE);
}

// Transforms vector to screen space?
void TransformVector(const vec3_t v, vec3_t out)
{
	out[0] = DotProduct(v, vright);
	out[1] = DotProduct(v, vup);
	out[2] = DotProduct(v, vpn);
}
//
// gl_misc.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

void GL_ScreenShot_f(void)
{
	NOT_IMPLEMENTED
}

void GL_Strings_f(void)
{
	NOT_IMPLEMENTED
}

void GL_SetDefaultState(void)
{
	qglClearColor(1.0f, 0.0f, 0.5f, 0.5f);
	qglCullFace(GL_FRONT);
	qglEnable(GL_TEXTURE_2D);

	qglEnable(GL_ALPHA_TEST);
	qglAlphaFunc(GL_GREATER, 0.666f);

	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_BLEND);

	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	qglShadeModel(GL_FLAT);

	GL_TextureMode(gl_texturemode->string);

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min); //mxd. Q2/H2: qglTexParameterf
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max); //mxd. Q2/H2: qglTexParameterf

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //mxd. Q2/H2: qglTexParameterf
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //mxd. Q2/H2: qglTexParameterf

	qglBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR); // Q2: (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

	GL_TexEnv(GL_REPLACE);

	if (qglPointParameterfEXT)
	{
		const float attenuations[3] =
		{
			gl_particle_att_a->value,
			gl_particle_att_b->value,
			gl_particle_att_c->value
		};

		qglEnable(GL_POINT_SMOOTH);
		qglPointParameterfEXT(GL_POINT_SIZE_MIN_EXT, gl_particle_min_size->value);
		qglPointParameterfEXT(GL_POINT_SIZE_MAX_EXT, gl_particle_max_size->value);
		qglPointParameterfvEXT(GL_DISTANCE_ATTENUATION_EXT, attenuations);
	}

	//mxd. Required 'GL_EXT_shared_texture_palette' extension is unsupported since GeForceFX...
	//if (qglColorTableEXT && (int)gl_ext_palettedtexture->value)
		//qglEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
}
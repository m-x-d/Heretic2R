//
// gl1_Misc.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Misc.h"
#include "gl1_Image.h"
#include "gl1_Light.h"
#include "gl1_Local.h"
#include "Vector.h"

void R_ScreenShot_f(void) // Q2: GL_ScreenShot_f()
{
	NOT_IMPLEMENTED
}

void R_Strings_f(void) // Q2: GL_Strings_f()
{
	ri.Con_Printf(PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string); //mxd. Com_Printf() -> ri.Con_Printf().
	ri.Con_Printf(PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string); //mxd. Com_Printf() -> ri.Con_Printf().
	ri.Con_Printf(PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string); //mxd. Com_Printf() -> ri.Con_Printf().
	//ri.Con_Printf("GL_EXT: %s\n", gl_config.extensions_string); //mxd. Disabled, because Com_Printf can't handle strings longer than 1024 chars. //TODO: implement?
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

// Q2 counterpart
void R_DrawNullModel(const entity_t* e) //mxd. Original logic uses 'currententity' global var.
{
	vec3_t shadelight;

	if (e->flags & RF_FULLBRIGHT)
		VectorSet(shadelight, 1.0f, 1.0f, 1.0f);
	else
		R_LightPoint(e->origin, shadelight);

	glPushMatrix();
	R_RotateForEntity(e);

	glDisable(GL_TEXTURE_2D);
	glColor3fv(shadelight);

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, -16.0f);
	for (int i = 0; i < 5; i++)
		glVertex3f(16.0f * cosf((float)i * ANGLE_90), 16.0f * sinf((float)i * ANGLE_90), 0.0f); //mxd. M_PI/2 -> ANGLE_90
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, 16.0f);
	for (int i = 4; i > -1; i--)
		glVertex3f(16.0f * cosf((float)i * ANGLE_90), 16.0f * sinf((float)i * ANGLE_90), 0.0f); //mxd. M_PI/2 -> ANGLE_90
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

// Transforms vector to screen space?
void R_TransformVector(const vec3_t v, vec3_t out)
{
	out[0] = DotProduct(v, vright);
	out[1] = DotProduct(v, vup);
	out[2] = DotProduct(v, vpn);
}

void R_RotateForEntity(const entity_t* e)
{
	glTranslatef(e->origin[0], e->origin[1], e->origin[2]);

	// H2: new RAD_TO_ANGLE scaler.
	glRotatef(e->angles[1] * RAD_TO_ANGLE, 0.0f, 0.0f, 1.0f);
	glRotatef(-e->angles[0] * RAD_TO_ANGLE, 0.0f, 1.0f, 0.0f);
	glRotatef(-e->angles[2] * RAD_TO_ANGLE, 1.0f, 0.0f, 0.0f);
}

void R_HandleTransparency(const entity_t* e) // H2: HandleTrans().
{
	if (e->flags & RF_TRANS_ADD)
	{
		if (e->flags & RF_ALPHA_TEXTURE)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.0f);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glColor4ub(e->color.r, e->color.g, e->color.b, e->color.a);
		}
		else
		{
			if ((int)r_fog->value || (int)cl_camera_under_surface->value) //mxd. Skipped gl_fog_broken check.
				glDisable(GL_FOG);

			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_ONE, GL_ONE);

			if (e->flags & RF_TRANS_ADD_ALPHA)
			{
				const float scaler = (float)e->color.a / 255.0f / 255.0f; //TODO: why is it divided twice?..
				glColor3f((float)e->color.r * scaler, (float)e->color.g * scaler, (float)e->color.b * scaler); //mxd. qglColor4f -> qglColor3f
			}
			else
			{
				glColor3ub(e->color.r, e->color.g, e->color.b); //mxd. qglColor4ub -> qglColor3ub
			}
		}
	}
	else
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.05f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// H2_1.07: qglBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR) when RF_TRANS_GHOST flag is set.
		if (!(e->flags & RF_TRANS_GHOST))
			glColor4ub(e->color.r, e->color.g, e->color.b, e->color.a);
	}

	glEnable(GL_BLEND);
}

void R_CleanupTransparency(const entity_t* e) // H2: CleanupTrans().
{
	glDisable(GL_BLEND);

	if (e->flags & (RF_TRANS_GHOST | RF_TRANS_ADD))
	{
		if ((int)r_fog->value || (int)cl_camera_under_surface->value) //mxd. Removed gl_fog_broken cvar check.
			glEnable(GL_FOG);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR.
	}
	else
	{
		glDisable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.666f);
	}
}
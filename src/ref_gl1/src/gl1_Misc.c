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
#include "vid.h"

void R_ScreenShot_f(void) // Based on YQ2 logic; Q2: GL_ScreenShot_f()
{
#define SCREENSHOT_COMP	3

	const int buf_size = viddef.width * viddef.height * SCREENSHOT_COMP;
	byte* buffer = malloc(buf_size);

	if (buffer == NULL)
	{
		ri.Con_Printf(PRINT_ALL, "R_ScreenShot_f: couldn't malloc %i bytes!\n", buf_size);
		return;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, viddef.width, viddef.height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	ri.Vid_WriteScreenshot(viddef.width, viddef.height, SCREENSHOT_COMP, buffer);
	free(buffer);
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
		R_LightPoint(e->origin, shadelight, false);

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

//mxd. Map object coordinates to window coordinates (slightly modified version of glhProjectf() from https://wikis.khronos.org/opengl/GluProject_and_gluUnProject_code).
qboolean R_PointToScreen(const vec3_t pos, vec3_t screen_pos)
{
	// Transformation vectors.
	float tmp[8];

	// Modelview transform.
	tmp[0] = r_world_matrix[0] * pos[0] + r_world_matrix[4] * pos[1] + r_world_matrix[8] *  pos[2] + r_world_matrix[12]; // w is always 1.
	tmp[1] = r_world_matrix[1] * pos[0] + r_world_matrix[5] * pos[1] + r_world_matrix[9] *  pos[2] + r_world_matrix[13];
	tmp[2] = r_world_matrix[2] * pos[0] + r_world_matrix[6] * pos[1] + r_world_matrix[10] * pos[2] + r_world_matrix[14];
	tmp[3] = r_world_matrix[3] * pos[0] + r_world_matrix[7] * pos[1] + r_world_matrix[11] * pos[2] + r_world_matrix[15];

	// Projection transform, the final row of projection matrix is always [0 0 -1 0], so we optimize for that.
	tmp[4] = r_projection_matrix[0] * tmp[0] + r_projection_matrix[4] * tmp[1] + r_projection_matrix[8] *  tmp[2] + r_projection_matrix[12] * tmp[3];
	tmp[5] = r_projection_matrix[1] * tmp[0] + r_projection_matrix[5] * tmp[1] + r_projection_matrix[9] *  tmp[2] + r_projection_matrix[13] * tmp[3];
	tmp[6] = r_projection_matrix[2] * tmp[0] + r_projection_matrix[6] * tmp[1] + r_projection_matrix[10] * tmp[2] + r_projection_matrix[14] * tmp[3];

	// The result normalizes between -1 and 1.
	if (tmp[2] == 0.0f) // The w value.
		return false;

	tmp[7] = 1.0f / -tmp[2];

	// Perspective division.
	tmp[4] *= tmp[7];
	tmp[5] *= tmp[7];
	tmp[6] *= tmp[7];

	// Window coordinates. Map x, y to range 0 - 1.
	screen_pos[0] = (tmp[4] * 0.5f + 0.5f) * (float)r_newrefdef.width +  (float)r_newrefdef.x;
	screen_pos[1] = (tmp[5] * 0.5f + 0.5f) * (float)r_newrefdef.height + (float)r_newrefdef.y;
	screen_pos[2] = (1.0f + tmp[6]) * 0.5f; // This is only correct when glDepthRange(0.0, 1.0).

	//mxd. y-coord needs flipping...
	screen_pos[1] = (float)r_newrefdef.height - screen_pos[1];

	return true;
}

paletteRGBA_t R_ModulateRGBA(const paletteRGBA_t a, const paletteRGBA_t b) //mxd
{
	const paletteRGBA_t c = { .r = a.r * b.r / 255, .g = a.g * b.g / 255, .b = a.b * b.b / 255, .a = a.a * b.a / 255 };
	return c;
}

paletteRGBA_t R_GetSpriteShadelight(const vec3_t origin, const byte alpha) //mxd
{
	static const vec3_t light_add = { 0.1f, 0.1f, 0.1f };

	vec3_t c;
	R_LightPoint(origin, c, false);
	Vec3AddAssign(light_add, c); // Make it slightly brighter than lightmap color.
	Vec3ScaleAssign(255.0f, c);

	// Make sure light color is valid...
	const float len = VectorLength(c);
	if (len > 255.0f)
		Vec3ScaleAssign(255.0f / len, c);

	const paletteRGBA_t color = { .r = (byte)c[0], .g = (byte)c[1], .b = (byte)c[2], alpha };

	return color;
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
		{
			if (e->flags & RF_LM_COLOR) //mxd
			{
				const paletteRGBA_t c = R_ModulateRGBA(e->color, R_GetSpriteShadelight(e->origin, e->color.a));
				glColor4ub(c.r, c.g, c.b, e->color.a);
			}
			else
			{
				glColor4ub(e->color.r, e->color.g, e->color.b, e->color.a);
			}
		}
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
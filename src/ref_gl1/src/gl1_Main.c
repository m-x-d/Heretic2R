//
// gl1_Main.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Draw.h"
#include "gl1_DrawBook.h"
#include "gl1_DrawCinematic.h"
#include "gl1_FindSurface.h"
#include "gl1_FlexModel.h"
#include "gl1_Image.h"
#include "gl1_Light.h"
#include "gl1_Misc.h"
#include "gl1_SDL.h"
#include "gl1_Sky.h"
#include "gl1_Sprite.h"
#include "gl1_Surface.h"
#include "gl1_Local.h"
#include "Reference.h"
#include "turbsin.h"
#include "Vector.h"
#include "vid.h"

#define REF_DECLSPEC	__declspec(dllexport)

viddef_t viddef; // H2: renamed from vid, defined in vid.h?
refimport_t ri;

model_t* r_worldmodel;

float gldepthmin;
float gldepthmax;

glconfig_t gl_config;
glstate_t gl_state;

// View origin.
vec3_t vup;
vec3_t vpn;
vec3_t vright;
vec3_t r_origin;

float r_world_matrix[16];
cplane_t frustum[4];

refdef_t r_newrefdef; // Screen size info.

int r_framecount; // Used for dlight push checking.

int r_viewcluster;
int r_viewcluster2;
int r_oldviewcluster;
int r_oldviewcluster2;

int c_brush_polys;
int c_alias_polys;

static float v_blend[4]; // Final blending color. //mxd. Made static.
static GLint fog_modes[] = { GL_LINEAR, GL_EXP, GL_EXP2 };

#pragma region ========================== CVARS  ==========================

cvar_t* r_norefresh;
cvar_t* r_fullbright;
cvar_t* r_drawentities;
cvar_t* r_drawworld;
cvar_t* r_novis;
cvar_t* r_nocull;
cvar_t* r_lerpmodels;
static cvar_t* r_speeds;
cvar_t* r_vsync; // YQ2

cvar_t* r_lightlevel; // FIXME: This is a HACK to get the client's light level

cvar_t* r_farclipdist;
cvar_t* r_fog;
cvar_t* r_fog_mode;
cvar_t* r_fog_density;
cvar_t* r_fog_startdist;
static cvar_t* r_fog_color_r;
static cvar_t* r_fog_color_g;
static cvar_t* r_fog_color_b;
static cvar_t* r_fog_color_a;
cvar_t* r_fog_lightmap_adjust;
cvar_t* r_fog_underwater;
static cvar_t* r_fog_underwater_mode;
static cvar_t* r_fog_underwater_density;
static cvar_t* r_fog_underwater_startdist;
static cvar_t* r_fog_underwater_color_r;
static cvar_t* r_fog_underwater_color_g;
static cvar_t* r_fog_underwater_color_b;
static cvar_t* r_fog_underwater_color_a;
static cvar_t* r_underwater_color;
cvar_t* r_frameswap;
cvar_t* r_references;

cvar_t* gl_noartifacts;

cvar_t* gl_modulate;
cvar_t* gl_lightmap;
cvar_t* gl_dynamic;
cvar_t* gl_nobind;
cvar_t* gl_showtris;
static cvar_t* gl_reporthash;
static cvar_t* gl_ztrick;
static cvar_t* gl_finish;
static cvar_t* gl_clear;
static cvar_t* gl_cull;
static cvar_t* gl_polyblend;
cvar_t* gl_flashblend;
cvar_t* gl_texturemode;
cvar_t* gl_lockpvs;

cvar_t* gl_drawflat;
cvar_t* gl_trans33;
cvar_t* gl_trans66;
cvar_t* gl_bookalpha;

cvar_t* gl_drawmode;
cvar_t* gl_drawbuffer;
cvar_t* gl_saturatelighting;

cvar_t* vid_fullscreen;
cvar_t* vid_gamma;
cvar_t* vid_brightness;
cvar_t* vid_contrast;
static cvar_t* vid_textures_refresh_required; //mxd

cvar_t* vid_ref;

cvar_t* vid_mode; // gl_mode in Q2
cvar_t* menus_active;
cvar_t* cl_camera_under_surface;
cvar_t* quake_amount;

#pragma endregion

// H2: simplified: no separate non-transparent/transparent drawing chains.
static void R_DrawEntitiesOnList(void)
{
	if (!(int)r_drawentities->value)
		return;

	for (int i = 0; i < r_newrefdef.num_entities; i++)
	{
		entity_t* ent = r_newrefdef.entities[i]; //mxd. Original logic uses 'currententity' global var.

		if (ent->model == NULL) // H2: extra sanity check.
		{
			ri.Con_Printf(PRINT_ALL, "Attempt to draw NULL model\n"); //mxd. Com_Printf() -> ri.Con_Printf().
			R_DrawNullModel(ent);

			continue;
		}

		const model_t* mdl = *ent->model; //mxd. Original logic uses 'currentmodel' global var.

		if (mdl == NULL)
		{
			R_DrawNullModel(ent);
			continue;
		}

		// H2: no mod_alias case, new mod_bad and mod_fmdl cases.
		switch (mdl->type)
		{
			case mod_bad:
				ri.Con_Printf(PRINT_ALL, "WARNING: currentmodel->type == 0; reload the map\n"); //mxd. Com_Printf() -> ri.Con_Printf().
				break;

			case mod_brush:
				R_DrawBrushModel(ent);
				break;

			case mod_sprite:
				R_DrawSpriteModel(ent);
				break;

			case mod_fmdl:
				R_DrawFlexModel(ent);
				break;

			default:
				ri.Sys_Error(ERR_DROP, "Bad modeltype"); // Q2: ri.Sys_Error //mxd. Sys_Error() -> ri.Sys_Error().
				break;
		}
	}
}

static void R_DrawParticles(const int num_particles, const particle_t* particles, const qboolean alpha_particle)
{
	static GLfloat particle_st_coords[NUM_PARTICLE_TYPES][4] =
	{
		{ 0.00390625f, 0.00390625f, 0.02734375f, 0.02734375f },
		{ 0.03515625f, 0.00390625f, 0.05859375f, 0.02734375f },
		{ 0.06640625f, 0.00390625f, 0.08984375f, 0.02734375f },
		{ 0.09765625f, 0.00390625f, 0.12109375f, 0.02734375f },
		{ 0.00390625f, 0.03515625f, 0.02734375f, 0.05859375f },
		{ 0.03515625f, 0.03515625f, 0.05859375f, 0.05859375f },
		{ 0.06640625f, 0.03515625f, 0.08984375f, 0.05859375f },
		{ 0.09765625f, 0.03515625f, 0.12109375f, 0.05859375f },
		{ 0.00390625f, 0.06640625f, 0.02734375f, 0.08984375f },
		{ 0.03515625f, 0.06640625f, 0.05859375f, 0.08984375f },
		{ 0.06640625f, 0.06640625f, 0.08984375f, 0.08984375f },
		{ 0.09765625f, 0.06640625f, 0.12109375f, 0.08984375f },
		{ 0.00390625f, 0.09765625f, 0.02734375f, 0.12109375f },
		{ 0.03515625f, 0.09765625f, 0.05859375f, 0.12109375f },
		{ 0.06640625f, 0.09765625f, 0.08984375f, 0.12109375f },
		{ 0.09765625f, 0.09765625f, 0.12109375f, 0.12109375f },
		{ 0.12890625f, 0.00390625f, 0.18359375f, 0.05859375f },
		{ 0.19140625f, 0.00390625f, 0.24609375f, 0.05859375f },
		{ 0.12890625f, 0.06640625f, 0.18359375f, 0.12109375f },
		{ 0.19140625f, 0.06640625f, 0.24609375f, 0.12109375f },
		{ 0.00390625f, 0.12890625f, 0.12109375f, 0.24609375f },
		{ 0.12890625f, 0.12890625f, 0.24609375f, 0.24609375f },
		{ 0.25390625f, 0.00390625f, 0.37109375f, 0.12109375f },
		{ 0.37890625f, 0.00390625f, 0.49609375f, 0.12109375f },
		{ 0.25390625f, 0.12890625f, 0.37109375f, 0.24609375f },
		{ 0.37890625f, 0.12890625f, 0.49609375f, 0.24609375f },
		{ 0.00390625f, 0.25390625f, 0.24609375f, 0.49609375f },
		{ 0.25390625f, 0.25390625f, 0.49609375f, 0.49609375f },
		{ 0.50390625f, 0.00390625f, 0.74609375f, 0.24609375f },
		{ 0.75390625f, 0.00390625f, 0.99609375f, 0.24609375f },
		{ 0.50390625f, 0.25390625f, 0.74609375f, 0.49609375f },
		{ 0.75390625f, 0.25390625f, 0.87109375f, 0.37109375f },
		{ 0.87890625f, 0.25390625f, 0.99609375f, 0.37109375f },
		{ 0.75390625f, 0.37890625f, 0.87109375f, 0.49609375f },
		{ 0.87890625f, 0.37890625f, 0.99609375f, 0.49609375f },
		{ 0.00390625f, 0.50390625f, 0.24609375f, 0.74609375f },
		{ 0.00390625f, 0.50390625f, 0.24609375f, 0.74609375f },
		{ 0.25390625f, 0.50390625f, 0.37109375f, 0.62109375f },
		{ 0.37890625f, 0.50390625f, 0.43359375f, 0.55859375f },
		{ 0.44140625f, 0.50390625f, 0.49609375f, 0.55859375f },
		{ 0.37890625f, 0.56640625f, 0.43359375f, 0.62109375f },
		{ 0.44140625f, 0.56640625f, 0.49609375f, 0.62109375f },
		{ 0.25390625f, 0.62890625f, 0.30859375f, 0.68359375f },
		{ 0.31640625f, 0.62890625f, 0.37109375f, 0.68359375f },
		{ 0.25390625f, 0.69140625f, 0.30859375f, 0.74609375f },
		{ 0.31640625f, 0.69140625f, 0.37109375f, 0.74609375f },
		{ 0.37890625f, 0.62890625f, 0.43359375f, 0.68359375f },
		{ 0.44140625f, 0.62890625f, 0.49609375f, 0.68359375f },
		{ 0.37890625f, 0.69140625f, 0.43359375f, 0.74609375f },
		{ 0.44140625f, 0.69140625f, 0.49609375f, 0.74609375f },
		{ 0.00390625f, 0.75390625f, 0.24609375f, 0.99609375f },
		{ 0.25390625f, 0.75390625f, 0.49609375f, 0.99609375f },
		{ 0.50390625f, 0.50390625f, 0.62109375f, 0.62109375f },
		{ 0.62890625f, 0.50390625f, 0.74609375f, 0.62109375f },
		{ 0.50390625f, 0.62890625f, 0.62109375f, 0.74609375f },
		{ 0.62890625f, 0.62890625f, 0.74609375f, 0.74609375f },
		{ 0.75390625f, 0.50390625f, 0.99609375f, 0.74609375f },
		{ 0.50390625f, 0.75390625f, 0.74609375f, 0.99609375f },
		{ 0.75390625f, 0.75390625f, 0.87109375f, 0.87109375f },
		{ 0.87890625f, 0.75390625f, 0.99609375f, 0.87109375f },
		{ 0.75390625f, 0.87890625f, 0.87109375f, 0.99609375f },
		{ 0.87890625f, 0.87890625f, 0.99609375f, 0.99609375f }
	};

	if (alpha_particle)
	{
		R_BindImage(r_aparticletexture);
		glBlendFunc(GL_ONE, GL_ONE);

		if ((int)r_fog->value || (int)cl_camera_under_surface->value) //mxd. Removed gl_fog_broken cvar check
			glDisable(GL_FOG);

		glDisable(GL_ALPHA_TEST);
	}
	else
	{
		R_BindImage(r_particletexture);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glEnable(GL_BLEND);
	R_TexEnv(GL_MODULATE);

	glBegin(GL_QUADS);

	const particle_t* p = &particles[0];
	for (int i = 0; i < num_particles; i++, p++)
	{
		vec3_t p_up;
		VectorScale(vup, p->scale, p_up);

		vec3_t p_right;
		VectorScale(vright, p->scale, p_right);

		paletteRGBA_t c = p->color;
		if (alpha_particle)
		{
			c.r = c.r * c.a / 255;
			c.g = c.g * c.a / 255;
			c.b = c.b * c.a / 255;
		}

		const byte p_type = p->type & 127; // Strip particle flags.

		glColor4ubv(c.c_array);

		glTexCoord2f(particle_st_coords[p_type][0], particle_st_coords[p_type][1]);
		glVertex3f(p->origin[0] + p_up[0], p->origin[1] + p_up[1], p->origin[2] + p_up[2]);

		glTexCoord2f(particle_st_coords[p_type][2], particle_st_coords[p_type][1]);
		glVertex3f(p->origin[0] + p_right[0], p->origin[1] + p_right[1], p->origin[2] + p_right[2]);

		glTexCoord2f(particle_st_coords[p_type][2], particle_st_coords[p_type][3]);
		glVertex3f(p->origin[0] - p_up[0], p->origin[1] - p_up[1], p->origin[2] - p_up[2]);

		glTexCoord2f(particle_st_coords[p_type][0], particle_st_coords[p_type][3]);
		glVertex3f(p->origin[0] - p_right[0], p->origin[1] - p_right[1], p->origin[2] - p_right[2]);
	}

	glEnd();

	if (alpha_particle)
	{
		if ((int)r_fog->value || (int)cl_camera_under_surface->value) //mxd. Removed gl_fog_broken cvar check.
			glEnable(GL_FOG);

		glEnable(GL_ALPHA_TEST);
	}

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	R_TexEnv(GL_REPLACE);
}

// Q2 counterpart
static byte R_SignbitsForPlane(const cplane_t* plane) //mxd. Changed return type to byte.
{
	// For fast box on planeside test.
	byte bits = 0;
	for (int i = 0; i < 3; i++)
		if (plane->normal[i] < 0.0f)
			bits |= 1 << i;

	return bits;
}

static void R_SetFrustum(void)
{
	RotatePointAroundVector(frustum[0].normal, vup,		vpn, -(90.0f - r_newrefdef.fov_x * 0.5f));	// Rotate VPN right by FOV_X/2 degrees.
	RotatePointAroundVector(frustum[1].normal, vup,		vpn,   90.0f - r_newrefdef.fov_x * 0.5f);	// Rotate VPN left by FOV_X/2 degrees.
	RotatePointAroundVector(frustum[2].normal, vright,	vpn,   90.0f - r_newrefdef.fov_y * 0.5f);	// Rotate VPN up by FOV_X/2 degrees.
	RotatePointAroundVector(frustum[3].normal, vright,	vpn, -(90.0f - r_newrefdef.fov_y * 0.5f));	// Rotate VPN down by FOV_X/2 degrees.

	for (int i = 0; i < 4; i++)
	{
		// H2:
		const float frustum_dist = VectorLength(frustum[i].normal);
		if (frustum_dist <= 0.999999f)
			ri.Con_Printf(PRINT_ALL, "Frustum normal dist %f < 1.0\n", (double)frustum_dist); //mxd. Com_Printf() -> ri.Con_Printf().

		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct(r_origin, frustum[i].normal);
		frustum[i].signbits = R_SignbitsForPlane(&frustum[i]);
	}
}

// Q2 counterpart
static void R_PolyBlend(void)
{
	if ((int)gl_polyblend->value && v_blend[3] != 0.0f)
	{
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);

		glLoadIdentity();

		// FIXME: get rid of these.
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);	// Put Z going up.
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);		// Put Z going up.

		glColor4fv(v_blend);

		glBegin(GL_QUADS);

		glVertex3f(10.0f, 100.0f, 100.0f);
		glVertex3f(10.0f, -100.0f, 100.0f);
		glVertex3f(10.0f, -100.0f, -100.0f);
		glVertex3f(10.0f, 100.0f, -100.0f);

		glEnd();

		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

// Q2 counterpart
static void R_SetupFrame(void)
{
	r_framecount++;

	// Build the transformation matrix for the given view angles.
	VectorCopy(r_newrefdef.vieworg, r_origin);
	AngleVectors(r_newrefdef.viewangles, vpn, vright, vup);

	// Current viewcluster.
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;

		const mleaf_t* leaf = Mod_PointInLeaf(r_origin, r_worldmodel);
		r_viewcluster = leaf->cluster;
		r_viewcluster2 = r_viewcluster;

		// Check above and below so crossing solid water doesn't draw wrong.
		vec3_t temp;
		VectorCopy(r_origin, temp);

		if (leaf->contents == 0)
			temp[2] -= 16.0f; // Look down a bit.
		else
			temp[2] += 16.0f; // Look up a bit.

		leaf = Mod_PointInLeaf(temp, r_worldmodel);
		if (!(leaf->contents & CONTENTS_SOLID))
			r_viewcluster2 = leaf->cluster;
	}

	for (int i = 0; i < 4; i++)
		v_blend[i] = r_newrefdef.blend[i];

	c_brush_polys = 0;
	c_alias_polys = 0;

	// Clear out the portion of the screen that the NOWORLDMODEL defines.
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
	{
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glScissor(r_newrefdef.x, viddef.height - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 0.0f, 0.5f, 0.5f);
		glDisable(GL_SCISSOR_TEST);
	}
}

static void R_SetPerspective(GLdouble fovy) // YQ2
{
	// gluPerspective style parameters.
	static const GLdouble zNear = 1.0; // Q2: 4.0
	const GLdouble zFar = r_farclipdist->value;
	const GLdouble aspectratio = (GLdouble)r_newrefdef.width / r_newrefdef.height;

	// Traditional gluPerspective calculations - https://youtu.be/YqSNGcF5nvM?t=644
	GLdouble ymax = zNear * tan(fovy * M_PI / 360.0);
	GLdouble xmax = ymax * aspectratio;

	GLdouble ymin = -ymax;
	GLdouble xmin = -xmax;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

static void R_SetupGL(void)
{
	//mxd. Removed unneeded integer multiplications/divisions.
	const int xl = r_newrefdef.x;
	const int xr = r_newrefdef.x + r_newrefdef.width;
	const int yt = viddef.height - r_newrefdef.y;
	const int yb = viddef.height - (r_newrefdef.y + r_newrefdef.height);

	glViewport(xl, yb, xr - xl, yt - yb);

	// Set up projection matrix.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	R_SetPerspective(r_newrefdef.fov_y);

	glCullFace(GL_FRONT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Put Z going up.
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f); // Put Z going up.
	glRotatef(-r_newrefdef.viewangles[2], 1.0f, 0.0f, 0.0f);
	glRotatef(-r_newrefdef.viewangles[0], 0.0f, 1.0f, 0.0f);
	glRotatef(-r_newrefdef.viewangles[1], 0.0f, 0.0f, 1.0f);
	glTranslatef(-r_newrefdef.vieworg[0], -r_newrefdef.vieworg[1], -r_newrefdef.vieworg[2]);

	glGetFloatv(GL_MODELVIEW_MATRIX, r_world_matrix);

	// Set drawing parms.
	if ((int)gl_cull->value)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	// H2: extra gl_drawmode logic.
	if ((int)gl_drawmode->value)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(1.0f, 0.0f, 0.5f, 0.5f);
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
	}
}

static void R_Fog(void) // H2: GL_Fog
{
	const int mode = ClampI((int)r_fog_mode->value, 0, sizeof(fog_modes) / sizeof(fog_modes[0])); //mxd. Added ClampI().
	glFogi(GL_FOG_MODE, fog_modes[mode]);

	if (mode == 0)
	{
		glFogf(GL_FOG_START, r_fog_startdist->value);
		glFogf(GL_FOG_END, r_farclipdist->value);
	}
	else
	{
		glFogf(GL_FOG_DENSITY, r_fog_density->value);
	}

	const float color[] = { r_fog_color_r->value, r_fog_color_g->value, r_fog_color_b->value, r_fog_color_a->value };
	glFogfv(GL_FOG_COLOR, color);
	glEnable(GL_FOG);

	glClearColor(color[0], color[1], color[2], color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

static void R_WaterFog(void) // H2: GL_WaterFog
{
	//TODO: GL_EXP2 fog mode is ignored. Why?
	const int mode = ClampI((int)r_fog_underwater_mode->value, 0, sizeof(fog_modes) / sizeof(fog_modes[0]) - 1); //mxd. Added ClampI().
	glFogi(GL_FOG_MODE, fog_modes[mode]);

	if (mode == 0)
	{
		glFogf(GL_FOG_START, r_fog_underwater_startdist->value);
		glFogf(GL_FOG_END, r_farclipdist->value);
	}
	else
	{
		glFogf(GL_FOG_DENSITY, r_fog_underwater_density->value);
	}

	const float color[] = { r_fog_underwater_color_r->value, r_fog_underwater_color_g->value, r_fog_underwater_color_b->value, r_fog_underwater_color_a->value };
	glFogfv(GL_FOG_COLOR, color);
	glEnable(GL_FOG);

	glClearColor(color[0], color[1], color[2], color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

static void R_Clear(void)
{
	if ((int)gl_ztrick->value) //TODO: mxd. No fog rendering when gl_ztrick is enabled. Curious...
	{
		static int trickframe;

		if ((int)gl_clear->value)
			glClear(GL_COLOR_BUFFER_BIT);

		trickframe++;

		if (trickframe & 1)
		{
			gldepthmin = 0.0f;
			gldepthmax = 0.49999f;
			glDepthFunc(GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1.0f;
			gldepthmax = 0.5f;
			glDepthFunc(GL_GEQUAL);
		}
	}
	else
	{
		// H2: extra fog rendering logic. //mxd. Removed gl_fog_broken cvar checks.
		if ((int)cl_camera_under_surface->value) //TODO: r_fog_underwater cvar check seems logical here, but isn't present in original dll.
		{
			R_WaterFog();
		}
		//mxd. Removed 'r_fog_startdist->value < r_farclipdist->value' check, because it's relevant only for fog mode 0.
		// Also there's no r_fog_underwater_startdist check in GL_WaterFog case in original .dll.
		else if ((int)r_fog->value)
		{
			R_Fog();
		}
		else
		{
			glDisable(GL_FOG);

			if ((int)gl_clear->value)
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			else
				glClear(GL_DEPTH_BUFFER_BIT);
		}

		gldepthmin = 0.0f;
		gldepthmax = 1.0f;
		glDepthFunc(GL_LEQUAL);
	}

	glDepthRange((double)gldepthmin, (double)gldepthmax);
}

static void R_Register(void)
{
	r_norefresh = ri.Cvar_Get("r_norefresh", "0", 0);
	r_fullbright = ri.Cvar_Get("r_fullbright", "0", 0);
	r_drawentities = ri.Cvar_Get("r_drawentities", "1", 0);
	r_drawworld = ri.Cvar_Get("r_drawworld", "1", 0);
	r_novis = ri.Cvar_Get("r_novis", "0", 0);
	r_nocull = ri.Cvar_Get("r_nocull", "0", 0);
	r_lerpmodels = ri.Cvar_Get("r_lerpmodels", "1", 0);
	r_speeds = ri.Cvar_Get("r_speeds", "0", 0);
	r_vsync = ri.Cvar_Get("r_vsync", "1", CVAR_ARCHIVE); // YQ2

	r_lightlevel = ri.Cvar_Get("r_lightlevel", "0", 0);

	// H2:
	r_farclipdist = ri.Cvar_Get("r_farclipdist", "4096.0", 0);
	r_fog = ri.Cvar_Get("r_fog", "0", 0);
	r_fog_mode = ri.Cvar_Get("r_fog_mode", "1", 0);
	r_fog_density = ri.Cvar_Get("r_fog_density", "0.004", 0);
	r_fog_startdist = ri.Cvar_Get("r_fog_startdist", "50.0", 0);
	r_fog_color_r = ri.Cvar_Get("r_fog_color_r", "1.0", 0);
	r_fog_color_g = ri.Cvar_Get("r_fog_color_g", "1.0", 0);
	r_fog_color_b = ri.Cvar_Get("r_fog_color_b", "1.0", 0);
	r_fog_color_a = ri.Cvar_Get("r_fog_color_a", "0.0", 0);
	r_fog_lightmap_adjust = ri.Cvar_Get("r_fog_lightmap_adjust", "5.0", 0);
	r_fog_underwater_mode = ri.Cvar_Get("r_fog_underwater_mode", "1", 0);
	r_fog_underwater_density = ri.Cvar_Get("r_fog_underwater_density", "0.0015", 0);
	r_fog_underwater_startdist = ri.Cvar_Get("r_fog_underwater_startdist", "100.0", 0);
	r_fog_underwater_color_r = ri.Cvar_Get("r_fog_underwater_color_r", "1.0", 0);
	r_fog_underwater_color_g = ri.Cvar_Get("r_fog_underwater_color_g", "1.0", 0);
	r_fog_underwater_color_b = ri.Cvar_Get("r_fog_underwater_color_b", "1.0", 0);
	r_fog_underwater_color_a = ri.Cvar_Get("r_fog_underwater_color_a", "0.0", 0);
	r_underwater_color = ri.Cvar_Get("r_underwater_color", "0x70c06000", 0);
	r_frameswap = ri.Cvar_Get("r_frameswap", "1.0", 0);
	r_references = ri.Cvar_Get("r_references", "1.0", 0);

	gl_noartifacts = ri.Cvar_Get("gl_noartifacts", "0", 0); // H2

	gl_modulate = ri.Cvar_Get("gl_modulate", "1", CVAR_ARCHIVE);
	gl_lightmap = ri.Cvar_Get("gl_lightmap", "0", 0);
	gl_dynamic = ri.Cvar_Get("gl_dynamic", "1", 0);
	gl_nobind = ri.Cvar_Get("gl_nobind", "0", 0);
	gl_showtris = ri.Cvar_Get("gl_showtris", "0", 0);
	gl_reporthash = ri.Cvar_Get("gl_reporthash", "0", 0);
	gl_ztrick = ri.Cvar_Get("gl_ztrick", "0", 0);
	gl_finish = ri.Cvar_Get("gl_finish", "0", 0);
	gl_clear = ri.Cvar_Get("gl_clear", "0", 0);
	gl_cull = ri.Cvar_Get("gl_cull", "1", 0);
	gl_polyblend = ri.Cvar_Get("gl_polyblend", "1", 0);
	gl_flashblend = ri.Cvar_Get("gl_flashblend", "0", 0);
	gl_texturemode = ri.Cvar_Get("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE);
	gl_lockpvs = ri.Cvar_Get("gl_lockpvs", "0", 0);

	// H2:
	gl_drawflat = ri.Cvar_Get("gl_drawflat", "0", 0);
	gl_trans33 = ri.Cvar_Get("gl_trans33", "0.33", 0); // H2_1.07: 0.33 -> 1
	gl_trans66 = ri.Cvar_Get("gl_trans66", "0.66", 0); // H2_1.07: 0.66 -> 1
	gl_bookalpha = ri.Cvar_Get("gl_bookalpha", "1.0", 0);

	gl_drawmode = ri.Cvar_Get("gl_drawmode", "0", 0);
	gl_drawbuffer = ri.Cvar_Get("gl_drawbuffer", "GL_BACK", 0);
	gl_saturatelighting = ri.Cvar_Get("gl_saturatelighting", "0", 0);

	vid_fullscreen = ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = ri.Cvar_Get("vid_gamma", "0.5", CVAR_ARCHIVE);
	vid_brightness = ri.Cvar_Get("vid_brightness", "0.5", CVAR_ARCHIVE); // H2
	vid_contrast = ri.Cvar_Get("vid_contrast", "0.5", CVAR_ARCHIVE); // H2
	vid_textures_refresh_required = ri.Cvar_Get("vid_textures_refresh_required", "0", 0); //mxd

	vid_ref = ri.Cvar_Get("vid_ref", "gl", CVAR_ARCHIVE);

	// H2:
	vid_mode = ri.Cvar_Get("vid_mode", "1", CVAR_ARCHIVE); // H2: 3
	menus_active = ri.Cvar_Get("menus_active", "0", 0);
	cl_camera_under_surface = ri.Cvar_Get("cl_camera_under_surface", "0", 0);
	quake_amount = ri.Cvar_Get("quake_amount", "0", 0);

	ri.Cmd_AddCommand("imagelist", R_ImageList_f);
	ri.Cmd_AddCommand("screenshot", R_ScreenShot_f);
	ri.Cmd_AddCommand("modellist", Mod_Modellist_f);
	ri.Cmd_AddCommand("gl_strings", R_Strings_f);

	R_InitGammaTable(); // H2
}

// Changes the video mode.
static rserr_t SetMode_impl(int* pwidth, int* pheight, const int mode) // YQ2
{
	ri.Con_Printf(PRINT_ALL, "Setting mode %d:", mode);

	if (!ri.Vid_GetModeInfo(pwidth, pheight, mode))
	{
		ri.Con_Printf(PRINT_ALL, " invalid mode\n");
		return RSERR_INVALID_MODE;
	}

	ri.Con_Printf(PRINT_ALL, " %dx%d\n", *pwidth, *pheight);

	return (ri.GLimp_InitGraphics(*pwidth, *pheight) ? RSERR_OK : RSERR_INVALID_MODE);
}

static qboolean R_SetMode(void)
{
	rserr_t err = SetMode_impl(&viddef.width, &viddef.height, (int)vid_mode->value);

	if (err == RSERR_OK)
	{
		gl_state.prev_mode = (int)vid_mode->value;

		ri.Cvar_SetValue("vid_fullscreen", (int)vid_mode->value == 0 ? 1.0f : 0.0f); //mxd. Fullscreen when Mode 0, windowed otherwise.
		vid_fullscreen->modified = false;

		return true;
	}

	if (err == RSERR_INVALID_MODE)
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_SetMode() - invalid mode\n");

		// Trying again would result in a crash anyway, give up already (this would happen if your initing fails at all and your resolution already was 640x480).
		if ((int)vid_mode->value == gl_state.prev_mode)
			return false;

		ri.Cvar_SetValue("vid_mode", (float)gl_state.prev_mode);
		vid_mode->modified = false;
	}
	else
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::R_SetMode() - unknown error %i!\n", err);
		return false;
	}

	// Try setting it back to something safe.
	err = SetMode_impl(&viddef.width, &viddef.height, (int)vid_mode->value);

	if (err == RSERR_OK)
	{
		ri.Cvar_SetValue("vid_fullscreen", (int)vid_mode->value == 0 ? 1.0f : 0.0f); //mxd. Fullscreen when Mode 0, windowed otherwise.
		vid_fullscreen->modified = false;

		return true;
	}

	ri.Con_Printf(PRINT_ALL, "ref_gl::R_SetMode() - could not revert to safe mode\n");
	return false;
}

static qboolean RI_Init(void)
{
	for (int j = 0; j < 256; j++)
		turbsin[j] *= 0.5f;

	ri.Con_Printf(PRINT_ALL, "Refresh: "REF_TITLE"\n"); //mxd. Com_Printf() -> ri.Con_Printf() (here and below).
	R_Register();

	// Set our "safe" mode.
	gl_state.prev_mode = 1; // H2: 3.

	// Create the window and set up the context.
	if (!R_SetMode())
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::RI_Init() - could not R_SetMode()\n");
		return false; //mxd. Decompiled code still returns -1 here...
	}

	// Get our various GL strings.
	gl_config.vendor_string = (const char*)glGetString(GL_VENDOR);
	ri.Con_Printf(PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string);

	gl_config.renderer_string = (const char*)glGetString(GL_RENDERER);
	ri.Con_Printf(PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string);

	gl_config.version_string = (const char*)glGetString(GL_VERSION);
	ri.Con_Printf(PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string);

	gl_config.extensions_string = (const char*)glGetString(GL_EXTENSIONS);
	//Com_Printf("GL_EXTENSIONS: %s\n", gl_config.extensions_string); // H2_1.07: "GL_EXT: hidden\n" //mxd. Modern extensions_string is longer than Com_Printf can handle... 

	// YQ2: Anisotropic texture filtering.
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_config.max_anisotropy);
	ri.Con_Printf(PRINT_ALL, "Max. anisotropy: %i.\n", (int)gl_config.max_anisotropy);

	//mxd. Check max. supported texture size. H2 expects at least 128x128. H2R expects at least 512x512 (for cinematics rendering without frame chopping shenanigans). Probably not needed: even GF2 supports 2048x2048 textures.
	int max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	if (max_texture_size < 512)
	{
		ri.Con_Printf(PRINT_ALL, "ref_gl::RI_Init() - maximum supported texture size too low! Expected at least 512, got %i\n", max_texture_size);
		return false;
	}

	R_SetDefaultState();
	R_InitImages();
	Mod_Init();
	Draw_InitLocal();

	const GLenum err = glGetError(); //TODO: missing in YQ2 version. Not needed?
	if (err != GL_NO_ERROR)
	{
		ri.Con_Printf(PRINT_ALL, "glGetError() = 0x%x\n", err);
		return false;
	}

	return true; //mxd. Return value missing in Q2
}

static void RI_Shutdown(void)
{
	ShutdownFonts(); // H2

	ri.Cmd_RemoveCommand("modellist");
	ri.Cmd_RemoveCommand("screenshot");
	ri.Cmd_RemoveCommand("imagelist");
	ri.Cmd_RemoveCommand("gl_strings");

	Mod_FreeAll();
	R_ShutdownImages();

	// Shutdown OS-specific OpenGL stuff like contexts, etc.
	RI_ShutdownContext(); // YQ2
}

static void RI_BeginFrame(const float camera_separation) //TODO: remove camera_separation arg?
{
	// Changed.
	if (vid_gamma->modified || vid_brightness->modified || vid_contrast->modified)
	{
		R_InitGammaTable();
		R_GammaAffect(false);

		vid_gamma->modified = false;
		vid_brightness->modified = false;
		vid_contrast->modified = false;
	}
	else if (vid_textures_refresh_required->value == 1.0f) //mxd. Roundabout way to apply gamma changes to ALL textures after Video menu is closed...
	{
		R_GammaAffect(true);
		ri.Cvar_SetValue("vid_textures_refresh_required", 0.0f);
	}

	// Go into 2D mode.
	glViewport(0, 0, viddef.width, viddef.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, viddef.width, viddef.height, 0.0, -99999.0, 99999.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw buffer stuff.
	if (gl_drawbuffer->modified)
	{
		glDrawBuffer((Q_stricmp(gl_drawbuffer->string, "GL_FRONT") == 0) ? GL_FRONT : GL_BACK);
		gl_drawbuffer->modified = false;
	}

	// Texturemode stuff.
	if (gl_texturemode->modified)
	{
		R_TextureMode(gl_texturemode->string);
		gl_texturemode->modified = false;
	}

	// Missing: gl_texturealphamode and gl_texturesolidmode logic

	// Swapinterval stuff.
	if (r_vsync->modified) // YQ2
	{
		R_SetVsync();
		r_vsync->modified = false;
	}

	// Clear screen if desired.
	R_Clear();
}

static void R_RenderView(const refdef_t* fd)
{
	if ((int)r_norefresh->value)
		return;

	r_newrefdef = *fd;

	if (r_worldmodel == NULL && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		ri.Sys_Error(ERR_DROP, "R_RenderView: NULL worldmodel"); //mxd. Sys_Error() -> ri.Sys_Error().

	if ((int)r_speeds->value)
	{
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	R_PushDlights();

	if ((int)gl_finish->value)
		glFinish();

	R_SetupFrame();
	R_SetFrustum();
	R_SetupGL();
	R_MarkLeaves(); // Done here so we know if we're in water.
	R_DrawWorld();
	R_DrawEntitiesOnList();
	R_RenderDlights();

	// Changed in H2:
	glDepthMask(GL_FALSE);
	R_SortAndDrawAlphaSurfaces();
	R_DrawParticles(r_newrefdef.num_particles, r_newrefdef.particles, false);
	R_DrawParticles(r_newrefdef.anum_particles, r_newrefdef.aparticles, true);
	glDepthMask(GL_TRUE);

	// Changed in H2: R_Flash() call replaced with R_PolyBlend() call (or optimization?).
	R_PolyBlend();

	if ((int)r_speeds->value)
		ri.Con_Printf(PRINT_ALL, "%4i wpoly %4i epoly %i tex %i lmaps\n", c_brush_polys, c_alias_polys, c_visible_textures, c_visible_lightmaps); // H2: ri.Con_Printf -> Com_Printf //mxd. Com_Printf() -> ri.Con_Printf().

	if ((int)gl_reporthash->value) // H2
		R_DisplayHashTable();
}

// Q2 counterpart
static void R_SetGL2D(void)
{
	// Set 2D virtual screen size.
	glViewport(0, 0, viddef.width, viddef.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (double)viddef.width, (double)viddef.height, 0.0, -99999.0, 99999.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

static void R_SetLightLevel(void)
{
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{
		// Save off light value for server to look at (BIG HACK!).
		vec3_t shadelight;
		R_LightPoint(r_newrefdef.clientmodelorg, shadelight); // H2: vieworg -> clientmodelorg

		// Pick the greatest component, which should be the same as the mono value returned by software.
		r_lightlevel->value = max(shadelight[0], max(shadelight[1], shadelight[2])) * 150.0f;
	}
}

static void R_ScreenFlash(const paletteRGBA_t color)
{
	glDepthMask(GL_FALSE);
	Draw_FadeScreen(color);
	glDepthMask(GL_TRUE);

	ri.Deactivate_Screen_Flash();
}

// H2: return type: void -> int //TODO: useless: always returns 0 
static int RI_RenderFrame(const refdef_t* fd)
{
	paletteRGBA_t color;

	if ((int)cl_camera_under_surface->value)
		color.c = strtoul(r_underwater_color->string, NULL, 0);
	else
		color.c = ri.Is_Screen_Flashing();

	if (color.a != 255)
	{
		// Q2 version calls these 3 functions only.
		R_RenderView(fd);
		R_SetLightLevel();
		R_SetGL2D();

		if (color.a == 0)
			return 0;
	}

	R_ScreenFlash(color);

	return 0;
}

static int RI_GetReferencedID(const struct model_s* model) // H2 //mxd. Named 'GetReferencedID' (in m_Reference.c) in original logic.
{
	const fmdl_t* temp = model->extradata;

	//mxd. H2 Toolkit code checks for qboolean model->model_type.
	if (model->type == mod_fmdl && temp->referenceType > REF_NULL && temp->referenceType < NUM_REFERENCED)
		return temp->referenceType;

	return REF_NULL;
}

REF_DECLSPEC refexport_t GetRefAPI(const refimport_t rimp)
{
	refexport_t re;

	ri = rimp;

	re.api_version = REF_API_VERSION;
	re.title = REF_TITLE; //mxd

	re.BeginRegistration = RI_BeginRegistration;
	re.RegisterModel = RI_RegisterModel;
	re.RegisterSkin = RI_RegisterSkin;
	re.RegisterPic = Draw_FindPic;
	re.SetSky = RI_SetSky;
	re.EndRegistration = RI_EndRegistration;
	re.GetReferencedID = RI_GetReferencedID;

	re.RenderFrame = RI_RenderFrame;

	re.DrawGetPicSize = Draw_GetPicSize;
	re.DrawPic = Draw_Pic;
	re.DrawStretchPic = Draw_StretchPic;
	re.DrawChar = Draw_Char;
	re.DrawTileClear = Draw_TileClear;
	re.DrawFill = Draw_Fill;
	re.DrawFadeScreen = Draw_FadeScreen;

	re.DrawBigFont = Draw_BigFont;
	re.BF_Strlen = BF_Strlen;
	re.BookDrawPic = Draw_BookPic;
	re.DrawInitCinematic = Draw_InitCinematic;
	re.DrawCloseCinematic = Draw_CloseCinematic;
	re.DrawCinematic = Draw_Cinematic;
	re.Draw_Name = Draw_Name;

	re.Init = RI_Init;
	re.Shutdown = RI_Shutdown;

	re.BeginFrame = RI_BeginFrame;
	re.EndFrame = RI_EndFrame;
	re.FindSurface = RI_FindSurface;

	re.PrepareForWindow = RI_PrepareForWindow; // YQ2
	re.InitContext = RI_InitContext; // YQ2
	re.ShutdownContext = RI_ShutdownContext; // YQ2

#ifdef _DEBUG
	//mxd. Debug draw logic.
	re.AddDebugBox = RI_AddDebugBox;
	re.AddDebugBbox = RI_AddDebugBbox;
	re.AddDebugEntityBbox = RI_AddDebugEntityBbox;

	re.AddDebugLine = RI_AddDebugLine;
	re.AddDebugArrow = RI_AddDebugArrow;
#endif

	// Unbound: A3D_RenderGeometry();
	//TODO: ri.Vid_RequestRestart(RESTART_NO) YQ2 logic.

	return re;
}
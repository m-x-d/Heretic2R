//
// gl_main.c
//
// Copyright 1998 Raven Software
//

#include "gl_debug.h" //mxd
#include "gl_local.h"
#include "gl_fmodel.h"
#include "q_Surface.h"
#include "turbsin.h"
#include "Vector.h"
#include "vid.h"

viddef_t viddef; // H2: renamed from vid, defined in vid.h?
refimport_t ri;

model_t* r_worldmodel;

float gldepthmin;
float gldepthmax;

glconfig_t gl_config;
glstate_t gl_state;

image_t* r_notexture; // Used for missing textures
image_t* r_particletexture;
image_t* r_aparticletexture;
image_t* r_reflecttexture;
image_t* r_font1;
image_t* r_font2;

entity_t* currententity;
model_t* currentmodel;

cplane_t frustum[4];

int r_framecount; // Used for dlight push checking

int c_brush_polys;
int c_alias_polys;

static float v_blend[4]; // Final blending color //mxd. Made static

// View origin
vec3_t vup;
vec3_t vpn;
vec3_t vright;
vec3_t r_origin;

float r_world_matrix[16];

refdef_t r_newrefdef; // Screen size info

int r_viewcluster;
int r_viewcluster2;
int r_oldviewcluster;
int r_oldviewcluster2;

cvar_t* r_norefresh;
cvar_t* r_fullbright;
cvar_t* r_drawentities;
cvar_t* r_drawworld;
cvar_t* r_novis;
cvar_t* r_nocull;
cvar_t* r_lerpmodels;
cvar_t* r_speeds;

cvar_t* r_lightlevel; // FIXME: This is a HACK to get the client's light level

cvar_t* r_farclipdist;
cvar_t* r_fog;
cvar_t* r_fog_mode;
cvar_t* r_fog_density;
cvar_t* r_fog_startdist;
cvar_t* r_fog_color_r;
cvar_t* r_fog_color_g;
cvar_t* r_fog_color_b;
cvar_t* r_fog_color_a;
cvar_t* r_fog_color_scale;
cvar_t* r_fog_lightmap_adjust;
cvar_t* r_fog_underwater;
cvar_t* r_fog_underwater_mode;
cvar_t* r_fog_underwater_density;
cvar_t* r_fog_underwater_startdist;
cvar_t* r_fog_underwater_color_r;
cvar_t* r_fog_underwater_color_g;
cvar_t* r_fog_underwater_color_b;
cvar_t* r_fog_underwater_color_a;
cvar_t* r_fog_underwater_color_scale;
cvar_t* r_fog_underwater_lightmap_adjust;
cvar_t* r_underwater_color;
cvar_t* r_frameswap;
cvar_t* r_references;

cvar_t* gl_nosubimage;
cvar_t* gl_allow_software; //TODO: ignored. Remove?

cvar_t* gl_particle_min_size;
cvar_t* gl_particle_max_size;
cvar_t* gl_particle_size;
cvar_t* gl_particle_att_a;
cvar_t* gl_particle_att_b;
cvar_t* gl_particle_att_c;
cvar_t* gl_noartifacts;

cvar_t* gl_modulate;
cvar_t* gl_log;
cvar_t* gl_bitdepth; //TODO: ignored (Win7+ can't into 8 and 16-bit color modes). Remove?
cvar_t* gl_lightmap;
cvar_t* gl_shadows;
cvar_t* gl_dynamic;
cvar_t* gl_nobind;
cvar_t* gl_round_down;
cvar_t* gl_showtris;
cvar_t* gl_reporthash;
cvar_t* gl_ztrick;
cvar_t* gl_finish;
cvar_t* gl_clear;
cvar_t* gl_cull;
cvar_t* gl_polyblend;
cvar_t* gl_flashblend;
cvar_t* gl_playermip;
cvar_t* gl_monolightmap;
cvar_t* gl_driver;
cvar_t* gl_texturemode;
cvar_t* gl_lockpvs;

cvar_t* gl_drawflat;
cvar_t* gl_devel1;
cvar_t* gl_trans33;
cvar_t* gl_trans66;
cvar_t* gl_picmip;
cvar_t* gl_skinmip;
cvar_t* gl_bookalpha;

cvar_t* gl_ext_swapinterval;
cvar_t* gl_ext_gamma;
cvar_t* gl_ext_palettedtexture;
cvar_t* gl_ext_multitexture;
cvar_t* gl_ext_pointparameters;
cvar_t* gl_drawmode;

cvar_t* gl_drawbuffer;
cvar_t* gl_swapinterval;
cvar_t* gl_sortmulti;

cvar_t* gl_saturatelighting;

cvar_t* gl_3dlabs_broken;
cvar_t* gl_lostfocus_broken;
cvar_t* gl_fog_broken;
cvar_t* gl_envmap_broken;
cvar_t* gl_screenshot_broken;

cvar_t* vid_fullscreen;
cvar_t* vid_gamma;
cvar_t* vid_brightness;
cvar_t* vid_contrast;

cvar_t* vid_ref;

cvar_t* vid_mode; // gl_mode in Q2
cvar_t* menus_active;
cvar_t* cl_camera_under_surface;
cvar_t* quake_amount;

qboolean R_CullBox(vec3_t mins, vec3_t maxs)
{
	if (!(int)r_nocull->value)
	{
		for (int i = 0; i < 4; i++)
			if (BoxOnPlaneSide(mins, maxs, &frustum[i]) == 2) // H2: BoxOnPlaneSide call instead of BOX_ON_PLANE_SIDE macro
				return true;
	}

	return false;
}

void R_RotateForEntity(const entity_t* e)
{
	qglTranslatef(e->origin[0], e->origin[1], e->origin[2]);

	// H2: new RAD_TO_ANGLE scaler
	qglRotatef( e->angles[1] * RAD_TO_ANGLE, 0.0f, 0.0f, 1.0f);
	qglRotatef(-e->angles[0] * RAD_TO_ANGLE, 0.0f, 1.0f, 0.0f);
	qglRotatef(-e->angles[2] * RAD_TO_ANGLE, 1.0f, 0.0f, 0.0f);
}

void HandleTrans(const entity_t* e)
{
	if (e->flags & RF_TRANS_ADD)
	{
		if (e->flags & RF_ALPHA_TEXTURE)
		{
			qglEnable(GL_ALPHA_TEST);
			qglAlphaFunc(GL_GREATER, 0.0f);
			qglBlendFunc(GL_SRC_ALPHA, GL_ONE);
			qglColor4ub(e->color.r, e->color.g, e->color.b, e->color.a);
		}
		else
		{
			if ((int)r_fog->value || (int)cl_camera_under_surface->value) //mxd. Skipped gl_fog_broken check
				qglDisable(GL_FOG);

			qglDisable(GL_ALPHA_TEST);
			qglBlendFunc(GL_ONE, GL_ONE);

			if (e->flags & RF_TRANS_ADD_ALPHA)
			{
				const float scaler = (float)e->color.a / 255.0f / 255.0f; //TODO: why is it divided twice?..
				qglColor3f((float)e->color.r * scaler, (float)e->color.g * scaler, (float)e->color.b * scaler); //mxd. qglColor4f -> qglColor3f
			}
			else
			{
				qglColor3ub(e->color.r, e->color.g, e->color.b); //mxd. qglColor4ub -> qglColor3ub
			}
		}
	}
	else
	{
		qglEnable(GL_ALPHA_TEST);
		qglAlphaFunc(GL_GREATER, 0.05f);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// H2_1.07: qglBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR) when RF_TRANS_GHOST flag is set.
		if (!(e->flags & RF_TRANS_GHOST))
			qglColor4ub(e->color.r, e->color.g, e->color.b, e->color.a);
	}

	qglEnable(GL_BLEND);
}

void CleanupTrans(const entity_t* e)
{
	qglDisable(GL_BLEND);

	if (e->flags & (RF_TRANS_GHOST | RF_TRANS_ADD))
	{
		if ((int)r_fog->value || (int)cl_camera_under_surface->value) //mxd. Removed gl_fog_broken cvar check
			qglEnable(GL_FOG);

		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR.
	}
	else
	{
		qglDisable(GL_ALPHA_TEST);
		qglAlphaFunc(GL_GREATER, 0.666f);
	}
}

// Q2 counterpart
void R_DrawNullModel(void)
{
	vec3_t shadelight;

	if (currententity->flags & RF_FULLBRIGHT)
		VectorSet(shadelight, 1.0f, 1.0f, 1.0f);
	else
		R_LightPoint(currententity->origin, shadelight);

	qglPushMatrix();
	R_RotateForEntity(currententity);

	qglDisable(GL_TEXTURE_2D);
	qglColor3fv(shadelight);

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3f(0.0f, 0.0f, -16.0f);
	for (int i = 0; i < 5; i++)
		qglVertex3f(16.0f * cosf((float)i * ANGLE_90), 16.0f * sinf((float)i * ANGLE_90), 0.0f); //mxd. M_PI/2 -> ANGLE_90
	qglEnd();

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3f(0.0f, 0.0f, 16.0f);
	for (int i = 4; i > -1; i--)
		qglVertex3f(16.0f * cosf((float)i * ANGLE_90), 16.0f * sinf((float)i * ANGLE_90), 0.0f); //mxd. M_PI/2 -> ANGLE_90
	qglEnd();

	qglColor3f(1.0f, 1.0f, 1.0f);
	qglPopMatrix();
	qglEnable(GL_TEXTURE_2D);
}

// H2: simplified: no separate non-transparent/transparent drawing chains
void R_DrawEntitiesOnList(void)
{
	if (!(int)r_drawentities->value)
		return;

	for (int i = 0; i < r_newrefdef.num_entities; i++)
	{
		currententity = r_newrefdef.entities[i];
		if (currententity->model == NULL) // H2: extra sanity check
		{
			Com_Printf("Attempt to draw NULL model\n");
			R_DrawNullModel();
			continue;
		}

		currentmodel = *currententity->model;
		if (currentmodel == NULL)
		{
			R_DrawNullModel();
			continue;
		}

		// H2: no mod_alias case, new mod_bad and mod_fmdl cases
		switch (currentmodel->type)
		{
			case mod_bad:
				Com_Printf("WARNING:  currentmodel->type == 0; reload the map\n");
				break;

			case mod_brush:
				R_DrawBrushModel(currententity);
				break;

			case mod_sprite:
				R_DrawSpriteModel(currententity);
				break;

			case mod_fmdl:
				R_DrawFlexModel(currententity);
				break;

			default:
				Sys_Error("Bad modeltype"); // Q2: ri.Sys_Error
				break;
		}
	}
}

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

static void GL_DrawParticles(const int num_particles, const particle_t* particles, const qboolean alpha_particle)
{
	int i;
	const particle_t* p;

	if (alpha_particle)
	{
		GL_BindImage(r_aparticletexture);
		qglBlendFunc(GL_ONE, GL_ONE);

		if ((int)r_fog->value || (int)cl_camera_under_surface->value) //mxd. Removed gl_fog_broken cvar check
			qglDisable(GL_FOG);

		qglDisable(GL_ALPHA_TEST);
	}
	else
	{
		GL_BindImage(r_particletexture);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	qglEnable(GL_BLEND);
	GL_TexEnv(GL_MODULATE);

	qglBegin(GL_QUADS);

	for (i = 0, p = particles; i < num_particles; i++, p++)
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

		const byte p_type = p->type & 127; // Strip particle flags

		qglColor4ubv(c.c_array);

		qglTexCoord2f(particle_st_coords[p_type][0], particle_st_coords[p_type][1]);
		qglVertex3f(p->origin[0] + p_up[0], p->origin[1] + p_up[1], p->origin[2] + p_up[2]);

		qglTexCoord2f(particle_st_coords[p_type][2], particle_st_coords[p_type][1]);
		qglVertex3f(p->origin[0] + p_right[0], p->origin[1] + p_right[1], p->origin[2] + p_right[2]);

		qglTexCoord2f(particle_st_coords[p_type][2], particle_st_coords[p_type][3]);
		qglVertex3f(p->origin[0] - p_up[0], p->origin[1] - p_up[1], p->origin[2] - p_up[2]);

		qglTexCoord2f(particle_st_coords[p_type][0], particle_st_coords[p_type][3]);
		qglVertex3f(p->origin[0] - p_right[0], p->origin[1] - p_right[1], p->origin[2] - p_right[2]);
	}

	qglEnd();

	if (alpha_particle)
	{
		if ((int)r_fog->value || (int)cl_camera_under_surface->value) //mxd. Removed gl_fog_broken cvar check
			qglEnable(GL_FOG);

		qglEnable(GL_ALPHA_TEST);
	}

	qglDisable(GL_BLEND);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	GL_TexEnv(GL_REPLACE);
}

// Q2 counterpart
void R_PolyBlend(void)
{
	if ((int)gl_polyblend->value && v_blend[3] != 0.0f)
	{
		qglDisable(GL_ALPHA_TEST);
		qglEnable(GL_BLEND);
		qglDisable(GL_DEPTH_TEST);
		qglDisable(GL_TEXTURE_2D);

		qglLoadIdentity();

		// FIXME: get rid of these
		qglRotatef(-90.0f, 1.0f, 0.0f, 0.0f);	// Put Z going up
		qglRotatef(90.0f, 0.0f, 0.0f, 1.0f);	// Put Z going up

		qglColor4fv(v_blend);

		qglBegin(GL_QUADS);

		qglVertex3f(10.0f,  100.0f,  100.0f);
		qglVertex3f(10.0f, -100.0f,  100.0f);
		qglVertex3f(10.0f, -100.0f, -100.0f);
		qglVertex3f(10.0f,  100.0f, -100.0f);

		qglEnd();

		qglDisable(GL_BLEND);
		qglEnable(GL_TEXTURE_2D);
		qglEnable(GL_ALPHA_TEST);

		qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

// Q2 counterpart
static byte SignbitsForPlane(const cplane_t* plane) //mxd. Changed return type to byte
{
	// For fast box on planeside test
	byte bits = 0;
	for (int i = 0; i < 3; i++)
		if (plane->normal[i] < 0.0f)
			bits |= 1 << i;

	return bits;
}

static void R_SetFrustum(void)
{
	RotatePointAroundVector(frustum[0].normal, vup,		vpn, -(90.0f - r_newrefdef.fov_x * 0.5f));	// Rotate VPN right by FOV_X/2 degrees
	RotatePointAroundVector(frustum[1].normal, vup,		vpn,   90.0f - r_newrefdef.fov_x * 0.5f);	// Rotate VPN left by FOV_X/2 degrees
	RotatePointAroundVector(frustum[2].normal, vright,	vpn,   90.0f - r_newrefdef.fov_y * 0.5f);	// Rotate VPN up by FOV_X/2 degrees
	RotatePointAroundVector(frustum[3].normal, vright,	vpn, -(90.0f - r_newrefdef.fov_y * 0.5f));	// Rotate VPN down by FOV_X/2 degrees

	for (int i = 0; i < 4; i++)
	{
		// H2:
		const float frustum_dist = VectorLength(frustum[i].normal);
		if (frustum_dist <= 0.999999f)
			Com_Printf("Frustum normal dist %f < 1.0\n", (double)frustum_dist);

		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct(r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane(&frustum[i]);
	}
}

// Q2 counterpart
void R_SetupFrame(void)
{
	r_framecount++;

	// Build the transformation matrix for the given view angles
	VectorCopy(r_newrefdef.vieworg, r_origin);
	AngleVectors(r_newrefdef.viewangles, vpn, vright, vup);

	// Current viewcluster
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;

		const mleaf_t* leaf = Mod_PointInLeaf(r_origin, r_worldmodel);
		r_viewcluster = leaf->cluster;
		r_viewcluster2 = r_viewcluster;

		// Check above and below so crossing solid water doesn't draw wrong
		vec3_t temp;
		VectorCopy(r_origin, temp);

		if (leaf->contents == 0)
			temp[2] -= 16.0f; // Look down a bit
		else
			temp[2] += 16.0f; // Look up a bit

		leaf = Mod_PointInLeaf(temp, r_worldmodel);
		if (!(leaf->contents & CONTENTS_SOLID) /*&& leaf->cluster != r_viewcluster2*/)
			r_viewcluster2 = leaf->cluster;
	}

	for (int i = 0; i < 4; i++)
		v_blend[i] = r_newrefdef.blend[i];

	c_brush_polys = 0;
	c_alias_polys = 0;

	// Clear out the portion of the screen that the NOWORLDMODEL defines
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
	{
		qglEnable(GL_SCISSOR_TEST);
		qglClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		qglScissor(r_newrefdef.x, viddef.height - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height);
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		qglClearColor(1.0f, 0.0f, 0.5f, 0.5f);
		qglDisable(GL_SCISSOR_TEST);
	}
}

// Q2 counterpart //mxd. Changed args types: GLdouble -> const float.
static void MYgluPerspective(const float fovy, const float aspect, const float zNear, const float zFar)
{
	//mxd. Original H2 logic sets additional global float FovY_rad to tan(fovy * M_PI / 360.0) here. Used only in unused GL_DrawBackPoly() function.
	const GLdouble ymax = (double)(zNear * tanf(fovy * (float)M_PI / 360.0f));
	const GLdouble ymin = -ymax;

	GLdouble xmin = ymin * (double)aspect;
	GLdouble xmax = ymax * (double)aspect;

	xmin += (double)(-(2 * gl_state.camera_separation) / zNear);
	xmax += (double)(-(2 * gl_state.camera_separation) / zNear);

	qglFrustum(xmin, xmax, ymin, ymax, (GLdouble)zNear, (GLdouble)zFar);
}

static void R_SetupGL(void)
{
	//mxd. Removed unneeded integer multiplications/divisions 
	const int xl = r_newrefdef.x;
	const int xr = r_newrefdef.x + r_newrefdef.width;
	const int yt = viddef.height - r_newrefdef.y;
	const int yb = viddef.height - (r_newrefdef.y + r_newrefdef.height);

	//mxd. Original logic:
	//const int xl = (int)floorf(r_newrefdef.x * viddef.width / viddef.width);
	//const int xr = (int)ceilf((r_newrefdef.x + r_newrefdef.width) * viddef.width / viddef.width);
	//const int yt = (int)floorf(viddef.height - r_newrefdef.y * viddef.height / viddef.height);
	//const int yb = (int)ceilf(viddef.height - (r_newrefdef.y + r_newrefdef.height) * viddef.height / viddef.height);

	qglViewport(xl, yb, xr - xl, yt - yb);

	// Set up projection matrix
	const float screenaspect = (float)r_newrefdef.width / (float)r_newrefdef.height;
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	MYgluPerspective(r_newrefdef.fov_y, screenaspect, 1.0f, r_farclipdist->value); // Q2: last 2 args are 4, 4096

	qglCullFace(GL_FRONT);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	qglRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Put Z going up
	qglRotatef( 90.0f, 0.0f, 0.0f, 1.0f); // Put Z going up
	qglRotatef(-r_newrefdef.viewangles[2], 1.0f, 0.0f, 0.0f);
	qglRotatef(-r_newrefdef.viewangles[0], 0.0f, 1.0f, 0.0f);
	qglRotatef(-r_newrefdef.viewangles[1], 0.0f, 0.0f, 1.0f);
	qglTranslatef(-r_newrefdef.vieworg[0], -r_newrefdef.vieworg[1], -r_newrefdef.vieworg[2]);

	qglGetFloatv(GL_MODELVIEW_MATRIX, r_world_matrix);

	// Set drawing parms
	qglToggle(GL_CULL_FACE, (int)gl_cull->value);
	qglDisable(GL_BLEND);
	qglDisable(GL_ALPHA_TEST);

	// H2: extra gl_drawmode logic
	if ((int)gl_drawmode->value)
	{
		qglClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		qglClear(GL_COLOR_BUFFER_BIT);
		qglClearColor(1.0f, 0.0f, 0.5f, 0.5f);
	}
	else
	{
		qglEnable(GL_DEPTH_TEST);
	}

	// H2 (never triggered: qboolean DoDrawBackPoly is never set)
	//if (DoDrawBackPoly)
		//GL_DrawBackPoly();
}

static void GL_Fog(void) // H2
{
	static GLint fog_modes[] = { GL_LINEAR, GL_EXP, GL_EXP2 };

	const int mode = ClampI((int)r_fog_mode->value, 0, sizeof(fog_modes) / sizeof(fog_modes[0])); //mxd. Added ClampI
	qglFogi(GL_FOG_MODE, fog_modes[mode]);

	if (mode == 0)
	{
		qglFogf(GL_FOG_START, r_fog_startdist->value);
		qglFogf(GL_FOG_END, r_farclipdist->value);
	}
	else
	{
		qglFogf(GL_FOG_DENSITY, r_fog_density->value);
	}

	const float color[] =
	{
		r_fog_color_r->value,
		r_fog_color_g->value,
		r_fog_color_b->value,
		r_fog_color_a->value
	};

	qglFogfv(GL_FOG_COLOR, color);
	qglEnable(GL_FOG);

	qglClearColor(color[0], color[1], color[2], color[3]);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

static void GL_WaterFog(void) // H2
{
	static GLint fog_modes[] = { GL_LINEAR, GL_EXP, GL_EXP2 };

	const int mode = ClampI((int)r_fog_underwater_mode->value, 0, sizeof(fog_modes) / sizeof(fog_modes[0]) - 1); //mxd. Added ClampI
	qglFogi(GL_FOG_MODE, fog_modes[mode]);

	if (mode == 0)
	{
		qglFogf(GL_FOG_START, r_fog_underwater_startdist->value);
		qglFogf(GL_FOG_END, r_farclipdist->value);
	}
	else
	{
		qglFogf(GL_FOG_DENSITY, r_fog_underwater_density->value);
	}

	const float color[] =
	{
		r_fog_underwater_color_r->value,
		r_fog_underwater_color_g->value,
		r_fog_underwater_color_b->value,
		r_fog_underwater_color_a->value
	};

	qglFogfv(GL_FOG_COLOR, color);
	qglEnable(GL_FOG);
	qglClearColor(color[0], color[1], color[2], color[3]);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

static void R_Clear(void)
{
	if ((int)gl_ztrick->value) //TODO: mxd. No fog rendering when gl_ztrick is enabled. Curious...
	{
		static int trickframe;

		if ((int)gl_clear->value)
			qglClear(GL_COLOR_BUFFER_BIT);

		trickframe++;
		if (trickframe & 1)
		{
			gldepthmin = 0.0f;
			gldepthmax = 0.49999f;
			qglDepthFunc(GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1.0f;
			gldepthmax = 0.5f;
			qglDepthFunc(GL_GEQUAL);
		}
	}
	else
	{
		// H2: extra fog rendering logic. //mxd. Removed gl_fog_broken cvar checks.
		if ((int)cl_camera_under_surface->value) //TODO: r_fog_underwater cvar check seems logical here, but isn't present in original dll.
		{
			GL_WaterFog();
		}
		//mxd. Removed 'r_fog_startdist->value < r_farclipdist->value' check, because it's relevant only for fog mode 0.
		// Also there's no r_fog_underwater_startdist check in GL_WaterFog case in original .dll.
		else if ((int)r_fog->value)
		{
			GL_Fog();
		}
		else
		{
			qglDisable(GL_FOG);

			if ((int)gl_clear->value)
				qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			else
				qglClear(GL_DEPTH_BUFFER_BIT);
		}

		gldepthmin = 0.0f;
		gldepthmax = 1.0f;
		qglDepthFunc(GL_LEQUAL);
	}

	qglDepthRange((double)gldepthmin, (double)gldepthmax);
}

void R_Register(void)
{
	r_norefresh = Cvar_Get("r_norefresh", "0", 0);
	r_fullbright = Cvar_Get("r_fullbright", "0", 0);
	r_drawentities = Cvar_Get("r_drawentities", "1", 0);
	r_drawworld = Cvar_Get("r_drawworld", "1", 0);
	r_novis = Cvar_Get("r_novis", "0", 0);
	r_nocull = Cvar_Get("r_nocull", "0", 0);
	r_lerpmodels = Cvar_Get("r_lerpmodels", "1", 0);
	r_speeds = Cvar_Get("r_speeds", "0", 0);

	r_lightlevel = Cvar_Get("r_lightlevel", "0", 0);

	// NEW:
	r_farclipdist = Cvar_Get("r_farclipdist", "4096.0", 0);
	r_fog = Cvar_Get("r_fog", "0", 0);
	r_fog_mode = Cvar_Get("r_fog_mode", "1", 0);
	r_fog_density = Cvar_Get("r_fog_density", "0.004", 0);
	r_fog_startdist = Cvar_Get("r_fog_startdist", "50.0", 0);
	r_fog_color_r = Cvar_Get("r_fog_color_r", "1.0", 0);
	r_fog_color_g = Cvar_Get("r_fog_color_g", "1.0", 0);
	r_fog_color_b = Cvar_Get("r_fog_color_b", "1.0", 0);
	r_fog_color_a = Cvar_Get("r_fog_color_a", "0.0", 0);
	r_fog_color_scale = Cvar_Get("r_fog_color_scale", "1.0", 0);
	r_fog_lightmap_adjust = Cvar_Get("r_fog_lightmap_adjust", "5.0", 0);
	r_fog_underwater = Cvar_Get("r_fog_underwater", "0", 0); //TODO: unused
	r_fog_underwater_mode = Cvar_Get("r_fog_underwater_mode", "1", 0);
	r_fog_underwater_density = Cvar_Get("r_fog_underwater_density", "0.0015", 0);
	r_fog_underwater_startdist = Cvar_Get("r_fog_underwater_startdist", "100.0", 0);
	r_fog_underwater_color_r = Cvar_Get("r_fog_underwater_color_r", "1.0", 0);
	r_fog_underwater_color_g = Cvar_Get("r_fog_underwater_color_g", "1.0", 0);
	r_fog_underwater_color_b = Cvar_Get("r_fog_underwater_color_b", "1.0", 0);
	r_fog_underwater_color_a = Cvar_Get("r_fog_underwater_color_a", "0.0", 0);
	r_fog_underwater_color_scale = Cvar_Get("r_fog_underwater_color_scale", "1.0", 0);
	r_fog_underwater_lightmap_adjust = Cvar_Get("r_fog_underwater_lightmap_adjust", "5.0", 0);
	r_underwater_color = Cvar_Get("r_underwater_color", "0x70c06000", 0);
	r_frameswap = Cvar_Get("r_frameswap", "1.0", 0);
	r_references = Cvar_Get("r_references", "1.0", 0);

	gl_nosubimage = Cvar_Get("gl_nosubimage", "0", 0);
	gl_allow_software = Cvar_Get("gl_allow_software", "0", 0);

	gl_particle_min_size = Cvar_Get("gl_particle_min_size", "2", CVAR_ARCHIVE);
	gl_particle_max_size = Cvar_Get("gl_particle_max_size", "40", CVAR_ARCHIVE);
	gl_particle_size = Cvar_Get("gl_particle_size", "40", CVAR_ARCHIVE);
	gl_particle_att_a = Cvar_Get("gl_particle_att_a", "0.01", CVAR_ARCHIVE);
	gl_particle_att_b = Cvar_Get("gl_particle_att_b", "0.0", CVAR_ARCHIVE);
	gl_particle_att_c = Cvar_Get("gl_particle_att_c", "0.01", CVAR_ARCHIVE);
	gl_noartifacts = Cvar_Get("gl_noartifacts", "0", 0); // NEW

	gl_modulate = Cvar_Get("gl_modulate", "1", CVAR_ARCHIVE);
	gl_log = Cvar_Get("gl_log", "0", 0);
	gl_bitdepth = Cvar_Get("gl_bitdepth", "0", 0);
	gl_lightmap = Cvar_Get("gl_lightmap", "0", 0);
	gl_shadows = Cvar_Get("gl_shadows", "0", CVAR_ARCHIVE);
	gl_dynamic = Cvar_Get("gl_dynamic", "1", 0);
	gl_nobind = Cvar_Get("gl_nobind", "0", 0);
	gl_round_down = Cvar_Get("gl_round_down", "1", 0);
	gl_showtris = Cvar_Get("gl_showtris", "0", 0);
	gl_reporthash = Cvar_Get("gl_reporthash", "0", 0);
	gl_ztrick = Cvar_Get("gl_ztrick", "0", 0);
	gl_finish = Cvar_Get("gl_finish", "0", 0);
	gl_clear = Cvar_Get("gl_clear", "0", 0);
	gl_cull = Cvar_Get("gl_cull", "1", 0);
	gl_polyblend = Cvar_Get("gl_polyblend", "1", 0);
	gl_flashblend = Cvar_Get("gl_flashblend", "0", 0);
	gl_playermip = Cvar_Get("gl_playermip", "0", 0);
	gl_monolightmap = Cvar_Get("gl_monolightmap", "0", 0); //TODO: unused. Remove?
	gl_driver = Cvar_Get("gl_driver", "opengl32", CVAR_ARCHIVE);
	gl_texturemode = Cvar_Get("gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE);
	// Missing: gl_texturealphamode, gl_texturesolidmode?
	gl_lockpvs = Cvar_Get("gl_lockpvs", "0", 0);

	// NEW:
	gl_drawflat = Cvar_Get("gl_drawflat", "0", 0);
	gl_devel1 = Cvar_Get("gl_devel1", "0", 0);
	gl_trans33 = Cvar_Get("gl_trans33", "0.33", 0); // H2_1.07: 0.33 -> 1
	gl_trans66 = Cvar_Get("gl_trans66", "0.66", 0); // H2_1.07: 0.66 -> 1
	gl_picmip = Cvar_Get("gl_picmip", "0", CVAR_ARCHIVE);
	gl_skinmip = Cvar_Get("gl_skinmip", "0", CVAR_ARCHIVE);
	gl_bookalpha = Cvar_Get("gl_bookalpha", "1.0", 0);

	gl_ext_swapinterval = Cvar_Get("gl_ext_swapinterval", "1", CVAR_ARCHIVE);
	gl_ext_gamma = Cvar_Get("gl_ext_gamma", "1", CVAR_ARCHIVE);
	gl_ext_palettedtexture = Cvar_Get("gl_ext_palettedtexture", "1", CVAR_ARCHIVE); //TODO: ignored. Remove?
	gl_ext_multitexture = Cvar_Get("gl_ext_multitexture", "1", CVAR_ARCHIVE);
	gl_ext_pointparameters = Cvar_Get("gl_ext_pointparameters", "1", CVAR_ARCHIVE);
	gl_drawmode = Cvar_Get("gl_drawmode", "0", 0);

	gl_drawbuffer = Cvar_Get("gl_drawbuffer", "GL_BACK", 0);
	gl_swapinterval = Cvar_Get("gl_swapinterval", "1", CVAR_ARCHIVE);
	gl_sortmulti = Cvar_Get("gl_sortmulti", "0", CVAR_ARCHIVE); // NEW

	gl_saturatelighting = Cvar_Get("gl_saturatelighting", "0", 0);

	// ri.Cvar_Get() in Q2
	gl_3dlabs_broken = ri.Cvar_FullSet("gl_3dlabs_broken", "1", 0);
	gl_lostfocus_broken = ri.Cvar_FullSet("gl_lostfocus_broken", "0", 0); // NEW
	gl_fog_broken = ri.Cvar_FullSet("gl_fog_broken", "0", 0); // NEW
	gl_envmap_broken = ri.Cvar_FullSet("gl_envmap_broken", "0", 0); // NEW
	gl_screenshot_broken = ri.Cvar_FullSet("gl_screenshot_broken", "0", 0); // NEW

	vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get("vid_gamma", "0.5", CVAR_ARCHIVE);
	vid_brightness = Cvar_Get("vid_brightness", "0.5", CVAR_ARCHIVE); // NEW
	vid_contrast = Cvar_Get("vid_contrast", "0.5", CVAR_ARCHIVE); // NEW

	vid_ref = Cvar_Get("vid_ref", "gl", CVAR_ARCHIVE);

	// NEW:
	vid_mode = Cvar_Get("vid_mode", "3", CVAR_ARCHIVE);
	menus_active = Cvar_Get("menus_active", "0", 0);
	cl_camera_under_surface = Cvar_Get("cl_camera_under_surface", "0", 0);
	quake_amount = Cvar_Get("quake_amount", "0", 0);

	ri.Cmd_AddCommand("imagelist", GL_ImageList_f);
	ri.Cmd_AddCommand("screenshot", GL_ScreenShot_f);
	ri.Cmd_AddCommand("modellist", Mod_Modellist_f);
	ri.Cmd_AddCommand("gl_strings", GL_Strings_f);

	InitGammaTable(); // NEW
}

qboolean R_SetMode(void)
{
	if (vid_fullscreen->modified && !gl_config.allow_cds)
	{
		Com_Printf("R_SetMode() - CDS not allowed with this driver\n");
		Cvar_SetValue("vid_fullscreen", (float)!(int)vid_fullscreen->value);
		vid_fullscreen->modified = false;
	}

	vid_fullscreen->modified = false;
	vid_mode->modified = false;

	rserr_t err = GLimp_SetMode(&viddef.width, &viddef.height, (int)vid_mode->value, (int)vid_fullscreen->value, true);
	if (err == rserr_ok)
	{
		gl_state.prev_mode = (int)vid_mode->value;
		return true;
	}

	if (err == rserr_invalid_fullscreen)
	{
		Cvar_SetValue("vid_fullscreen", 0);
		vid_fullscreen->modified = false;
		Com_Printf("ref_gl::R_SetMode() - fullscreen unavailable in this mode\n");

		err = GLimp_SetMode(&viddef.width, &viddef.height, (int)vid_mode->value, false, true);
		if (err == rserr_ok)
			return true;
	}
	else if (err == rserr_invalid_mode)
	{
		Cvar_SetValue("vid_mode", (float)gl_state.prev_mode);
		vid_mode->modified = false;
		Com_Printf("ref_gl::R_SetMode() - invalid mode\n");
	}

	// Try setting it back to something safe
	err = GLimp_SetMode(&viddef.width, &viddef.height, gl_state.prev_mode, false, true);
	if (err != rserr_ok)
	{
		Com_Printf("ref_gl::R_SetMode() - could not revert to safe mode\n");
		return false;
	}

	return true;
}

qboolean R_Init(void* hinstance, void* hWnd)
{
	for (int j = 0; j < 256; j++)
		turbsin[j] *= 0.5f;

	Com_Printf("ref_gl version: "REF_VERSION"\n"); // ri.Con_Printf in Q2 (here and below)
	R_Register();

	// Initialize our QGL dynamic bindings
	char driver_path[256];
	Com_sprintf(driver_path, 256, "Drivers/%s", gl_driver->string); // H2: extra local driver QGL_Init call 

	if (!QGL_Init(driver_path) && !QGL_Init(gl_driver->string))
	{
		QGL_Shutdown();
		Com_Printf("%s - could not load \"%s\"\n", __func__, gl_driver->string);
		return false; //mxd. Decompiled code still returns -1 here...
	}

	// Initialize OS - specific parts of OpenGL
	if (!GLimp_Init(hinstance, hWnd))
	{
		QGL_Shutdown();
		return false; //mxd. Decompiled code still returns -1 here...
	}

	// Set our "safe" modes
	gl_state.prev_mode = 3;

	// Create the window and set up the context
	if (!R_SetMode())
	{
		QGL_Shutdown();
		Com_Printf("ref_gl::R_Init() - could not R_SetMode()\n");
		return false; //mxd. Decompiled code still returns -1 here...
	}

	// Get our various GL strings
	gl_config.vendor_string = (const char*)(*qglGetString)(GL_VENDOR);
	Com_Printf("GL_VENDOR: %s\n", gl_config.vendor_string);
	gl_config.renderer_string = (const char*)(*qglGetString)(GL_RENDERER);
	Com_Printf("GL_RENDERER: %s\n", gl_config.renderer_string);
	gl_config.version_string = (const char*)(*qglGetString)(GL_VERSION);
	Com_Printf("GL_VERSION: %s\n", gl_config.version_string);
	gl_config.extensions_string = (const char*)(*qglGetString)(GL_EXTENSIONS);
	//Com_Printf("GL_EXTENSIONS: %s\n", gl_config.extensions_string); // H2_1.07: "GL_EXT: hidden\n" //mxd. Modern extensions_string is longer than Com_Printf can handle...

	//mxd. Skip copious amounts of ancient videocard checks, assume everything works...
	gl_config.renderer = GL_RENDERER_DEFAULT;
	gl_config.allow_cds = true;

	// Grab extensions
#ifdef _WIN32
	if (strstr(gl_config.extensions_string, "WGL_EXT_swap_control"))
	{
		qwglSwapIntervalEXT = (BOOL (WINAPI*)(int))qwglGetProcAddress("wglSwapIntervalEXT");
		Com_Printf("...enabling WGL_EXT_swap_control\n");
	}
	else
	{
		Com_Printf("...WGL_EXT_swap_control not found\n");
	}
#endif

	if (strstr(gl_config.extensions_string, "GL_EXT_point_parameters"))
	{
		if ((int)gl_ext_pointparameters->value)
		{
			qglPointParameterfEXT = (void (APIENTRY*)(GLenum, GLfloat))qwglGetProcAddress("glPointParameterfEXT");
			qglPointParameterfvEXT = (void (APIENTRY*)(GLenum, const GLfloat*))qwglGetProcAddress("glPointParameterfvEXT");
			Com_Printf("...using GL_EXT_point_parameters\n");
		}
		else
		{
			Com_Printf("...ignoring GL_EXT_point_parameters\n");
		}
	}
	else
	{
		Com_Printf("...GL_EXT_point_parameters not found\n");
	}

	//mxd. Skip qglColorTableEXT logic. Required 'GL_EXT_shared_texture_palette' extension is unsupported since GeForceFX
	//https://community.khronos.org/t/does-not-support-ext-paletted-texture-on-geforcefx

	if (strstr(gl_config.extensions_string, "GL_ARB_multitexture"))
	{
		if ((int)gl_ext_multitexture->value)
		{
			Com_Printf("...using GL_ARB_multitexture\n");
			qglMultiTexCoord2fARB = (void (APIENTRY*)(GLenum, GLfloat, GLfloat))qwglGetProcAddress("glMultiTexCoord2fARB");
			qglActiveTextureARB = (void (APIENTRY*)(GLenum))qwglGetProcAddress("glActiveTextureARB");
		}
		else
		{
			Com_Printf("...ignoring GL_ARB_multitexture\n");
		}
	}
	else
	{
		Com_Printf("...GL_ARB_multitexture not found\n");
	}

	//mxd. GL_SGIS_multitexture logic skipped
	//mxd. Glide version check logic skipped
	//mxd. "WE DO NOT SUPPORT THE POWERVR" logic skipped

	GL_SetDefaultState();
	//GL_DrawStereoPattern(); // Enabled in H2. Did H3D paid iD the money they owed them?..
	GL_InitImages();
	Mod_Init();
	Draw_InitLocal();

	const GLenum err = qglGetError();
	if (err != GL_NO_ERROR)
	{
		Com_Printf("glGetError() = 0x%x\n", err);
		return false;
	}

	return true; //mxd. Return value missing in Q2
}

void R_Shutdown(void)
{
	ShutdownFonts(); // H2

	ri.Cmd_RemoveCommand("modellist");
	ri.Cmd_RemoveCommand("screenshot");
	ri.Cmd_RemoveCommand("imagelist");
	ri.Cmd_RemoveCommand("gl_strings");

	Mod_FreeAll();
	GL_ShutdownImages();

	// Shutdown OS-specific OpenGL stuff like contexts, etc.
	GLimp_Shutdown();

	// Shutdown our QGL subsystem
	QGL_Shutdown();
}

void R_BeginFrame(const float camera_separation)
{
	gl_state.camera_separation = camera_separation;

	// Change modes if necessary
	if (vid_mode->modified || vid_fullscreen->modified)
	{
		// FIXME: only restart if CDS is required
		cvar_t* ref = Cvar_Get("vid_ref", "gl", 0); //TODO: can't we just use vid_ref global var here? 
		ref->modified = true;
	}

	if (gl_log->modified)
	{
		GLimp_EnableLogging((qboolean)gl_log->value);
		gl_log->modified = false;
	}

	if ((int)gl_log->value)
		GLimp_LogNewFrame();

	// Changed
	if (vid_gamma->modified || vid_brightness->modified || vid_contrast->modified)
	{
		InitGammaTable();
		GL_GammaAffect();

		vid_gamma->modified = false;
		vid_brightness->modified = false;
		vid_contrast->modified = false;
	}

	GLimp_BeginFrame(camera_separation);

	// Go into 2D mode
	qglViewport(0, 0, viddef.width, viddef.height);
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglOrtho(0.0, viddef.width, viddef.height, 0.0, -99999.0, 99999.0);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();
	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_BLEND);
	qglEnable(GL_ALPHA_TEST);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw buffer stuff
	if (gl_drawbuffer->modified)
	{
		gl_drawbuffer->modified = false;

		if (!gl_state.stereo_enabled || gl_state.camera_separation == 0.0f)
		{
			if (Q_stricmp(gl_drawbuffer->string, "GL_FRONT") == 0)
				qglDrawBuffer(GL_FRONT);
			else
				qglDrawBuffer(GL_BACK);
		}
	}

	// Texturemode stuff
	if (gl_texturemode->modified)
	{
		GL_TextureMode(gl_texturemode->string);
		gl_texturemode->modified = false;
	}

	// Missing: gl_texturealphamode and gl_texturesolidmode logic

	// Swapinterval stuff
	GL_UpdateSwapInterval();

	// Clear screen if desired
	R_Clear();
}

static void R_RenderView(const refdef_t* fd)
{
	if ((int)r_norefresh->value)
		return;

	r_newrefdef = *fd;

	if (r_worldmodel == NULL && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		Sys_Error("R_RenderView: NULL worldmodel"); // Q2: ri.Sys_Error(ERR_DROP, "R_RenderView: NULL worldmodel");

	if ((int)r_speeds->value)
	{
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	R_PushDlights();

	if ((int)gl_finish->value)
		qglFinish();

	R_SetupFrame();
	R_SetFrustum();
	R_SetupGL();
	R_MarkLeaves(); // Done here so we know if we're in water
	R_DrawWorld();
	R_DrawEntitiesOnList();
	R_RenderDlights();

	// Changed in H2:
	qglDepthMask(GL_FALSE);
	R_SortAndDrawAlphaSurfaces();
	GL_DrawParticles(r_newrefdef.num_particles, r_newrefdef.particles, false);
	GL_DrawParticles(r_newrefdef.anum_particles, r_newrefdef.aparticles, true);
	qglDepthMask(GL_TRUE);

	// Changed in H2: R_Flash() call replaced with R_PolyBlend() call (or optimization?)
	R_PolyBlend();

	if ((int)r_speeds->value)
		Com_Printf("%4i wpoly %4i epoly %i tex %i lmaps\n", c_brush_polys, c_alias_polys, c_visible_textures, c_visible_lightmaps); // H2: ri.Con_Printf -> Com_Printf

	if ((int)gl_reporthash->value) // H2
		GL_DisplayHashTable();
}

// Q2 counterpart
static void R_SetGL2D(void)
{
	// Set 2D virtual screen size
	qglViewport(0, 0, viddef.width, viddef.height);
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglOrtho(0.0, (double)viddef.width, (double)viddef.height, 0.0, -99999.0, 99999.0);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();
	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_BLEND);
	qglEnable(GL_ALPHA_TEST);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

static void R_SetLightLevel(void)
{
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
	{
		vec3_t shadelight;

		// Save off light value for server to look at (BIG HACK!)
		R_LightPoint(r_newrefdef.clientmodelorg, shadelight); // H2: vieworg -> clientmodelorg

		// Pick the greatest component, which should be the same as the mono value returned by software
		r_lightlevel->value = max(shadelight[0], max(shadelight[1], shadelight[2])) * 150.0f;
	}
}

static void GL_ScreenFlash(const paletteRGBA_t color)
{
	qglDepthMask(GL_FALSE);
	Draw_FadeScreen(color);
	qglDepthMask(GL_TRUE);

	ri.Deactivate_Screen_Flash();
}

// H2: return type: void -> int //TODO: useless: always returns 0 
static int R_RenderFrame(const refdef_t* fd)
{
	paletteRGBA_t color;

	color.c = ri.Is_Screen_Flashing();
	if ((int)cl_camera_under_surface->value)
		color.c = strtoul(r_underwater_color->string, NULL, 0);

	if (color.a != 255)
	{
		// Q2 version calls these 3 functions only
		R_RenderView(fd);
		R_SetLightLevel();
		R_SetGL2D();

		if (color.a == 0)
			return 0;
	}

	GL_ScreenFlash(color);

	return 0;
}

static int R_GetReferencedID(const struct model_s* model) //mxd. Named 'GetReferencedID' (in m_Reference.c) in original logic.
{
	const fmdl_t* temp = model->extradata;

	//mxd. H2 Toolkit code checks for model->model_type, decompiled code checks for model->skeletal_model...
	//TODO: check which one is correct!
	if (model->skeletal_model && temp->referenceType > REF_NULL && temp->referenceType < NUM_REFERENCED)
		return temp->referenceType;

	return REF_NULL;
}

// Referenced by GetRefAPI only:
void R_BeginRegistration(const char* model);
struct image_s* R_RegisterSkin(const char* name, qboolean* retval);
void R_SetSky(const char* name, float rotate, const vec3_t axis);
void R_EndRegistration(void);

void Draw_InitCinematic(int w, int h, char* overlay, char* backdrop);
void Draw_CloseCinematic(void);
void Draw_Cinematic(int cols, int rows, const byte* data, const paletteRGB_t* palette, float alpha);

void Draw_Name(const vec3_t origin, const char* name, paletteRGBA_t color);
int FindSurface(vec3_t start, vec3_t end, struct Surface_s* surface);

refexport_t GetRefAPI(const refimport_t rimp)
{
	refexport_t re;

	ri = rimp;

	re.api_version = API_VERSION;
	re.render = false; //mxd. Shut up compiler...

	re.BeginRegistration = R_BeginRegistration;
	re.RegisterModel = R_RegisterModel;
	re.RegisterSkin = R_RegisterSkin;
	re.RegisterPic = Draw_FindPic;
	re.SetSky = R_SetSky;
	re.EndRegistration = R_EndRegistration;
	re.GetReferencedID = R_GetReferencedID;

	re.RenderFrame = R_RenderFrame;

	re.DrawGetPicSize = Draw_GetPicSize;
	re.DrawPic = Draw_Pic;
	re.DrawStretchPic = Draw_StretchPic;
	re.DrawChar = Draw_Char;
	re.DrawTileClear = Draw_TileClear;
	re.DrawFill = Draw_Fill;
	re.DrawFadeScreen = Draw_FadeScreen;
	// Missing: Draw_StretchRaw

	re.DrawBigFont = Draw_BigFont;
	re.BF_Strlen = BF_Strlen;
	re.BookDrawPic = Draw_BookPic;
	re.DrawInitCinematic = Draw_InitCinematic;
	re.DrawCloseCinematic = Draw_CloseCinematic;
	re.DrawCinematic = Draw_Cinematic;
	re.Draw_Name = Draw_Name;

	re.Init = R_Init;
	re.Shutdown = R_Shutdown;

	// Missing: R_SetPalette
	re.BeginFrame = R_BeginFrame;
	re.EndFrame = GLimp_EndFrame;
	re.AppActivate = GLimp_AppActivate;
	re.FindSurface = FindSurface;

#ifdef _DEBUG
	//mxd. Debug draw logic.
	re.AddDebugBox = R_AddDebugBox;
	re.AddDebugBbox = R_AddDebugBbox;
	re.AddDebugEntityBbox = R_AddDebugEntityBbox;

	re.AddDebugLine = R_AddDebugLine;
	re.AddDebugArrow = R_AddDebugArrow;
#endif

	// Missing: Swap_Init();
	// Unbound: A3D_RenderGeometry();

	return re;
}
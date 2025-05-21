//
// gl_surf.c -- surface-related refresh code
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "gl_fmodel.h"
#include "Vector.h"

static vec3_t modelorg; // Relative to viewpoint
static msurface_t* r_alpha_surfaces;

#define LIGHTMAP_BYTES			4

#define BLOCK_WIDTH				128
#define BLOCK_HEIGHT			128

#define MAX_LIGHTMAPS			128
#define MAX_TALLWALL_LIGHTMAPS	512 // H2

int c_visible_lightmaps;
int c_visible_textures;

static int r_visframecount; // Bumped when going to a new PVS // Q2: defined in gl_rmain.c //mxd. Moved here & made static
static int num_sorted_multitextures; // H2

#define GL_LIGHTMAP_FORMAT		GL_RGBA

typedef struct
{
	int internal_format;
	int current_lightmap_texture;

	msurface_t* lightmap_surfaces[MAX_LIGHTMAPS];
	msurface_t* tallwall_lightmap_surfaces[MAX_TALLWALL_LIGHTMAPS]; // H2
	int tallwall_lightmaptexturenum; // H2

	int allocated[BLOCK_WIDTH];

	// The lightmap texture data needs to be kept in main memory so texsubimage can update properly
	byte lightmap_buffer[BLOCK_WIDTH * BLOCK_HEIGHT * 4];
} gllightmapstate_t;

static gllightmapstate_t gl_lms;

#pragma region ========================== BRUSH MODELS ==========================

// Returns the proper texture for a given time and base texture
static image_t* R_TextureAnimation(const mtexinfo_t* tex)
{
	if (tex->next != NULL)
	{
		int frame;
		if (tex->flags & SURF_ANIMSPEED && tex->image->num_frames > 0) // H2: extra SURF_ANIMSPEED logic
			frame = (int)((float)tex->image->num_frames * r_newrefdef.time);
		else
			frame = currententity->frame;

		frame %= tex->numframes;

		while (frame--)
			tex = tex->next;
	}

	return tex->image;
}

// Q2 counterpart
static void DrawGLPoly(const glpoly_t* p)
{
	int i;
	const float* v;
	
	qglBegin(GL_POLYGON);

	for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
	{
		qglTexCoord2f(v[3], v[4]);
		qglVertex3fv(v);
	}

	qglEnd();
}

// Q2 counterpart
static void DrawGLPolyChain(glpoly_t* p, const float soffset, const float toffset)
{
	int i;
	float* v;

	//mxd. Removed optimized case when soffset and toffset are 0.
	for (; p != NULL; p = p->chain)
	{
		qglBegin(GL_POLYGON);

		for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
		{
			qglTexCoord2f(v[5] - soffset, v[6] - toffset);
			qglVertex3fv(v);
		}

		qglEnd();
	}
}

static void LM_InitBlock(void);
static void LM_UploadBlock(qboolean dynamic);
static qboolean LM_AllocBlock(int w, int h, int* x, int* y);

// This routine takes all the given light mapped surfaces in the world and blends them into the framebuffer.
static void R_BlendLightmaps(void)
{
	int i;
	int j;
	msurface_t* surf;
	float* v;

	// Don't bother if we're set to fullbright
	if ((int)r_fullbright->value || r_worldmodel->lightdata == NULL)
		return;

	// Don't bother writing Z
	qglDepthMask(GL_FALSE);

	// Set the appropriate blending mode unless we're only looking at the lightmaps.
	if (!(int)gl_lightmap->value)
	{
		qglEnable(GL_BLEND);

		if ((int)gl_saturatelighting->value)
			qglBlendFunc(GL_ONE, GL_ONE);
		else
			qglBlendFunc(GL_ZERO, GL_SRC_COLOR); //mxd. Skipping gl_monolightmap logic
	}

	if (currentmodel == r_worldmodel)
		c_visible_lightmaps = 0;

	// H2: set fog values
	const qboolean render_fog = Q_ftol(r_fog->value);
	const int fog_mode = Q_ftol(r_fog_mode->value);

	if (render_fog) //mxd. Removed gl_fog_broken check
	{
		if (fog_mode == 0)
		{
			qglFogf(GL_FOG_START, r_fog_startdist->value * r_fog_lightmap_adjust->value);
			qglFogf(GL_FOG_END, r_farclipdist->value * r_fog_lightmap_adjust->value);
		}
		else
		{
			qglFogf(GL_FOG_DENSITY, r_fog_lightmap_adjust->value * r_fog_density->value);
		}
	}

	qglDisable(GL_TEXTURE_2D);

	// H2: draw tallwalls
	for (i = 0, surf = gl_lms.tallwall_lightmap_surfaces[0]; i < gl_lms.tallwall_lightmaptexturenum; i++, surf++)
	{
		qglColor4ub(surf->styles[0], surf->styles[1], surf->styles[2], surf->styles[3]);
		qglBegin(GL_POLYGON);

		for (j = 0, v = surf->polys->verts[0]; j < surf->polys->numverts; j++, v += VERTEXSIZE)
			qglVertex3fv(v);

		qglEnd();
	}

	qglEnable(GL_TEXTURE_2D);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Render static lightmaps first
	for (i = 1; i < MAX_LIGHTMAPS; i++)
	{
		if (gl_lms.lightmap_surfaces[i])
		{
			if (currentmodel == r_worldmodel)
				c_visible_lightmaps++;

			GL_Bind(gl_state.lightmap_textures + i);

			for (surf = gl_lms.lightmap_surfaces[i]; surf != NULL; surf = surf->lightmapchain)
				if (surf->polys != NULL)
					DrawGLPolyChain(surf->polys, 0.0f, 0.0f);
		}
	}

	// Render dynamic lightmaps
	if ((int)gl_dynamic->value)
	{
		LM_InitBlock();
		GL_Bind(gl_state.lightmap_textures);

		if (currentmodel == r_worldmodel)
			c_visible_lightmaps++;

		const msurface_t* newdrawsurf = gl_lms.lightmap_surfaces[0];

		for (surf = gl_lms.lightmap_surfaces[0]; surf != NULL; surf = surf->lightmapchain)
		{
			const int smax = (surf->extents[0] >> 4) + 1;
			const int tmax = (surf->extents[1] >> 4) + 1;

			if (!LM_AllocBlock(smax, tmax, &surf->dlight_s, &surf->dlight_t))
			{
				// Upload what we have so far
				LM_UploadBlock(true);

				// Draw all surfaces that use this lightmap
				for (; newdrawsurf != surf; newdrawsurf = newdrawsurf->lightmapchain)
				{
					if (newdrawsurf->polys != NULL)
					{
						DrawGLPolyChain(newdrawsurf->polys,
							(float)(newdrawsurf->light_s - newdrawsurf->dlight_s) * (1.0f / 128.0f),
							(float)(newdrawsurf->light_t - newdrawsurf->dlight_t) * (1.0f / 128.0f));
					}
				}

				// Clear the block
				LM_InitBlock();

				// Try uploading the block now
				if (!LM_AllocBlock(smax, tmax, &surf->dlight_s, &surf->dlight_t))
					ri.Sys_Error(ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed (dynamic)\n", smax, tmax);
			}

			byte* base = gl_lms.lightmap_buffer;
			base += (surf->dlight_t * BLOCK_WIDTH + surf->dlight_s) * LIGHTMAP_BYTES;

			R_BuildLightMap(surf, base, BLOCK_WIDTH * LIGHTMAP_BYTES);
		}

		// Draw remainder of dynamic lightmaps that haven't been uploaded yet
		if (newdrawsurf != NULL)
			LM_UploadBlock(true);

		for (; newdrawsurf != NULL; newdrawsurf = newdrawsurf->lightmapchain)
		{
			if (newdrawsurf->polys != NULL)
			{
				DrawGLPolyChain(newdrawsurf->polys, 
					(float)(newdrawsurf->light_s - newdrawsurf->dlight_s) * (1.0f / 128.0f),
					(float)(newdrawsurf->light_t - newdrawsurf->dlight_t) * (1.0f / 128.0f));
			}
		}
	}

	// H2: new fog logic
	if (render_fog) //mxd. Removed gl_fog_broken check
	{
		if (fog_mode == 0)
		{
			qglFogf(GL_FOG_START, r_fog_startdist->value);
			qglFogf(GL_FOG_END, r_farclipdist->value);
		}
		else
		{
			qglFogf(GL_FOG_DENSITY, r_fog_density->value);
		}
	}

	// Restore state
	qglDisable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR.
	qglDepthMask(GL_TRUE);
}

// Q2 counterpart
static void R_DrawTriangleOutlines(void)
{
	if (!(int)gl_showtris->value)
		return;

	qglDisable(GL_TEXTURE_2D);
	qglDisable(GL_DEPTH_TEST);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	for (int i = 0; i < MAX_LIGHTMAPS; i++)
	{
		for (const msurface_t* surf = gl_lms.lightmap_surfaces[i]; surf != NULL; surf = surf->lightmapchain)
		{
			for (const glpoly_t* p = surf->polys; p != NULL; p = p->chain)
			{
				for (int j = 2; j < p->numverts; j++)
				{
					qglBegin(GL_LINE_STRIP);
					qglVertex3fv(p->verts[0]);
					qglVertex3fv(p->verts[j - 1]);
					qglVertex3fv(p->verts[j]);
					qglVertex3fv(p->verts[0]);
					qglEnd();
				}
			}
		}
	}

	qglEnable(GL_DEPTH_TEST);
	qglEnable(GL_TEXTURE_2D);
}

//mxd. Seems to be called only when r_fullbright == 1
static void R_RenderBrushPoly(msurface_t* fa)
{
	c_brush_polys++;

	GL_BindImage(R_TextureAnimation(fa->texinfo)); // Q2: GL_Bind

	// H2: new cl_camera_under_surface logic
	if ((int)cl_camera_under_surface->value) 
	{
		EmitUnderWaterPolys(fa);
		GL_TexEnv(GL_REPLACE);

		return;
	}

	// H2: new quake_amount logic
	if ((int)quake_amount->value) 
	{
		EmitQuakeFloorPolys(fa);
		GL_TexEnv(GL_REPLACE);

		return;
	}

	if (fa->flags & SURF_DRAWTURB)
	{
		// Warp texture, no lightmaps
		GL_TexEnv(GL_MODULATE);
		qglColor4f(gl_state.inverse_intensity, gl_state.inverse_intensity, gl_state.inverse_intensity, 1.0f);
		EmitWaterPolys(fa, fa->flags & SURF_UNDULATE);
		GL_TexEnv(GL_REPLACE);

		return;
	}

	GL_TexEnv(GL_REPLACE);

	// H2: missing SURF_FLOWING flag logic
	DrawGLPoly(fa->polys);

	int map;
	qboolean is_dynamic = false;

	// Check for lightmap modification
	for (map = 0; map < MAXLIGHTMAPS && fa->styles[map] != 255; map++)
	{
		if (r_newrefdef.lightstyles[fa->styles[map]].white != fa->cached_light[map])
		{
			is_dynamic = true; //mxd. Avoid unnecessary gotos.
			break;
		}
	}

	// Dynamic this frame or dynamic previously
	if (fa->dlightframe == r_framecount || is_dynamic)
	{
		if ((int)gl_dynamic->value && !(fa->texinfo->flags & SURF_FULLBRIGHT)) //mxd. SURF_FULLBRIGHT define
		{
			if ((fa->styles[map] >= 32 || fa->styles[map] == 0) && fa->dlightframe != r_framecount)
			{
				uint temp[34 * 34];
				const int smax = (fa->extents[0] >> 4) + 1;
				const int tmax = (fa->extents[1] >> 4) + 1;

				R_BuildLightMap(fa, (byte*)temp, smax * 4);
				R_SetCacheState(fa);
				GL_Bind(gl_state.lightmap_textures + fa->lightmaptexturenum);

				qglTexSubImage2D(GL_TEXTURE_2D, 0, fa->light_s, fa->light_t, smax, tmax, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, temp);

				fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
				gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
			}
			else
			{
				fa->lightmapchain = gl_lms.lightmap_surfaces[0];
				gl_lms.lightmap_surfaces[0] = fa;
			}

			return;
		}
	}

	// H2: new tall wall logic:
	if (!(fa->texinfo->flags & SURF_TALL_WALL))
	{
		fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
		gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
	}
	else if (gl_lms.tallwall_lightmaptexturenum < 512)
	{
		gl_lms.tallwall_lightmap_surfaces[gl_lms.tallwall_lightmaptexturenum] = fa;
		gl_lms.tallwall_lightmaptexturenum++;
	}
	else
	{
		Com_Printf("WARNING: To many tall wall surfaces!");
	}
}

static void R_RenderFlatShadedBrushPoly(msurface_t* fa) // H2
{
	c_brush_polys++;

	// Use fa->polys pointer as random, but constant color...
	paletteRGBA_t color;
	color.c = (uint)fa->polys;
	qglColor3ubv(color.c_array); //mxd. qglColor3f -> qglColor3ubv

	qglBegin(GL_POLYGON);

	float* v = fa->polys->verts[0];
	for (int i = 0; i < fa->polys->numverts; i++, v += VERTEXSIZE)
		qglVertex3fv(v);

	qglEnd();

	// Done when gl_drawflat == 1
	if ((int)gl_drawflat->value == 1)
		return;

	// Draw lightmaps (gl_drawflat >= 2). Same logic as in R_RenderBrushPoly().
	int map;
	qboolean is_dynamic = false;

	// Check for lightmap modification
	for (map = 0; map < MAXLIGHTMAPS && fa->styles[map] != 255; map++)
	{
		if (r_newrefdef.lightstyles[fa->styles[map]].white != fa->cached_light[map])
		{
			is_dynamic = true; //mxd. Avoid unnecessary gotos.
			break;
		}
	}

	// Dynamic this frame or dynamic previously
	if (fa->dlightframe == r_framecount || is_dynamic)
	{
		if ((int)gl_dynamic->value && !(fa->texinfo->flags & SURF_FULLBRIGHT)) //mxd. SURF_FULLBRIGHT define
		{
			if ((fa->styles[map] >= 32 || fa->styles[map] == 0) && fa->dlightframe != r_framecount)
			{
				uint temp[34 * 34];
				const int smax = (fa->extents[0] >> 4) + 1;
				const int tmax = (fa->extents[1] >> 4) + 1;

				R_BuildLightMap(fa, (byte*)temp, smax * 4);
				R_SetCacheState(fa);
				GL_Bind(gl_state.lightmap_textures + fa->lightmaptexturenum);

				qglTexSubImage2D(GL_TEXTURE_2D, 0, fa->light_s, fa->light_t, smax, tmax, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, temp);

				fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
				gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
			}
			else
			{
				fa->lightmapchain = gl_lms.lightmap_surfaces[0];
				gl_lms.lightmap_surfaces[0] = fa;
			}

			return;
		}
	}

	if (!(fa->texinfo->flags & SURF_TALL_WALL))
	{
		fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
		gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
	}
}

#pragma region ========================== ALPHA SURFACES RENDERING ==========================

//mxd. Reconstructed data type. Original name unknown
typedef struct
{
	union
	{
		entity_t* entity;
		msurface_t* surface;
	};
	float depth;
} AlphaSurfaceSortInfo_t;

static int AlphaSurfComp(const AlphaSurfaceSortInfo_t* info1, const AlphaSurfaceSortInfo_t* info2) // H2
{
	return (int)((info2->depth - info1->depth) * 1000.0f);
}

//TODO: logic identical to for loop logic in R_DrawEntitiesOnList(). Move to gl_rmain as R_DrawEntity and replace said logic?
static void R_DrawAlphaEntity(entity_t* ent) // H2
{
	currententity = ent;

	if (!(int)r_drawentities->value)
		return;

	if (ent->model == NULL)
	{
		Com_DPrintf("Attempt to draw NULL alpha model\n");
		R_DrawNullModel();

		return;
	}

	if (*ent->model == NULL)
	{
		R_DrawNullModel();
		return;
	}

	currentmodel = *ent->model;

	switch (currentmodel->type)
	{
		case mod_bad:
			Com_Printf("WARNING:  currentmodel->type == 0; reload the map\n");
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
			Sys_Error("Bad modeltype");
			break;
	}
}

static void R_DrawAlphaSurface(const msurface_t* surf) // H2
{
	qglEnable(GL_ALPHA_TEST);
	qglAlphaFunc(GL_GREATER, 0.05f);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR
	qglLoadMatrixf(r_world_matrix);
	qglEnable(GL_BLEND);
	GL_TexEnv(GL_MODULATE);

	GL_BindImage(surf->texinfo->image);
	c_brush_polys += 1;
	currentmodel = r_worldmodel;

	float alpha;
	if (surf->texinfo->flags & SURF_TRANS33)
		alpha = gl_trans33->value;
	else if (surf->texinfo->flags & SURF_TRANS66)
		alpha = gl_trans66->value;
	else
		alpha = 1.0f;

	qglColor4f(gl_state.inverse_intensity, gl_state.inverse_intensity, gl_state.inverse_intensity, alpha);

	if (surf->flags & SURF_DRAWTURB)
		EmitWaterPolys(surf, surf->flags & SURF_UNDULATE);
	else
		DrawGLPoly(surf->polys);

	GL_TexEnv(GL_REPLACE);
	qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	qglDisable(GL_BLEND);
	qglDisable(GL_ALPHA_TEST);
	qglAlphaFunc(GL_GREATER, 0.666f);
}

void R_SortAndDrawAlphaSurfaces(void)
{
	#define MAX_ALPHA_SURFACES 512 //TODO: is max number of alpha surfaces actually defined somewhere?

	AlphaSurfaceSortInfo_t sorted_ents[MAX_ALPHA_ENTITIES + 1]; //mxd. Extra slot for terminator (depth -100000) entry.
	AlphaSurfaceSortInfo_t sorted_surfs[MAX_ALPHA_SURFACES + 1]; //mxd. Extra slot for terminator (depth -100000) entry.

	// Add alpha entities to array...
	AlphaSurfaceSortInfo_t* info = &sorted_ents[0];
	for (int i = 0; i < r_newrefdef.num_alpha_entities; i++, info++)
	{
		entity_t* ent = r_newrefdef.alpha_entities[i];

		info->entity = ent;
		info->depth = ent->depth;
	}

	VectorScale(r_origin, -1.0f, modelorg);

	// Initialize last entity entry...
	info = &sorted_ents[r_newrefdef.num_alpha_entities];
	info->entity = NULL;
	info->depth = -100000.0f;

	currentmodel = r_worldmodel;

	// Add alpha surfaces to array.
	int num_surfaces;
	msurface_t* surf = r_alpha_surfaces;
	info = &sorted_surfs[0];
	for (num_surfaces = 0; surf != NULL; num_surfaces++, surf = surf->texturechain, info++)
	{
		info->surface = surf;
		info->depth = -100000.0f;

		for (int i = 0; i < surf->numedges; i++)
		{
			const int lindex = r_worldmodel->surfedges[surf->firstedge + i];
			float* vec;

			if (lindex > 0)
			{
				const medge_t* edge = &r_worldmodel->edges[lindex];
				vec = r_worldmodel->vertexes[edge->v[0]].position;
			}
			else
			{
				const medge_t* edge = &r_worldmodel->edges[-lindex];
				vec = r_worldmodel->vertexes[edge->v[1]].position;
			}

			vec3_t diff;
			VectorSubtract(vec, r_origin, diff);

			vec3_t screen_pos;
			TransformVector(diff, screen_pos);

			info->depth = max(info->depth, screen_pos[2]);
		}

		if (num_surfaces >= MAX_ALPHA_SURFACES)
		{
			Com_DPrintf("Warning: attempting to draw too many alpha surfaces.\n");
			break;
		}
	}

	// Initialize last surface entry...
	info = &sorted_surfs[num_surfaces];
	info->surface = NULL;
	info->depth = -100000.0f;

	// Sort surfaces...
	qsort(sorted_surfs, num_surfaces, sizeof(AlphaSurfaceSortInfo_t), (int (*)(const void*, const void*))AlphaSurfComp);

	const int num_elements = r_newrefdef.num_alpha_entities + num_surfaces;
	const AlphaSurfaceSortInfo_t* sorted_ent = &sorted_ents[0];
	const AlphaSurfaceSortInfo_t* sorted_surf = &sorted_surfs[0];

	// Draw them all.
	for (int i = 0; i < num_elements; i++)
	{
		if (sorted_surf->depth > sorted_ent->depth)
		{
			currentmodel = r_worldmodel;
			R_DrawAlphaSurface(sorted_surf->surface);
			sorted_surf++;
		}
		else
		{
			R_DrawAlphaEntity(sorted_ent->entity);
			sorted_ent++;
		}
	}

	r_alpha_surfaces = NULL;
}

#pragma endregion

//mxd. Similar to Q2's GL_RenderLightmappedPoly (except for missing SURF_FLOWING logic). Original H2 .dll also includes GL_RenderLightmappedPoly_SGIS variant.
static void GL_RenderLightmappedPoly_ARB(msurface_t* surf)
{
	int map;
	int lmtex = surf->lightmaptexturenum;
	qboolean lightmap_updated = false;

	for (map = 0; map < MAXLIGHTMAPS && surf->styles[map] != 255; map++)
	{
		if (r_newrefdef.lightstyles[surf->styles[map]].white != surf->cached_light[map])
		{
			lightmap_updated = true; //mxd. Avoid unnecessary gotos
			break;
		}
	}

	// Dynamic this frame or dynamic previously
	qboolean is_dynamic = false;
	if (lightmap_updated || surf->dlightframe == r_framecount)
		is_dynamic = ((int)gl_dynamic->value && !(surf->texinfo->flags & SURF_FULLBRIGHT)); //mxd. SURF_FULLBRIGHT define

	if (is_dynamic)
	{
		uint temp[BLOCK_WIDTH * BLOCK_HEIGHT];

		const int smax = (surf->extents[0] >> 4) + 1;
		const int tmax = (surf->extents[1] >> 4) + 1;

		if ((surf->styles[map] >= 32 || surf->styles[map] == 0) && surf->dlightframe != r_framecount)
		{
			R_BuildLightMap(surf, (byte*)temp, smax * 4);
			R_SetCacheState(surf);
			GL_MBind(GL_TEXTURE1, surf->lightmaptexturenum + gl_state.lightmap_textures);
			lmtex = surf->lightmaptexturenum;
		}
		else
		{
			R_BuildLightMap(surf, (byte*)temp, smax * 4);
			GL_MBind(GL_TEXTURE1, gl_state.lightmap_textures);
			lmtex = 0;
		}

		qglTexSubImage2D(GL_TEXTURE_2D, 0, surf->light_s, surf->light_t, smax, tmax, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, temp);
	}

	c_brush_polys++;

	GL_MBindImage(GL_TEXTURE0, R_TextureAnimation(surf->texinfo)); // H2: GL_MBind -> GL_MBindImage
	GL_MBind(GL_TEXTURE1, gl_state.lightmap_textures + lmtex);

	// Missing: SURF_FLOWING logic
	for (glpoly_t* p = surf->polys; p != NULL; p = p->chain)
	{
		float* v = p->verts[0];
		qglBegin(GL_POLYGON);
		for (int i = 0; i < surf->polys->numverts; i++, v += VERTEXSIZE)
		{
			qglMultiTexCoord2fARB(GL_TEXTURE0, v[3], v[4]);
			qglMultiTexCoord2fARB(GL_TEXTURE1, v[5], v[6]);
			qglVertex3fv(v);
		}
		qglEnd();
	}
}

static void DrawTextureChains(void)
{
	int i;
	image_t* image;

	c_visible_textures = 0;

	// H2: extra gl_sortmulti logic:
	if ((int)gl_sortmulti->value && num_sorted_multitextures > 0)
	{
		GL_EnableMultitexture(true);
		GL_SelectTexture(GL_TEXTURE0);
		GL_TexEnv(GL_REPLACE);
		GL_SelectTexture(GL_TEXTURE1);
		GL_TexEnv((int)gl_lightmap->value ? GL_REPLACE : GL_MODULATE);

		for (i = 0, image = gltextures; i < numgltextures; i++, image++)
		{
			if (image->registration_sequence == 0 || image->multitexturechain == NULL)
				continue;

			c_visible_textures++;

			for (msurface_t* s = image->multitexturechain; s != NULL; s = s->texturechain)
				GL_RenderLightmappedPoly_ARB(s);

			image->multitexturechain = NULL;
		}

		GL_EnableMultitexture(false);
		num_sorted_multitextures = 0;
	}

	void (*render_brush_poly)(msurface_t*) = ((int)gl_drawflat->value ? R_RenderFlatShadedBrushPoly : R_RenderBrushPoly); // H2: new gl_drawflat logic

	// Original Q2 logic:
	if (qglActiveTextureARB == NULL)
	{
		for (i = 0, image = gltextures; i < numgltextures; i++, image++)
		{
			if (!image->registration_sequence || image->texturechain == NULL)
				continue;

			c_visible_textures++;

			for (msurface_t* s = image->texturechain; s != NULL; s = s->texturechain)
				render_brush_poly(s); // H2: new gl_drawflat logic

			image->texturechain = NULL;
		}
	}
	else
	{
		for (i = 0, image = gltextures; i < numgltextures; i++, image++)
		{
			if (!image->registration_sequence || image->texturechain == NULL)
				continue;

			c_visible_textures++;

			for (msurface_t* s = image->texturechain; s != NULL; s = s->texturechain)
				if (!(s->flags & SURF_DRAWTURB))
					render_brush_poly(s); // H2: new gl_drawflat logic
		}

		GL_EnableMultitexture(false);

		for (i = 0, image = gltextures; i < numgltextures; i++, image++)
		{
			if (!image->registration_sequence || image->texturechain == NULL)
				continue;

			for (msurface_t* s = image->texturechain; s != NULL; s = s->texturechain)
				if (s->flags & SURF_DRAWTURB)
					render_brush_poly(s); // H2: new gl_drawflat logic

			image->texturechain = NULL;
		}
	}

	GL_TexEnv(GL_REPLACE);
}

static void R_DrawInlineBModel(void)
{
	#define BACKFACE_EPSILON 0.01f // Q2: defined in gl_local.h

	// Calculate dynamic lighting for bmodel
	if (!(int)gl_flashblend->value)
	{
		dlight_t* lt = r_newrefdef.dlights;
		for (int k = 0; k < r_newrefdef.num_dlights; k++, lt++)
			R_MarkLights(lt, 1 << k, currentmodel->nodes + currentmodel->firstnode);
	}

	msurface_t* psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

	// H2: extra RF_TRANS_ADD and RF_TRANS_GHOST flags
	if (currententity->flags & RF_TRANS_ANY)
	{
		qglEnable(GL_BLEND);
		qglColor4f(1.0f, 1.0f, 1.0f, 0.25f);
		GL_TexEnv(GL_MODULATE);
	}

	// Draw texture
	for (int i = 0; i < currentmodel->nummodelsurfaces; i++, psurf++)
	{
		// Find which side of the node we are on
		const cplane_t* pplane = psurf->plane;
		const float dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

		// Draw the polygon
		if ( ((psurf->flags & SURF_PLANEBACK) && dot < -BACKFACE_EPSILON) ||
			(!(psurf->flags & SURF_PLANEBACK) && dot > BACKFACE_EPSILON))
		{
			if (psurf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66))
			{
				// Add to the translucent chain
				psurf->texturechain = r_alpha_surfaces;
				r_alpha_surfaces = psurf;
			}
			else if (qglMultiTexCoord2fARB != NULL && !(psurf->flags & SURF_DRAWTURB) && !(int)r_fullbright->value && !(int)gl_drawflat->value) // H2: extra r_fullbright and gl_drawflat checks
			{
				GL_RenderLightmappedPoly_ARB(psurf); // Q2: GL_RenderLightmappedPoly
			}
			else //mxd. Skipped qglMTexCoord2fSGIS check
			{
				GL_EnableMultitexture(false);
				R_RenderBrushPoly(psurf);
				GL_EnableMultitexture(true);
			}
		}
	}

	// H2: extra RF_TRANS_ADD and RF_TRANS_GHOST flags
	if (!(currententity->flags & RF_TRANS_ANY))
	{
		if (qglMultiTexCoord2fARB == NULL) //mxd. Removed qglMTexCoord2fSGIS check
			R_BlendLightmaps();
	}
	else
	{
		qglDisable(GL_BLEND);
		qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		GL_TexEnv(GL_REPLACE);
	}
}

void R_DrawBrushModel(entity_t* e)
{
	vec3_t mins;
	vec3_t maxs;
	qboolean rotated;

	if (currentmodel->nummodelsurfaces == 0)
		return;

	// H2: missing: currententity = e;
	gl_state.currenttextures[0] = -1;
	gl_state.currenttextures[1] = -1;

	if (e->angles[0] != 0.0f || e->angles[1] != 0.0f || e->angles[2] != 0.0f)
	{
		for (int i = 0; i < 3; i++)
		{
			mins[i] = e->origin[i] - currentmodel->radius;
			maxs[i] = e->origin[i] + currentmodel->radius;
		}

		rotated = true;
	}
	else
	{
		VectorAdd(e->origin, currentmodel->mins, mins);
		VectorAdd(e->origin, currentmodel->maxs, maxs);

		rotated = false;
	}

	if (R_CullBox(mins, maxs))
		return;

	// H2: new gl_drawmode logic
	if ((int)gl_drawmode->value)
	{
		qglColor4f(1.0f, 1.0f, 1.0f, 0.4f);
		qglEnable(GL_BLEND);
		qglDisable(GL_DEPTH_TEST);
	}
	else
	{
		qglColor3f(1.0f, 1.0f, 1.0f);
	}

	memset(gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
	VectorSubtract(r_newrefdef.vieworg, e->origin, modelorg);

	if (rotated)
	{
		vec3_t temp;
		vec3_t angles;
		vec3_t forward;
		vec3_t right;
		vec3_t up;

		VectorScale(e->angles, RAD_TO_ANGLE, angles); // H2: new RAD_TO_ANGLE rescale
		VectorCopy(modelorg, temp);
		AngleVectors(angles, forward, right, up);

		modelorg[0] = DotProduct(temp, forward);
		modelorg[1] = -DotProduct(temp, right);
		modelorg[2] = DotProduct(temp, up);
	}

	qglPushMatrix();
	e->angles[0] *= -1.0f; // stupid quake bug
	e->angles[2] *= -1.0f; // stupid quake bug
	R_RotateForEntity(e);
	e->angles[0] *= -1.0f; // stupid quake bug
	e->angles[2] *= -1.0f; // stupid quake bug

	GL_EnableMultitexture(true);
	GL_SelectTexture(GL_TEXTURE0);
	GL_TexEnv(GL_REPLACE);
	GL_SelectTexture(GL_TEXTURE1);
	GL_TexEnv(GL_MODULATE);

	R_DrawInlineBModel();
	GL_EnableMultitexture(false);

	// H2: new gl_drawmode logic
	if ((int)gl_drawmode->value)
	{
		qglDisable(GL_BLEND);
		qglEnable(GL_DEPTH_TEST);
	}

	qglPopMatrix();
}

#pragma endregion

#pragma region ========================== WORLD MODEL ==========================

static void R_RecursiveWorldNode(mnode_t* node)
{
	msurface_t* surf;
	int c;
	int side;
	int sidebit;
	
	if (node->contents == CONTENTS_SOLID || node->visframe != r_visframecount || R_CullBox(node->minmaxs, node->minmaxs + 3))
		return;

	// If a leaf node, draw stuff
	if (node->contents != -1)
	{
		const mleaf_t* pleaf = (mleaf_t*)node;

		// Check for door connected areas
		if (r_newrefdef.areabits && !(r_newrefdef.areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
			return; // Not visible

		msurface_t** mark = pleaf->firstmarksurface;
		for (int i = pleaf->nummarksurfaces; i > 0; i--)
		{
			(*mark)->visframe = r_framecount;
			mark++;
		}

		return;
	}

	// Node is just a decision point, so go down the appropriate sides.

	// Find which side of the node we are on.
	const cplane_t* plane = node->plane;
	float dot;

	switch (plane->type)
	{
		case PLANE_X:
		case PLANE_Y:
		case PLANE_Z:
			dot = modelorg[plane->type] - plane->dist;
			break;

		default:
			dot = DotProduct(modelorg, plane->normal) - plane->dist;
			break;
	}
	
	if (dot >= 0.0f)
	{
		side = 0;
		sidebit = 0;
	}
	else
	{
		side = 1;
		sidebit = SURF_PLANEBACK;
	}

	// Recurse down the children, front side first
	R_RecursiveWorldNode(node->children[side]);

	// Draw stuff
	for (c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c > 0; c--, surf++)
	{
		if (surf->visframe != r_framecount || (surf->flags & SURF_PLANEBACK) != sidebit)
			continue; // Wrong frame or side

		if (surf->texinfo->flags & SURF_SKY)
		{
			// Just adds to visible sky bounds
			R_AddSkySurface(surf);
		}
		else if (surf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66))
		{
			// Add to the translucent chain
			surf->texturechain = r_alpha_surfaces;
			r_alpha_surfaces = surf;
		}
		else if (qglMultiTexCoord2fARB != NULL 
				&& !(surf->flags & SURF_DRAWTURB) && !(surf->flags & SURF_TALL_WALL)
				&& !(int)r_fullbright->value && !(int)gl_drawflat->value) // H2: extra SURF_TALL_WALL, r_fullbright, gl_drawflat checks.
		{
			// H2: extra gl_sortmulti cvar logic
			if ((int)gl_sortmulti->value)
			{
				// The polygon is visible, so add it to the multi-texture sorted chain
				image_t* image = R_TextureAnimation(surf->texinfo);
				surf->texturechain = image->multitexturechain;
				image->multitexturechain = surf;
				num_sorted_multitextures += 1;
			}
			else
			{
				GL_RenderLightmappedPoly_ARB(surf);
			}
		}
		else //mxd. Skipping qglMTexCoord2fSGIS logic...
		{
			// The polygon is visible, so add it to the texture sorted chain
			// FIXME: this is a hack for animation
			image_t* image = R_TextureAnimation(surf->texinfo);
			surf->texturechain = image->texturechain;
			image->texturechain = surf;
		}
	}

	// Recurse down the back side
	R_RecursiveWorldNode(node->children[!side]);
}

void R_DrawWorld(void)
{
	if (!(int)r_drawworld->value || r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	currentmodel = r_worldmodel;
	VectorCopy(r_newrefdef.vieworg, modelorg);

	// Auto cycle the world frame for texture animation
	entity_t ent = { 0 }; //mxd. memset -> zero initialization.
	ent.frame = (int)(r_newrefdef.time * 2);
	currententity = &ent;

	gl_state.currenttextures[0] = -1;
	gl_state.currenttextures[1] = -1;

	if ((int)gl_drawmode->value) // H2: new gl_drawmode logic
	{
		qglColor4f(1.0f, 1.0f, 1.0f, 0.4f);
		qglEnable(GL_BLEND);
	}
	else
	{
		qglColor3f(1.0f, 1.0f, 1.0f);
	}

	memset(gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
	gl_lms.tallwall_lightmaptexturenum = 0; // H2
	R_ClearSkyBox();
	num_sorted_multitextures = 0; // H2

	// H2: new r_fullbright and gl_drawflat cvar checks
	if (!(int)r_fullbright->value && !(int)gl_drawflat->value && qglMultiTexCoord2fARB != NULL)
	{
		GL_EnableMultitexture(true);

		GL_SelectTexture(GL_TEXTURE0);
		GL_TexEnv(GL_REPLACE);
		GL_SelectTexture(GL_TEXTURE1);

		if ((int)gl_lightmap->value)
			GL_TexEnv(GL_REPLACE);
		else
			GL_TexEnv(GL_MODULATE);

		R_RecursiveWorldNode(r_worldmodel->nodes);

		GL_EnableMultitexture(false);
	}
	else
	{
		R_RecursiveWorldNode(r_worldmodel->nodes);
	}

	// Theoretically nothing should happen in the next two functions if multitexture is enabled

	// H2: new gl_drawflat cvar logic
	if ((int)gl_drawflat->value)
	{
		qglDisable(GL_TEXTURE_2D);
		DrawTextureChains();
		qglEnable(GL_TEXTURE_2D);
		qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		DrawTextureChains();
	}

	R_BlendLightmaps();

	// H2: new gl_drawmode cvar logic
	if ((int)gl_drawmode->value)
	{
		qglDisable(GL_BLEND);
		qglEnable(GL_DEPTH_TEST);
	}

	R_DrawSkyBox();
	R_DrawTriangleOutlines();
}

// Q2 counterpart
// Mark the leaves and nodes that are in the PVS for the current cluster
void R_MarkLeaves(void)
{
	int i;
	mleaf_t* leaf;
	byte fatvis[MAX_MAP_LEAFS / 8];

	if (r_oldviewcluster == r_viewcluster && r_oldviewcluster2 == r_viewcluster2 && !(int)r_novis->value && r_viewcluster != -1)
		return;

	// Development aid to let you run around and see exactly where the pvs ends
	if ((int)gl_lockpvs->value)
		return;

	r_visframecount++;
	r_oldviewcluster = r_viewcluster;
	r_oldviewcluster2 = r_viewcluster2;

	if ((int)r_novis->value || r_viewcluster == -1 || r_worldmodel->vis == NULL)
	{
		// Mark everything
		for (i = 0; i < r_worldmodel->numleafs; i++)
			r_worldmodel->leafs[i].visframe = r_visframecount;

		for (i = 0; i < r_worldmodel->numnodes; i++)
			r_worldmodel->nodes[i].visframe = r_visframecount;

		return;
	}

	byte* vis = Mod_ClusterPVS(r_viewcluster, r_worldmodel);

	// May have to combine two clusters because of solid water boundaries
	if (r_viewcluster2 != r_viewcluster)
	{
		memcpy(fatvis, vis, (r_worldmodel->numleafs + 7) / 8);
		vis = Mod_ClusterPVS(r_viewcluster2, r_worldmodel);

		const int c = (r_worldmodel->numleafs + 31) / 32;
		for (i = 0; i < c; i++)
			((int*)fatvis)[i] |= ((int*)vis)[i];

		vis = fatvis;
	}

	for (i = 0, leaf = r_worldmodel->leafs; i < r_worldmodel->numleafs; i++, leaf++)
	{
		const int cluster = leaf->cluster;
		if (cluster == -1)
			continue;

		if (vis[cluster >> 3] & 1 << (cluster & 7))
		{
			mnode_t* node = (mnode_t*)leaf;
			do
			{
				if (node->visframe == r_visframecount)
					break;

				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
}

#pragma endregion

#pragma region ========================== LIGHTMAP ALLOCATION ==========================

// Q2 counterpart
static void LM_InitBlock(void)
{
	memset(gl_lms.allocated, 0, sizeof(gl_lms.allocated));
}

// Q2 counterpart
static void LM_UploadBlock(const qboolean dynamic)
{
	GL_Bind(gl_state.lightmap_textures + (dynamic ? 0 : gl_lms.current_lightmap_texture));

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri

	if (dynamic)
	{
		int height = 0;
		for (int i = 0; i < BLOCK_WIDTH; i++)
			height = max(gl_lms.allocated[i], height);

		qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BLOCK_WIDTH, height, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, gl_lms.lightmap_buffer);
	}
	else
	{
		qglTexImage2D(GL_TEXTURE_2D, 0, gl_lms.internal_format, BLOCK_WIDTH, BLOCK_HEIGHT, 0, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, gl_lms.lightmap_buffer);
		gl_lms.current_lightmap_texture++;

		if (gl_lms.current_lightmap_texture == MAX_LIGHTMAPS)
			ri.Sys_Error(ERR_DROP, "LM_UploadBlock() - MAX_LIGHTMAPS exceeded\n");
	}
}

// Q2 counterpart. Returns a texture number and the position inside it.
static qboolean LM_AllocBlock(const int w, const int h, int* x, int* y)
{
	int j;
	int best = BLOCK_HEIGHT;

	for (int i = 0; i < BLOCK_WIDTH - w; i++)
	{
		int best2 = 0;

		for (j = 0; j < w; j++)
		{
			if (gl_lms.allocated[i + j] >= best)
				break;

			if (gl_lms.allocated[i + j] > best2)
				best2 = gl_lms.allocated[i + j];
		}

		if (j == w)
		{
			// This is a valid spot
			*x = i;
			*y = best2;
			best = best2;
		}
	}

	if (best + h > BLOCK_HEIGHT)
		return false;

	for (int i = 0; i < w; i++)
		gl_lms.allocated[*x + i] = best + h;

	return true;
}

#pragma endregion

#pragma region ========================== LIGHTMAP BUILDING ==========================

// Q2 counterpart
void GL_BuildPolygonFromSurface(msurface_t* fa)
{
	// Reconstruct the polygon
	const medge_t* pedges = currentmodel->edges;
	const int lnumverts = fa->numedges;

	// Draw texture
	glpoly_t* poly = Hunk_Alloc((int)sizeof(glpoly_t) + (lnumverts - 4) * VERTEXSIZE * sizeof(float));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for (int i = 0; i < lnumverts; i++)
	{
		const int lindex = currentmodel->surfedges[fa->firstedge + i];

		float* vec;
		if (lindex > 0)
		{
			const medge_t* r_pedge = &pedges[lindex];
			vec = currentmodel->vertexes[r_pedge->v[0]].position;
		}
		else
		{
			const medge_t* r_pedge = &pedges[-lindex];
			vec = currentmodel->vertexes[r_pedge->v[1]].position;
		}

		float s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= (float)fa->texinfo->image->width;

		float t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= (float)fa->texinfo->image->height;

		//VectorAdd(total, vec, total);
		VectorCopy(vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		// Lightmap texture coordinates
		s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s -= (float)fa->texturemins[0];
		s += (float)fa->light_s * 16;
		s += 8;
		s /= BLOCK_WIDTH * 16;

		t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= (float)fa->texturemins[1];
		t += (float)fa->light_t * 16;
		t += 8;
		t /= BLOCK_HEIGHT * 16;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;
	}
}

// Q2 counterpart
void GL_CreateSurfaceLightmap(msurface_t* surf)
{
	if (surf->flags & (SURF_DRAWSKY | SURF_DRAWTURB))
		return;

	const int smax = ((int)surf->extents[0] >> 4) + 1;
	const int tmax = ((int)surf->extents[1] >> 4) + 1;

	if (!LM_AllocBlock(smax, tmax, &surf->light_s, &surf->light_t))
	{
		LM_UploadBlock(false);
		LM_InitBlock();

		if (!LM_AllocBlock(smax, tmax, &surf->light_s, &surf->light_t))
			ri.Sys_Error(ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed\n", smax, tmax);
	}

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

	byte* base = gl_lms.lightmap_buffer;
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

	R_SetCacheState(surf);
	R_BuildLightMap(surf, base, BLOCK_WIDTH * LIGHTMAP_BYTES);
}

// Q2 counterpart. //TODO: parameter 'm' is never used
void GL_BeginBuildingLightmaps(model_t* m)
{
	static lightstyle_t	lightstyles[MAX_LIGHTSTYLES];
	static uint dummy[BLOCK_WIDTH * BLOCK_HEIGHT]; //mxd. Made it static.

	memset(gl_lms.allocated, 0, sizeof(gl_lms.allocated));

	r_framecount = 1; // No dlightcache.

	GL_EnableMultitexture(true);
	GL_SelectTexture(GL_TEXTURE1);

	// Setup the base lightstyles so the lightmaps won't have to be regenerated the first time they're seen.
	for (int i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		lightstyles[i].rgb[0] = 1.0f;
		lightstyles[i].rgb[1] = 1.0f;
		lightstyles[i].rgb[2] = 1.0f;
		lightstyles[i].white = 3.0f;
	}

	r_newrefdef.lightstyles = lightstyles;
	gl_state.lightmap_textures = TEXNUM_LIGHTMAPS; //mxd. Set only here, never changed.
	gl_lms.current_lightmap_texture = 1;

	//mxd. Skip gl_monolightmap logic.
	gl_lms.internal_format = GL_TEX_SOLID_FORMAT;

	// Initialize the dynamic lightmap texture.
	GL_Bind(gl_state.lightmap_textures);

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri
	qglTexImage2D(GL_TEXTURE_2D, 0, gl_lms.internal_format, BLOCK_WIDTH, BLOCK_HEIGHT, 0, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, dummy);
}

// Q2 counterpart
void GL_EndBuildingLightmaps(void)
{
	LM_UploadBlock(false);
	GL_EnableMultitexture(false);
}

#pragma endregion
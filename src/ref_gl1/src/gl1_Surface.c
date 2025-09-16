//
// gl1_Surface.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Surface.h"
#include "gl1_FlexModel.h"
#include "gl1_Image.h"
#include "gl1_Light.h"
#include "gl1_Lightmap.h"
#include "gl1_Misc.h"
#include "gl1_Sky.h"
#include "gl1_Sprite.h"
#include "gl1_Warp.h"
#include "Vector.h"

//mxd. Reconstructed data type. Original name unknown.
typedef struct
{
	union
	{
		entity_t* entity;
		msurface_t* surface;
	};
	float depth;
} AlphaSurfaceSortInfo_t;

int c_visible_lightmaps;
int c_visible_textures;

static int r_visframecount; // Bumped when going to a new PVS // Q2: defined in gl_rmain.c //mxd. Moved here & made static.
static qboolean multitexture_mode; // H2

static vec3_t modelorg; // Relative to viewpoint.

static msurface_t* r_alpha_surfaces;

#pragma region ========================== ALPHA SURFACES RENDERING ==========================

static int AlphaSurfComp(const AlphaSurfaceSortInfo_t* info1, const AlphaSurfaceSortInfo_t* info2) // H2
{
	return (int)((info2->depth - info1->depth) * 1000.0f);
}

//TODO: logic identical to for loop logic in R_DrawEntitiesOnList(). Move to gl_rmain as R_DrawEntity and replace said logic?
static void R_DrawAlphaEntity(entity_t* ent) // H2
{
	if (!(int)r_drawentities->value)
		return;

	if (ent->model == NULL)
	{
		ri.Con_Printf(PRINT_DEVELOPER, "Attempt to draw NULL alpha model\n"); //mxd. Com_DPrintf() -> ri.Con_Printf().
		R_DrawNullModel(ent);

		return;
	}

	const model_t* mdl = *ent->model; //mxd. Original logic uses 'currentmodel' global var.

	if (mdl == NULL)
	{
		R_DrawNullModel(ent);
		return;
	}

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
			ri.Sys_Error(ERR_DROP, "Bad modeltype"); //mxd. Sys_Error() -> ri.Sys_Error().
			break;
	}
}

static void R_DrawAlphaSurface(const msurface_t* fa) // H2
{
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR
	glLoadMatrixf(r_world_matrix);
	glEnable(GL_BLEND);
	R_TexEnv(GL_MODULATE);

	R_BindImage(fa->texinfo->image);
	c_brush_polys += 1;

	float alpha;
	if (fa->texinfo->flags & SURF_TRANS33)
		alpha = gl_trans33->value;
	else if (fa->texinfo->flags & SURF_TRANS66)
		alpha = gl_trans66->value;
	else
		alpha = 1.0f;

	glColor4f(gl_state.inverse_intensity, gl_state.inverse_intensity, gl_state.inverse_intensity, alpha);

	if (fa->flags & SURF_DRAWTURB)
		R_EmitWaterPolys(fa, fa->flags & SURF_UNDULATE);
	else
		R_DrawGLPoly(fa->polys);

	R_TexEnv(GL_REPLACE);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666f);
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
			R_TransformVector(diff, screen_pos);

			info->depth = max(info->depth, screen_pos[2]);
		}

		if (num_surfaces >= MAX_ALPHA_SURFACES)
		{
			ri.Con_Printf(PRINT_DEVELOPER, "Warning: attempting to draw too many alpha surfaces\n"); //mxd. Com_DPrintf() -> ri.Con_Printf().
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

#pragma region ========================== BRUSH MODELS RENDERING ==========================

// Returns the proper texture for a given time and base texture.
static image_t* R_TextureAnimation(const entity_t* ent, const mtexinfo_t* tex) //mxd. Original logic uses 'currententity' global var.
{
	if (tex->next != NULL)
	{
		int frame;

		if ((tex->flags & SURF_ANIMSPEED) && tex->image->num_frames > 0) // H2: extra SURF_ANIMSPEED logic.
			frame = (int)((float)tex->image->num_frames * r_newrefdef.time);
		else if (ent != NULL) //mxd. Added sanity check.
			frame = ent->frame;
		else
			return tex->image;

		frame %= tex->numframes;

		while (frame-- > 0 && tex->next != NULL) //mxd. Added tex->next sanity check.
			tex = tex->next;
	}

	return tex->image;
}

// Q2 counterpart
static void R_DrawGLPoly(const glpoly_t* p)
{
	glBegin(GL_POLYGON);

	const float* v = p->verts[0];
	for (int i = 0; i < p->numverts; i++, v += VERTEXSIZE)
	{
		glTexCoord2f(v[3], v[4]);
		glVertex3fv(v);
	}

	glEnd();
}

// Q2 counterpart
static void R_DrawGLPolyChain(glpoly_t* p, const float soffset, const float toffset)
{
	//mxd. Removed optimized case when soffset and toffset are 0.
	for (; p != NULL; p = p->chain)
	{
		glBegin(GL_POLYGON);

		float* v = p->verts[0];
		for (int i = 0; i < p->numverts; i++, v += VERTEXSIZE)
		{
			glTexCoord2f(v[5] - soffset, v[6] - toffset);
			glVertex3fv(v);
		}

		glEnd();
	}
}

// This routine takes all the given lightmapped surfaces in the world and blends them into the framebuffer.
static void R_BlendLightmaps(model_t* mdl) //mxd. Original logic uses 'currentmodel' global var.
{
	// Don't bother if we're set to fullbright.
	if ((int)r_fullbright->value || r_worldmodel->lightdata == NULL)
		return;

	// Don't bother writing Z.
	glDepthMask(GL_FALSE);

	// Set the appropriate blending mode unless we're only looking at the lightmaps.
	if (!(int)gl_lightmap->value)
	{
		glEnable(GL_BLEND);

		if ((int)gl_saturatelighting->value)
			glBlendFunc(GL_ONE, GL_ONE);
		else
			glBlendFunc(GL_ZERO, GL_SRC_COLOR); //mxd. Skipping gl_monolightmap logic
	}

	if (mdl == r_worldmodel)
		c_visible_lightmaps = 0;

	// H2: set fog values.
	const qboolean render_fog = Q_ftol(r_fog->value);
	const int fog_mode = Q_ftol(r_fog_mode->value);

	if (render_fog) //mxd. Removed gl_fog_broken check.
	{
		if (fog_mode == 0)
		{
			glFogf(GL_FOG_START, r_fog_startdist->value * r_fog_lightmap_adjust->value);
			glFogf(GL_FOG_END, r_farclipdist->value * r_fog_lightmap_adjust->value);
		}
		else
		{
			glFogf(GL_FOG_DENSITY, r_fog_lightmap_adjust->value * r_fog_density->value);
		}
	}

	glDisable(GL_TEXTURE_2D);

	// H2: draw tallwalls.
	msurface_t* surf = gl_lms.tallwall_lightmap_surfaces[0];
	for (int i = 0; i < gl_lms.tallwall_lightmaptexturenum; i++, surf++)
	{
		glColor4ub(surf->styles[0], surf->styles[1], surf->styles[2], surf->styles[3]);
		glBegin(GL_POLYGON);

		float* v = surf->polys->verts[0];
		for (int j = 0; j < surf->polys->numverts; j++, v += VERTEXSIZE)
			glVertex3fv(v);

		glEnd();
	}

	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Render static lightmaps first.
	for (int i = 1; i < MAX_LIGHTMAPS; i++)
	{
		if (gl_lms.lightmap_surfaces[i] == NULL)
			continue;

		if (mdl == r_worldmodel)
			c_visible_lightmaps++;

		R_Bind(gl_state.lightmap_textures + i);

		for (surf = gl_lms.lightmap_surfaces[i]; surf != NULL; surf = surf->lightmapchain)
			if (surf->polys != NULL)
				R_DrawGLPolyChain(surf->polys, 0.0f, 0.0f);
	}

	// Render dynamic lightmaps.
	if ((int)gl_dynamic->value)
	{
		LM_InitBlock();
		R_Bind(gl_state.lightmap_textures);

		if (mdl == r_worldmodel)
			c_visible_lightmaps++;

		const msurface_t* newdrawsurf = gl_lms.lightmap_surfaces[0];

		for (surf = gl_lms.lightmap_surfaces[0]; surf != NULL; surf = surf->lightmapchain)
		{
			const int smax = (surf->extents[0] >> 4) + 1;
			const int tmax = (surf->extents[1] >> 4) + 1;

			if (!LM_AllocBlock(smax, tmax, &surf->dlight_s, &surf->dlight_t))
			{
				// Upload what we have so far.
				LM_UploadBlock(true);

				// Draw all surfaces that use this lightmap.
				for (; newdrawsurf != surf; newdrawsurf = newdrawsurf->lightmapchain)
				{
					if (newdrawsurf->polys == NULL)
						continue;

					R_DrawGLPolyChain(newdrawsurf->polys,
						(float)(newdrawsurf->light_s - newdrawsurf->dlight_s) * (1.0f / 128.0f),
						(float)(newdrawsurf->light_t - newdrawsurf->dlight_t) * (1.0f / 128.0f));
				}

				// Clear the block.
				LM_InitBlock();

				// Try uploading the block now.
				if (!LM_AllocBlock(smax, tmax, &surf->dlight_s, &surf->dlight_t))
					ri.Sys_Error(ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed (dynamic)\n", smax, tmax);
			}

			byte* base = gl_lms.lightmap_buffer;
			base += (surf->dlight_t * LM_BLOCK_WIDTH + surf->dlight_s) * LIGHTMAP_BYTES;

			R_BuildLightMap(surf, base, LM_BLOCK_WIDTH * LIGHTMAP_BYTES);
		}

		// Draw remainder of dynamic lightmaps that haven't been uploaded yet.
		if (newdrawsurf != NULL)
			LM_UploadBlock(true);

		for (; newdrawsurf != NULL; newdrawsurf = newdrawsurf->lightmapchain)
		{
			if (newdrawsurf->polys == NULL)
				continue;

			R_DrawGLPolyChain(newdrawsurf->polys,
				(float)(newdrawsurf->light_s - newdrawsurf->dlight_s) * (1.0f / 128.0f),
				(float)(newdrawsurf->light_t - newdrawsurf->dlight_t) * (1.0f / 128.0f));
		}
	}

	// H2: new fog logic.
	if (render_fog) //mxd. Removed gl_fog_broken check.
	{
		if (fog_mode == 0)
		{
			glFogf(GL_FOG_START, r_fog_startdist->value);
			glFogf(GL_FOG_END, r_farclipdist->value);
		}
		else
		{
			glFogf(GL_FOG_DENSITY, r_fog_density->value);
		}
	}

	// Restore state.
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // H2_1.07: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR.
	glDepthMask(GL_TRUE);
}

// Q2 counterpart
static void R_DrawTriangleOutlines(void)
{
	if (!(int)gl_showtris->value)
		return;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	for (int i = 0; i < MAX_LIGHTMAPS; i++)
	{
		for (const msurface_t* surf = gl_lms.lightmap_surfaces[i]; surf != NULL; surf = surf->lightmapchain)
		{
			for (const glpoly_t* p = surf->polys; p != NULL; p = p->chain)
			{
				for (int j = 2; j < p->numverts; j++)
				{
					glBegin(GL_LINE_STRIP);
					glVertex3fv(p->verts[0]);
					glVertex3fv(p->verts[j - 1]);
					glVertex3fv(p->verts[j]);
					glVertex3fv(p->verts[0]);
					glEnd();
				}
			}
		}
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}

static void R_RenderBrushPoly(const entity_t* ent, msurface_t* fa) //mxd. Added 'ent' arg.
{
	c_brush_polys++;

	R_BindImage(R_TextureAnimation(ent, fa->texinfo)); // Q2: GL_Bind().

	// H2: new cl_camera_under_surface logic.
	if ((int)cl_camera_under_surface->value)
	{
		R_EmitUnderwaterPolys(fa);
		R_TexEnv(GL_REPLACE);

		return;
	}

	// H2: new quake_amount logic.
	if ((int)quake_amount->value)
	{
		R_EmitQuakeFloorPolys(fa);
		R_TexEnv(GL_REPLACE);

		return;
	}

	if (fa->flags & SURF_DRAWTURB)
	{
		// Warp texture, no lightmaps.
		R_TexEnv(GL_MODULATE);
		glColor4f(gl_state.inverse_intensity, gl_state.inverse_intensity, gl_state.inverse_intensity, 1.0f);
		R_EmitWaterPolys(fa, fa->flags & SURF_UNDULATE);
		R_TexEnv(GL_REPLACE);

		return;
	}

	R_TexEnv(GL_REPLACE);

	// H2: missing SURF_FLOWING flag logic.
	R_DrawGLPoly(fa->polys);

	int map;
	qboolean is_dynamic = false;

	// Check for lightmap modification.
	for (map = 0; map < MAXLIGHTMAPS && fa->styles[map] != 255; map++)
	{
		if (r_newrefdef.lightstyles[fa->styles[map]].white != fa->cached_light[map])
		{
			is_dynamic = true; //mxd. Avoid unnecessary gotos.
			break;
		}
	}

	// Dynamic this frame or dynamic previously.
	if (fa->dlightframe == r_framecount || is_dynamic)
	{
		if ((int)gl_dynamic->value && !(fa->texinfo->flags & SURF_FULLBRIGHT)) //mxd. SURF_FULLBRIGHT define.
		{
			if ((fa->styles[map] >= 32 || fa->styles[map] == 0) && fa->dlightframe != r_framecount)
			{
				uint temp[34 * 34];
				const int smax = (fa->extents[0] >> 4) + 1;
				const int tmax = (fa->extents[1] >> 4) + 1;

				R_BuildLightMap(fa, (byte*)temp, smax * 4);
				R_SetCacheState(fa);
				R_Bind(gl_state.lightmap_textures + fa->lightmaptexturenum);

				glTexSubImage2D(GL_TEXTURE_2D, 0, fa->light_s, fa->light_t, smax, tmax, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, temp);

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
	else if (gl_lms.tallwall_lightmaptexturenum < MAX_TALLWALL_LIGHTMAPS)
	{
		gl_lms.tallwall_lightmap_surfaces[gl_lms.tallwall_lightmaptexturenum] = fa;
		gl_lms.tallwall_lightmaptexturenum++;
	}
	else
	{
		ri.Con_Printf(PRINT_ALL, "WARNING: too many tall wall surfaces!"); //mxd. Com_Printf() -> ri.Con_Printf().
	}
}

static void R_RenderFlatShadedBrushPoly(const entity_t* ent, msurface_t* fa) // H2 //mxd. Added 'ent' arg.
{
	c_brush_polys++;

	// Use fa->polys pointer as random, but constant color...
	paletteRGBA_t color;
	color.c = (uint)fa->polys;
	glColor3ubv(color.c_array); //mxd. qglColor3f -> qglColor3ubv

	glBegin(GL_POLYGON);

	float* v = fa->polys->verts[0];
	for (int i = 0; i < fa->polys->numverts; i++, v += VERTEXSIZE)
		glVertex3fv(v);

	glEnd();

	// Done when gl_drawflat == 1.
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

	// Dynamic this frame or dynamic previously.
	if (fa->dlightframe == r_framecount || is_dynamic)
	{
		if ((int)gl_dynamic->value && !(fa->texinfo->flags & SURF_FULLBRIGHT)) //mxd. SURF_FULLBRIGHT define.
		{
			if ((fa->styles[map] >= 32 || fa->styles[map] == 0) && fa->dlightframe != r_framecount)
			{
				uint temp[34 * 34];
				const int smax = (fa->extents[0] >> 4) + 1;
				const int tmax = (fa->extents[1] >> 4) + 1;

				R_BuildLightMap(fa, (byte*)temp, smax * 4);
				R_SetCacheState(fa);
				R_Bind(gl_state.lightmap_textures + fa->lightmaptexturenum);

				glTexSubImage2D(GL_TEXTURE_2D, 0, fa->light_s, fa->light_t, smax, tmax, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, temp);

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

//mxd. Similar to Q2's GL_RenderLightmappedPoly (except for missing SURF_FLOWING logic). Original H2 .dll also includes GL_RenderLightmappedPoly_SGIS variant.
static void R_RenderLightmappedPoly(const entity_t* ent, msurface_t* surf) //mxd. Added 'ent' arg.
{
	static uint lightmap_pixels[LM_BLOCK_WIDTH * LM_BLOCK_HEIGHT]; //mxd. Made static.

	int map;
	int lmtex = surf->lightmaptexturenum;
	qboolean lightmap_updated = false;

	for (map = 0; map < MAXLIGHTMAPS && surf->styles[map] != 255; map++)
	{
		if (r_newrefdef.lightstyles[surf->styles[map]].white != surf->cached_light[map])
		{
			lightmap_updated = true; //mxd. Avoid unnecessary gotos.
			break;
		}
	}

	// Dynamic this frame or dynamic previously.
	qboolean is_dynamic = false;
	if (lightmap_updated || surf->dlightframe == r_framecount)
		is_dynamic = ((int)gl_dynamic->value && !(surf->texinfo->flags & SURF_FULLBRIGHT)); //mxd. SURF_FULLBRIGHT define.

	if (is_dynamic)
	{
		const int smax = (surf->extents[0] >> 4) + 1;
		const int tmax = (surf->extents[1] >> 4) + 1;

		R_BuildLightMap(surf, (byte*)lightmap_pixels, smax * 4);

		if ((surf->styles[map] >= 32 || surf->styles[map] == 0) && surf->dlightframe != r_framecount)
		{
			R_SetCacheState(surf);
			R_MBind(GL_TEXTURE1, surf->lightmaptexturenum + gl_state.lightmap_textures);
			lmtex = surf->lightmaptexturenum;
		}
		else
		{
			R_MBind(GL_TEXTURE1, gl_state.lightmap_textures);
			lmtex = 0;
		}

		glTexSubImage2D(GL_TEXTURE_2D, 0, surf->light_s, surf->light_t, smax, tmax, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, lightmap_pixels);
	}

	c_brush_polys++;

	R_MBindImage(GL_TEXTURE0, R_TextureAnimation(ent, surf->texinfo)); // H2: GL_MBind -> GL_MBindImage
	R_MBind(GL_TEXTURE1, gl_state.lightmap_textures + lmtex);

	// Missing: SURF_FLOWING logic.
	for (glpoly_t* p = surf->polys; p != NULL; p = p->chain)
	{
		float* v = p->verts[0];
		glBegin(GL_POLYGON);
		for (int i = 0; i < surf->polys->numverts; i++, v += VERTEXSIZE)
		{
			glMultiTexCoord2f(GL_TEXTURE0, v[3], v[4]);
			glMultiTexCoord2f(GL_TEXTURE1, v[5], v[6]);
			glVertex3fv(v);
		}
		glEnd();
	}
}

static void R_DrawTextureChains(const entity_t* ent) // Q2: DrawTextureChains(). //mxd. Added 'ent' arg.
{
	c_visible_textures = 0;

	// H2: extra gl_sortmulti logic:
	if (multitexture_mode)
	{
		R_EnableMultitexture(true);
		R_SelectTexture(GL_TEXTURE0);
		R_TexEnv(GL_REPLACE);
		R_SelectTexture(GL_TEXTURE1);
		R_TexEnv((int)gl_lightmap->value ? GL_REPLACE : GL_MODULATE);

		image_t* image = &gltextures[0];
		for (int i = 0; i < numgltextures; i++, image++)
		{
			if (image->registration_sequence == 0 || image->multitexturechain == NULL)
				continue;

			c_visible_textures++;

			for (msurface_t* s = image->multitexturechain; s != NULL; s = s->texturechain)
				R_RenderLightmappedPoly(ent, s);

			image->multitexturechain = NULL;
		}

		R_EnableMultitexture(false);
		multitexture_mode = false;
	}

	void (*render_brush_poly)(const entity_t*, msurface_t*) = ((int)gl_drawflat->value ? R_RenderFlatShadedBrushPoly : R_RenderBrushPoly); // H2: new gl_drawflat logic.

	// Original Q2 logic:

	// Render lightmapped surfaces.
	image_t* image = &gltextures[0];
	for (int i = 0; i < numgltextures; i++, image++)
	{
		if (!image->registration_sequence || image->texturechain == NULL)
			continue;

		c_visible_textures++;

		for (msurface_t* s = image->texturechain; s != NULL; s = s->texturechain)
			if (!(s->flags & SURF_DRAWTURB))
				render_brush_poly(ent, s); // H2: new gl_drawflat logic.
	}

	R_EnableMultitexture(false);

	// Render warping (water) surfaces (no lightmaps).
	image = &gltextures[0];
	for (int i = 0; i < numgltextures; i++, image++)
	{
		if (!image->registration_sequence || image->texturechain == NULL)
			continue;

		for (msurface_t* s = image->texturechain; s != NULL; s = s->texturechain)
			if (s->flags & SURF_DRAWTURB)
				render_brush_poly(ent, s); // H2: new gl_drawflat logic.

		image->texturechain = NULL;
	}

	R_TexEnv(GL_REPLACE);
}

static qboolean R_CullBox(const vec3_t mins, const vec3_t maxs)
{
	if (!(int)r_nocull->value)
	{
		for (int i = 0; i < 4; i++)
			if (BoxOnPlaneSide(mins, maxs, &frustum[i]) == 2) // H2: BoxOnPlaneSide call instead of BOX_ON_PLANE_SIDE macro.
				return true;
	}

	return false;
}

static void R_DrawInlineBModel(const entity_t* ent) //mxd. Original logic uses 'currententity' global var.
{
#define BACKFACE_EPSILON 0.01f // Q2: defined in gl_local.h

	const model_t* mdl = *ent->model; //mxd. Original logic uses 'currentmodel' global var instead.

	// Calculate dynamic lighting for bmodel.
	if (!(int)gl_flashblend->value)
	{
		dlight_t* lt = r_newrefdef.dlights;
		for (int k = 0; k < r_newrefdef.num_dlights; k++, lt++)
			R_MarkLights(lt, 1 << k, mdl->nodes + mdl->firstnode);
	}

	msurface_t* psurf = &mdl->surfaces[mdl->firstmodelsurface];

	// H2: extra RF_TRANS_ADD and RF_TRANS_GHOST flags.
	if (ent->flags & RF_TRANS_ANY)
	{
		glEnable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
		R_TexEnv(GL_MODULATE);
	}

	// Draw texture.
	for (int i = 0; i < mdl->nummodelsurfaces; i++, psurf++)
	{
		// Find which side of the node we are on.
		const cplane_t* pplane = psurf->plane;
		const float dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

		// Draw the polygon.
		if (((psurf->flags & SURF_PLANEBACK) && dot < -BACKFACE_EPSILON) ||
			(!(psurf->flags & SURF_PLANEBACK) && dot > BACKFACE_EPSILON))
		{
			if (psurf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66))
			{
				// Add to the translucent chain.
				psurf->texturechain = r_alpha_surfaces;
				r_alpha_surfaces = psurf;
			}
			else if (!(psurf->flags & SURF_DRAWTURB) && !(int)r_fullbright->value && !(int)gl_drawflat->value) // H2: extra r_fullbright and gl_drawflat checks
			{
				R_RenderLightmappedPoly(ent, psurf); // Q2: GL_RenderLightmappedPoly
			}
			else //mxd. Skipped qglMTexCoord2fSGIS check.
			{
				R_EnableMultitexture(false);
				R_RenderBrushPoly(ent, psurf);
				R_EnableMultitexture(true);
			}
		}
	}

	// H2: extra RF_TRANS_ADD and RF_TRANS_GHOST flags.
	if (ent->flags & RF_TRANS_ANY)
	{
		glDisable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		R_TexEnv(GL_REPLACE);
	}
}

void R_DrawBrushModel(entity_t* ent)
{
	const model_t* mdl = *ent->model; //mxd. Original logic uses 'currentmodel' global var instead.

	if (mdl->nummodelsurfaces == 0)
		return;

	// H2: missing: currententity = ent;
	gl_state.currenttextures[0] = -1;
	gl_state.currenttextures[1] = -1;

	vec3_t mins;
	vec3_t maxs;
	qboolean rotated;

	if (ent->angles[0] != 0.0f || ent->angles[1] != 0.0f || ent->angles[2] != 0.0f)
	{
		for (int i = 0; i < 3; i++)
		{
			mins[i] = ent->origin[i] - mdl->radius;
			maxs[i] = ent->origin[i] + mdl->radius;
		}

		rotated = true;
	}
	else
	{
		VectorAdd(ent->origin, mdl->mins, mins);
		VectorAdd(ent->origin, mdl->maxs, maxs);

		rotated = false;
	}

	if (R_CullBox(mins, maxs))
		return;

	//mxd. Skip H2 gl_drawmode logic.
	glColor3f(1.0f, 1.0f, 1.0f);

	memset((void*)gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
	VectorSubtract(r_newrefdef.vieworg, ent->origin, modelorg);

	if (rotated)
	{
		vec3_t angles;
		VectorScale(ent->angles, RAD_TO_ANGLE, angles); // H2: new RAD_TO_ANGLE rescale.

		vec3_t temp;
		VectorCopy(modelorg, temp);

		vec3_t forward;
		vec3_t right;
		vec3_t up;
		AngleVectors(angles, forward, right, up);

		modelorg[0] = DotProduct(temp, forward);
		modelorg[1] = -DotProduct(temp, right);
		modelorg[2] = DotProduct(temp, up);
	}

	glPushMatrix();
	ent->angles[0] *= -1.0f; // stupid quake bug.
	ent->angles[2] *= -1.0f; // stupid quake bug.
	R_RotateForEntity(ent);
	ent->angles[0] *= -1.0f; // stupid quake bug.
	ent->angles[2] *= -1.0f; // stupid quake bug.

	R_EnableMultitexture(true);
	R_SelectTexture(GL_TEXTURE0);
	R_TexEnv(GL_REPLACE);
	R_SelectTexture(GL_TEXTURE1);
	R_TexEnv(GL_MODULATE);

	R_DrawInlineBModel(ent);
	R_EnableMultitexture(false);

	//mxd. Skip H2 gl_drawmode logic.
	glPopMatrix();
}

#pragma endregion

#pragma region ========================== WORLD MODEL RENDERING ==========================

static void R_RecursiveWorldNode(const entity_t* ent, mnode_t* node) //mxd. Added 'ent' arg.
{
	if (node->contents == CONTENTS_SOLID || node->visframe != r_visframecount || R_CullBox(node->minmaxs, node->minmaxs + 3))
		return;

	// If a leaf node, draw stuff.
	if (node->contents != -1)
	{
		const mleaf_t* pleaf = (mleaf_t*)node;

		// Check for door connected areas.
		if (r_newrefdef.areabits != NULL && !(r_newrefdef.areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
			return; // Not visible.

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

	const int side = ((dot >= 0.0f) ? 0 : 1);
	const int sidebit = ((dot >= 0.0f) ? 0 : SURF_PLANEBACK);

	// Recurse down the children, front side first.
	R_RecursiveWorldNode(ent, node->children[side]);

	// Draw stuff.
	msurface_t* surf = &r_worldmodel->surfaces[node->firstsurface];
	for (int c = node->numsurfaces; c > 0; c--, surf++)
	{
		if (surf->visframe != r_framecount || (surf->flags & SURF_PLANEBACK) != sidebit)
			continue; // Wrong frame or side.

		if (surf->texinfo->flags & SURF_SKY)
		{
			// Just adds to visible sky bounds.
			R_AddSkySurface(surf);
		}
		else if (surf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66))
		{
			// Add to the translucent texture chain.
			surf->texturechain = r_alpha_surfaces;
			r_alpha_surfaces = surf;
		}
		else if (!(surf->flags & SURF_DRAWTURB) && !(surf->flags & SURF_TALL_WALL) && !(int)r_fullbright->value && !(int)gl_drawflat->value) // H2: extra SURF_TALL_WALL, r_fullbright, gl_drawflat checks.
		{
			// The polygon is visible, so add it to the sorted multi-texture chain.
			image_t* image = R_TextureAnimation(ent, surf->texinfo);
			surf->texturechain = image->multitexturechain;
			image->multitexturechain = surf;

			multitexture_mode = true;
		}
		else //mxd. Skipping qglMTexCoord2fSGIS logic...
		{
			// The polygon is visible, so add it to the sorted texture chain.
			// FIXME: this is a hack for animation.
			image_t* image = R_TextureAnimation(ent, surf->texinfo);
			surf->texturechain = image->texturechain;
			image->texturechain = surf;
		}
	}

	// Recurse down the back side.
	R_RecursiveWorldNode(ent, node->children[!side]);
}

void R_DrawWorld(void)
{
	if (!(int)r_drawworld->value || (r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		return;

	VectorCopy(r_newrefdef.vieworg, modelorg);

	// Auto cycle the world frame for texture animation.
	const entity_t ent = { .frame = (int)(r_newrefdef.time * 2.0f) }; //mxd. memset -> zero initialization.

	gl_state.currenttextures[0] = -1;
	gl_state.currenttextures[1] = -1;

	//mxd. Skip H2 gl_drawmode logic.
	glColor3f(1.0f, 1.0f, 1.0f);

	memset((void*)gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
	gl_lms.tallwall_lightmaptexturenum = 0; // H2
	multitexture_mode = false; // H2

	R_ClearSkyBox();

	// H2: new r_fullbright and gl_drawflat cvar checks.
	if (!(int)r_fullbright->value && !(int)gl_drawflat->value)
	{
		R_EnableMultitexture(true);

		R_SelectTexture(GL_TEXTURE0);
		R_TexEnv(GL_REPLACE);
		R_SelectTexture(GL_TEXTURE1);
		R_TexEnv((int)gl_lightmap->value ? GL_REPLACE : GL_MODULATE);

		R_RecursiveWorldNode(&ent, r_worldmodel->nodes);

		R_EnableMultitexture(false);
	}
	else
	{
		R_RecursiveWorldNode(&ent, r_worldmodel->nodes);
	}

	// Theoretically nothing should happen in the next two functions if multitexture is enabled.

	// H2: new gl_drawflat cvar logic
	if ((int)gl_drawflat->value)
	{
		glDisable(GL_TEXTURE_2D);

		R_DrawTextureChains(&ent);

		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		R_DrawTextureChains(&ent);
	}

	R_BlendLightmaps(r_worldmodel);

	//mxd. Skip H2 gl_drawmode logic.
	R_DrawSkyBox();
	R_DrawTriangleOutlines();

#ifdef _DEBUG
	R_DrawDebugPrimitives(); //mxd.
#endif
}

// Q2 counterpart
// Mark the leaves and nodes that are in the PVS for the current cluster.
void R_MarkLeaves(void)
{
	static byte fatvis[MAX_MAP_LEAFS / 8]; //mxd. Made static.

	if (r_oldviewcluster == r_viewcluster && r_oldviewcluster2 == r_viewcluster2 && !(int)r_novis->value && r_viewcluster != -1)
		return;

	// Development aid to let you run around and see exactly where the pvs ends.
	if ((int)gl_lockpvs->value)
		return;

	r_visframecount++;
	r_oldviewcluster = r_viewcluster;
	r_oldviewcluster2 = r_viewcluster2;

	if ((int)r_novis->value || r_viewcluster == -1 || r_worldmodel->vis == NULL)
	{
		// Mark everything.
		for (int i = 0; i < r_worldmodel->numleafs; i++)
			r_worldmodel->leafs[i].visframe = r_visframecount;

		for (int i = 0; i < r_worldmodel->numnodes; i++)
			r_worldmodel->nodes[i].visframe = r_visframecount;

		return;
	}

	byte* vis = Mod_ClusterPVS(r_viewcluster, r_worldmodel);

	// May have to combine two clusters because of solid water boundaries.
	if (r_viewcluster2 != r_viewcluster)
	{
		memcpy(fatvis, vis, (r_worldmodel->numleafs + 7) / 8);
		vis = Mod_ClusterPVS(r_viewcluster2, r_worldmodel);

		const int c = (r_worldmodel->numleafs + 31) / 32;
		for (int i = 0; i < c; i++)
			((int*)fatvis)[i] |= ((int*)vis)[i];

		vis = fatvis;
	}

	mleaf_t* leaf = &r_worldmodel->leafs[0];
	for (int i = 0; i < r_worldmodel->numleafs; i++, leaf++)
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
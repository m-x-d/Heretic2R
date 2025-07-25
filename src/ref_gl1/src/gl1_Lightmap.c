//
// gl1_Lightmap.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Lightmap.h"
#include "gl1_Image.h"
#include "gl1_Light.h"
#include "Hunk.h"
#include "Vector.h"

gllightmapstate_t gl_lms;

#pragma region ========================== LIGHTMAP ALLOCATION ==========================

// Q2 counterpart
void LM_InitBlock(void)
{
	memset(gl_lms.allocated, 0, sizeof(gl_lms.allocated));
}

// Q2 counterpart
void LM_UploadBlock(const qboolean dynamic)
{
	R_Bind(gl_state.lightmap_textures + (dynamic ? 0 : gl_lms.current_lightmap_texture));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri

	if (dynamic)
	{
		int height = 0;
		for (int i = 0; i < LM_BLOCK_WIDTH; i++)
			height = max(gl_lms.allocated[i], height);

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, LM_BLOCK_WIDTH, height, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, gl_lms.lightmap_buffer);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, gl_lms.internal_format, LM_BLOCK_WIDTH, LM_BLOCK_HEIGHT, 0, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, gl_lms.lightmap_buffer);
		gl_lms.current_lightmap_texture++;

		if (gl_lms.current_lightmap_texture == MAX_LIGHTMAPS)
			ri.Sys_Error(ERR_DROP, "LM_UploadBlock() - MAX_LIGHTMAPS exceeded\n");
	}
}

// Q2 counterpart. Returns a texture number and the position inside it.
qboolean LM_AllocBlock(const int w, const int h, int* x, int* y)
{
	int j;
	int best = LM_BLOCK_HEIGHT;

	for (int i = 0; i < LM_BLOCK_WIDTH - w; i++)
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
			// This is a valid spot.
			*x = i;
			*y = best2;
			best = best2;
		}
	}

	if (best + h > LM_BLOCK_HEIGHT)
		return false;

	for (int i = 0; i < w; i++)
		gl_lms.allocated[*x + i] = best + h;

	return true;
}

#pragma endregion

#pragma region ========================== LIGHTMAP BUILDING ==========================

// Q2 counterpart
void LM_BuildPolygonFromSurface(const model_t* mdl, msurface_t* fa) //mxd. Original logic uses 'currentmodel' global var here.
{
	// Reconstruct the polygon.
	const medge_t* pedges = mdl->edges;
	const int lnumverts = fa->numedges;

	// Draw texture.
	glpoly_t* poly = Hunk_Alloc((int)sizeof(glpoly_t) + (lnumverts - 4) * VERTEXSIZE * sizeof(float));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for (int i = 0; i < lnumverts; i++)
	{
		const int lindex = mdl->surfedges[fa->firstedge + i];

		float* vec;
		if (lindex > 0)
		{
			const medge_t* r_pedge = &pedges[lindex];
			vec = mdl->vertexes[r_pedge->v[0]].position;
		}
		else
		{
			const medge_t* r_pedge = &pedges[-lindex];
			vec = mdl->vertexes[r_pedge->v[1]].position;
		}

		float s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= (float)fa->texinfo->image->width;

		float t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= (float)fa->texinfo->image->height;

		VectorCopy(vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		// Lightmap texture coordinates.
		s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s -= (float)fa->texturemins[0];
		s += (float)fa->light_s * 16;
		s += 8;
		s /= LM_BLOCK_WIDTH * 16;

		t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= (float)fa->texturemins[1];
		t += (float)fa->light_t * 16;
		t += 8;
		t /= LM_BLOCK_HEIGHT * 16;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;
	}
}

// Q2 counterpart
void LM_CreateSurfaceLightmap(msurface_t* surf)
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
	base += (surf->light_t * LM_BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

	R_SetCacheState(surf);
	R_BuildLightMap(surf, base, LM_BLOCK_WIDTH * LIGHTMAP_BYTES);
}

// Q2 counterpart
void LM_BeginBuildingLightmaps(void) //mxd. Removed unused model_t* arg.
{
	static lightstyle_t lightstyles[MAX_LIGHTSTYLES];
	static uint dummy[LM_BLOCK_WIDTH * LM_BLOCK_HEIGHT]; //mxd. Made it static.

	memset(gl_lms.allocated, 0, sizeof(gl_lms.allocated));

	r_framecount = 1; // No dlightcache.

	R_EnableMultitexture(true);
	R_SelectTexture(GL_TEXTURE1);

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
	R_Bind(gl_state.lightmap_textures);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //mxd. qglTexParameterf -> qglTexParameteri
	glTexImage2D(GL_TEXTURE_2D, 0, gl_lms.internal_format, LM_BLOCK_WIDTH, LM_BLOCK_HEIGHT, 0, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, dummy);
}

// Q2 counterpart
void LM_EndBuildingLightmaps(void)
{
	LM_UploadBlock(false);
	R_EnableMultitexture(false);
}

#pragma endregion
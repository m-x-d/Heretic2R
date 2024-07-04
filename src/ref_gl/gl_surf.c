//
// gl_surf.c -- surface-related refresh code
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "Vector.h"

static vec3_t modelorg; // Relative to viewpoint
static msurface_t* r_alpha_surfaces;

#define LIGHTMAP_BYTES			4

#define BLOCK_WIDTH				128
#define BLOCK_HEIGHT			128

#define MAX_LIGHTMAPS			128
#define MAX_TALLWALL_LIGHTMAPS	512 // New in H2

int c_visible_lightmaps;
int c_visible_textures;

static int r_visframecount; // Bumped when going to a new PVS // Q2: defined in gl_rmain.c //mxd. Moved here & made static
static int num_sorted_multitextures; // New in H2

#define GL_LIGHTMAP_FORMAT		GL_RGBA

typedef struct
{
	int internal_format;
	int current_lightmap_texture;

	msurface_t* lightmap_surfaces[MAX_LIGHTMAPS];
	msurface_t* tallwall_lightmap_surfaces[MAX_TALLWALL_LIGHTMAPS]; // New in H2
	int tallwall_lightmaptexturenum; // New in H2

	int allocated[BLOCK_WIDTH];

	// The lightmap texture data needs to be kept in main memory so texsubimage can update properly
	byte lightmap_buffer[BLOCK_WIDTH * BLOCK_HEIGHT * 4];
} gllightmapstate_t;

static gllightmapstate_t gl_lms;

#pragma region ========================== BRUSH MODELS ==========================

static image_t* R_TextureAnimation(mtexinfo_t* tex)
{
	NOT_IMPLEMENTED
	return NULL;
}

static void R_BlendLightmaps(void)
{
	NOT_IMPLEMENTED
}

static void R_DrawTriangleOutlines(void)
{
	NOT_IMPLEMENTED
}

void R_SortAndDrawAlphaSurfaces(void)
{
	NOT_IMPLEMENTED
}

static void DrawTextureChains(void)
{
	NOT_IMPLEMENTED
}

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
	gl_lms.tallwall_lightmaptexturenum = 0; // New in H2
	R_ClearSkyBox();
	num_sorted_multitextures = 0; // New in H2

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
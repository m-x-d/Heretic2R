//
// gl1_Surface.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Surface.h"
#include "gl1_Image.h"
#include "gl1_Lightmap.h"
#include "gl1_Sky.h"
#include "Vector.h"

int c_visible_lightmaps;
int c_visible_textures;

static int r_visframecount; // Bumped when going to a new PVS // Q2: defined in gl_rmain.c //mxd. Moved here & made static.
static qboolean multitexture_mode; // H2

static vec3_t modelorg; // Relative to viewpoint.

static msurface_t* r_alpha_surfaces;

#pragma region ========================== ALPHA SURFACES RENDERING ==========================

void R_SortAndDrawAlphaSurfaces(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== BRUSH MODELS RENDERING ==========================

// Returns the proper texture for a given time and base texture.
static image_t* R_TextureAnimation(const mtexinfo_t* tex)
{
	if (tex->next != NULL)
	{
		int frame;

		if ((tex->flags & SURF_ANIMSPEED) && tex->image->num_frames > 0) // H2: extra SURF_ANIMSPEED logic.
			frame = (int)((float)tex->image->num_frames * r_newrefdef.time);
		else if (currententity != NULL) //mxd. Added sanity check.
			frame = currententity->frame;
		else
			return tex->image;

		frame %= tex->numframes;

		while (frame-- > 0 && tex->next != NULL) //mxd. Added tex->next sanity check.
			tex = tex->next;
	}

	return tex->image;
}

static void R_BlendLightmaps(void)
{
	NOT_IMPLEMENTED
}

static void R_DrawTriangleOutlines(void)
{
	NOT_IMPLEMENTED
}

static void R_RenderBrushPoly(msurface_t* fa)
{
	NOT_IMPLEMENTED
}

static void R_RenderFlatShadedBrushPoly(msurface_t* fa) // H2
{
	NOT_IMPLEMENTED
}

//mxd. Similar to Q2's GL_RenderLightmappedPoly (except for missing SURF_FLOWING logic). Original H2 .dll also includes GL_RenderLightmappedPoly_SGIS variant.
static void R_RenderLightmappedPoly(msurface_t* surf)
{
	NOT_IMPLEMENTED
}

static void R_DrawTextureChains(void) // Q2: DrawTextureChains().
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
				R_RenderLightmappedPoly(s);

			image->multitexturechain = NULL;
		}

		R_EnableMultitexture(false);
		multitexture_mode = false;
	}

	void (*render_brush_poly)(msurface_t*) = ((int)gl_drawflat->value ? R_RenderFlatShadedBrushPoly : R_RenderBrushPoly); // H2: new gl_drawflat logic.

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
				render_brush_poly(s); // H2: new gl_drawflat logic.
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
				render_brush_poly(s); // H2: new gl_drawflat logic.

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

#pragma endregion

#pragma region ========================== WORLD MODEL RENDERING ==========================

static void R_RecursiveWorldNode(mnode_t* node)
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
	R_RecursiveWorldNode(node->children[side]);

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
			image_t* image = R_TextureAnimation(surf->texinfo);
			surf->texturechain = image->multitexturechain;
			image->multitexturechain = surf;

			multitexture_mode = true;
		}
		else //mxd. Skipping qglMTexCoord2fSGIS logic...
		{
			// The polygon is visible, so add it to the sorted texture chain.
			// FIXME: this is a hack for animation.
			image_t* image = R_TextureAnimation(surf->texinfo);
			surf->texturechain = image->texturechain;
			image->texturechain = surf;
		}
	}

	// Recurse down the back side.
	R_RecursiveWorldNode(node->children[!side]);
}

void R_DrawWorld(void)
{
	if (!(int)r_drawworld->value || (r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		return;

	currentmodel = r_worldmodel;
	VectorCopy(r_newrefdef.vieworg, modelorg);

	// Auto cycle the world frame for texture animation.
	entity_t ent = { .frame = (int)(r_newrefdef.time * 2.0f) }; //mxd. memset -> zero initialization.
	currententity = &ent;

	gl_state.currenttextures[0] = -1;
	gl_state.currenttextures[1] = -1;

	if ((int)gl_drawmode->value) // H2: new gl_drawmode logic.
	{
		glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
		glEnable(GL_BLEND);
	}
	else
	{
		glColor3f(1.0f, 1.0f, 1.0f);
	}

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

		if ((int)gl_lightmap->value)
			R_TexEnv(GL_REPLACE);
		else
			R_TexEnv(GL_MODULATE);

		R_RecursiveWorldNode(r_worldmodel->nodes);

		R_EnableMultitexture(false);
	}
	else
	{
		R_RecursiveWorldNode(r_worldmodel->nodes);
	}

	// Theoretically nothing should happen in the next two functions if multitexture is enabled.

	// H2: new gl_drawflat cvar logic
	if ((int)gl_drawflat->value)
	{
		glDisable(GL_TEXTURE_2D);

		R_DrawTextureChains();

		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		R_DrawTextureChains();
	}

	R_BlendLightmaps();

	// H2: new gl_drawmode cvar logic.
	if ((int)gl_drawmode->value)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

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
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
static int num_sorted_multitextures; // H2

static vec3_t modelorg; // Relative to viewpoint.

#pragma region ========================== ALPHA SURFACES RENDERING ==========================

void R_SortAndDrawAlphaSurfaces(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== BRUSH MODELS RENDERING ==========================

static void R_BlendLightmaps(void)
{
	NOT_IMPLEMENTED
}

static void R_DrawTriangleOutlines(void)
{
	NOT_IMPLEMENTED
}

static void DrawTextureChains(void)
{
	NOT_IMPLEMENTED
}

#pragma endregion

#pragma region ========================== WORLD MODEL RENDERING ==========================

static void R_RecursiveWorldNode(mnode_t* node)
{
	NOT_IMPLEMENTED
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
	num_sorted_multitextures = 0; // H2

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

		DrawTextureChains();

		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		DrawTextureChains();
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
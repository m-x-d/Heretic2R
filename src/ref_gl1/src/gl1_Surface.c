//
// gl1_Surface.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Surface.h"

int c_visible_lightmaps;
int c_visible_textures;

static int r_visframecount; // Bumped when going to a new PVS // Q2: defined in gl_rmain.c //mxd. Moved here & made static.

void R_SortAndDrawAlphaSurfaces(void)
{
	NOT_IMPLEMENTED
}

void R_DrawWorld(void)
{
	NOT_IMPLEMENTED
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
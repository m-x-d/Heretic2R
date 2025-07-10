//
// gl1_Model.c
//
// Copyright 1998 Raven Software
//

#include "gl1_Local.h"
#include "gl1_FlexModel.h"
#include "gl1_Image.h"
#include "gl1_Lightmap.h"
#include "gl1_Warp.h"
#include "Hunk.h"
#include "Vector.h"

int registration_sequence;

static byte mod_novis[MAX_MAP_LEAFS / 8];

#define MAX_MOD_KNOWN 512
static model_t mod_known[MAX_MOD_KNOWN];
static int mod_numknown;

// The inline ('*1', '*2', ...) models from the current map are kept separate.
static model_t mod_inline[MAX_MOD_KNOWN];

// Q2 counterpart
mleaf_t* Mod_PointInLeaf(vec3_t p, const model_t* model)
{
	if (model == NULL || model->nodes == NULL)
		ri.Sys_Error(ERR_DROP, "Mod_PointInLeaf: bad model");

	mnode_t* node = model->nodes;
	while (node->contents == -1)
	{
		const cplane_t* plane = node->plane;
		if (DotProduct(p, plane->normal) - plane->dist > 0.0f)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return (mleaf_t*)node;
}

static byte* Mod_DecompressVis(const byte* in, const model_t* model)
{
	static byte decompressed[MAX_MAP_LEAFS / 8];

	const int row = (model->vis->numclusters + 7) >> 3;
	byte* out = decompressed;

	if (in == NULL)
	{
		// No vis info, so make all visible.
		memset(decompressed, 0xff, row); // H2: memset instead of manual assign.
		return decompressed;
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		const uint c = in[1];
		in += 2;

		memset(out, 0, c); // H2: memset instead of manual assign.
		out += c;
	} while (out - decompressed < row);

	return decompressed;
}

// Q2 counterpart
byte* Mod_ClusterPVS(const int cluster, const model_t* model)
{
	if (cluster > -1 && model->vis != NULL)
		return Mod_DecompressVis((byte*)model->vis + model->vis->bitofs[cluster][DVIS_PVS], model);

	return mod_novis;
}

// Q2 counterpart
void Mod_Modellist_f(void)
{
	int total = 0;
	ri.Con_Printf(PRINT_ALL, "Loaded models:\n");

	model_t* mod = &mod_known[0];
	for (int i = 0; i < mod_numknown; i++, mod++)
	{
		if (mod->name[0])
		{
			ri.Con_Printf(PRINT_ALL, "%8i : %s\n", mod->extradatasize, mod->name);
			total += mod->extradatasize;
		}
	}

	ri.Con_Printf(PRINT_ALL, "Total resident: %i\n", total);
}

// Q2 counterpart
void Mod_Init(void)
{
	memset(mod_novis, 0xff, sizeof(mod_novis));
}

// Q2 counterpart
static void Mod_Free(model_t* mod)
{
	Hunk_Free(mod->extradata);
	memset(mod, 0, sizeof(*mod));
}

// Q2 counterpart
void Mod_FreeAll(void)
{
	for (int i = 0; i < mod_numknown; i++)
		if (mod_known[i].extradatasize)
			Mod_Free(&mod_known[i]);
}

static void Mod_LoadBookModel(model_t* mod, const void* buffer, const int length) // H2
{
	const book_t* book_in = buffer;
	book_t* book_out = Hunk_Alloc(length);

	if (book_in->bheader.version != BOOK_VERSION)
		ri.Sys_Error(ERR_DROP, "Mod_LoadBookModel: '%s' has wrong version number (%i should be %i)!", mod->name, book_in->bheader.version, BOOK_VERSION); //mxd. Sys_Error() -> ri.Sys_Error().

	if (book_in->bheader.num_segments > MAX_FRAMES)
		ri.Sys_Error(ERR_DROP, "Mod_LoadBookModel: '%s' has too many frames (%i > %i)!", mod->name, book_in->bheader.num_segments, MAX_FRAMES); //mxd. Sys_Error() -> ri.Sys_Error().

	// Copy everything.
	memcpy(book_out, book_in, book_in->bheader.num_segments * sizeof(bookframe_t) + sizeof(bookheader_t));

	// Pre-load frame images.
	bookframe_t* frame = &book_out->bframes[0];
	for (int i = 0; i < book_out->bheader.num_segments; i++, frame++)
	{
		char frame_name[MAX_QPATH];
		Com_sprintf(frame_name, sizeof(frame_name), "Book/%s", frame->name);

		mod->skins[i] = R_FindImage(frame_name, it_pic);
	}

	// Set model type.
	mod->type = mod_book;
}

static void Mod_LoadSpriteModel(model_t* mod, const void* buffer, const int length)
{
	char sprite_name[MAX_OSPATH]; // H2

	const dsprite_t* sprin = buffer;
	dsprite_t* sprout = Hunk_Alloc(length);

	sprout->ident = LittleLong(sprin->ident);
	sprout->version = LittleLong(sprin->version);
	sprout->numframes = LittleLong(sprin->numframes);

	if (sprout->version != SPRITE_VERSION)
		ri.Sys_Error(ERR_DROP, "'%s' has wrong version number (%i should be %i)", mod->name, sprout->version, SPRITE_VERSION);

	if (sprout->numframes > MAX_FRAMES)
		ri.Sys_Error(ERR_DROP, "'%s' has too many frames (%i > %i)", mod->name, sprout->numframes, MAX_FRAMES);

	// Byte-swap everything.
	for (int i = 0; i < sprout->numframes; i++)
	{
		sprout->frames[i].width = LittleLong(sprin->frames[i].width);
		sprout->frames[i].height = LittleLong(sprin->frames[i].height);
		sprout->frames[i].origin_x = LittleLong(sprin->frames[i].origin_x);
		sprout->frames[i].origin_y = LittleLong(sprin->frames[i].origin_y);
		memcpy(sprout->frames[i].name, sprin->frames[i].name, MAX_FRAMENAME);

		Com_sprintf(sprite_name, sizeof(sprite_name), "Sprites/%s", sprout->frames->name); // H2
		mod->skins[i] = R_FindImage(sprite_name, it_sprite);
	}

	mod->type = mod_sprite;
}

#pragma region ========================== BRUSHMODEL LOADING ==========================

// Q2 counterpart
static void Mod_LoadVertexes(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	const dvertex_t* in = (const void*)(mod_base + l->fileofs);

	if (l->filelen % sizeof(dvertex_t) != 0)
		ri.Sys_Error(ERR_DROP, "Mod_LoadVertexes: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(dvertex_t);
	mvertex_t* out = Hunk_Alloc(count * (int)sizeof(mvertex_t));

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		out->position[0] = LittleFloat(in->point[0]);
		out->position[1] = LittleFloat(in->point[1]);
		out->position[2] = LittleFloat(in->point[2]);
	}
}

// Q2 counterpart
static void Mod_LoadEdges(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	const dedge_t* in = (const void*)(mod_base + l->fileofs);

	if (l->filelen % sizeof(dedge_t) != 0)
		ri.Sys_Error(ERR_DROP, "Mod_LoadEdges: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(dedge_t);
	medge_t* out = Hunk_Alloc((count + 1) * (int)sizeof(medge_t));

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		out->v[0] = (ushort)LittleShort(in->v[0]);
		out->v[1] = (ushort)LittleShort(in->v[1]);
	}
}

// Q2 counterpart
static void Mod_LoadSurfedges(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	const int* in = (const void*)(mod_base + l->fileofs);

	if (l->filelen % sizeof(int) != 0)
		ri.Sys_Error(ERR_DROP, "Mod_LoadSurfedges: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(int);

	if (count < 1 || count >= MAX_MAP_SURFEDGES)
		ri.Sys_Error(ERR_DROP, "Mod_LoadSurfedges: bad surfedges count in '%s': %i", loadmodel->name, count);

	int* out = Hunk_Alloc(count * (int)sizeof(int));

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for (int i = 0; i < count; i++)
		out[i] = LittleLong(in[i]);
}

// Q2 counterpart
static void Mod_LoadLighting(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	if (l->filelen > 0)
	{
		loadmodel->lightdata = Hunk_Alloc(l->filelen);
		memcpy(loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
	}
	else
	{
		loadmodel->lightdata = NULL;
	}
}

// Q2 counterpart
static void Mod_LoadPlanes(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	const dplane_t* in = (const void*)(mod_base + l->fileofs);

	if (l->filelen % sizeof(dplane_t) != 0)
		ri.Sys_Error(ERR_DROP, "Mod_LoadPlanes: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(dplane_t);
	cplane_t* out = Hunk_Alloc(count * (int)sizeof(cplane_t) * 2);

	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		int bits = 0;

		for (int j = 0; j < 3; j++)
		{
			out->normal[j] = LittleFloat(in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1 << j;
		}

		out->dist = LittleFloat(in->dist);
		out->type = (byte)LittleLong(in->type);
		out->signbits = (byte)bits;
	}
}

static void Mod_LoadTexinfo(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	char name[MAX_QPATH];

	const texinfo_t* in = (const void*)(mod_base + l->fileofs);
	if (l->filelen % sizeof(texinfo_t) != 0)
		ri.Sys_Error(ERR_DROP, "Mod_LoadTexinfo: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(texinfo_t);
	mtexinfo_t* out = Hunk_Alloc(count * (int)sizeof(mtexinfo_t));

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		for (int j = 0; j < 4; j++)
		{
			out->vecs[0][j] = LittleFloat(in->vecs[0][j]);
			out->vecs[1][j] = LittleFloat(in->vecs[1][j]);
		}

		out->flags = LittleLong(in->flags);
		const int next = LittleLong(in->nexttexinfo);

		if (next > 0)
			out->next = loadmodel->texinfo + next;
		else
			out->next = NULL;

		Com_sprintf(name, sizeof(name), "textures/%s.m8", in->texture); // H2: .wal -> .m8
		out->image = R_FindImage(name, it_wall);

		if (out->image == NULL)
		{
			ri.Con_Printf(PRINT_ALL, "Couldn't load '%s'\n", name);
			out->image = r_notexture;
		}
	}

	// Count animation frames.
	for (int i = 0; i < count; i++)
	{
		out = &loadmodel->texinfo[i];
		out->numframes = 1;

		for (const mtexinfo_t* step = out->next; step != NULL && step != out; step = step->next)
			out->numframes++;
	}
}

// Q2 counterpart. Fills in s->texturemins[] and s->extents[]
static void CalcSurfaceExtents(model_t* loadmodel, msurface_t* s)
{
	float mins[2];
	float maxs[2];
	mvertex_t* v;

	mins[0] = 999999.0f;
	mins[1] = 999999.0f;

	maxs[0] = -99999.0f;
	maxs[1] = -99999.0f;

	const mtexinfo_t* tex = s->texinfo;

	for (int i = 0; i < s->numedges; i++)
	{
		const int edge = loadmodel->surfedges[s->firstedge + i];
		if (edge >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[edge].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-edge].v[1]];

		for (int j = 0; j < 2; j++)
		{
			const float val = v->position[0] * tex->vecs[j][0] +
				v->position[1] * tex->vecs[j][1] +
				v->position[2] * tex->vecs[j][2] +
				tex->vecs[j][3];

			mins[j] = min(mins[j], val);
			maxs[j] = max(maxs[j], val);
		}
	}

	for (int i = 0; i < 2; i++)
	{
		const int bmin = (const int)floorf(mins[i] / 16);
		const int bmax = (const int)ceilf(maxs[i] / 16);

		s->texturemins[i] = (short)(bmin * 16);
		s->extents[i] = (short)((bmax - bmin) * 16);
	}
}

static void Mod_LoadFaces(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	const dface_t* in = (const void*)(mod_base + l->fileofs);
	if (l->filelen % sizeof(dface_t) != 0)
		ri.Sys_Error(ERR_DROP, "Mod_LoadFaces: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(dface_t);
	msurface_t* out = Hunk_Alloc(count * (int)sizeof(msurface_t));

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	currentmodel = loadmodel;

	LM_BeginBuildingLightmaps();

	for (int surfnum = 0; surfnum < count; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);
		out->flags = 0;
		out->polys = NULL;

		const int planenum = LittleShort(in->planenum);

		if (LittleShort(in->side) != 0)
			out->flags |= SURF_PLANEBACK;

		out->plane = loadmodel->planes + planenum;

		const int texinfo = LittleShort(in->texinfo);
		if (texinfo < 0 || texinfo >= loadmodel->numtexinfo)
			ri.Sys_Error(ERR_DROP, "Mod_LoadFaces: bad texinfo number");

		out->texinfo = loadmodel->texinfo + texinfo;

		CalcSurfaceExtents(loadmodel, out);

		// Lighting info
		for (int i = 0; i < MAXLIGHTMAPS; i++)
			out->styles[i] = in->styles[i];

		const int lightofs = LittleLong(in->lightofs);
		if (lightofs == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata + lightofs;

		// Set the drawing flags
		if (out->texinfo->flags & SURF_WARP)
		{
			out->flags |= SURF_DRAWTURB;

			if (out->texinfo->flags & SURF_UNDULATE) // H2
				out->flags |= SURF_UNDULATE;

			for (int i = 0; i < 2; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}

			// Cut up polygon for warps.
			R_SubdivideSurface(loadmodel, out);
		}

		// Create lightmaps and polygons.
		if (!(out->texinfo->flags & SURF_FULLBRIGHT)) //mxd. SURF_FULLBRIGHT define.
			LM_CreateSurfaceLightmap(out);

		if (!(out->texinfo->flags & SURF_WARP))
			LM_BuildPolygonFromSurface(loadmodel, out);
	}

	LM_EndBuildingLightmaps();
}

// Q2 counterpart
static void Mod_LoadMarksurfaces(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	const short* in = (const void*)(mod_base + l->fileofs);
	if (l->filelen % sizeof(short) != 0)
		ri.Sys_Error(ERR_DROP, "Mod_LoadMarksurfaces: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(short);
	msurface_t** out = (msurface_t**)Hunk_Alloc(count * (int)sizeof(msurface_t*));

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for (int i = 0; i < count; i++)
	{
		const int j = LittleShort(in[i]);
		if (j < 0 || j >= loadmodel->numsurfaces)
			ri.Sys_Error(ERR_DROP, "Mod_ParseMarksurfaces: bad surface number");

		out[i] = loadmodel->surfaces + j;
	}
}

// Q2 counterpart
static void Mod_LoadVisibility(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	if (l->filelen == 0)
	{
		loadmodel->vis = NULL;
		return;
	}

	loadmodel->vis = Hunk_Alloc(l->filelen);
	memcpy(loadmodel->vis, mod_base + l->fileofs, l->filelen);

	loadmodel->vis->numclusters = LittleLong(loadmodel->vis->numclusters);
	for (int i = 0; i < loadmodel->vis->numclusters; i++)
	{
		loadmodel->vis->bitofs[i][0] = LittleLong(loadmodel->vis->bitofs[i][0]);
		loadmodel->vis->bitofs[i][1] = LittleLong(loadmodel->vis->bitofs[i][1]);
	}
}

static void Mod_LoadLeafs(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	const dleaf_t* in = (const void*)(mod_base + l->fileofs);
	if (l->filelen % sizeof(dleaf_t))
		ri.Sys_Error(ERR_DROP, "Mod_LoadLeafs: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(dleaf_t);
	mleaf_t* out = Hunk_Alloc(count * (int)sizeof(mleaf_t));

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	const float offset = ((int)gl_noartifacts->value ? 32.0f : 0.0f); // H2

	for (int i = 0; i < count; i++, in++, out++)
	{
		for (int j = 0; j < 3; j++)
		{
			out->minmaxs[j + 0] = LittleShort(in->mins[j]) - offset; // H2: +offset
			out->minmaxs[j + 3] = LittleShort(in->maxs[j]) + offset; // H2: +offset
		}

		out->contents = LittleLong(in->contents);
		out->cluster = LittleShort(in->cluster);
		out->area = LittleShort(in->area);
		out->firstmarksurface = loadmodel->marksurfaces + LittleShort(in->firstleafface);
		out->nummarksurfaces = LittleShort(in->numleaffaces);
	}
}

// Q2 counterpart
static void Mod_SetParent(mnode_t* node, mnode_t* parent)
{
	node->parent = parent;

	if (node->contents == -1)
	{
		Mod_SetParent(node->children[0], node);
		Mod_SetParent(node->children[1], node);
	}
}

static void Mod_LoadNodes(model_t* loadmodel, const byte* mod_base, const lump_t* l)
{
	const dnode_t* in = (const void*)(mod_base + l->fileofs);
	if (l->filelen % sizeof(dnode_t))
		ri.Sys_Error(ERR_DROP, "Mod_LoadNodes: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(dnode_t);
	mnode_t* out = Hunk_Alloc(count * (int)sizeof(mnode_t));

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	const float offset = ((int)gl_noartifacts->value ? 32.0f : 0.0f); // H2

	for (int i = 0; i < count; i++, in++, out++)
	{
		for (int j = 0; j < 3; j++)
		{
			out->minmaxs[j + 0] = LittleShort(in->mins[j]) - offset; // H2: +offset
			out->minmaxs[j + 3] = LittleShort(in->maxs[j]) + offset; // H2: +offset
		}

		out->plane = loadmodel->planes + LittleLong(in->planenum);

		out->firstsurface = LittleShort(in->firstface);
		out->numsurfaces = LittleShort(in->numfaces);
		out->contents = -1;	// Differentiate from leafs.

		for (int j = 0; j < 2; j++)
		{
			const int child_index = LittleLong(in->children[j]);
			if (child_index >= 0)
				out->children[j] = loadmodel->nodes + child_index;
			else
				out->children[j] = (mnode_t*)(loadmodel->leafs + (-1 - child_index));
		}
	}

	// Set nodes and leafs.
	Mod_SetParent(loadmodel->nodes, NULL);
}

// Q2 counterpart
static float RadiusFromBounds(const vec3_t mins, const vec3_t maxs)
{
	vec3_t corner;

	for (int i = 0; i < 3; i++)
		corner[i] = max(fabsf(mins[i]), fabsf(maxs[i]));

	return VectorLength(corner);
}

// Q2 counterpart
static void Mod_LoadSubmodels(model_t* loadmodel, byte* mod_base, const lump_t* l)
{
	dmodel_t* in = (void*)(mod_base + l->fileofs);
	if (l->filelen % sizeof(dmodel_t))
		ri.Sys_Error(ERR_DROP, "Mod_LoadSubmodels: funny lump size in '%s'", loadmodel->name);

	const int count = l->filelen / (int)sizeof(dmodel_t);
	mmodel_t* out = Hunk_Alloc(count * (int)sizeof(mmodel_t));

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		for (int j = 0; j < 3; j++)
		{
			// Spread the mins / maxs by a pixel.
			out->mins[j] = LittleFloat(in->mins[j]) - 1;
			out->maxs[j] = LittleFloat(in->maxs[j]) + 1;
			out->origin[j] = LittleFloat(in->origin[j]);
		}

		out->radius = RadiusFromBounds(out->mins, out->maxs);
		out->headnode = LittleLong(in->headnode);
		out->firstface = LittleLong(in->firstface);
		out->numfaces = LittleLong(in->numfaces);
	}
}

// Q2 counterpart
static void Mod_LoadBrushModel(model_t* mod, void* buffer)
{
	mod->type = mod_brush;
	if (mod != mod_known)
		ri.Sys_Error(ERR_DROP, "Loaded a brush model after the world");

	dheader_t* header = buffer;

	if (header->version != BSPVERSION)
		ri.Sys_Error(ERR_DROP, "Mod_LoadBrushModel: '%s' has wrong version number (%i should be %i)", mod->name, header->version, BSPVERSION);

	// Swap all the lumps.
	byte* mod_base = (byte*)header;

	for (uint i = 0; i < sizeof(dheader_t) / 4; i++)
		((int*)header)[i] = LittleLong(((int*)header)[i]);

	Mod_LoadVertexes(mod, mod_base, &header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges(mod, mod_base, &header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges(mod, mod_base, &header->lumps[LUMP_SURFEDGES]);
	Mod_LoadLighting(mod, mod_base, &header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes(mod, mod_base, &header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo(mod, mod_base, &header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces(mod, mod_base, &header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces(mod, mod_base, &header->lumps[LUMP_LEAFFACES]);
	Mod_LoadVisibility(mod, mod_base, &header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs(mod, mod_base, &header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes(mod, mod_base, &header->lumps[LUMP_NODES]);
	Mod_LoadSubmodels(mod, mod_base, &header->lumps[LUMP_MODELS]);

	// Set up the submodels.
	for (int i = 0; i < mod->numsubmodels; i++)
	{
		const mmodel_t* bm = &mod->submodels[i];
		model_t* starmod = &mod_inline[i];

		*starmod = *mod;

		starmod->firstmodelsurface = bm->firstface;
		starmod->nummodelsurfaces = bm->numfaces;
		starmod->firstnode = bm->headnode;

		if (starmod->firstnode >= mod->numnodes)
			ri.Sys_Error(ERR_DROP, "Inline model %i has bad firstnode", i);

		VectorCopy(bm->maxs, starmod->maxs);
		VectorCopy(bm->mins, starmod->mins);
		starmod->radius = bm->radius;

		if (i == 0)
			*mod = *starmod;

		mod_inline[i].numleafs = bm->visleafs;
	}
}

#pragma endregion

static model_t* Mod_ForName(const char* name, const qboolean crash)
{
	int i;
	model_t* mod;

	if (!name[0])
		ri.Sys_Error(ERR_DROP, "Mod_ForName: NULL name");

	// Inline models are grabbed only from worldmodel.
	if (name[0] == '*')
	{
		const int index = (int)strtol(name + 1, NULL, 10); //mxd. atoi -> strtol
		if (index < 1 || r_worldmodel == NULL || index >= r_worldmodel->numsubmodels)
			ri.Sys_Error(ERR_DROP, "Mod_ForName: bad inline model number");

		return &mod_inline[index];
	}

	// Search the currently loaded models.
	for (i = 0, mod = &mod_known[0]; i < mod_numknown; i++, mod++)
		if (mod->name[0] != 0 && strcmp(mod->name, name) == 0)
			return mod;

	// Find a free model slot.
	for (i = 0, mod = &mod_known[0]; i < mod_numknown; i++, mod++)
		if (mod->name[0] == 0)
			break; // Free slot.

	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			ri.Sys_Error(ERR_DROP, "Mod_ForName: mod_numknown == MAX_MOD_KNOWN");

		mod_numknown++;
	}

	strcpy_s(mod->name, sizeof(mod->name), name); //mxd. strcpy -> strcpy_s

	// Load the file.
	char* buf;
	const int modfilelen = ri.FS_LoadFile(mod->name, (void**)&buf);

	if (buf == NULL)
	{
		if (crash)
			ri.Sys_Error(ERR_DROP, "Mod_ForName: '%s' not found", mod->name);

		memset(mod, 0, sizeof(mod->name));
		return NULL;
	}

	// H2: check for FlexModel header...
	if (modfilelen > 6 && Q_strncasecmp(buf, "header", 6) == 0)
	{
		int datasize = 0x200000;
		if (strstr(name, "players/") || strstr(name, "models/player/"))
			datasize = 0x400000;

		mod->extradata = Hunk_Begin(datasize);
		Mod_LoadFlexModel(mod, buf, modfilelen);
	}
	else
	{
		// Call the appropriate loader.
		switch (LittleLong(*(uint*)buf))
		{
			// Missing: case IDALIASHEADER
			case IDSPRITEHEADER:
				mod->extradata = Hunk_Begin(0x10000);
				Mod_LoadSpriteModel(mod, buf, modfilelen);
				break;

			case IDBOOKHEADER: // H2
				mod->extradata = Hunk_Begin(0x10000);
				Mod_LoadBookModel(mod, buf, modfilelen);
				break;

			case IDBSPHEADER:
				mod->extradata = Hunk_Begin(0x1000000);
				Mod_LoadBrushModel(mod, buf);
				break;

			default:
				ri.Sys_Error(ERR_DROP, "Mod_ForName: unknown file id for '%s'", mod->name);
				break;
		}
	}

	mod->extradatasize = Hunk_End();
	ri.FS_FreeFile(buf);

	return mod;
}

void RI_BeginRegistration(const char* model)
{
	char fullname[MAX_QPATH];

	registration_sequence++;
	r_oldviewcluster = -1; // Force markleafs.

	Com_sprintf(fullname, sizeof(fullname), "maps/%s.bsp", model);

	// Explicitly free the old map if different. This guarantees that mod_known[0] is the world map.
	const cvar_t* flushmap = ri.Cvar_Get("flushmap", "0", 0);
	if (strcmp(mod_known[0].name, fullname) != 0 || (int)flushmap->value)
		Mod_Free(mod_known);

	r_worldmodel = Mod_ForName(fullname, true);
	r_viewcluster = -1;

	R_FreeUnusedImages(); // H2
}

struct model_s* RI_RegisterModel(const char* name)
{
	char img_name[MAX_OSPATH];

	model_t* mod = Mod_ForName(name, false);

	if (mod == NULL)
		return NULL;

	mod->registration_sequence = registration_sequence;

	switch (mod->type) //mxd. No mod_alias case.
	{
		case mod_brush:
			for (int i = 0; i < mod->numtexinfo; i++)
				mod->texinfo[i].image->registration_sequence = registration_sequence;
			break;

		case mod_sprite:
		{
			dsprite_t* sprout = mod->extradata;
			for (int i = 0; i < sprout->numframes; i++)
			{
				Com_sprintf(img_name, sizeof(img_name), "Sprites/%s", sprout->frames[i].name); // H2: extra "Sprites/" prefix
				mod->skins[i] = R_FindImage(img_name, it_sprite);
			}
		} break;

		case mod_fmdl: // H2
			Mod_RegisterFlexModel(mod);
			break;

		case mod_book: // H2
		{
			book_t* book = mod->extradata;
			bookframe_t* bframe = book->bframes;
			for (int i = 0; i < book->bheader.num_segments; i++, bframe++)
			{
				Com_sprintf(img_name, sizeof(img_name), "Book/%s", bframe->name);
				mod->skins[i] = R_FindImage(img_name, it_pic);
			}
		} break;

		default:
			ri.Sys_Error(ERR_DROP, "RI_RegisterModel '%s' failed\n", name); //mxd. Sys_Error() -> ri.Sys_Error().
			return NULL;
	}

	return mod;
}

// Q2 counterpart
void RI_EndRegistration(void)
{
	model_t* mod = &mod_known[0];
	for (int i = 0; i < mod_numknown; i++, mod++)
		if (!mod->name[0] && mod->registration_sequence != registration_sequence)
			Mod_Free(mod); // Don't need this model.

	R_FreeUnusedImages();
}
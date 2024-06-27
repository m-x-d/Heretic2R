//
// gl_model.h
//
// Copyright 1998 Raven Software
//

// d*_t structures are on-disk representations
// m*_t structures are in-memory

#pragma once

#include "gl_local.h"
#include "qfiles.h"
#include "q_shared.h"
#include "q_Typedef.h"

#pragma region ========================== BRUSH MODELS ==========================

// Q2 counterpart
typedef struct
{
	vec3_t position;
} mvertex_t;

// Q2 counterpart
typedef struct
{
	vec3_t mins;
	vec3_t maxs;
	vec3_t origin;	// For sounds or lights
	float radius;
	int headnode;
	int visleafs;	// Not including the solid leaf 0
	int firstface;
	int numfaces;
} mmodel_t;

#define SURF_PLANEBACK		2
#define SURF_DRAWTURB		16

// Q2 counterpart
typedef struct
{
	ushort v[2];
	uint cachededgeoffset;
} medge_t;

// Q2 counterpart
typedef struct mtexinfo_s
{
	float vecs[2][4];
	int flags;
	int numframes;
	struct mtexinfo_s* next; // Animation chain
	image_t* image;
} mtexinfo_t;

#define	VERTEXSIZE	7

// Q2 counterpart
typedef struct glpoly_s
{
	struct glpoly_s* next;
	struct glpoly_s* chain;
	int numverts;
	int flags;
	float verts[4][VERTEXSIZE]; // Variable sized (xyz s1t1 s2t2)
} glpoly_t;

// Q2 counterpart
typedef struct msurface_s
{
	int visframe; // Should be drawn when node is crossed

	cplane_t* plane;
	int flags;

	int firstedge; // Look up in model->surfedges[], negative numbers are backwards edges
	int numedges;

	short texturemins[2];
	short extents[2];

	int light_s; // gl lightmap coordinates
	int light_t;

	int dlight_s; // gl lightmap coordinates for dynamic lightmaps
	int dlight_t;

	glpoly_t* polys; // Multiple if warped
	struct msurface_s* texturechain;
	struct msurface_s* lightmapchain;

	mtexinfo_t* texinfo;

	// Lighting info
	int dlightframe;
	int dlightbits;

	int lightmaptexturenum;
	byte styles[MAXLIGHTMAPS];
	float cached_light[MAXLIGHTMAPS]; // Values currently used in lightmap
	byte* samples; // [numstyles * surfsize]
} msurface_t;

// Q2 counterpart
typedef struct mnode_s
{
	// Common with leaf
	int contents; // -1, to differentiate from leafs
	int visframe; // Node needs to be traversed if current

	float minmaxs[6]; // For bounding box culling

	struct mnode_s* parent;

	// Node specific
	cplane_t* plane;
	struct mnode_s* children[2];

	ushort firstsurface;
	ushort numsurfaces;
} mnode_t;

// Q2 counterpart
typedef struct mleaf_s
{
	// Common with node
	int contents; // Will be a negative contents number
	int visframe; // Node needs to be traversed if current

	float minmaxs[6]; // For bounding box culling

	struct mnode_t* parent;

	// Leaf specific
	int cluster;
	int area;

	msurface_t** firstmarksurface;
	int nummarksurfaces;
} mleaf_t;

#pragma endregion

#pragma region ========================== WHOLE MODEL ==========================

typedef enum
{
	mod_bad,
	mod_brush,
	mod_sprite,
	mod_alias,	//TODO: unused in H2.Remove?
	mod_unknown,//TODO: unused in H2.Remove?
	mod_fmdl,	// New in H2
	mod_book	// New in H2
} modtype_t;

typedef struct model_s
{
	char name[MAX_QPATH];

	int registration_sequence;

	modtype_t type;
	int numframes;

	int flags;

	// Volume occupied by the model graphics
	vec3_t mins;
	vec3_t maxs;
	float radius;

	// Solid volume for clipping 
	qboolean clipbox;
	vec3_t clipmins;
	vec3_t clipmaxs;

	// Brush model
	int firstmodelsurface;
	int nummodelsurfaces;
	int lightmap; // Only for submodels

	int numsubmodels;
	mmodel_t* submodels;

	int numplanes;
	cplane_t* planes;

	int numleafs; // Number of visible leafs, not counting 0
	mleaf_t* leafs;

	int numvertexes;
	mvertex_t* vertexes;

	int numedges;
	medge_t* edges;

	int numnodes;
	int firstnode;
	mnode_t* nodes;

	int numtexinfo;
	mtexinfo_t* texinfo;

	int numsurfaces;
	msurface_t* surfaces;

	int numsurfedges;
	int* surfedges;

	int nummarksurfaces;
	msurface_t** marksurfaces;

	dvis_t* vis;
	byte* lightdata;

	// For models and skins
	image_t* skins[MAX_MD2SKINS];

	int extradatasize;
	void* extradata;

	qboolean skeletal_model; // false - Sprite/Book/Bsp; true - Skeletal model?
} model_t;

#pragma endregion

void Mod_Init(void);
void Mod_Modellist_f(void);
void Mod_Free(model_t* mod);
void Mod_FreeAll(void);
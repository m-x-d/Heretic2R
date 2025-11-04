//
// qfiles.h -- Heretic II file formats. This file must be identical in the Heretic II and utils directories.
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h" //mxd

#define MAX_FRAMES		64
#define MAX_FRAMENAME	64

#pragma region ========================== .PAK ==========================

// The .pak files are just a linear collapse of a directory tree.
#define IDPAKHEADER		(('K' << 24) + ('C' << 16) + ('A' << 8) + 'P')

typedef struct
{
	char name[56];
	int filepos;
	int filelen;
} dpackfile_t;

typedef struct
{
	int ident; // == IDPAKHEADER
	int dirofs;
	int dirlen;
} dpackheader_t;

#define MAX_FILES_IN_PACK	6144 // Q2: 4096

#pragma endregion

#pragma region ========================== .BK ==========================

#define IDBOOKHEADER	(('K' << 24) + ('O' << 16) + ('O' << 8) + 'B')
#define BOOK_VERSION	2

typedef struct bookframe_s
{
	int x;
	int y;
	int w;
	int h;
	char name[MAX_FRAMENAME]; // Name of the gfx file.
} bookframe_t;

typedef struct bookheader_s
{
	uint ident;
	uint version;
	int num_segments;
	int total_w;
	int total_h;
} bookheader_t;

typedef struct book_s
{
	bookheader_t bheader;
	bookframe_t	bframes[MAX_FRAMES];
} book_t;

#pragma endregion

#pragma region ========================== .SP2 sprite file format ==========================

#define IDSPRITEHEADER	(('2' << 24) + ('S' << 16) + ('D' << 8) + 'I') // Little-endian "IDS2".
#define SPRITE_VERSION	2

typedef struct
{
	int width;
	int height;
	int origin_x; // Raster coordinates inside pic.
	int origin_y;
	char name[MAX_FRAMENAME]; // Name of the .sp2 file.
} dsprframe_t;

typedef struct
{
	int ident;
	int version;
	int numframes;
	dsprframe_t frames[1]; // Variable sized (up to MAX_FRAMES).
} dsprite_t;

#pragma endregion

#pragma region ========================== .M8 texture file format ==========================

#define MIP_VERSION		2
#define PAL_SIZE		256
#define	MIPLEVELS		16

typedef struct miptex_s
{
	int version;
	char name[32];
	uint width[MIPLEVELS];
	uint height[MIPLEVELS];
	uint offsets[MIPLEVELS];	// Four mip maps stored.
	char animname[32];			// Next frame in animation chain.
	paletteRGB_t palette[PAL_SIZE];
	int flags;
	int contents;
	int value;
} miptex_t;

#pragma endregion

#pragma region ========================== .M32 texture file format ==========================

#define MIP32_VERSION	4

typedef struct miptex32_s
{
	int version;
	char name[128];
	char altname[128];		// Texture substitution. //TODO: unused.
	char animname[128];		// Next frame in animation chain. //TODO: unused.
	char damagename[128];	// Image that should be shown when damaged. //TODO: unused.
	uint width[MIPLEVELS];
	uint height[MIPLEVELS];
	uint offsets[MIPLEVELS];
	int flags; //TODO: unused.
	int contents; //TODO: unused.
	int num_frames; //mxd. Named 'value' in original logic.
	float scale_x; //TODO: unused.
	float scale_y; //TODO: unused.
	int mip_scale; //TODO: unused.

	// Detail texturing info. //TODO: none of these are unused.
	char dt_name[128];		// Detailed texture name.
	float dt_scale_x;
	float dt_scale_y;
	float dt_u;
	float dt_v;
	float dt_alpha;
	int dt_src_blend_mode;
	int dt_dst_blend_mode;

	int unused[20];			// Future expansion to maintain compatibility with h2 //TODO: ???
} miptex32_t;

#pragma endregion

#pragma region ========================== .BSP file format ==========================

#define IDBSPHEADER	(('P' << 24) + ('S' << 16) + ('B' << 8) + 'I') // Little-endian "IBSP"
#define BSPVERSION	38

// Upper design bounds.
// leaffaces, leafbrushes, planes, and verts are still bounded by 16 bit short limits.
#define MAX_MAP_MODELS		1024
#define MAX_MAP_BRUSHES		10240	//mxd. 8192 in Q2
#define MAX_MAP_ENTITIES	2048
#define MAX_MAP_ENTSTRING	0x40000
#define MAX_MAP_TEXINFO		8192

#define MAX_MAP_AREAS		256
#define MAX_MAP_AREAPORTALS	1024
#define MAX_MAP_PLANES		65536
#define MAX_MAP_NODES		65536
#define MAX_MAP_BRUSHSIDES	65536
#define MAX_MAP_LEAFS		65536
#define MAX_MAP_VERTS		65536
#define MAX_MAP_FACES		65536
#define MAX_MAP_LEAFFACES	65536
#define MAX_MAP_LEAFBRUSHES 65536
#define MAX_MAP_PORTALS		65536
#define MAX_MAP_EDGES		128000
#define MAX_MAP_SURFEDGES	256000
#define MAX_MAP_LIGHTING	0x200000
#define MAX_MAP_VISIBILITY	0x180000

// Key / value pair sizes.
#define MAX_KEY		32
#define MAX_VALUE	1024

typedef struct
{
	int fileofs;
	int filelen;
} lump_t;

#define LUMP_ENTITIES		0
#define LUMP_PLANES			1
#define LUMP_VERTEXES		2
#define LUMP_VISIBILITY		3
#define LUMP_NODES			4
#define LUMP_TEXINFO		5
#define LUMP_FACES			6
#define LUMP_LIGHTING		7
#define LUMP_LEAFS			8
#define LUMP_LEAFFACES		9
#define LUMP_LEAFBRUSHES	10
#define LUMP_EDGES			11
#define LUMP_SURFEDGES		12
#define LUMP_MODELS			13
#define LUMP_BRUSHES		14
#define LUMP_BRUSHSIDES		15
#define LUMP_POP			16
#define LUMP_AREAS			17
#define LUMP_AREAPORTALS	18
#define HEADER_LUMPS		19

typedef struct
{
	int ident;
	int version;
	lump_t lumps[HEADER_LUMPS];
} dheader_t;

typedef struct
{
	float mins[3];
	float maxs[3];
	float origin[3]; // For sounds or lights.
	int headnode;
	int firstface; // Submodels just draw faces without walking the bsp tree.
	int numfaces;
} dmodel_t;

typedef struct
{
	float point[3];
} dvertex_t;

// 0-2 are axial planes.
#define PLANE_X			0
#define PLANE_Y			1
#define PLANE_Z			2

// 3-5 are non-axial planes snapped to the nearest.
#define PLANE_ANYX		3
#define PLANE_ANYY		4
#define PLANE_ANYZ		5

// Planes (x&~1) and (x&~1)+1 are always opposites.
typedef struct
{
	float normal[3];
	float dist;
	int type; // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate.
} dplane_t;

typedef struct
{
	int planenum;
	int children[2];	// Negative numbers are -(leafs+1), not nodes.
	short mins[3];		// For frustum culling.
	short maxs[3];
	ushort firstface;
	ushort numfaces;	// Counting both sides.
} dnode_t;

typedef struct texinfo_s
{
	float vecs[2][4];	// [s/t][xyz offset]
	int flags;			// Miptex flags + overrides.
	int value;			// Light emission, etc.
	char texture[32];	// Texture name (textures/*.wal).
	int nexttexinfo;	// For animations, -1 = end of chain.
} texinfo_t;

// Note that edge 0 is never used, because negative edge nums are used for counterclockwise use of the edge in a face.
typedef struct
{
	ushort v[2]; // Vertex numbers.
} dedge_t;

#define MAXLIGHTMAPS	4

typedef struct
{
	ushort planenum;
	short side;

	int firstedge; // We must support > 64k edges.
	short numedges;	
	short texinfo;

	// Lighting info.
	byte styles[MAXLIGHTMAPS];
	int lightofs; // Start of [numstyles*surfsize] samples.
} dface_t;

typedef struct
{
	int contents; // OR of all brushes (not needed?).

	short cluster;
	short area;

	short mins[3]; // For frustum culling.
	short maxs[3];

	ushort firstleafface;
	ushort numleaffaces;

	ushort firstleafbrush;
	ushort numleafbrushes;
} dleaf_t;

typedef struct
{
	ushort planenum; // Facing out of the leaf.
	short texinfo;
} dbrushside_t;

typedef struct
{
	int firstside;
	int numsides;
	int contents;
} dbrush_t;

// The visibility lump consists of a header with a count, then byte offsets for the PVS and PHS of each cluster,
// then the raw compressed bit vectors.
#define DVIS_PVS	0
#define DVIS_PHS	1

typedef struct
{
	int numclusters;
	int bitofs[8][2]; // bitofs[numclusters][2]
} dvis_t;

// Each area has a list of portals that lead into other areas when portals are closed.
// Other areas may not be visible or hearable even if the vis info says that it should be.
typedef struct
{
	int portalnum;
	int otherarea;
} dareaportal_t;

typedef struct
{
	int numareaportals;
	int firstareaportal;
} darea_t;

#pragma endregion
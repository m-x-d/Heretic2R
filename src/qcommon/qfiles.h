//
// qfiles.h -- quake file formats. This file must be identical in the quake and utils directories
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h" //mxd

#pragma region ========================== .PAK ==========================

// The .pak files are just a linear collapse of a directory tree
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

#define MAX_FILES_IN_PACK	6144

#pragma endregion

#pragma region ========================== .PCX ==========================

// PCX files are used for as many images as possible
//TODO: unused. Remove?
typedef struct
{
	char manufacturer;
	char version;
	char encoding;
	char bits_per_pixel;
	ushort xmin;
	ushort ymin;
	ushort xmax;
	ushort ymax;
	ushort hres;
	ushort vres;
	byte palette[48];
	char reserved;
	char color_planes;
	ushort bytes_per_line;
	ushort palette_type;
	char filler[58];
	byte data; // Unbounded
} pcx_t;

#pragma endregion

#pragma region ========================== .MD2 ==========================

//TODO: unused?
// .MD2 compressed triangle model file format
#define IDCOMPRESSEDALIASHEADER	(('2' << 24) + ('C' << 16) + ('D' << 8) + 'I')

// .MD2 compressed triangle model file format
#define IDJOINTEDALIASHEADER	(('2' << 24) + ('J' << 16) + ('D' << 8) + 'I')

// .MD2 triangle model file format
#define IDALIASHEADER			(('2' << 24) + ('P' << 16) + ('D' << 8) + 'I')
#define ALIAS_VERSION	8

#define MAX_TRIANGLES	4096
#define MAX_VERTS		2048
#define MAX_FRAMES		512
#define MAX_MD2SKINS	64	//mxd. 32 in Q2
#define MAX_SKINNAME	64

typedef struct
{
	short s;
	short t;
} dstvert_t;

typedef struct 
{
	short index_xyz[3];
	short index_st[3];
} dtriangle_t;

typedef struct
{
	union
	{
		struct
		{
			byte v[3]; // Scaled byte to fit in frame mins/maxs
			byte lightnormalindex;
		};

		int vert;
	};
} dtrivertx_t;

#define DTRIVERTX_V0   0
#define DTRIVERTX_V1   1
#define DTRIVERTX_V2   2
#define DTRIVERTX_LNI  3
#define DTRIVERTX_SIZE 4

typedef struct
{
	float scale[3];			// Multiply byte verts by this...
	float translate[3];		// ...then add this
	char name[16];			// Frame name from grabbing.
	dtrivertx_t verts[1];	// Variable sized.
} daliasframe_t;


// The glcmd format:
// A positive integer starts a tristrip command, followed by that many vertex structures.
// A negative integer starts a trifan command, followed by -x vertexes.
// A zero indicates the end of the command list.
// A vertex consists of a floating point s, a floating point t, and an integer vertex index.

typedef struct
{
	int ident;
	int version;

	int skinwidth;
	int skinheight;
	int framesize;	// Byte size of each frame

	int num_skins;
	int num_xyz;
	int num_st;		// Greater than num_xyz for seams
	int num_tris;
	int num_glcmds;	// dwords in strip/fan command list
	int num_frames;

	int ofs_skins;	// Each skin is a MAX_SKINNAME string
	int ofs_st;		// Byte offset from start for stverts
	int ofs_tris;	// Offset for dtriangles
	int ofs_frames;	// Offset for first frame
	int ofs_glcmds;	
	int ofs_end;	// End of file
} dmdl_t;

// Compressed model
typedef struct dcompmdl_s
{
	dmdl_t header;
	short CompressedFrameSize;
	short UniqueVerts;
	short* remap;
	float* translate;
	float* scale;	// Multiply byte verts by this
	char* mat;
	char* frames;
	char* base;
	float* ctranslate;
	float* cscale;	
	char data[1];	// Variable sized.
} dcompmdl_t;

typedef struct 
{
	dcompmdl_t compModInfo;
	int rootCluster;
	int skeletalType;
	struct ModelSkeleton_s* skeletons;
} JointedModel_t;

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
	char name[MAX_SKINNAME]; // Name of gfx file
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
	bookframe_t	bframes[MAX_MD2SKINS];
} book_t;

#pragma endregion

#pragma region ========================== .SP2 sprite file format ==========================

#define IDSPRITEHEADER	(('2' << 24) + ('S' << 16) + ('D' << 8) + 'I') // little-endian "IDS2"
#define SPRITE_VERSION	2

typedef struct
{
	int width;
	int height;
	int origin_x; // Raster coordinates inside pic
	int origin_y;
	char name[MAX_SKINNAME]; // Name of pcx file
} dsprframe_t;

typedef struct
{
	int ident;
	int version;
	int numframes;
	dsprframe_t frames[1]; // Variable sized
} dsprite_t;

#pragma endregion

#pragma region ========================== .M8 texture file format ==========================

typedef struct palette_s
{
	union
	{
		struct
		{
			byte r;
			byte g;
			byte b;
		};
	};
} palette_t;

#define MIP_VERSION		2
#define PAL_SIZE		256
#define	MIPLEVELS		16

typedef struct miptex_s
{
	int version;
	char name[32];
	uint width[MIPLEVELS];
	uint height[MIPLEVELS];
	uint offsets[MIPLEVELS];	// Four mip maps stored
	char animname[32];			// Next frame in animation chain
	palette_t palette[PAL_SIZE];
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
	char altname[128];		// Texture substitution
	char animname[128];		// Next frame in animation chain
	char damagename[128];	// Image that should be shown when damaged
	uint width[MIPLEVELS];
	uint height[MIPLEVELS];
	uint offsets[MIPLEVELS];
	int flags;
	int contents;
	int value;
	float scale_x;
	float scale_y;
	int mip_scale;

	// Detail texturing info
	char dt_name[128];		// Detailed texture name
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

// Upper design bounds
// leaffaces, leafbrushes, planes, and verts are still bounded by 16 bit short limits
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

// Key / value pair sizes
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
	float origin[3]; // For sounds or lights
	int headnode;
	int firstface; // Submodels just draw faces without walking the bsp tree
	int numfaces;
} dmodel_t;

typedef struct
{
	float point[3];
} dvertex_t;

// 0-2 are axial planes
#define PLANE_X			0
#define PLANE_Y			1
#define PLANE_Z			2

// 3-5 are non-axial planes snapped to the nearest
#define PLANE_ANYX		3
#define PLANE_ANYY		4
#define PLANE_ANYZ		5

// planes (x&~1) and (x&~1)+1 are allways opposites
typedef struct
{
	float normal[3];
	float dist;
	int type; // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
} dplane_t;

// Contents flags are separate bits.
// A given brush can contribute multiple content bits.
// Multiple brushes can be in a single leaf.

// These definitions also need to be in q_shared.h!

// Contents flags. Lower bits are stronger, and will eat weaker brushes completely.

#define CONTENTS_SOLID			0x00000001	// An eye is never valid in a solid.
#define CONTENTS_WINDOW			0x00000002	// Translucent, but not watery.
#define CONTENTS_AUX			0x00000004
#define CONTENTS_LAVA			0x00000008
#define CONTENTS_SLIME			0x00000010
#define CONTENTS_WATER			0x00000020
#define CONTENTS_MIST			0x00000040
#define LAST_VISIBLE_CONTENTS	CONTENTS_MIST

// Remaining contents are non-visible, and don't eat brushes.
#define CONTENTS_AREAPORTAL		0x00008000
#define CONTENTS_PLAYERCLIP		0x00010000
#define CONTENTS_MONSTERCLIP	0x00020000

// Currents can be added to any other contents, and may be mixed.
#define CONTENTS_CURRENT_0		0x00040000
#define CONTENTS_CURRENT_90		0x00080000
#define CONTENTS_CURRENT_180	0x00100000
#define CONTENTS_CURRENT_270	0x00200000
#define CONTENTS_CURRENT_UP		0x00400000
#define CONTENTS_CURRENT_DOWN	0x00800000
#define CONTENTS_ORIGIN			0x01000000	// Removed before bsping an entity.
#define CONTENTS_MONSTER		0x02000000	// Should never be on a brush, only in game.
#define CONTENTS_DEADMONSTER	0x04000000
#define CONTENTS_DETAIL			0x08000000	// Brushes to be added after vis leaves.
#define CONTENTS_TRANSLUCENT	0x10000000	// Auto set if any surface has transparency.
#define CONTENTS_LADDER			0x20000000
#define CONTENTS_CAMERANOBLOCK	0x40000000	// Camera LOS ignores any brushes with this flag.

typedef struct
{
	int planenum;
	int children[2];	// Negative numbers are -(leafs+1), not nodes
	short mins[3];		// For frustum culling
	short maxs[3];
	ushort firstface;
	ushort numfaces;	// Counting both sides
} dnode_t;

typedef struct texinfo_s
{
	float vecs[2][4];	// [s/t][xyz offset]
	int flags;			// Miptex flags + overrides
	int value;			// Light emission, etc
	char texture[32];	// Texture name (textures/*.wal)
	int nexttexinfo;	// For animations, -1 = end of chain
} texinfo_t;

// Note that edge 0 is never used, because negative edge nums are used for counterclockwise use of the edge in a face.
typedef struct
{
	ushort v[2]; // Vertex numbers
} dedge_t;

#define MAXLIGHTMAPS	4
typedef struct
{
	ushort planenum;
	short side;

	int firstedge; // We must support > 64k edges
	short numedges;	
	short texinfo;

	// Lighting info
	byte styles[MAXLIGHTMAPS];
	int lightofs; // Start of [numstyles*surfsize] samples
} dface_t;

typedef struct
{
	int contents; // OR of all brushes (not needed?)

	short cluster;
	short area;

	short mins[3]; // For frustum culling
	short maxs[3];

	ushort firstleafface;
	ushort numleaffaces;

	ushort firstleafbrush;
	ushort numleafbrushes;
} dleaf_t;

typedef struct
{
	ushort planenum; // Facing out of the leaf
	short texinfo;
} dbrushside_t;

typedef struct
{
	int firstside;
	int numsides;
	int contents;
} dbrush_t;

#define ANGLE_UP	-1
#define ANGLE_DOWN	-2

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
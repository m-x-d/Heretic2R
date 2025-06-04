//
// fmodel.h -- .FM triangle flexible model file format
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h" //mxd
#include "FlexModel.h" //mxd
#include "ref.h" //mxd
#include "Reference.h" //mxd

#define	MAX_FM_TRIANGLES	2048
#define MAX_FM_VERTS		2048
#define MAX_FM_FRAMES		2048

//#define SKINPAGE_WIDTH		640
#define SKINPAGE_HEIGHT		480

// Skin Header
#define FM_SKIN_NAME	"skin"
#define FM_SKIN_VER		1

// ST Coord Header
#define FM_ST_NAME		"st coord"
#define FM_ST_VER		1

typedef struct
{
	short s;
	short t;
} fmstvert_t;

// Tri Header
#define FM_TRI_NAME		"tris"
#define FM_TRI_VER		1

typedef struct 
{
	short index_xyz[3];
	short index_st[3];
} fmtriangle_t;

// Frame Header
#define FM_FRAME_NAME	"frames"
#define FM_FRAME_VER	1

// Frame for compression, just the names
#define FM_SHORT_FRAME_NAME	"short frames"
#define FM_SHORT_FRAME_VER	1

// Normals for compressed frames
#define FM_NORMAL_NAME	"normals"
#define FM_NORMAL_VER	1

// Compressed Frame Data
#define FM_COMP_NAME	"comp data"
#define FM_COMP_VER	1

// GL Cmds Header
#define FM_GLCMDS_NAME	"glcmds"
#define FM_GLCMDS_VER	1

// Mesh Nodes Header
#define FM_MESH_NAME	"mesh nodes"
#define FM_MESH_VER		3

// Skeleton Header
#define FM_SKELETON_NAME "skeleton"
#define FM_SKELETON_VER	1

// References Header
#define FM_REFERENCES_NAME "references"
#define FM_REFERENCES_VER	1

typedef struct
{
	union
	{
		byte tris[MAX_FM_TRIANGLES >> 3]; // 2048 >> 3 == 256

		struct
		{
			short* triIndicies;
			int num_tris;
		};
	};

	byte verts[MAX_FM_VERTS >> 3]; // 2048 >> 3 == 256
	short start_glcmds;
	short num_glcmds;
} fmmeshnode_t;

//=================================================================

// Frame info
typedef struct
{
	byte v[3]; // Scaled byte to fit in frame mins/maxs
	byte lightnormalindex;
} fmtrivertx_t;

typedef struct
{
	float scale[3];			// Multiply byte verts by this
	float translate[3];		// Then add this
	char name[16];			// Frame name from grabbing
	fmtrivertx_t verts[1];	// Variable sized
} fmaliasframe_t;

typedef struct
{
	int start_frame;
	int num_frames;
	int degrees;
	char* mat;
	char* ccomp;
	byte* cbase;
	float* cscale;
	float* coffset;
	float trans[3];
	float scale[3];
	float bmin[3];
	float bmax[3];
	float* complerp;
} fmgroup_t;

typedef struct fmdl_s
{
	fmheader_t header;
	fmstvert_t* st_verts;
	fmtriangle_t* tris;
	fmaliasframe_t* frames;
	int* glcmds;
	char* skin_names;
	fmmeshnode_t* mesh_nodes;

	// Compression stuff
	int ngroups;
	fmgroup_t* compdata;
	int* frame_to_group;
	char* framenames;
	byte* lightnormalindex;

	int skeletalType;
	int rootCluster;
	struct ModelSkeleton_s* skeletons;

	int referenceType;
	Placement_t* refsForFrame; //mxd. 'struct M_Reference_s*' in original logic.
} fmdl_t;

// Flex Model Loading Routines
extern fmdl_t* fmodel;

extern void Mod_LoadFlexModel(struct model_s* mod, void* buffer, int length);
extern void Mod_RegisterFlexModel(struct model_s* mod);
extern void R_DrawFlexModel(entity_t* e);

extern void GL_LerpVert(vec3_t newPoint, vec3_t oldPoint, vec3_t interpolatedPoint, float move[3], float frontv[3], float backv[3]);
//
// gl1_FlexModel.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"
#include "FlexModel.h"
#include "Reference.h"

#define	MAX_FM_TRIANGLES	2048
#define MAX_FM_VERTS		2048
#define MAX_FM_FRAMES		2048

#define SKINPAGE_WIDTH		640
#define SKINPAGE_HEIGHT		480

// Skin header.
#define FM_SKIN_NAME		"skin"
#define FM_SKIN_VER			1

// ST coord header.
#define FM_ST_NAME			"st coord"
#define FM_ST_VER			1

// Tri header.
#define FM_TRI_NAME			"tris"
#define FM_TRI_VER			1

// Frame header.
#define FM_FRAME_NAME		"frames"
#define FM_FRAME_VER		1

// Frame for compression, just the names.
#define FM_SHORT_FRAME_NAME	"short frames"
#define FM_SHORT_FRAME_VER	1

// Normals for compressed frames.
#define FM_NORMAL_NAME		"normals"
#define FM_NORMAL_VER		1

// Compressed frame data.
#define FM_COMP_NAME		"comp data"
#define FM_COMP_VER			1

// GLCmds header.
#define FM_GLCMDS_NAME		"glcmds"
#define FM_GLCMDS_VER		1

// Mesh nodes header.
#define FM_MESH_NAME		"mesh nodes"
#define FM_MESH_VER			3

// Skeleton header.
#define FM_SKELETON_NAME	"skeleton"
#define FM_SKELETON_VER		1

// References header.
#define FM_REFERENCES_NAME	"references"
#define FM_REFERENCES_VER	1

typedef struct
{
	short s;
	short t;
} fmstvert_t;

typedef struct
{
	short index_xyz[3];
	short index_st[3];
} fmtriangle_t;

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

// Frame info.
typedef struct
{
	byte v[3]; // Scaled byte to fit in frame mins/maxs.
	byte lightnormalindex;
} fmtrivertx_t;

typedef struct
{
	float scale[3];			// Multiply byte verts by this.
	float translate[3];		// Then add this.
	char name[16];			// Frame name from grabbing.
	fmtrivertx_t verts[1];	// Variable sized.
} fmaliasframe_t;

typedef struct
{
	int start_frame; //TODO: unused.
	int num_frames; //TODO: unused.
	int degrees; //TODO: unused.
	char* mat; //TODO: unused.
	char* ccomp; //TODO: unused.
	byte* cbase; //TODO: unused.
	float* cscale; //TODO: unused.
	float* coffset; //TODO: unused.
	float trans[3]; //TODO: unused.
	float scale[3]; //TODO: unused.
	float bmin[3];
	float bmax[3];
	float* complerp; //TODO: unused.
} fmgroup_t;

typedef struct fmdl_s
{
	fmheader_t header;
	fmstvert_t* st_verts; //TODO: unused.
	fmtriangle_t* tris; //TODO: unused.
	fmaliasframe_t* frames;
	int* glcmds;
	char* skin_names;
	fmmeshnode_t* mesh_nodes;

	// Compression stuff.
	int ngroups; //TODO: unused.
	fmgroup_t* compdata;
	int* frame_to_group;
	char* framenames; //TODO: unused.
	byte* lightnormalindex;

	int skeletalType;
	int rootCluster;
	struct ModelSkeleton_s* skeletons;

	int referenceType;
	Placement_t* refsForFrame; //mxd. 'struct M_Reference_s*' in original logic.
} fmdl_t;

extern fmdl_t* fmodel;

// Flex model loading.
extern void Mod_LoadFlexModel(model_t* mod, void* buffer, int length);
extern void Mod_RegisterFlexModel(model_t* mod);

// Flex model rendering.
extern void R_DrawFlexModel(entity_t* e);
extern void R_LerpVert(const vec3_t new_point, const vec3_t old_point, vec3_t interpolated_point, const float move[3], const float frontv[3], const float backv[3]);
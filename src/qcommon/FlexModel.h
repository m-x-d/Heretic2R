//
// FlexModel.h -- mxd. Needed here to make CL_RequestNextDownload() work...
//
// Copyright 1998 Raven Software
//

#pragma once

//mxd. FlexModel on-disk block header. Named 'header_t' and stored in qcommon\flex.h in Heretic II Toolkit v1.06.
#define FMDL_BLOCK_IDENT_SIZE 32

typedef struct
{
	char ident[FMDL_BLOCK_IDENT_SIZE];
	int version;
	int size;
} fmdl_blockheader_t;

// Initial Header
#define FM_HEADER_NAME	"header"
#define FM_HEADER_VER	2

typedef struct
{
	int skinwidth;
	int skinheight;
	int framesize;		// Byte size of each frame

	int num_skins;
	int num_xyz;
	int num_st;			// Greater than num_xyz for seams
	int num_tris;
	int num_glcmds;		// dwords in strip/fan command list
	int num_frames;
	int num_mesh_nodes;
} fmheader_t;
//
// fmodel.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "fmodel.h"

fmdl_t* fmodel;

static qboolean fmLoadHeader(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadSkin(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadST(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadTris(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadFrames(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadGLCmds(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadMeshNodes(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadShortFrames(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadNormal(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadComp(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadSkeleton(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

static qboolean fmLoadReferences(model_t* model, int version, int datasize, void* buffer)
{
	NOT_IMPLEMENTED
	return false;
}

//mxd. FlexModel on-disk block header. Named 'header_t' and stored in qcommon\flex.h in Heretic II Toolkit v1.06.
#define FMDL_BLOCK_IDENT_SIZE 32

typedef struct
{
	char ident[FMDL_BLOCK_IDENT_SIZE];
	int version;
	int size;
} fmdl_blockheader_t;

// FlexModel block loaders
typedef struct
{
	char ident[FMDL_BLOCK_IDENT_SIZE];
	qboolean(*load)(model_t* model, int version, int datasize, void* buffer);
} fmdl_loader_t;

fmdl_loader_t fmblocks[] =
{
	{ FM_HEADER_NAME,			fmLoadHeader },
	{ FM_SKIN_NAME,			fmLoadSkin },
	{ FM_ST_NAME,				fmLoadST },
	{ FM_TRI_NAME,			fmLoadTris },
	{ FM_FRAME_NAME,			fmLoadFrames },
	{ FM_GLCMDS_NAME,			fmLoadGLCmds },
	{ FM_MESH_NAME,			fmLoadMeshNodes },
	{ FM_SHORT_FRAME_NAME,	fmLoadShortFrames },
	{ FM_NORMAL_NAME,			fmLoadNormal },
	{ FM_COMP_NAME,			fmLoadComp },
	{ FM_SKELETON_NAME,		fmLoadSkeleton },
	{ FM_REFERENCES_NAME,		fmLoadReferences },
	{ "",					NULL },
};

void Mod_LoadFlexModel(model_t* mod, void* buffer, int length)
{
	mod->type = mod_fmdl;

	fmodel = Hunk_Alloc(sizeof(fmdl_t));
	fmodel->skeletalType = -1;
	fmodel->referenceType = -1;

	byte* in = buffer;

	while (length > 0)
	{
		fmdl_blockheader_t* header = (fmdl_blockheader_t*)in;
		in += sizeof(fmdl_blockheader_t); // Block data is stored after block header.

		// Find appropriate loader...
		fmdl_loader_t* loader;
		for (loader = fmblocks; loader->ident[0] != 0; loader++)
		{
			if (Q_stricmp(loader->ident, header->ident) == 0)
			{
				if (!loader->load(mod, header->version, header->size, in)) //mxd. Added sanity check
					Com_Error(ERR_DROP,"Mod_LoadFlexModel: failed to load block %s\n", header->ident);

				break;
			}
		}

		if (loader->ident[0] == 0)
			ri.Con_Printf(0, "Mod_LoadFlexModel: unknown block %s\n", header->ident);

		in += header->size; // Skip to next block header...
		length -= (header->size + (int)sizeof(fmdl_blockheader_t));
	}
}

void Mod_RegisterFlexModel(model_t* mod)
{
	NOT_IMPLEMENTED
}

void R_DrawFlexModel(entity_t* e)
{
	NOT_IMPLEMENTED
}

void GL_LerpVert(vec3_t newPoint, vec3_t oldPoint, vec3_t interpolatedPoint, float move[3], float frontv[3], float backv[3])
{
	NOT_IMPLEMENTED
}
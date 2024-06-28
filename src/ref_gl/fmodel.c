//
// fmodel.c -- Heretic 2 FlexModel loading
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "fmodel.h"
#include "Vector.h"

fmdl_t* fmodel;

static qboolean fmLoadHeader(model_t* model, const int version, const int datasize, const void* buffer)
{
	if (version != FM_HEADER_VER)
		ri.Sys_Error(ERR_DROP, "invalid HEADER version for block %s: %d != %d\n", FM_HEADER_NAME, FM_HEADER_VER, version);

	// Read header...
	memcpy(&fmodel->header, buffer, sizeof(fmheader_t));

	// Sanity checks...
	if (fmodel->header.skinheight > SKINPAGE_HEIGHT)
		ri.Sys_Error(ERR_DROP, "model %s has a skin taller than %d", model->name, SKINPAGE_HEIGHT);

	if (fmodel->header.num_xyz < 1)
		ri.Sys_Error(ERR_DROP, "model %s has no vertices", model->name);

	if (fmodel->header.num_xyz > MAX_FM_VERTS)
		ri.Sys_Error(ERR_DROP, "model %s has too many vertices", model->name);

	if (fmodel->header.num_st < 1)
		ri.Sys_Error(ERR_DROP, "model %s has no st vertices", model->name);

	if (fmodel->header.num_tris < 1)
		ri.Sys_Error(ERR_DROP, "model %s has no triangles", model->name);

	if (fmodel->header.num_frames < 1)
		ri.Sys_Error(ERR_DROP, "model %s has no frames", model->name);

	//TODO: skinwidth, MAX_FM_TRIANGLES, MAX_FM_FRAMES checks?

	VectorSet(model->mins, -32.0f, -32.0f, -32.0f);
	VectorSet(model->maxs, 32.0f, 32.0f, 32.0f);

	return true;
}

static qboolean fmLoadSkin(model_t* model, const int version, const int datasize, const void* buffer)
{
	if (version != FM_SKIN_VER)
		ri.Sys_Error(ERR_DROP, "invalid SKIN version for block %s: %d != %d\n", FM_SKIN_NAME, FM_SKIN_VER, version);

	const int skin_names_size = fmodel->header.num_skins * MAX_QPATH;
	if (skin_names_size != datasize)
	{
		ri.Con_Printf(PRINT_ALL, "skin sizes do not match: %d != %d\n", datasize, skin_names_size);
		return false;
	}

	fmodel->skin_names = (char*)Hunk_Alloc(skin_names_size);
	memcpy(fmodel->skin_names, buffer, skin_names_size);

	// Precache skins...
	char* skin_name = fmodel->skin_names;
	for (int i = 0; i < fmodel->header.num_skins; i++, skin_name += MAX_QPATH)
		model->skins[i] = GL_FindImage(skin_name, it_skin);

	return true;
}

static qboolean fmLoadST(model_t* model, const int version, const int datasize, const void* buffer)
{
	return true;
}

static qboolean fmLoadTris(model_t* model, const int version, const int datasize, const void* buffer)
{
	return true;
}

static qboolean fmLoadFrames(model_t* model, const int version, const int datasize, const void* buffer)
{
	if (version != FM_FRAME_VER)
		ri.Sys_Error(ERR_DROP, "invalid FRAMES version for block %s: %d != %d\n", FM_FRAME_NAME, FM_FRAME_VER, version);

	fmodel->frames = Hunk_Alloc(fmodel->header.num_frames * fmodel->header.framesize);

	for (int i = 0; i < fmodel->header.num_frames; i++)
	{
		const fmaliasframe_t* in = (const fmaliasframe_t*)((const byte*)buffer + i * fmodel->header.framesize);
		fmaliasframe_t* out = &fmodel->frames[i];

		VectorCopy(in->scale, out->scale);
		VectorCopy(in->translate, out->translate);

		memcpy(out->name, in->name, sizeof(out->name));
		memcpy(out->verts, in->verts, fmodel->header.num_xyz * sizeof(fmtrivertx_t));
	}

	return true;
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
			ri.Con_Printf(PRINT_ALL, "Mod_LoadFlexModel: unknown block %s\n", header->ident);

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
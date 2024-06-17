//
// SurfaceProps.c
//
// Copyright 1998 Raven Software
//

#include "SurfaceProps.h"
#include "q_shared.h" //TODO: remove (referenced in p_types.h)

//TODO: Why are you like this?..
char* SurfaceMaterialNames[MAX_SURFACE_MAT_NAMES] =
{
	"gravel", "metal", "stone",	"wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone",	"wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone",	"wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone",	"wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
	"gravel", "metal", "stone", "wood",
};

//TODO: mxd. Seems to be used from Player.dll only. Move there?
//TODO: mxd. GROSS HACKS (because I don't want to include Player.dll sources just yet...)
//H2COMMON_API char* GetClientGroundSurfaceMaterialName(const playerinfo_t* playerinfo)
H2COMMON_API char* GetClientGroundSurfaceMaterialName(const void* playerinfo)
{
	//const csurface_t* surf = playerinfo->GroundSurface;
	csurface_t* surf = *(csurface_t**)((int)playerinfo + 256);
	if (surf != NULL)
	{
		const int mat_index = surf->flags >> 24;
		return SurfaceMaterialNames[mat_index];
	}

	return NULL;
}
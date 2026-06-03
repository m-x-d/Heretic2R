//
// SurfaceProps.c
//
// Copyright 1998 Raven Software
//

#include "SurfaceProps.h"
#include "p_types.h"

//TODO: mxd. Seems to be used in Player.dll only. Move there?
// NB: the prototype in SurfaceProps.h declares 'info' as const void* (to avoid
// dragging playerinfo_t into every includer); MSVC tolerates the mismatched
// definition, GCC does not, so match the prototype and cast here.
H2COMMON_API char* GetClientGroundSurfaceMaterialName(const void* info_)
{
#define NUM_MATERIALS (sizeof(material_names) / sizeof(material_names[0]))
	static char* material_names[] = { "gravel", "metal", "stone", "wood" };
	const playerinfo_t* info = info_;

	if (info->GroundSurface != NULL)
	{
		const int mat_index = (info->GroundSurface->flags >> 24);
		return material_names[mat_index % NUM_MATERIALS];
	}

	return NULL;
}
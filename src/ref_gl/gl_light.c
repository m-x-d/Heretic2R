//
// gl_light.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"

// Q2 counterpart
void R_SetCacheState(msurface_t* surf)
{
	for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
		surf->cached_light[maps] = r_newrefdef.lightstyles[surf->styles[maps]].white;
}

void R_BuildLightMap(msurface_t* surf, byte* dest, int stride)
{
	NOT_IMPLEMENTED
}
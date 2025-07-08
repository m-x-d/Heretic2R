//
// gl1_Light.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

extern void R_RenderDlights(void);
extern void R_MarkLights(dlight_t* light, int bit, const mnode_t* node);
extern void R_PushDlights(void);

extern void R_SetCacheState(msurface_t* surf);
extern void R_BuildLightMap(const msurface_t* surf, byte* dest, int stride);
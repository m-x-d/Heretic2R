//
// gl1_Lightmap.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

extern void LM_BuildPolygonFromSurface(msurface_t* fa);
extern void LM_CreateSurfaceLightmap(msurface_t* surf);
extern void LM_BeginBuildingLightmaps(model_t* m);
extern void LM_EndBuildingLightmaps(void);
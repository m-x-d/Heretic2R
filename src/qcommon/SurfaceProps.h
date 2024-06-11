//
// SurfaceProps.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "H2Common.h" //mxd. For H2COMMON_API
////#include "p_types.h"
//#include "Player.h"

#define MAX_SURFACE_MAT_NAMES 256

//extern char* SurfaceMaterialNames[MAX_SURFACE_MAT_NAMES]; //TODO: mxd. Disabled. Not exported in original H2Common.dll. Doesn't seem to be used anywhere.

//TODO: fix definition
//extern H2COMMON_API char* GetClientGroundSurfaceMaterialName(const playerinfo_t* playerinfo);
extern H2COMMON_API char* GetClientGroundSurfaceMaterialName(const void* playerinfo);
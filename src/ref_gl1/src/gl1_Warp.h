//
// gl1_Warp.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "gl1_Local.h"

// Polygon generation.
extern void R_EmitWaterPolys(const msurface_t* fa, qboolean undulate);
extern void R_EmitUnderwaterPolys(const msurface_t* fa);
extern void R_EmitQuakeFloorPolys(const msurface_t* fa);

extern void R_SubdivideSurface(const model_t* mdl, msurface_t* fa); //mxd. Added 'mdl' arg.
//
// r_SkeletonLerp.h -- flexible model skeleton interpolation
//
// Copyright 1998 Raven Software
//

#pragma once

#include "fmodel.h"

//mxd. Reconstructed data type. Original name unknown.
typedef struct
{
	vec3_t front_vector;
	vec3_t back_vector;
	fmtrivertx_t* verts;
	fmtrivertx_t* old_verts;
	fmtrivertx_t* unknown_verts; //TODO: better name
} SkeletonFrameLerpInfo_t;

extern int fmdl_num_xyz;
extern float fmdl_backlep;
extern float fmdl_inverted_backlep;
extern SkeletonFrameLerpInfo_t sfl;

void FrameLerp(void);
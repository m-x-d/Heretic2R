//
// m_SkeletalCluster.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

typedef struct M_SkeletalCluster_s
{
	int children; // Must be the first field.
	int numVerticies;
	int* verticies;
	qboolean inUse;
} M_SkeletalCluster_t;
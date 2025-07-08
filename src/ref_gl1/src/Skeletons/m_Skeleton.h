//
// m_Skeleton.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Reference.h"

typedef struct M_SkeletalJoint_s
{
	int children;		// Must be the first field.
	Placement_t model;	// Relative to the model, used for dynamic software rotation.
	Placement_t parent;	// Relative to the parent joint (or model in case of root joint), used for inverse kinematics.
	matrix3_t rotation;
	qboolean inUse;
} M_SkeletalJoint_t;

typedef struct ModelSkeleton_s
{
	M_SkeletalJoint_t* rootJoint;
	struct ArrayedListNode_s* rootNode;
} ModelSkeleton_t;
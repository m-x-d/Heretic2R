//
// Skeletons.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "ArrayedList.h"
#include "q_Typedef.h" //mxd. For uint.

#define JN_YAW_CHANGED		1
#define JN_PITCH_CHANGED	2
#define JN_ROLL_CHANGED		4

// Skeleton types.
enum
{
	SKEL_NULL = -1,
	SKEL_RAVEN,
	SKEL_BOX,
	SKEL_BEETLE,
	SKEL_ELFLORD,
	SKEL_PLAGUE_ELF,
	SKEL_CORVUS,
	NUM_SKELETONS
};

// Raven Skeletal joints.
enum
{
	RAVEN_LOWERBACK,
	RAVEN_UPPERBACK,
	RAVEN_HEAD,
	NUM_JOINTS_RAVEN
};

// Box Skeletal joints.
enum
{
	BOX_CENTER,
	NUM_JOINTS_BOX
};

// Beetle Skeletal joints.
enum
{
	BEETLE_NECK,
	BEETLE_HEAD,
	NUM_JOINTS_BEETLE
};

// Elflord Skeletal joints.
enum
{
	ELFLORD_,
	ELFLORD__,
	NUM_JOINTS_ELFLORD
};

// Plague Elf Skeletal joints.
enum
{
	PLAGUE_ELF_LOWERBACK,
	PLAGUE_ELF_UPPERBACK,
	PLAGUE_ELF_HEAD,
	NUM_JOINTS_PLAGUE_ELF
};

// Corvus Skeletal joints.
enum
{
	CORVUS_LOWERBACK,
	CORVUS_UPPERBACK,
	CORVUS_HEAD,
	NUM_JOINTS_CORVUS
};

#define NO_SWAP_FRAME	(-1)
#define NULL_ROOT_JOINT	(-1)

#define MAX_ARRAYED_SKELETAL_JOINTS		255	// Has max of 65,535 (if this remains at 255, net code can be changed to reflect).
#define MAX_ARRAYED_JOINT_NODES			(MAX_ARRAYED_SKELETAL_JOINTS - 1)

#define MAX_JOINTS_PER_SKELETON			8	// Arbitrary small number.
#define MAX_JOINT_NODES_PER_SKELETON	(MAX_JOINTS_PER_SKELETON - 1)

typedef struct CL_SkeletalJoint_s //mxd
{
	int children; // Must be the first field.
	vec3_t destAngles;
	vec3_t angVels;
	vec3_t angles;
	qboolean changed;
	qboolean inUse;
} CL_SkeletalJoint_t;

extern int numJointsInSkeleton[];
extern int numNodesInSkeleton[];

typedef void (*CreateSkeleton_t)(void* skeletalJoints, uint jointSize, ArrayedListNode_t* jointNodes, int rootIndex);

extern CreateSkeleton_t SkeletonCreators[NUM_SKELETONS];
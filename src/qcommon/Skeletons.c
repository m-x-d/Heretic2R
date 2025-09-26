//
// Skeletons.c
//
// Copyright 1998 Raven Software
//

#include "Skeletons.h"

int numJointsInSkeleton[] = 
{
	NUM_JOINTS_RAVEN,
	NUM_JOINTS_BOX,
	NUM_JOINTS_BEETLE,
	NUM_JOINTS_ELFLORD,
	NUM_JOINTS_PLAGUE_ELF,
	NUM_JOINTS_CORVUS,
};

int numNodesInSkeleton[] =
{
	2,	// RAVEN
	0,	// BOX
	1,	// BEETLE
	-1,	// ELFLORD
	2,	// PLAGUE ELF
	2,	// CORVUS
};

static void CreateRavenSkel(void* skeletalJoints, const uint jointSize, ArrayedListNode_t* jointNodes, const int rootIndex)
{
	char* root = (char*)skeletalJoints + rootIndex * jointSize;

	int* children = (int*)(root + RAVEN_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	int nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int*)(root + RAVEN_UPPERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + RAVEN_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int*)(root + RAVEN_LOWERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + RAVEN_UPPERBACK;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

static void CreateBoxSkel(void* skeletalJoints, const uint jointSize, ArrayedListNode_t* jointNodes, const int rootIndex)
{
	char* root = (char*)skeletalJoints + rootIndex * jointSize;
	int* children = (int*)(root + RAVEN_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;
}

static void CreateBeetleSkel(void* skeletalJoints, const uint jointSize, ArrayedListNode_t* jointNodes, const int rootIndex)
{
	char* root = (char*)skeletalJoints + rootIndex * jointSize;

	int* children = (int*)(root + BEETLE_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	const int nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int*)(root + BEETLE_NECK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + BEETLE_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

static void CreateElfLordSkel(void* skeletalJoints, const uint jointSize, ArrayedListNode_t* jointNodes, const int rootIndex)
{
	char* root = (char*)skeletalJoints + rootIndex * jointSize;

	int* children = (int*)(root + BEETLE_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	const int nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int*)(root + BEETLE_NECK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + BEETLE_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

static void CreatePlagueElfSkel(void* skeletalJoints, const uint jointSize, ArrayedListNode_t* jointNodes, const int rootIndex)
{
	char* root = (char*)skeletalJoints + rootIndex * jointSize;

	int* children = (int*)(root + PLAGUE_ELF_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	int nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int*)(root + PLAGUE_ELF_UPPERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + PLAGUE_ELF_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + PLAGUE_ELF_LOWERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + PLAGUE_ELF_UPPERBACK;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

CreateSkeleton_t SkeletonCreators[NUM_SKELETONS] =
{
	CreateRavenSkel,
	CreateBoxSkel,
	CreateBeetleSkel,
	CreateElfLordSkel,
	CreatePlagueElfSkel,
	CreatePlagueElfSkel, // Corvus has the same structure as the Plague Elf.
};
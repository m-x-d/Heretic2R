//
// Skeletons.c
//
// Copyright 1998 Raven Software
//
// Heretic II
//

#include "Skeletons.h"
#include "ArrayedList.h"

char *skeletonRootNames[] =
{
	"RAVEN_ROOT",
	"BOX_ROOT",
	"BEETLE_ROOT",
	"ELFLORD_ROOT",
	"PLAGUELF_ROOT",
	"ELF_BACKROOT",
};

int skeletonRNameOffsets[] =
{
	0,	// RAVEN
	1,	// BOX
	2,	// BEETLE
	3,	// ELFLORD
	4,	// PLAGUE ELF
	5,	// CORVUS
};

char *skeletonJointNames[] =
{
	"RAVEN_LOWERBACK",	// 0
	"RAVEN_UPPERBACK",
	"RAVEN_NECK",
	"BOX_CENTER",		// 3
	"BEETLE_NECK",		// 4
	"BEETLE_HEAD",
	"PLAGUELF_BACKB",	// 6
	"PLAGUELF_BACKC",
	"PLAGUELF_NECK",
	"ELF_BACKB",		// 9
	"ELF_BACKC",
	"ELF_NECKB",
};

int skeletonNameOffsets[] =
{
	0,	// RAVEN
	3,	// BOX
	4,	// BEETLE
	-1,	// ELFLORD
	6,	// PLAGUE ELF
	9,	// CORVUS
};

char *skeletonEffectorNames[] =
{
	"BEETLE_EYES",		// 0
	"CORVUS_EYES",		// 1
};

int skeletonENameOffsets[] =
{
	-1,	// RAVEN
	-1,	// BOX
	0,	// BEETLE
	-1,	// ELFLORD
	1,	// PLAGUE ELF
};

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

void CreateRavenSkel(void *skeletalJoints, size_t jointSize, struct ArrayedListNode_s *jointNodes, int root);
void CreateBoxSkel(void *skeletalJoints, size_t jointSize, struct ArrayedListNode_s *jointNodes, int root);
void CreateBeetleSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex);
void CreateElfLordSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex);
void CreatePlagueElfSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex);

CreateSkeleton_t SkeletonCreators[NUM_SKELETONS] =
{
	CreateRavenSkel,
	CreateBoxSkel,
	CreateBeetleSkel,
	CreateElfLordSkel,
	CreatePlagueElfSkel,
	CreatePlagueElfSkel,	// Corvus has the same structure as the Plague Elf
};

void CreateRavenSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + RAVEN_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + RAVEN_UPPERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + RAVEN_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + RAVEN_LOWERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + RAVEN_UPPERBACK;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

void CreateBoxSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + RAVEN_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;
}

void CreateBeetleSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + BEETLE_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + BEETLE_NECK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + BEETLE_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

void CreateElfLordSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + BEETLE_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + BEETLE_NECK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + BEETLE_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

void CreatePlagueElfSkel(void *skeletalJoints, size_t jointSize, ArrayedListNode_t *jointNodes, int rootIndex)
{
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)skeletalJoints + rootIndex * jointSize;

	children = (int *)(root + PLAGUE_ELF_HEAD * jointSize);
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + PLAGUE_ELF_UPPERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + PLAGUE_ELF_HEAD;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode(jointNodes, MAX_ARRAYED_JOINT_NODES);

	children = (int *)(root + PLAGUE_ELF_LOWERBACK * jointSize);
	*children = nodeIndex;

	jointNodes[nodeIndex].data = rootIndex + PLAGUE_ELF_UPPERBACK;
	jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}
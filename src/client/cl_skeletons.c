//
// cl_skeletons.c
//
// Copyright 1998 Raven Software
//

#include "cl_skeletons.h"

CL_SkeletalJoint_t skeletal_joints[MAX_ARRAYED_SKELETAL_JOINTS];
ArrayedListNode_t joint_nodes[MAX_ARRAYED_JOINT_NODES];

//mxd. Similar to UpdateSkeletons() in game/g_Skeletons.c
void SK_UpdateSkeletons(void)
{
	NOT_IMPLEMENTED
}

void SK_ClearJoints(int joint_index)
{
	NOT_IMPLEMENTED
}

void SK_CreateSkeleton(const int structure, const int root_index)
{
	if (skeletal_joints[root_index].inUse)
		return;

	SkeletonCreators[structure](skeletal_joints, sizeof(skeletal_joints[0]), joint_nodes, root_index);

	for (int i = 0; i < numJointsInSkeleton[structure]; i++)
		skeletal_joints[root_index + i].changed = true;

	skeletal_joints[root_index].inUse = true;
}
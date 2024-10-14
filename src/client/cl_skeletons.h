//
// cl_skeletons.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "client.h"
#include "ArrayedList.h"
#include "Skeletons.h"

extern CL_SkeletalJoint_t skeletal_joints[MAX_ARRAYED_SKELETAL_JOINTS];
extern ArrayedListNode_t joint_nodes[MAX_ARRAYED_JOINT_NODES];

void SK_UpdateSkeletons(void);
void SK_ClearJoints(int joint_index);
void SK_SetJointAngles(playerinfo_t* playerinfo);
void SK_ResetJointAngles(playerinfo_t* playerinfo);
void SK_CreateSkeleton(int structure, int root_index);
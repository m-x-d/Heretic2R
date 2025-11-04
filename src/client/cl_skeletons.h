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

extern void SK_UpdateSkeletons(void);
extern void SK_ClearJoints(int joint_index);
extern void SK_SetJointAngles(const playerinfo_t* playerinfo);
extern void SK_ResetJointAngles(const playerinfo_t* playerinfo);
extern void SK_CreateSkeleton(int structure, int root_index);
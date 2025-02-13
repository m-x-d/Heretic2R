//
// g_Skeletons.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "g_Skeleton.h"
#include "Skeletons.h"

extern G_SkeletalJoint_t skeletalJoints[MAX_ARRAYED_SKELETAL_JOINTS];
extern struct ArrayedListNode_s jointNodes[MAX_ARRAYED_JOINT_NODES];

extern int CreateSkeleton(int structure);
extern void UpdateSkeletons(void); //mxd
extern void FreeSkeleton(int root);
extern float GetJointAngle(int joint_index, int angle_index);
extern qboolean SetJointAngVel(int joint_index, int angle_index, float dest_angle, float angular_velocity);
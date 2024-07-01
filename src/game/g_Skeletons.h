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

int CreateSkeleton(int structure);
void FreeSkeleton(int root);
float GetJointAngle(int jointIndex, int angleIndex);
qboolean SetJointAngVel(int jointIndex, int angleIndex, float destAngle, float angSpeed);
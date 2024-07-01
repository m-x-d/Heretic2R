#ifndef G_SKELETON_H
#define G_SKELETON_H
#include "g_Skeleton.h"
#endif
#ifndef SKELETONS_H
#define SKELETONS_H
#include "Skeletons.h"
#endif

extern G_SkeletalJoint_t skeletalJoints[MAX_ARRAYED_SKELETAL_JOINTS];
extern struct ArrayedListNode_s jointNodes[MAX_ARRAYED_JOINT_NODES];

int CreateSkeleton(int structure);
void FreeSkeleton(int root);
float GetJointAngle(int jointIndex, int angleIndex);
qboolean SetJointAngVel(int jointIndex, int angleIndex, float destAngle, float angSpeed);
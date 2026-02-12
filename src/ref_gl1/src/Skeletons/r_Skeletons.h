//
// r_Skeletons.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "m_Skeleton.h"
#include "m_SkeletalCluster.h"
#include "Skeletons.h"

extern M_SkeletalCluster_t SkeletalClusters[MAX_ARRAYED_SKELETAL_JOINTS];
extern struct ArrayedListNode_s ClusterNodes[MAX_ARRAYED_JOINT_NODES];

extern int CreateSkeleton(int structure);
extern void CreateSkeletonAsHunk(int structure, ModelSkeleton_t* skel);
extern void CreateSkeletonInPlace(int structure, const ModelSkeleton_t* skel);
extern void ClearSkeleton(ModelSkeleton_t* skel, int root);

extern void SetupJointRotations(ModelSkeleton_t* skel, int jointIndex, int anglesIndex);
extern void FinishJointRotations(ModelSkeleton_t* skel, int jointIndex);
extern void LinearlyInterpolateJoints(ModelSkeleton_t* newSkel, int newIndex, ModelSkeleton_t* oldSkel, int oldIndex, ModelSkeleton_t* liSkel, int liIndex, float move[3], float frontv[3], float backv[3]);
extern void RotateModelSegments(ModelSkeleton_t* skel, int jointIndex, int modelClusterIndex, int anglesIndex,	vec3_t* modelVerticies);
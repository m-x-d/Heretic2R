#ifndef M_SKELETON_H
#define M_SKELETON_H
#include "m_Skeleton.h"
#endif
#ifndef M_SKELETALCLUSTER_H
#define M_SKELETALCLUSTER_H
#include "m_SkeletalCluster.h"
#endif
#ifndef SKELETONS_H
#define SKELETONS_H
#include "Skeletons.h"
#endif

extern M_SkeletalCluster_t SkeletalClusters[MAX_ARRAYED_SKELETAL_JOINTS];
extern struct ArrayedListNode_s ClusterNodes[MAX_ARRAYED_JOINT_NODES];

int CreateSkeleton(int structure);
void CreateSkeletonAsHunk(int structure, ModelSkeleton_t *skel);
void CreateSkeletonInPlace(int structure, ModelSkeleton_t *skel);
void ClearSkeleton(ModelSkeleton_t *skel, int root);

void SetupJointRotations(ModelSkeleton_t *skel, int jointIndex, int anglesIndex);
void FinishJointRotations(ModelSkeleton_t *skel, int jointIndex);
void LinearllyInterpolateJoints(ModelSkeleton_t *newSkel, int newIndex, 
	ModelSkeleton_t *oldSkel, int oldIndex, ModelSkeleton_t *liSkel, int liIndex,
	float move[3], float frontv[3], float backv[3]);
void SetupCompressedJoints(ModelSkeleton_t *liSkel, int liIndex,
	float *lerp);
void RotateModelSegments(ModelSkeleton_t *skel, int jointIndex, int modelClusterIndex, int anglesIndex, 
	vec3_t *modelVerticies);
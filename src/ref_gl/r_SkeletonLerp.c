//
// r_SkeletonLerp.c -- flexible model skeleton interpolation
//
// Copyright 1998 Raven Software
//

#include "r_SkeletonLerp.h"
#include "gl_local.h"
#include "ArrayedList.h"
#include "m_SkeletalCluster.h"
#include "m_Skeleton.h"
#include "r_Skeletons.h"

int fmdl_num_xyz;
float fmdl_backlep;
float fmdl_inverted_backlep;
SkeletonFrameLerpInfo_t sfl;

ModelSkeleton_t fmdl_skeleton1;
M_SkeletalJoint_t fmdl_skeleton1_root_joint;
ArrayedListNode_t fmdl_skeleton1_root_node;

ModelSkeleton_t fmdl_skeleton2;
M_SkeletalJoint_t fmdl_skeleton2_root_joint;
ArrayedListNode_t fmdl_skeleton2_root_node;

struct LERPedReferences_s* fmdl_referenceInfo;
M_SkeletalCluster_t* fmdl_cur_skeletal_cluster;

static void StandardFrameLerp(void)
{
	NOT_IMPLEMENTED
}

static void CompressedFrameLerp(void)
{
	NOT_IMPLEMENTED
}

void FrameLerp(void)
{
	fmdl_skeleton1.rootJoint = &fmdl_skeleton1_root_joint;
	fmdl_skeleton1.rootNode = &fmdl_skeleton1_root_node;
	fmdl_skeleton2.rootJoint = &fmdl_skeleton2_root_joint;
	fmdl_skeleton2.rootNode = &fmdl_skeleton2_root_node;

	if (fmodel->skeletalType != -1)
		fmdl_cur_skeletal_cluster = SkeletalClusters + fmodel->rootCluster + currententity->swapCluster;

	fmdl_referenceInfo = currententity->referenceInfo;

	if (fmodel->referenceType == -1)
		fmdl_referenceInfo = NULL;

	fmdl_inverted_backlep = 1.0f - fmdl_backlep;
	fmdl_num_xyz = fmodel->header.num_xyz;

	if (fmodel->frames != NULL)
		StandardFrameLerp();
	else
		CompressedFrameLerp();

	if (currententity->swapFrame != -1)
		ClearSkeleton(&fmdl_skeleton1, 0);

	if (currententity->rootJoint != -1)
		ClearSkeleton(&fmdl_skeleton2, 0);
}
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
#include "Vector.h"

int fmdl_num_xyz;
float framelerp; //mxd. Named 'fl' in original .dll
float framelerp_inv;
SkeletonFrameLerpInfo_t sfl;

vec3_t s_lerped[MAX_VERTS];

static ModelSkeleton_t fmdl_skeleton1;
static M_SkeletalJoint_t fmdl_skeleton1_root_joint;
static ArrayedListNode_t fmdl_skeleton1_root_node;

static ModelSkeleton_t fmdl_skeleton2;
static M_SkeletalJoint_t fmdl_skeleton2_root_joint;
static ArrayedListNode_t fmdl_skeleton2_root_node;

static struct LERPedReferences_s* fmdl_referenceInfo;
static M_SkeletalCluster_t* fmdl_cur_skeletal_cluster;

static vec3_t qfl;

static void LerpVerts(int num_verts, fmtrivertx_t* verts, fmtrivertx_t* old_verts, vec3_t* translation, vec3_t move, vec3_t front, vec3_t back)
{
	NOT_IMPLEMENTED
}

static void LerpStandardSkeleton(void)
{
	NOT_IMPLEMENTED
}

static void LerpReferences(void)
{
	NOT_IMPLEMENTED
}

static void StandardFrameLerp(void)
{
	int frame = currententity->frame;
	int oldframe = currententity->oldframe;

	if (frame < 0 || frame >= fmodel->header.num_frames)
		frame = 0;

	if (oldframe < 0 || oldframe >= fmodel->header.num_frames)
		oldframe = 0;

	fmaliasframe_t* pframe = (fmaliasframe_t*)((byte*)fmodel->frames->scale + frame * fmodel->header.framesize);
	fmaliasframe_t* poldframe = (fmaliasframe_t*)((byte*)fmodel->frames->scale + oldframe * fmodel->header.framesize);

	sfl.verts = pframe->verts;
	sfl.old_verts = poldframe->verts;
	sfl.unknown_verts = sfl.verts;

	for (int i = 0; i < 3; i++)
	{
		qfl[i] = framelerp * pframe->translate[i] + framelerp_inv * poldframe->translate[i];
		sfl.front_vector[i] = framelerp * pframe->scale[i];
		sfl.back_vector[i] = framelerp_inv * poldframe->scale[i];
	}

	if (currententity->scale != 1.0)
	{
		VectorScale(qfl, currententity->scale, qfl);
		VectorScale(sfl.front_vector, currententity->scale, sfl.front_vector);
		VectorScale(sfl.back_vector, currententity->scale, sfl.back_vector);
	}

	LerpVerts(fmdl_num_xyz, sfl.verts, sfl.old_verts, s_lerped, qfl, sfl.front_vector, sfl.back_vector);

	if (fmodel->skeletalType != -1)
		LerpStandardSkeleton();

	if (fmdl_referenceInfo != NULL && !(currententity->flags & RF_IGNORE_REFS))
		LerpReferences();
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

	framelerp_inv = 1.0f - framelerp;
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
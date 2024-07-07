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
static SkeletonFrameLerpInfo_t sfl_skel1;

static ModelSkeleton_t fmdl_skeleton2;
static M_SkeletalJoint_t fmdl_skeleton2_root_joint;
static ArrayedListNode_t fmdl_skeleton2_root_node;

static struct LERPedReferences_s* fmdl_referenceInfo;
static M_SkeletalCluster_t* fmdl_cur_skeletal_cluster;

static vec3_t qfl;
static vec3_t fmdl_move;

static void LerpVerts(const int num_verts, fmtrivertx_t* verts, fmtrivertx_t* old_verts, vec3_t* translation, const vec3_t move, const vec3_t front, const vec3_t back)
{
	for (int i = 0; i < num_verts; i++, verts++, old_verts++, translation++)
		for (int j = 0; j < 3; j++)
			(*translation)[j] = (float)verts->v[j] * front[j] + (float)old_verts->v[j] * back[j] + move[j];
}

static void DoSkeletalRotations(void)
{
	NOT_IMPLEMENTED
}

static void LerpStandardSkeleton(void)
{
	static vec3_t lerped[2048];
	entity_t* e = currententity;

	if (e->swapFrame != -1)
	{
		CreateSkeletonInPlace(fmodel->skeletalType, &fmdl_skeleton1);
		e->swapCluster = 0;

		fmaliasframe_t* pframe = (fmaliasframe_t*)((byte*)fmodel->frames + e->swapFrame * fmodel->header.framesize);
		fmaliasframe_t* poldframe = (fmaliasframe_t*)((byte*)fmodel->frames + e->oldSwapFrame * fmodel->header.framesize);

		sfl_skel1.verts = pframe->verts;
		sfl_skel1.old_verts = poldframe->verts;
		sfl_skel1.unknown_verts = sfl_skel1.verts;

		for (int i = 0; i < 3; i++)
		{
			fmdl_move[i] = framelerp * pframe->translate[i] + framelerp_inv * poldframe->translate[i];
			sfl_skel1.front_vector[i] = framelerp * pframe->scale[i];
			sfl_skel1.back_vector[i] = framelerp_inv * poldframe->scale[i];
		}
		
		LerpVerts(fmdl_num_xyz, sfl_skel1.verts, sfl_skel1.old_verts, lerped, fmdl_move, sfl_skel1.front_vector, sfl_skel1.back_vector);

		for (int i = 0; i < fmdl_cur_skeletal_cluster->numVerticies; i++)
		{
			const int vert_index = fmdl_cur_skeletal_cluster->verticies[i];
			VectorCopy(lerped[vert_index], s_lerped[vert_index]);
		}

		LinearllyInterpolateJoints(&fmodel->skeletons[e->swapFrame], 0, &fmodel->skeletons[e->oldSwapFrame], 0, &fmdl_skeleton1, 0, fmdl_move, sfl_skel1.front_vector, sfl_skel1.back_vector);
	}

	if (e->rootJoint != -1 || e->swapFrame != -1)
	{
		CreateSkeletonInPlace(fmodel->skeletalType, &fmdl_skeleton2);
		LinearllyInterpolateJoints(&fmodel->skeletons[e->frame], 0, &fmodel->skeletons[e->oldframe], 0, &fmdl_skeleton2, 0, qfl, sfl.front_vector, sfl.back_vector);
	}

	DoSkeletalRotations();
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

	fmaliasframe_t* pframe = (fmaliasframe_t*)((byte*)fmodel->frames + frame * fmodel->header.framesize);
	fmaliasframe_t* poldframe = (fmaliasframe_t*)((byte*)fmodel->frames + oldframe * fmodel->header.framesize);

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
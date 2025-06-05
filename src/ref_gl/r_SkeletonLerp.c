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
#include "Reference.h"
#include "r_Skeletons.h"
#include "Vector.h"

int fmdl_num_xyz;
float framelerp; //mxd. Named 'fl' in original .dll
float framelerp_inv;

vec3_t s_lerped[MAX_FM_VERTS];

static ModelSkeleton_t cur_skeleton;
static M_SkeletalJoint_t fmdl_cur_skeleton_joints[MAX_JOINTS_PER_SKELETON];
static ArrayedListNode_t fmdl_cur_skeleton_nodes[MAX_JOINT_NODES_PER_SKELETON];
SkeletonFrameLerpInfo_t sfl_cur_skel;
static vec3_t cur_skel_move; //mxd. Named 'qfl' in original .dll

static ModelSkeleton_t swap_skeleton;
static M_SkeletalJoint_t fmdl_swap_skeleton_joints[MAX_JOINTS_PER_SKELETON];
static ArrayedListNode_t fmdl_swap_skeleton_nodes[MAX_JOINT_NODES_PER_SKELETON];
static SkeletonFrameLerpInfo_t sfl_swap_skel;
static vec3_t swap_skel_move;

static LERPedReferences_t* fmdl_referenceInfo;
static M_SkeletalCluster_t* fmdl_cur_skeletal_cluster;

static void LerpVerts(const int num_verts, fmtrivertx_t* verts, fmtrivertx_t* old_verts, vec3_t* translation, const vec3_t move, const vec3_t front, const vec3_t back)
{
	for (int i = 0; i < num_verts; i++, verts++, old_verts++, translation++)
		for (int j = 0; j < 3; j++)
			(*translation)[j] = (float)verts->v[j] * front[j] + (float)old_verts->v[j] * back[j] + move[j];
}

static void DoSkeletalRotations(void)
{
	if (currententity->rootJoint == -1)
		return;

	if (currententity->swapFrame != -1)
	{
		RotateModelSegments(&swap_skeleton, 0, fmodel->rootCluster, currententity->rootJoint, s_lerped);

		for (int i = 0; i < fmdl_cur_skeletal_cluster->numVerticies; i++)
		{
			vec3_t* v = &s_lerped[fmdl_cur_skeletal_cluster->verticies[i]];
			VectorSubtract(*v, swap_skeleton.rootJoint->model.origin, *v);
			VectorAdd(*v, cur_skeleton.rootJoint->model.origin, *v);
		}
	}
	else
	{
		RotateModelSegments(&cur_skeleton, 0, fmodel->rootCluster, currententity->rootJoint, s_lerped);
	}
}

static void LerpStandardSkeleton(void)
{
	static vec3_t lerped[2048];
	entity_t* e = currententity;

	if (e->swapFrame != -1)
	{
		CreateSkeletonInPlace(fmodel->skeletalType, &swap_skeleton);
		e->swapCluster = 0;

		fmaliasframe_t* pframe = (fmaliasframe_t*)((byte*)fmodel->frames + e->swapFrame * fmodel->header.framesize);
		fmaliasframe_t* poldframe = (fmaliasframe_t*)((byte*)fmodel->frames + e->oldSwapFrame * fmodel->header.framesize);

		sfl_swap_skel.verts = pframe->verts;
		sfl_swap_skel.old_verts = poldframe->verts;

		for (int i = 0; i < 3; i++)
		{
			swap_skel_move[i] = framelerp_inv * pframe->translate[i] + framelerp * poldframe->translate[i];
			sfl_swap_skel.front_vector[i] = framelerp_inv * pframe->scale[i];
			sfl_swap_skel.back_vector[i] = framelerp * poldframe->scale[i];
		}
		
		LerpVerts(fmdl_num_xyz, sfl_swap_skel.verts, sfl_swap_skel.old_verts, lerped, swap_skel_move, sfl_swap_skel.front_vector, sfl_swap_skel.back_vector);

		for (int i = 0; i < fmdl_cur_skeletal_cluster->numVerticies; i++)
		{
			const int vert_index = fmdl_cur_skeletal_cluster->verticies[i];
			VectorCopy(lerped[vert_index], s_lerped[vert_index]);
		}

		LinearllyInterpolateJoints(&fmodel->skeletons[e->swapFrame], 0, &fmodel->skeletons[e->oldSwapFrame], 0, &swap_skeleton, 0, swap_skel_move, sfl_swap_skel.front_vector, sfl_swap_skel.back_vector);
	}

	if (e->rootJoint != -1 || e->swapFrame != -1)
	{
		CreateSkeletonInPlace(fmodel->skeletalType, &cur_skeleton);
		LinearllyInterpolateJoints(&fmodel->skeletons[e->frame], 0, &fmodel->skeletons[e->oldframe], 0, &cur_skeleton, 0, cur_skel_move, sfl_cur_skel.front_vector, sfl_cur_skel.back_vector);
	}

	DoSkeletalRotations();
}

static void ApplySkeletonToRef(Placement_t* placement, const int joint_index, const qboolean update_placement)
{
	M_SkeletalJoint_t* joint;
	const int joint_id = fmdl_referenceInfo->jointIDs[joint_index];

	if (update_placement)
		joint = &swap_skeleton.rootJoint[joint_id];
	else
		joint = &cur_skeleton.rootJoint[joint_id];

	TransformPoint(joint->rotation, joint->model.origin, joint->parent.origin, placement->origin);
	TransformPoint(joint->rotation, joint->model.origin, joint->parent.origin, placement->direction);
	TransformPoint(joint->rotation, joint->model.origin, joint->parent.origin, placement->up);

	if (update_placement)
	{
		VectorSubtract(placement->origin, swap_skeleton.rootJoint->model.origin, placement->origin);
		VectorAdd(placement->origin, cur_skeleton.rootJoint->model.origin, placement->origin);
	}
}

static void LerpReferences(void)
{
	assert(fmdl_referenceInfo->jointIDs != NULL); //mxd

	const float delta = r_newrefdef.time - fmdl_referenceInfo->lastUpdate;
	fmdl_referenceInfo->lastUpdate = r_newrefdef.time;

	const int num_refs = numReferences[fmodel->referenceType];

	if (currententity->rootJoint != -1)
	{
		HACK_Pitch_Adjust = true; //mxd. Interestingly, Loki version doesn't set/unset this here... //TODO: check Loki version of Matricies3FromDirAndUp()?

		SetupJointRotations(&cur_skeleton, 0, currententity->rootJoint);
		FinishJointRotations(&cur_skeleton, 0);

		if (currententity->swapFrame != -1)
		{
			SetupJointRotations(&swap_skeleton, 0, currententity->rootJoint);
			FinishJointRotations(&swap_skeleton, 0);
		}

		HACK_Pitch_Adjust = false;
	}

	for (int i = 0; i < num_refs; i++)
	{
		Placement_t* cur_placement = &fmdl_referenceInfo->references[i].placement;
		Placement_t* old_placement = &fmdl_referenceInfo->oldReferences[i].placement;
		qboolean update_placement = false;

		if (delta <= 1.0f)
			memcpy(old_placement, cur_placement, sizeof(Placement_t));

		if (fmodel->frames == NULL)
		{
			if (currententity->swapFrame == -1 || fmdl_referenceInfo->jointIDs[i] < currententity->swapCluster)
			{
				VectorCopy(s_lerped[fmdl_num_xyz + i * 3 + 0], cur_placement->origin);
				VectorCopy(s_lerped[fmdl_num_xyz + i * 3 + 1], cur_placement->direction);
				VectorCopy(s_lerped[fmdl_num_xyz + i * 3 + 2], cur_placement->up);
			}
			else
			{
				//mxd. Skipping compressed frame lerp logic for now (decompiled code is VERY cursed; is it even used in H2?)...
				NOT_IMPLEMENTED
			}
		}
		else if (currententity->swapFrame == -1 || fmdl_referenceInfo->jointIDs[i] < currententity->swapCluster)
		{
			Placement_t* frame = &fmodel->refsForFrame[currententity->frame * num_refs + i];
			Placement_t* oldframe = &fmodel->refsForFrame[currententity->oldframe * num_refs + i];

			GL_LerpVert(frame->origin,			oldframe->origin,			cur_placement->origin,		cur_skel_move,	sfl_cur_skel.front_vector,	sfl_cur_skel.back_vector);
			GL_LerpVert(frame->direction,		oldframe->direction,		cur_placement->direction,	cur_skel_move,	sfl_cur_skel.front_vector,	sfl_cur_skel.back_vector);
			GL_LerpVert(frame->up,				oldframe->up,				cur_placement->up,			cur_skel_move,	sfl_cur_skel.front_vector,	sfl_cur_skel.back_vector);
		}
		else
		{
			Placement_t* swapFrame = &fmodel->refsForFrame[currententity->swapFrame * num_refs + i];
			Placement_t* oldSwapFrame = &fmodel->refsForFrame[currententity->oldSwapFrame * num_refs + i];

			GL_LerpVert(swapFrame->origin,		oldSwapFrame->origin,		cur_placement->origin,		swap_skel_move,	sfl_swap_skel.front_vector,	sfl_swap_skel.back_vector);
			GL_LerpVert(swapFrame->direction,	oldSwapFrame->direction,	cur_placement->direction,	swap_skel_move,	sfl_swap_skel.front_vector,	sfl_swap_skel.back_vector);
			GL_LerpVert(swapFrame->up,			oldSwapFrame->up,			cur_placement->up,			swap_skel_move,	sfl_swap_skel.front_vector,	sfl_swap_skel.back_vector);

			update_placement = true;
		}

		if (fmdl_referenceInfo->jointIDs != NULL && fmdl_referenceInfo->jointIDs[i] != -1 && (int)r_references->value) //TODO: fmdl_referenceInfo->jointIDs NULL check should be either removed or done at the beginning of the function.
			ApplySkeletonToRef(cur_placement, i, update_placement);

		if (delta > 1.0f)
			memcpy(old_placement, cur_placement, sizeof(Placement_t));
	}
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

	sfl_cur_skel.verts = pframe->verts;
	sfl_cur_skel.old_verts = poldframe->verts;

	for (int i = 0; i < 3; i++)
	{
		cur_skel_move[i] = framelerp_inv * pframe->translate[i] + framelerp * poldframe->translate[i];
		sfl_cur_skel.front_vector[i] = framelerp_inv * pframe->scale[i];
		sfl_cur_skel.back_vector[i] = framelerp * poldframe->scale[i];
	}

	if (currententity->scale != 1.0f)
	{
		VectorScale(cur_skel_move, currententity->scale, cur_skel_move);
		VectorScale(sfl_cur_skel.front_vector, currententity->scale, sfl_cur_skel.front_vector);
		VectorScale(sfl_cur_skel.back_vector, currententity->scale, sfl_cur_skel.back_vector);
	}

	LerpVerts(fmdl_num_xyz, sfl_cur_skel.verts, sfl_cur_skel.old_verts, s_lerped, cur_skel_move, sfl_cur_skel.front_vector, sfl_cur_skel.back_vector);

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
	swap_skeleton.rootJoint = fmdl_swap_skeleton_joints;
	swap_skeleton.rootNode = fmdl_swap_skeleton_nodes;
	cur_skeleton.rootJoint = fmdl_cur_skeleton_joints;
	cur_skeleton.rootNode = fmdl_cur_skeleton_nodes;

	if (fmodel->skeletalType != -1)
		fmdl_cur_skeletal_cluster = SkeletalClusters + fmodel->rootCluster + currententity->swapCluster;

	fmdl_referenceInfo = ((fmodel->referenceType == -1) ? NULL : currententity->referenceInfo);
	framelerp_inv = 1.0f - framelerp;
	fmdl_num_xyz = fmodel->header.num_xyz;

	if (fmodel->frames != NULL)
		StandardFrameLerp();
	else
		CompressedFrameLerp();

	if (currententity->swapFrame != -1)
		ClearSkeleton(&swap_skeleton, 0);

	if (currententity->rootJoint != -1)
		ClearSkeleton(&cur_skeleton, 0);
}
//
// r_SkeletonLerp.c -- flexible model skeleton interpolation
//
// Copyright 1998 Raven Software
//

#include "r_SkeletonLerp.h"
#include "r_Skeletons.h"
#include "m_SkeletalCluster.h"
#include "m_Skeleton.h"
#include "ArrayedList.h"
#include "Matrix.h"
#include "Reference.h"
#include "Vector.h"
#include "../gl1_Local.h"

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

static void DoSkeletalRotations(const fmdl_t* fmdl, const entity_t* e) //mxd. Original logic uses 'fmodel' and 'currententity' global vars.
{
	if (e->rootJoint == NULL_ROOT_JOINT)
		return;

	if (e->swapFrame != NO_SWAP_FRAME)
	{
		RotateModelSegments(&swap_skeleton, 0, fmdl->rootCluster, e->rootJoint, s_lerped);

		for (int i = 0; i < fmdl_cur_skeletal_cluster->numVerticies; i++)
		{
			vec3_t* v = &s_lerped[fmdl_cur_skeletal_cluster->verticies[i]];
			Vec3SubtractAssign(swap_skeleton.rootJoint->model.origin, *v);
			Vec3AddAssign(cur_skeleton.rootJoint->model.origin, *v);
		}
	}
	else
	{
		RotateModelSegments(&cur_skeleton, 0, fmdl->rootCluster, e->rootJoint, s_lerped);
	}
}

static void LerpStandardSkeleton(const fmdl_t* fmdl, entity_t* e) //mxd. Original logic uses 'fmodel' and 'currententity' global vars.
{
	static vec3_t lerped[2048];

	if (e->swapFrame != NO_SWAP_FRAME)
	{
		CreateSkeletonInPlace(fmdl->skeletalType, &swap_skeleton);
		e->swapCluster = 0;

		fmaliasframe_t* pframe = (fmaliasframe_t*)((byte*)fmdl->frames + e->swapFrame * fmdl->header.framesize);
		fmaliasframe_t* poldframe = (fmaliasframe_t*)((byte*)fmdl->frames + e->oldSwapFrame * fmdl->header.framesize);

		sfl_swap_skel.verts = pframe->verts;
		sfl_swap_skel.old_verts = poldframe->verts;

		const float lerp = 1.0f - e->backlerp; //mxd

		for (int i = 0; i < 3; i++)
		{
			swap_skel_move[i] = lerp * pframe->translate[i] + e->backlerp * poldframe->translate[i];
			sfl_swap_skel.front_vector[i] = lerp * pframe->scale[i];
			sfl_swap_skel.back_vector[i] = e->backlerp * poldframe->scale[i];
		}
		
		LerpVerts(fmdl->header.num_xyz, sfl_swap_skel.verts, sfl_swap_skel.old_verts, lerped, swap_skel_move, sfl_swap_skel.front_vector, sfl_swap_skel.back_vector);

		for (int i = 0; i < fmdl_cur_skeletal_cluster->numVerticies; i++)
		{
			const int vert_index = fmdl_cur_skeletal_cluster->verticies[i];
			VectorCopy(lerped[vert_index], s_lerped[vert_index]);
		}

		LinearllyInterpolateJoints(&fmdl->skeletons[e->swapFrame], 0, &fmdl->skeletons[e->oldSwapFrame], 0, &swap_skeleton, 0, swap_skel_move, sfl_swap_skel.front_vector, sfl_swap_skel.back_vector);
	}

	if (e->rootJoint != NULL_ROOT_JOINT || e->swapFrame != NO_SWAP_FRAME)
	{
		CreateSkeletonInPlace(fmdl->skeletalType, &cur_skeleton);
		LinearllyInterpolateJoints(&fmdl->skeletons[e->frame], 0, &fmdl->skeletons[e->oldframe], 0, &cur_skeleton, 0, cur_skel_move, sfl_cur_skel.front_vector, sfl_cur_skel.back_vector);
	}

	DoSkeletalRotations(fmdl, e);
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

static void LerpReferences(const fmdl_t* fmdl, const entity_t* e) //mxd. Original logic uses 'fmodel' and 'currententity' global vars.
{
	const float delta = r_newrefdef.time - fmdl_referenceInfo->lastUpdate;
	fmdl_referenceInfo->lastUpdate = r_newrefdef.time;

	const int num_refs = numReferences[fmdl->referenceType];

	if (e->rootJoint != NULL_ROOT_JOINT)
	{
		HACK_Pitch_Adjust = true; //mxd. Interestingly, Loki version doesn't set/unset this here... //TODO: check Loki version of Matricies3FromDirAndUp()?

		SetupJointRotations(&cur_skeleton, 0, e->rootJoint);
		FinishJointRotations(&cur_skeleton, 0);

		if (e->swapFrame != NO_SWAP_FRAME)
		{
			SetupJointRotations(&swap_skeleton, 0, e->rootJoint);
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

		if (fmdl->frames == NULL) //TODO: can't happen? There's fmodel->frames NULL-check in FrameLerp().
		{
			if (e->swapFrame == NO_SWAP_FRAME || (fmdl_referenceInfo->jointIDs != NULL && fmdl_referenceInfo->jointIDs[i] < e->swapCluster)) //mxd. Added jointIDs NULL check.
			{
				VectorCopy(s_lerped[fmdl->header.num_xyz + i * 3 + 0], cur_placement->origin);
				VectorCopy(s_lerped[fmdl->header.num_xyz + i * 3 + 1], cur_placement->direction);
				VectorCopy(s_lerped[fmdl->header.num_xyz + i * 3 + 2], cur_placement->up);
			}
			else
			{
				//mxd. Skipping compressed frame lerp logic for now (decompiled code is VERY cursed; is it even used in H2?)...
				ri.Com_Error(ERR_DROP, "LerpReferences: fmodel compressed frame lerp logic not implemented...");
			}
		}
		else if (e->swapFrame == NO_SWAP_FRAME || (fmdl_referenceInfo->jointIDs != NULL && fmdl_referenceInfo->jointIDs[i] < e->swapCluster)) //mxd. Added jointIDs NULL check.
		{
			const Placement_t* frame = &fmdl->refsForFrame[e->frame * num_refs + i];
			const Placement_t* oldframe = &fmdl->refsForFrame[e->oldframe * num_refs + i];

			R_LerpVert(frame->origin,			oldframe->origin,			cur_placement->origin,		cur_skel_move,	sfl_cur_skel.front_vector,	sfl_cur_skel.back_vector);
			R_LerpVert(frame->direction,		oldframe->direction,		cur_placement->direction,	cur_skel_move,	sfl_cur_skel.front_vector,	sfl_cur_skel.back_vector);
			R_LerpVert(frame->up,				oldframe->up,				cur_placement->up,			cur_skel_move,	sfl_cur_skel.front_vector,	sfl_cur_skel.back_vector);
		}
		else
		{
			const Placement_t* swapFrame = &fmdl->refsForFrame[e->swapFrame * num_refs + i];
			const Placement_t* oldSwapFrame = &fmdl->refsForFrame[e->oldSwapFrame * num_refs + i];

			R_LerpVert(swapFrame->origin,		oldSwapFrame->origin,		cur_placement->origin,		swap_skel_move,	sfl_swap_skel.front_vector,	sfl_swap_skel.back_vector);
			R_LerpVert(swapFrame->direction,	oldSwapFrame->direction,	cur_placement->direction,	swap_skel_move,	sfl_swap_skel.front_vector,	sfl_swap_skel.back_vector);
			R_LerpVert(swapFrame->up,			oldSwapFrame->up,			cur_placement->up,			swap_skel_move,	sfl_swap_skel.front_vector,	sfl_swap_skel.back_vector);

			update_placement = true;
		}

		if (fmdl_referenceInfo->jointIDs != NULL && fmdl_referenceInfo->jointIDs[i] != -1 && (int)r_references->value) //TODO: fmdl_referenceInfo->jointIDs NULL check should be either removed or done at the beginning of the function.
			ApplySkeletonToRef(cur_placement, i, update_placement);

		if (delta > 1.0f)
			memcpy(old_placement, cur_placement, sizeof(Placement_t));
	}
}

static void StandardFrameLerp(const fmdl_t* fmdl, entity_t* e) //mxd. Original logic uses 'fmodel' and 'currententity' global vars.
{
	int frame = e->frame;
	int oldframe = e->oldframe;

	if (frame < 0 || frame >= fmdl->header.num_frames)
		frame = 0;

	if (oldframe < 0 || oldframe >= fmdl->header.num_frames)
		oldframe = 0;

	fmaliasframe_t* pframe = (fmaliasframe_t*)((byte*)fmdl->frames + frame * fmdl->header.framesize);
	fmaliasframe_t* poldframe = (fmaliasframe_t*)((byte*)fmdl->frames + oldframe * fmdl->header.framesize);

	sfl_cur_skel.verts = pframe->verts;
	sfl_cur_skel.old_verts = poldframe->verts;

	const float lerp = 1.0f - e->backlerp; //mxd

	for (int i = 0; i < 3; i++)
	{
		cur_skel_move[i] = lerp * pframe->translate[i] + e->backlerp * poldframe->translate[i];
		sfl_cur_skel.front_vector[i] = lerp * pframe->scale[i];
		sfl_cur_skel.back_vector[i] = e->backlerp * poldframe->scale[i];
	}

	if (e->scale != 1.0f)
	{
		Vec3ScaleAssign(e->scale, cur_skel_move);
		Vec3ScaleAssign(e->scale, sfl_cur_skel.front_vector);
		Vec3ScaleAssign(e->scale, sfl_cur_skel.back_vector);
	}

	LerpVerts(fmdl->header.num_xyz, sfl_cur_skel.verts, sfl_cur_skel.old_verts, s_lerped, cur_skel_move, sfl_cur_skel.front_vector, sfl_cur_skel.back_vector);

	if (fmdl->skeletalType != SKEL_NULL)
		LerpStandardSkeleton(fmdl, e);

	if (fmdl_referenceInfo != NULL && !(e->flags & RF_IGNORE_REFS))
		LerpReferences(fmdl, e);
}

static void CompressedFrameLerp(entity_t* e)
{
	ri.Com_Error(ERR_DROP, "CompressedFrameLerp not implemented..."); //TODO: is this ever used? Remove?
}

void FrameLerp(const fmdl_t* fmdl, entity_t* e) //mxd. Original logic uses 'fmodel' and 'currententity' global vars.
{
	swap_skeleton.rootJoint = fmdl_swap_skeleton_joints;
	swap_skeleton.rootNode = fmdl_swap_skeleton_nodes;
	cur_skeleton.rootJoint = fmdl_cur_skeleton_joints;
	cur_skeleton.rootNode = fmdl_cur_skeleton_nodes;

	if (fmdl->skeletalType != SKEL_NULL)
		fmdl_cur_skeletal_cluster = &SkeletalClusters[fmdl->rootCluster + e->swapCluster];

	fmdl_referenceInfo = ((fmdl->referenceType == REF_NULL) ? NULL : e->referenceInfo);

	if (fmdl->frames != NULL)
		StandardFrameLerp(fmdl, e);
	else
		CompressedFrameLerp(e);

	if (e->swapFrame != NO_SWAP_FRAME)
		ClearSkeleton(&swap_skeleton, 0);

	if (e->rootJoint != NULL_ROOT_JOINT)
		ClearSkeleton(&cur_skeleton, 0);
}
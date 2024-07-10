//
// r_SkeletonLerp.c -- flexible model skeleton interpolation
//
// Copyright 1998 Raven Software
//

#include "r_SkeletonLerp.h"
#include "gl_local.h"
#include "ArrayedList.h"
#include "m_Reference.h"
#include "m_SkeletalCluster.h"
#include "m_Skeleton.h"
#include "Reference.h"
#include "r_Skeletons.h"
#include "Vector.h"

int fmdl_num_xyz;
float framelerp; //mxd. Named 'fl' in original .dll
float framelerp_inv;
SkeletonFrameLerpInfo_t sfl;

vec3_t s_lerped[MAX_VERTS];

static ModelSkeleton_t fmdl_skeleton1;
static M_SkeletalJoint_t fmdl_skeleton1_joints[MAX_JOINTS_PER_SKELETON];
static ArrayedListNode_t fmdl_skeleton1_nodes[MAX_JOINT_NODES_PER_SKELETON];
static SkeletonFrameLerpInfo_t sfl_skel1;

static ModelSkeleton_t fmdl_skeleton2;
static M_SkeletalJoint_t fmdl_skeleton2_joints[MAX_JOINTS_PER_SKELETON];
static ArrayedListNode_t fmdl_skeleton2_nodes[MAX_JOINT_NODES_PER_SKELETON];

static LERPedReferences_t* fmdl_referenceInfo;
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
	if (currententity->rootJoint == -1)
		return;

	if (currententity->swapFrame != -1)
	{
		RotateModelSegments(&fmdl_skeleton1, 0, fmodel->rootCluster, currententity->rootJoint, s_lerped);

		for (int i = 0; i < fmdl_cur_skeletal_cluster->numVerticies; i++)
		{
			vec3_t* v = &s_lerped[fmdl_cur_skeletal_cluster->verticies[i]];
			VectorSubtract(*v, fmdl_skeleton1.rootJoint->model.origin, *v);
			VectorAdd(*v, fmdl_skeleton2.rootJoint->model.origin, *v);
		}
	}
	else
	{
		RotateModelSegments(&fmdl_skeleton2, 0, fmodel->rootCluster, currententity->rootJoint, s_lerped);
	}
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
			fmdl_move[i] = framelerp_inv * pframe->translate[i] + framelerp * poldframe->translate[i];
			sfl_skel1.front_vector[i] = framelerp_inv * pframe->scale[i];
			sfl_skel1.back_vector[i] = framelerp * poldframe->scale[i];
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

static void ApplySkeletonToRef(Placement_t* placement, const int joint_index, const qboolean update_placement)
{
	M_SkeletalJoint_t* joint;
	const int joint_id = fmdl_referenceInfo->jointIDs[joint_index];

	if (update_placement)
		joint = &fmdl_skeleton1.rootJoint[joint_id];
	else
		joint = &fmdl_skeleton2.rootJoint[joint_id];

	TransformPoint(joint->rotation, joint->model.origin, joint->parent.origin, placement->origin);
	TransformPoint(joint->rotation, joint->model.origin, joint->parent.origin, placement->direction);
	TransformPoint(joint->rotation, joint->model.origin, joint->parent.origin, placement->up);

	if (update_placement)
	{
		VectorSubtract(placement->origin, fmdl_skeleton1.rootJoint->model.origin, placement->origin);
		VectorAdd(placement->origin, fmdl_skeleton2.rootJoint->model.origin, placement->origin);
	}
}

static void LerpReferences(void)
{
	const float delta = r_newrefdef.time - fmdl_referenceInfo->lastUpdate;
	fmdl_referenceInfo->lastUpdate = r_newrefdef.time;

	const int num_refs = numReferences[fmodel->referenceType];

	if (currententity->rootJoint != -1)
	{
		HACK_Pitch_Adjust = true; //mxd. Interestingly, Loki version doesn't set/unset this here... //TODO: check Loki version of Matricies3FromDirAndUp()?

		SetupJointRotations(&fmdl_skeleton2, 0, currententity->rootJoint);
		FinishJointRotations(&fmdl_skeleton2, 0);

		if (currententity->swapFrame != -1)
		{
			SetupJointRotations(&fmdl_skeleton1, 0, currententity->rootJoint);
			FinishJointRotations(&fmdl_skeleton1, 0);
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
			Placement_t* frame = &fmodel->refsForFrame[currententity->frame * num_refs + i].placement;
			Placement_t* oldframe = &fmodel->refsForFrame[currententity->oldframe * num_refs + i].placement;

			GL_LerpVert(frame->origin,		oldframe->origin,	cur_placement->origin,		qfl, sfl.front_vector, sfl.back_vector);
			GL_LerpVert(frame->direction,	oldframe->direction,	cur_placement->direction,	qfl, sfl.front_vector, sfl.back_vector);
			GL_LerpVert(frame->up,			oldframe->up,		cur_placement->up,			qfl, sfl.front_vector, sfl.back_vector);
		}
		else
		{
			Placement_t* swapFrame = &fmodel->refsForFrame[currententity->swapFrame * num_refs].placement;
			Placement_t* oldSwapFrame = &fmodel->refsForFrame[currententity->oldSwapFrame * num_refs].placement;

			GL_LerpVert(swapFrame->origin,		oldSwapFrame->origin,	cur_placement->origin,		fmdl_move, sfl_skel1.front_vector, sfl_skel1.back_vector);
			GL_LerpVert(swapFrame->direction,	oldSwapFrame->direction,	cur_placement->direction,	fmdl_move, sfl_skel1.front_vector, sfl_skel1.back_vector);
			GL_LerpVert(swapFrame->up,			oldSwapFrame->up,		cur_placement->up,			fmdl_move, sfl_skel1.front_vector, sfl_skel1.back_vector);

			update_placement = true;
		}

		if (fmdl_referenceInfo->jointIDs != NULL && fmdl_referenceInfo->jointIDs[i] != -1 && (int)r_references->value)
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

	sfl.verts = pframe->verts;
	sfl.old_verts = poldframe->verts;
	sfl.unknown_verts = sfl.verts;

	for (int i = 0; i < 3; i++)
	{
		qfl[i] = framelerp_inv * pframe->translate[i] + framelerp * poldframe->translate[i];
		sfl.front_vector[i] = framelerp_inv * pframe->scale[i];
		sfl.back_vector[i] = framelerp * poldframe->scale[i];
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
	fmdl_skeleton1.rootJoint = fmdl_skeleton1_joints;
	fmdl_skeleton1.rootNode = fmdl_skeleton1_nodes;
	fmdl_skeleton2.rootJoint = fmdl_skeleton2_joints;
	fmdl_skeleton2.rootNode = fmdl_skeleton2_nodes;

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
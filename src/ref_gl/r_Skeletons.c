//
// r_Skeletons.c
//
// Copyright 1998 Raven Software
//

#include "gl_local.h"
#include "gl_fmodel.h"
#include "m_SkeletalCluster.h"
#include "m_Skeleton.h"
#include "r_Skeletons.h"
#include "Skeletons.h"
#include "Vector.h"

M_SkeletalCluster_t SkeletalClusters[MAX_ARRAYED_SKELETAL_JOINTS];
ArrayedListNode_t ClusterNodes[MAX_ARRAYED_JOINT_NODES];

void CreateSkeletonAsHunk(const int structure, ModelSkeleton_t* skel)
{
	skel->rootJoint = Hunk_Alloc(numJointsInSkeleton[structure] * (int)sizeof(M_SkeletalJoint_t));
	skel->rootNode = Hunk_Alloc(numNodesInSkeleton[structure] * (int)sizeof(ArrayedListNode_t));

	SkeletonCreators[structure](skel->rootJoint, sizeof(M_SkeletalJoint_t), skel->rootNode, 0);
}

void CreateSkeletonInPlace(const int structure, const ModelSkeleton_t* skel)
{
	SkeletonCreators[structure](skel->rootJoint, sizeof(M_SkeletalJoint_t), skel->rootNode, 0);
}

static int GetRootIndex(const int max, const int num_joints)
{
	qboolean skip_block = false;

	for (int i = 0; i < max; i++)
	{
		if (SkeletalClusters[i].inUse)
			continue;

		const int requred_size = num_joints + i;

		// Check the size of the array.
		if (requred_size > max)
			return -1;

		// Check for a big enough unused block.
		for (int j = i + 1; j < requred_size; j++)
		{
			if (SkeletalClusters[j].inUse)
			{
				i = j;
				skip_block = true;

				break;
			}
		}

		// Not a big enough block, so continue searching.
		if (skip_block)
		{
			skip_block = false;
			continue;
		}

		// Found a block, mark it as used.
		for (int j = i; j < requred_size; j++)
			SkeletalClusters[j].inUse = true;

		return i;
	}

	// Couldn't find a block.
	return -1;
}

int CreateSkeleton(const int structure)
{
	const int index = GetRootIndex(MAX_ARRAYED_SKELETAL_JOINTS, numJointsInSkeleton[structure]);
	SkeletonCreators[structure](SkeletalClusters, sizeof(M_SkeletalCluster_t), ClusterNodes, index);

	return index;
}

void ClearSkeleton(ModelSkeleton_t* skel, const int root)
{
	for (int child = skel->rootJoint[root].children; child != ARRAYEDLISTNODE_NULL; child = skel->rootNode[child].next)
	{
		ClearSkeleton(skel, skel->rootNode[child].data);
		FreeNode(skel->rootNode, child);
	}

	skel->rootJoint[root].inUse = false;
}

static void RotateModelSegment(const M_SkeletalJoint_t* joint, vec3_t* modelVerticies, vec3_t angles, const M_SkeletalCluster_t* modelCluster)
{
	matrix3_t rotation;
	matrix3_t rotation2;
	matrix3_t toWorld;
	matrix3_t partialBackToLocal;
	vec3_t localAngles;

	VectorCopy(angles, localAngles);
	localAngles[ROLL] += (float)Matricies3FromDirAndUp(joint->model.direction, joint->model.up, toWorld, partialBackToLocal);

	memset(rotation, 0, sizeof(rotation));
	Matrix3FromAngles(localAngles, rotation);
	Matrix3MultByMatrix3(rotation, toWorld, rotation2);
	Matrix3MultByMatrix3(partialBackToLocal, rotation2, rotation);

	for (int i = 0; i < modelCluster->numVerticies; i++)
		RotatePointAboutLocalOrigin(rotation, joint->model.origin, modelVerticies[modelCluster->verticies[i]]);
}

void RotateModelSegments(ModelSkeleton_t* skel, int jointIndex, int modelClusterIndex, int anglesIndex, vec3_t* modelVerticies)
{
	const M_SkeletalCluster_t* modelCluster = &SkeletalClusters[modelClusterIndex];
	CL_SkeletalJoint_t* modelJointAngles = &ri.skeletalJoints[anglesIndex];
	const M_SkeletalJoint_t* joint = skel->rootJoint + jointIndex;

	if (modelCluster->children != ARRAYEDLISTNODE_NULL)
	{
		int child;
		int child2;
		int jointChild;

		// Big hack here - to avoid a weird assert crash. 
		for (child = modelCluster->children, child2 = modelJointAngles->children, jointChild = joint->children; 
			 child != ARRAYEDLISTNODE_NULL && child2 != ARRAYEDLISTNODE_NULL; 
			 child = ClusterNodes[child].next, child2 = ri.jointNodes[child2].next, jointChild = skel->rootNode[jointChild].next)
		{
			assert(jointChild != ARRAYEDLISTNODE_NULL);
			RotateModelSegments(skel, skel->rootNode[jointChild].data, ClusterNodes[child].data, ri.jointNodes[child2].data, modelVerticies);
		}
	}

	RotateModelSegment(joint, modelVerticies, modelJointAngles->angles, modelCluster);
}

static void TransformPlacement(matrix3_t rotation, vec3_t origin, Placement_t* placement)
{
	RotatePointAboutLocalOrigin(rotation, origin, placement->origin);
	RotatePointAboutLocalOrigin(rotation, origin, placement->direction);
	RotatePointAboutLocalOrigin(rotation, origin, placement->up);
}

static void RotateDecendents(ModelSkeleton_t* skel, M_SkeletalJoint_t* joint, M_SkeletalJoint_t* ancestor)
{
	for (int jointChild = joint->children; jointChild != ARRAYEDLISTNODE_NULL; jointChild = skel->rootNode[jointChild].next)
	{
		joint = skel->rootJoint + skel->rootNode[jointChild].data;

		TransformPlacement(ancestor->rotation, ancestor->parent.origin, &joint->parent);
		RotateDecendents(skel, joint, ancestor);
	}
}

static void RotateJoint(ModelSkeleton_t* skel, M_SkeletalJoint_t* joint, vec3_t angles)
{
	matrix3_t rotation;
	matrix3_t rotation2;
	matrix3_t toWorld;
	matrix3_t partialBackToLocal;
	vec3_t localAngles;

	VectorCopy(angles, localAngles);
	localAngles[ROLL] += (float)Matricies3FromDirAndUp(joint->model.direction, joint->model.up, toWorld, partialBackToLocal);

	memset(rotation, 0, sizeof(rotation));
	Matrix3FromAngles(localAngles, rotation);
	Matrix3MultByMatrix3(rotation, toWorld, rotation2);
	Matrix3MultByMatrix3(partialBackToLocal, rotation2, joint->rotation);

	VectorCopy(joint->model.origin, joint->parent.origin);

	Matrix3MultByVec3(joint->rotation, joint->model.direction, joint->parent.direction);
	Vec3ScaleAssign(10.0f, joint->parent.direction);
	Vec3AddAssign(joint->parent.origin, joint->parent.direction);

	Matrix3MultByVec3(joint->rotation, joint->model.up, joint->parent.up);
	Vec3ScaleAssign(10.0f, joint->parent.up);
	Vec3AddAssign(joint->parent.origin, joint->parent.up);

	RotateDecendents(skel, joint, joint);
}

void SetupJointRotations(ModelSkeleton_t* skel, const int jointIndex, const int anglesIndex)
{
	CL_SkeletalJoint_t* modelJointAngles = &ri.skeletalJoints[anglesIndex];
	M_SkeletalJoint_t* joint = skel->rootJoint + jointIndex;

	if (joint->children != ARRAYEDLISTNODE_NULL)
	{
		int child2;
		int jointChild;

		assert(modelJointAngles->children != ARRAYEDLISTNODE_NULL);

		for (child2 = modelJointAngles->children, jointChild = joint->children; 
			 child2 != ARRAYEDLISTNODE_NULL; 
			 child2 = ri.jointNodes[child2].next, jointChild = skel->rootNode[jointChild].next)
		{
			assert(jointChild != ARRAYEDLISTNODE_NULL);
			SetupJointRotations(skel, skel->rootNode[jointChild].data, ri.jointNodes[child2].data);
		}
	}

	RotateJoint(skel, joint, modelJointAngles->angles);
}

void FinishJointRotations(ModelSkeleton_t* skel, const int jointIndex)
{
	matrix3_t rotation;
	matrix3_t rotation2;
	matrix3_t toWorld;
	matrix3_t partialBackToLocal;
	vec3_t localAngles;

	M_SkeletalJoint_t* joint = skel->rootJoint + jointIndex;

	for (int jointChild = joint->children; jointChild != ARRAYEDLISTNODE_NULL; jointChild = skel->rootNode[jointChild].next)
		FinishJointRotations(skel, skel->rootNode[jointChild].data);

	Vec3SubtractAssign(joint->parent.origin, joint->parent.direction);
	Vec3SubtractAssign(joint->parent.origin, joint->parent.up);

	VectorNormalize(joint->parent.direction);
	VectorNormalize(joint->parent.up);

	localAngles[YAW] = 0;
	localAngles[PITCH] = 0;
	localAngles[ROLL] = (float)Matricies3FromDirAndUp(joint->parent.direction, joint->parent.up, toWorld, partialBackToLocal);

	Matricies3FromDirAndUp(joint->model.direction, joint->model.up, toWorld, NULL);

	memset(rotation, 0, sizeof(rotation));
	Matrix3FromAngles(localAngles, rotation);

	Matrix3MultByMatrix3(rotation, toWorld, rotation2);
	Matrix3MultByMatrix3(partialBackToLocal, rotation2, joint->rotation);
}

void LinearllyInterpolateJoints(ModelSkeleton_t* newSkel, const int newIndex, ModelSkeleton_t* oldSkel, const int oldIndex, ModelSkeleton_t* liSkel, const int liIndex, float move[3], float frontv[3], float backv[3])
{
	M_SkeletalJoint_t* newJoint = newSkel->rootJoint + newIndex;
	M_SkeletalJoint_t* oldJoint = oldSkel->rootJoint + oldIndex;
	M_SkeletalJoint_t* liJoint = liSkel->rootJoint + liIndex;

	if (newJoint->children != ARRAYEDLISTNODE_NULL)
	{
		int newChild;
		int oldChild;
		int liChild;

		assert(oldJoint->children != ARRAYEDLISTNODE_NULL);
		assert(liJoint->children != ARRAYEDLISTNODE_NULL);

		for (newChild = newJoint->children, oldChild = oldJoint->children, liChild = liJoint->children; 
			 newChild != ARRAYEDLISTNODE_NULL;
			 newChild = newSkel->rootNode[newChild].next, oldChild = oldSkel->rootNode[oldChild].next, liChild = liSkel->rootNode[liChild].next)
		{
			assert(oldChild != ARRAYEDLISTNODE_NULL);
			assert(liChild != ARRAYEDLISTNODE_NULL);

			LinearllyInterpolateJoints(newSkel, newSkel->rootNode[newChild].data, oldSkel, oldSkel->rootNode[oldChild].data, liSkel, liSkel->rootNode[liChild].data, move, frontv, backv);
		}
	}

	GL_LerpVert(newJoint->model.origin, oldJoint->model.origin, liJoint->model.origin, move, frontv, backv);

	// Linearly interpolate direction and up vectors, which will un-normalize them relative to their local origin
	GL_LerpVert(newJoint->model.direction, oldJoint->model.direction, liJoint->model.direction, move, frontv, backv);
	GL_LerpVert(newJoint->model.up, oldJoint->model.up, liJoint->model.up, move, frontv, backv);

	Vec3SubtractAssign(liJoint->model.origin, liJoint->model.direction);
	Vec3SubtractAssign(liJoint->model.origin, liJoint->model.up);

	// Re-normalize them
	VectorNormalize(liJoint->model.direction);
	VectorNormalize(liJoint->model.up);
}
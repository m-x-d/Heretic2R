//
// g_Skeletons.c
//
// Copyright 1998 Raven Software
//

#include <stdlib.h>
#include "g_local.h"
#include "g_Skeletons.h"
#include "ArrayedList.h"

G_SkeletalJoint_t skeletalJoints[MAX_ARRAYED_SKELETAL_JOINTS];
ArrayedListNode_t jointNodes[MAX_ARRAYED_JOINT_NODES];

#pragma region ========================== Skeleton Managment ==========================

static int GetRootIndex(const int max, const int numJoints)
{
	qboolean cont = false;

	for (int i = 0; i < max; i++)
	{
		if (!skeletalJoints[i].inUse)
		{
			const int max2 = numJoints + i;

			// Check the size of the array
			if (max2 > max)
				return -1;

			// Check for a big enough unused block
			for (int j = i + 1; j < max2; j++)
			{
				if (skeletalJoints[j].inUse)
				{
					i = j;
					cont = true;

					break;
				}
			}

			// Not a big enough block, so continue searching
			if (cont) 
			{
				cont = false;
				continue;
			}

			// Found a block, mark it as used
			for (int j = i; j < max2; j++)
				skeletalJoints[j].inUse = true;

			return i;
		}
	}

	// Couldn't find a block
	return -1;
}

int CreateSkeleton(const int structure)
{
	const int index = GetRootIndex(MAX_ARRAYED_SKELETAL_JOINTS, numJointsInSkeleton[structure]);
	SkeletonCreators[structure](skeletalJoints, sizeof(G_SkeletalJoint_t), jointNodes, index);

	return index;
}

void FreeSkeleton(const int root)
{
	for (int child = skeletalJoints[root].children; child != ARRAYEDLISTNODE_NULL; child = jointNodes[child].next)
	{
		FreeSkeleton(jointNodes[child].data);
		FreeNode(jointNodes, child);
	}

	skeletalJoints[root].inUse = false;
}

//TODO: unused?
void UpdateSkeletons(void)
{
	for (int i = 0; i < MAX_ARRAYED_SKELETAL_JOINTS; i++)
	{
		G_SkeletalJoint_t* joint = &skeletalJoints[i];

		if (!joint->inUse)
			continue;

		for (int j = 0; j < 3; j++)
		{
			joint->changed[j] = false;

			if (joint->angles[j] == joint->destAngles[j])
				continue;

			joint->angles[j] += joint->angVels[j] * (float)FRAMETIME;

			if (joint->angVels[j] > 0.0f)
				joint->angles[j] = min(joint->destAngles[j], joint->angles[j]);
			else if (joint->angVels[j] < 0.0f)
				joint->angles[j] = max(joint->destAngles[j], joint->angles[j]);
		}
	}
}

#pragma endregion

#pragma region ========================== Skeletal Manipulation (mirrored in cl_Skeletons.c) ==========================

float GetJointAngle(const int jointIndex, const int angleIndex)
{
	return skeletalJoints[jointIndex].angles[angleIndex];
}

qboolean SetJointAngVel(const int jointIndex, const int angleIndex, const float destAngle, const float angSpeed)
{
	const G_SkeletalJoint_t* joint = &skeletalJoints[jointIndex];

	if (destAngle < joint->destAngles[angleIndex])
	{
		skeletalJoints[jointIndex].destAngles[angleIndex] = destAngle;
		skeletalJoints[jointIndex].angVels[angleIndex] = -angSpeed;
		skeletalJoints[jointIndex].changed[angleIndex] = true;

		return true;
	}

	if (destAngle > joint->destAngles[angleIndex])
	{
		skeletalJoints[jointIndex].destAngles[angleIndex] = destAngle;
		skeletalJoints[jointIndex].angVels[angleIndex] = angSpeed;
		skeletalJoints[jointIndex].changed[angleIndex] = true;

		return true;
	}

	return false;
}

#pragma endregion
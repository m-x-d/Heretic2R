//
// g_Skeletons.c
//
// Copyright 1998 Raven Software
//

#include "g_Skeletons.h"
#include "ArrayedList.h"
#include "g_local.h"

G_SkeletalJoint_t skeletalJoints[MAX_ARRAYED_SKELETAL_JOINTS];
ArrayedListNode_t jointNodes[MAX_ARRAYED_JOINT_NODES];

#pragma region ========================== Skeleton management stuff ==========================

static int GetRootIndex(const int max, const int num_joints)
{
	qboolean skip_block = false;

	for (int i = 0; i < max; i++)
	{
		if (skeletalJoints[i].inUse)
			continue;

		const int requred_size = num_joints + i;

		// Check the size of the array.
		if (requred_size > max)
		{
			assert(0);
			return -1;
		}

		// Check for a big enough unused block.
		for (int j = i + 1; j < requred_size; j++)
		{
			if (skeletalJoints[j].inUse)
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
			skeletalJoints[j].inUse = true;

		return i;
	}

	// Couldn't find a block.
	assert(0);
	return -1;
}

int CreateSkeleton(const int structure)
{
	const int index = GetRootIndex(MAX_ARRAYED_SKELETAL_JOINTS, numJointsInSkeleton[structure]);
	SkeletonCreators[structure](skeletalJoints, sizeof(G_SkeletalJoint_t), jointNodes, index);

	return index;
}

void FreeSkeleton(int root)
{
	int child;

	for(child = skeletalJoints[root].children; child != ARRAYEDLISTNODE_NULL; child = jointNodes[child].next)
	{
		FreeSkeleton(jointNodes[child].data);

		FreeNode(jointNodes, child);
	}

	skeletalJoints[root].inUse = false;
}

void UpdateSkeletons(void)
{
	int i, j;

	for(i = 0; i < MAX_ARRAYED_SKELETAL_JOINTS; ++i)
	{
		G_SkeletalJoint_t *joint;
		joint = &skeletalJoints[i];

		if(joint->inUse)
		{
			for(j = 0; j < 3; ++j)
			{
				float destAngle, *angle;

				joint->changed[j] = false;
				destAngle = joint->destAngles[j];
				angle = joint->angles + j;

				if(*angle != destAngle)
				{
					*angle += joint->angVels[j]*FRAMETIME;

					if(joint->angVels[j] > 0)
					{
						if(*angle > destAngle)
						{
							*angle = destAngle;
						}
					}
					else if(joint->angVels[j] < 0)
					{
						if(*angle < destAngle)
						{
							*angle = destAngle;
						}
					}
				}
			}
		}
	}	
}

#pragma endregion

// ********************************************************************
// Skeletal Manipulation stuff
//
// Mirrored in cl_Skeletons.c
// ********************************************************************

float GetJointAngle(int jointIndex, int angleIndex)
{
	return skeletalJoints[jointIndex].angles[angleIndex];
}

qboolean SetJointAngVel(int jointIndex, int angleIndex, float destAngle, float angSpeed)
{
	G_SkeletalJoint_t *joint;

	joint = &skeletalJoints[jointIndex];

	if(destAngle < joint->destAngles[angleIndex])
	{
		skeletalJoints[jointIndex].destAngles[angleIndex] = destAngle;
		skeletalJoints[jointIndex].angVels[angleIndex] = -angSpeed;
		skeletalJoints[jointIndex].changed[angleIndex] = true;
		return true;
	}
	else if(destAngle > joint->destAngles[angleIndex])
	{
		skeletalJoints[jointIndex].destAngles[angleIndex] = destAngle;
		skeletalJoints[jointIndex].angVels[angleIndex] = angSpeed;
		skeletalJoints[jointIndex].changed[angleIndex] = true;
		return true;
	}
	else
	{
		return false;
	}
}
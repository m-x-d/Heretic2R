//
// cl_skeletons.c
//
// Copyright 1998 Raven Software
//

#include "cl_skeletons.h"
#include "Angles.h"

CL_SkeletalJoint_t skeletal_joints[MAX_ARRAYED_SKELETAL_JOINTS];
ArrayedListNode_t joint_nodes[MAX_ARRAYED_JOINT_NODES];

//mxd. Similar to UpdateSkeletons() in game/g_Skeletons.c
void SK_UpdateSkeletons(void)
{
	for (int i = 0; i < MAX_ARRAYED_SKELETAL_JOINTS; i++)
	{
		CL_SkeletalJoint_t* joint = &skeletal_joints[i];

		if (!joint->changed)
			continue;

		for (int j = 0; j < 3; j++)
		{
			if (joint->angles[j] == joint->destAngles[j])
				continue;

			joint->angles[j] += joint->angVels[j] * cls.rframetime;

			if ((joint->angVels[j] < 0.0f && joint->angles[j] < joint->destAngles[j]) ||
				(joint->angVels[j] > 0.0f && joint->angles[j] > joint->destAngles[j]))
			{
				joint->angles[j] = joint->destAngles[j];
			}
		}
	}
}

void SK_ClearJoints(const int joint_index)
{
	int child = skeletal_joints[joint_index].children;

	while (child != -1)
	{
		SK_ClearJoints(joint_nodes[child].data);
		joint_nodes[child].in_use = false;
		child = joint_nodes[child].next;
	}

	skeletal_joints[joint_index].changed = false;
	skeletal_joints[joint_index].inUse = false;
}

//mxd. Similar to SetJointAngVel() in game/g_Skeletons.c.
static qboolean SetJointAngVel(const int joint_index, const int angle_index, const float dest_angle, const float angular_velocity)
{
	CL_SkeletalJoint_t* joint = &skeletal_joints[joint_index];

	if (dest_angle < joint->destAngles[angle_index])
	{
		joint->destAngles[angle_index] = dest_angle;
		joint->angVels[angle_index] = -angular_velocity;

		return true;
	}

	if (dest_angle > joint->destAngles[angle_index])
	{
		joint->destAngles[angle_index] = dest_angle;
		joint->angVels[angle_index] = angular_velocity;

		return true;
	}

	return false;
}

// Set the player model's joint angles.
//mxd. Similar to G_SetJointAngles() in game/p_funcs.c.
void SK_SetJointAngles(const playerinfo_t* playerinfo)
{
	const centity_t* self = playerinfo->self;

	SetJointAngVel(self->current.rootJoint + CORVUS_HEAD, PITCH, playerinfo->targetjointangles[PITCH], ANGLE_45);
	SetJointAngVel(self->current.rootJoint + CORVUS_HEAD, ROLL,  playerinfo->targetjointangles[YAW],   ANGLE_45);

	if (!playerinfo->headjointonly)
	{
		SetJointAngVel(self->current.rootJoint + CORVUS_UPPERBACK, PITCH, playerinfo->targetjointangles[PITCH], ANGLE_45);
		SetJointAngVel(self->current.rootJoint + CORVUS_LOWERBACK, PITCH, playerinfo->targetjointangles[PITCH], ANGLE_45);
		SetJointAngVel(self->current.rootJoint + CORVUS_UPPERBACK, ROLL,  playerinfo->targetjointangles[YAW],   ANGLE_45);
		SetJointAngVel(self->current.rootJoint + CORVUS_LOWERBACK, ROLL,  playerinfo->targetjointangles[YAW],   ANGLE_45);
	}
	else
	{
		SetJointAngVel(self->current.rootJoint + CORVUS_UPPERBACK, PITCH, 0, ANGLE_45);
		SetJointAngVel(self->current.rootJoint + CORVUS_LOWERBACK, PITCH, 0, ANGLE_45);
		SetJointAngVel(self->current.rootJoint + CORVUS_UPPERBACK, ROLL,  0, ANGLE_45);
		SetJointAngVel(self->current.rootJoint + CORVUS_LOWERBACK, ROLL,  0, ANGLE_45);
	}
}

// Reset the player model's joint angles.
//mxd. Similar to G_ResetJointAngles() in game/p_funcs.c.
void SK_ResetJointAngles(const playerinfo_t* playerinfo)
{
	const centity_t* self = playerinfo->self;

	SetJointAngVel(self->current.rootJoint + CORVUS_HEAD,		PITCH, 0.0f, ANGLE_45);
	SetJointAngVel(self->current.rootJoint + CORVUS_UPPERBACK,	PITCH, 0.0f, ANGLE_45);
	SetJointAngVel(self->current.rootJoint + CORVUS_LOWERBACK,	PITCH, 0.0f, ANGLE_45);

	SetJointAngVel(self->current.rootJoint + CORVUS_HEAD,		ROLL, 0.0f, ANGLE_45);
	SetJointAngVel(self->current.rootJoint + CORVUS_UPPERBACK,	ROLL, 0.0f, ANGLE_45);
	SetJointAngVel(self->current.rootJoint + CORVUS_LOWERBACK,	ROLL, 0.0f, ANGLE_45);
}

void SK_CreateSkeleton(const int structure, const int root_index)
{
	if (skeletal_joints[root_index].inUse)
		return;

	SkeletonCreators[structure](skeletal_joints, sizeof(skeletal_joints[0]), joint_nodes, root_index);

	for (int i = 0; i < numJointsInSkeleton[structure]; i++)
		skeletal_joints[root_index + i].changed = true;

	skeletal_joints[root_index].inUse = true;
}
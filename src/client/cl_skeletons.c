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
	NOT_IMPLEMENTED
}

void SK_ClearJoints(int joint_index)
{
	NOT_IMPLEMENTED
}

//mxd. Similar to SetJointAngVel() in game/g_Skeletons.c.
static qboolean SetJointAngVel(const int joint_index, const int angleIndex, const float destAngle, const float angSpeed)
{
	const CL_SkeletalJoint_t* joint = &skeletal_joints[joint_index];

	if (destAngle < joint->destAngles[angleIndex])
	{
		skeletal_joints[joint_index].destAngles[angleIndex] = destAngle;
		skeletal_joints[joint_index].angVels[angleIndex] = -angSpeed;
		return true;
	}

	if (destAngle > joint->destAngles[angleIndex])
	{
		skeletal_joints[joint_index].destAngles[angleIndex] = destAngle;
		skeletal_joints[joint_index].angVels[angleIndex] = angSpeed;
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

void SK_ResetJointAngles(playerinfo_t* playerinfo)
{
	NOT_IMPLEMENTED
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
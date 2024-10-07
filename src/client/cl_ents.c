//
// cl_ents.c -- Entity parsing and management.
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "cl_skeletons.h"
#include "ResourceManager.h"
#include "sound.h"

ResourceManager_t cl_FXBufMngr;
int camera_timer; // H2

static void DeallocateLERPedReference(void* data) // H2
{
	NOT_IMPLEMENTED
}

void CL_ClearSkeletalEntities(void) // H2
{
	S_StopAllSounds();

	if (fxe.Clear != NULL)
		fxe.Clear();

	centity_t* ent = cl_entities;
	for (int i = 0; i < MAX_NETWORKABLE_EDICTS; i++, ent++)
	{
		if (ent->referenceInfo != NULL)
			DeallocateLERPedReference(ent->referenceInfo);

		memset(ent, 0, sizeof(centity_t));

		ent->baseline.skeletalType = -1;
		ent->baseline.rootJoint = -1;
		ent->baseline.swapFrame = -1;
		ent->current.skeletalType = -1;
		ent->current.rootJoint = -1;
		ent->current.swapFrame = -1;
		ent->prev.skeletalType = -1;
		ent->prev.rootJoint = -1;
		ent->prev.swapFrame = -1;
	}

	memset(skeletal_joints, 0, sizeof(skeletal_joints));
	memset(joint_nodes, 0, sizeof(joint_nodes));
}

// Returns the entity number and the header bits.
int CL_ParseEntityBits(byte* bf, byte* bfNonZero)
{
	bfNonZero[0] = (byte)MSG_ReadByte(&net_message);

	for (int i = 0; i < NUM_ENTITY_HEADER_BITS; i++)
	{
		if (GetB(bfNonZero, i))
			bf[i] = (byte)MSG_ReadByte(&net_message);
		else
			bf[i] = 0;
	}

	if (GetB(bf, U_NUMBER16))
		return MSG_ReadShort(&net_message);

	return MSG_ReadByte(&net_message);
}

void CL_ParseDelta(entity_state_t* from, entity_state_t* to, int number, byte* bf)
{
	NOT_IMPLEMENTED
}

void CL_ParseFrame(void)
{
	NOT_IMPLEMENTED
}

void CL_AddEntities(void)
{
	NOT_IMPLEMENTED
}

void CL_GetEntitySoundOrigin(int ent, vec3_t org)
{
	NOT_IMPLEMENTED
}

void CL_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int brushmask, int flags, trace_t* t)
{
	NOT_IMPLEMENTED
}
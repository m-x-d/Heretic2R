#if 0
#include "g_PhysicsInfo.h" 
#include "ResourceManager.h"
#include "g_BoundingForm.h"

#include <memory.h>

static ResourceManager_t PhysicsInfoMngr;

void InitPhysicsInfoMngr()
{
#define PHYSICSINFO_BLOCK_SIZE 64

	ResMngr_Con(&PhysicsInfoMngr, sizeof(PhysicsInfo_t), PHYSICSINFO_BLOCK_SIZE);
}

void ReleasePhysicsInfoMngr()
{
	ResMngr_Des(&PhysicsInfoMngr);
}

PhysicsInfo_t *PhysicsInfo_new()
{
	PhysicsInfo_t *newInfo;

	newInfo = ResMngr_AllocateResource(&PhysicsInfoMngr, sizeof(*newInfo));

	memset(newInfo, 0, sizeof(*newInfo));

	newInfo->gravity = 1.0f;
	newInfo->friction = 1.0f;

	return newInfo;
}

void PhysicsInfo_delete(PhysicsInfo_t *toDelete)
{
	if(toDelete->intentBoundingForm)
	{
		BoundingForm_delete(toDelete->intentBoundingForm);
	}

	ResMngr_DeallocateResource(&PhysicsInfoMngr, toDelete, sizeof(*toDelete));	
}
#endif
//
// ResourceManager.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "H2Common.h"
#include "q_Typedef.h" //mxd. For uint...

typedef struct ResourceManager_s
{
	uint resSize;
	uint resPerBlock;
	uint nodeSize;
	struct ResMngr_Block_s* blockList;
	char** free;
#if _DEBUG
	uint numResourcesAllocated;
#endif
} ResourceManager_t;

extern H2COMMON_API void ResMngr_Con(ResourceManager_t* resource, uint init_resSize, uint init_resPerBlock);
extern H2COMMON_API void ResMngr_Des(ResourceManager_t* resource);
extern H2COMMON_API void* ResMngr_AllocateResource(ResourceManager_t* resource, uint size);
extern H2COMMON_API void ResMngr_DeallocateResource(ResourceManager_t* resource, void* toDeallocate, uint size);

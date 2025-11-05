//
// ResourceManager.c
//
// Copyright 1998 Raven Software
//

#if _DEBUG
	#include <windows.h>
#endif

#include "ResourceManager.h"
#include "q_shared.h"

typedef struct ResMngr_Block_s
{
	char* start;
	uint size;
	struct ResMngr_Block_s* next;
} ResMngr_Block_t;

static void ResMngr_CreateBlock(ResourceManager_t* resource)
{
	const uint block_size = resource->nodeSize * resource->resPerBlock;
	char* block = malloc(block_size);

	assert(block);

	ResMngr_Block_t* temp = malloc(sizeof(ResMngr_Block_t));

	temp->start = block;
	temp->size = block_size;
	temp->next = resource->blockList;

	resource->blockList = temp; 
	resource->free = (char**)block;

	char** current = resource->free;

	for (uint i = 0; i < resource->resPerBlock - 1; i++)
	{
		// Set current->next to point to next node.
		*current = (char*)current + resource->nodeSize;

		// Set current node to current->next.
		current = (char**)(*current);
	}

	//mxd. No current->next for the last block.
	*current = NULL;
}

H2COMMON_API void ResMngr_Con(ResourceManager_t* resource, const uint init_resSize, const uint init_resPerBlock)
{
	resource->resSize = init_resSize;
	resource->resPerBlock = init_resPerBlock;
	resource->nodeSize = resource->resSize + sizeof(*resource->free);
	resource->blockList = NULL;

#if _DEBUG
	resource->numResourcesAllocated = 0;
#endif

	ResMngr_CreateBlock(resource);
}

// ResourceManager destructor.
H2COMMON_API void ResMngr_Des(ResourceManager_t* resource)
{
#if _DEBUG
	if (resource->numResourcesAllocated > 0)
	{
		char msg[100];
		Com_sprintf(msg, sizeof(msg), "Potential memory leak: %d bytes unfreed\n", resource->resSize * resource->numResourcesAllocated); //mxd. sprintf -> Com_sprintf.
		OutputDebugString(msg);
	}
#endif

	while (resource->blockList)
	{
		ResMngr_Block_t* toDelete = resource->blockList;
		resource->blockList = resource->blockList->next;
		free(toDelete->start);
		free(toDelete);
	}
}

H2COMMON_API void* ResMngr_AllocateResource(ResourceManager_t* resource, const uint size)
{
	assert(size == resource->resSize);

#if _DEBUG
	resource->numResourcesAllocated++;
#endif

	// Constructor not called; possibly due to a static object containing a static ResourceManagerFastLarge
	// member being constructed before its own static members.
	assert(resource->free);	

	char** toPop = resource->free;

	// Set unallocated to the next node and check for NULL (end of list).
	resource->free = (char**)(*resource->free);
	if (resource->free == NULL)
		ResMngr_CreateBlock(resource); // If at end, create new block.

	// Set next to NULL.
	*toPop = NULL;

	// Return the resource for the node.
	return toPop + 1;
}

H2COMMON_API void ResMngr_DeallocateResource(ResourceManager_t* resource, void* toDeallocate, const uint size)
{
	assert(size == resource->resSize);

#if _DEBUG
	assert(resource->numResourcesAllocated > 0);
	resource->numResourcesAllocated--;
#endif

	char** toPush = (char**)toDeallocate - 1;

	// See same assert at top of AllocateResource.
	assert(resource->free);	

	// Set toPush->next to current unallocated front.
	*toPush = (char*)resource->free;

	// Set unallocated to the node removed from allocated.
	resource->free = toPush;
}
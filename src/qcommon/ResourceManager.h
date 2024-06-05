//
// ResourceManager.h
//
// Copyright 1998 Raven Software
//

#include "H2Common.h"
#include <stdlib.h>		// needed here for size_t

typedef struct ResourceManager_s
{
	size_t resSize;
	unsigned int resPerBlock;
	unsigned int nodeSize;
	struct ResMngr_Block_s *blockList;
	char **free;
#ifndef NDEBUG
	unsigned numResourcesAllocated;
#endif
} ResourceManager_t;

extern H2COMMON_API void ResMngr_Con(ResourceManager_t *resource, size_t init_resSize, unsigned int init_resPerBlock);
extern H2COMMON_API void ResMngr_Des(ResourceManager_t *resource);
extern H2COMMON_API void *ResMngr_AllocateResource(ResourceManager_t *resource, size_t size);
extern H2COMMON_API void ResMngr_DeallocateResource(ResourceManager_t *resource, void *toDeallocate, size_t size);



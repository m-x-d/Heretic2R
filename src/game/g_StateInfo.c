#include "g_StateInfo.h" 
#include "ResourceManager.h"
#include "g_local.h"

static ResourceManager_t ActionInfoMngr;

void InitActionInfoMngr()
{
#define ActionINFO_BLOCK_SIZE 32

	ResMngr_Con(&ActionInfoMngr, sizeof(ActionInfo_t), ActionINFO_BLOCK_SIZE);
}

void ReleaseActionInfoMngr()
{
	ResMngr_Des(&ActionInfoMngr);
}

ActionInfo_t *ActionInfo_new()
{
	ActionInfo_t *newInfo;

	newInfo = ResMngr_AllocateResource(&ActionInfoMngr, sizeof(*newInfo));

	memset(newInfo, 0, sizeof(*newInfo));

	return newInfo;
}

void ActionInfo_delete(ActionInfo_t *toDelete)
{
	ResMngr_DeallocateResource(&ActionInfoMngr, toDelete, sizeof(*toDelete));	
}

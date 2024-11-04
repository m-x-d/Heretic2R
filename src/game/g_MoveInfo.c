#include "g_MoveInfo.h" 
#include "ResourceManager.h"
#include "g_local.h"

static ResourceManager_t MoveInfoMngr;

void InitMoveInfoMngr()
{
#define MOVEINFO_BLOCK_SIZE 32

	ResMngr_Con(&MoveInfoMngr, sizeof(MoveInfo_t), MOVEINFO_BLOCK_SIZE);
}

void ReleaseMoveInfoMngr()
{
	ResMngr_Des(&MoveInfoMngr);
}

MoveInfo_t *MoveInfo_new()
{
	MoveInfo_t *newInfo;

	newInfo = ResMngr_AllocateResource(&MoveInfoMngr, sizeof(*newInfo));

	memset(newInfo, 0, sizeof(*newInfo));

	newInfo->pivotDirection = 1.0;

	return newInfo;
}

void MoveInfo_delete(MoveInfo_t *toDelete)
{
	ResMngr_DeallocateResource(&MoveInfoMngr, toDelete, sizeof(*toDelete));	
}

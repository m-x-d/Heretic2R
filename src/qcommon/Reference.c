#include <string.h>
#include "Reference.h"
#include "ArrayedList.h"
#include "ResourceManager.h"
#include "Skeletons.h"

char *referenceRootNames[] =
{
	"elf_Lhandroot",//0
	"elf_Rhandroot",
	"elf_Rfootroot",
	"elf_Lfootroot",
	"elf_Bstaffroot",
	"elf_bladeroot",
	"elf_hellroot",
	"StaffBone",//7
	"SwordBone",
	"SpearBone",
	"RFootBone",
	"LFootBone",
	"hp_backroot",//12
	"hp_staffroot",
	"hp_lhandroot",
	"hp_rhandroot",
	"hp_rfootroot",
	"hp_lfootroot",
	"staffroot",//18
	"rfootroot",
	"lfootroot",
	"rhandroot",
	"lhandroot",
	"leyeroot",
	"reyeroot"
};

int referenceRootNameOffsets[NUM_REFERENCED] =
{
	0,	// CORVUS
	7,	// INSECT
	12, // HIGH PRIESTESS
	18,	// MORCALAVIN
};

int numReferences[NUM_REFERENCED] = 
{
	NUM_REFERENCES_CORVUS,
	NUM_REFERENCES_INSECT,
	NUM_REFERENCES_PRIESTESS,
	NUM_REFERENCES_MORK,
};

int corvusJointIDs[NUM_REFERENCES_CORVUS] =
{
	CORVUS_UPPERBACK,
	CORVUS_UPPERBACK,
	-1,
	-1,
	CORVUS_UPPERBACK,
	CORVUS_UPPERBACK,
	CORVUS_UPPERBACK,
};

int *jointIDs[NUM_REFERENCED] =
{
	corvusJointIDs,
};

static ResourceManager_t ReferenceMngr;

void InitReferenceMngr()
{
#define	REFERENCE_BLOCK_SIZE 8

	ResMngr_Con(&ReferenceMngr, sizeof(LERPedReferences_t), REFERENCE_BLOCK_SIZE);
}

void ReleaseReferenceMngr()
{
	ResMngr_Des(&ReferenceMngr);
}

LERPedReferences_t *LERPedReferences_new(int init_refType)
{
	LERPedReferences_t	*newRefs;

	newRefs = ResMngr_AllocateResource(&ReferenceMngr, sizeof(*newRefs));
	newRefs->refType = init_refType;
	newRefs->jointIDs = jointIDs[init_refType];
	newRefs->lastUpdate = -(REF_MINCULLTIME*2.0);

	memset(newRefs->references, 0, MAX_REFPOINTS*sizeof(Reference_t));
	memset(newRefs->oldReferences, 0, MAX_REFPOINTS*sizeof(Reference_t));

	return newRefs;
}

void LERPedReferences_delete(LERPedReferences_t *toDelete)
{
	ResMngr_DeallocateResource(&ReferenceMngr, toDelete, sizeof(*toDelete));
}

// end
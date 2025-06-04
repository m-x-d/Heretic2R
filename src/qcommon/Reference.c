//
// Reference.c
//
// Copyright 1998 Raven Software
//

#include "Reference.h"
#include "Skeletons.h"

int numReferences[NUM_REFERENCED] =
{
	NUM_REFERENCES_CORVUS,
	NUM_REFERENCES_INSECT,
	NUM_REFERENCES_PRIESTESS,
	NUM_REFERENCES_MORK,
};

static int corvusJointIDs[NUM_REFERENCES_CORVUS] =
{
	CORVUS_UPPERBACK,
	CORVUS_UPPERBACK,
	-1,
	-1,
	CORVUS_UPPERBACK,
	CORVUS_UPPERBACK,
	CORVUS_UPPERBACK,
};

int* jointIDs[NUM_REFERENCED] =
{
	corvusJointIDs,
};
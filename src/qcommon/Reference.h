//
// Reference.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "Placement.h"

#define MAX_REFPOINTS			16
#define REF_MINCULLTIME			1.0f //mxd. double -> float
#define LERPEDREF_SIZE			1292 //mxd. == sizeof(LERPedReferences_t)
#define LERPEDREF_BLOCK_SIZE	8 //mxd

typedef struct Reference_s
{
	int activecount; //TODO: unused. Replace Reference_t with Placement_t?.. Will need to change LERPEDREF_SIZE as well (or replace with sizeof(LERPedReferences_t)).
	Placement_t placement;
} Reference_t;

typedef struct LERPedReferences_s
{
	int refType; //TODO: set, but never used?
	int* jointIDs;
	float lastUpdate;
	Reference_t references[MAX_REFPOINTS];
	Reference_t oldReferences[MAX_REFPOINTS];
} LERPedReferences_t;

// Reference Types.
enum
{
	REF_NULL = -1,
	REF_CORVUS,
	REF_INSECT,
	REF_PRIESTESS,
	REF_MORK,

	NUM_REFERENCED
};

// Corvus Reference Points.
enum
{
	CORVUS_LEFTHAND,
	CORVUS_RIGHTHAND,
	CORVUS_LEFTFOOT,
	CORVUS_RIGHTFOOT,
	CORVUS_STAFF,
	CORVUS_BLADE,
	CORVUS_HELL_HEAD,

	NUM_REFERENCES_CORVUS
};

// Tchekrik Reference Points.
enum
{
	INSECT_STAFF,
	INSECT_SWORD,
	INSECT_SPEAR,
	INSECT_RIGHTFOOT,
	INSECT_LEFTFOOT,

	NUM_REFERENCES_INSECT
};

// High Priestess Reference Points.
enum
{
	PRIESTESS_BACK,
	PRIESTESS_STAFF,
	PRIESTESS_LHAND,
	PRIESTESS_RHAND,
	PRIESTESS_RFOOT,
	PRIESTESS_LFOOT,

	NUM_REFERENCES_PRIESTESS
};

// Morcalavin Reference Points.
enum 
{
	MORK_STAFFREF,
	MORK_RFOOTREF,
	MORK_LFOOTREF,
	MORK_RHANDREF,
	MORK_LHANDREF,
	MORK_LEYEREF,
	MORK_REYEREF,

	NUM_REFERENCES_MORK
};

#define CORVUS_LIMBS_MASK	((1 << CORVUS_LEFTHAND) | (1 << CORVUS_RIGHTHAND) | (1 << CORVUS_LEFTFOOT) | (1 << CORVUS_RIGHTFOOT))
#define CORVUS_WEAPON_MASK	((1 << CORVUS_STAFF) | (1 << CORVUS_BLADE) | (1 << CORVUS_HELL_HEAD))
#define CORVUS_MASK			(CORVUS_LIMBS_MASK | CORVUS_WEAPON_MASK)

#define INSECT_MASK			((1 << INSECT_STAFF) | (1 << INSECT_SWORD) | (1 << INSECT_SPEAR) | (1 << INSECT_RIGHTFOOT) | (1 << INSECT_LEFTFOOT))
#define PRIESTESS_MASK		((1 << PRIESTESS_BACK) | (1 << PRIESTESS_STAFF) | (1 << PRIESTESS_LHAND) | (1 << PRIESTESS_RHAND) | (1 << PRIESTESS_RFOOT) | (1 << PRIESTESS_LFOOT))
#define MORK_MASK			((1 << MORK_STAFFREF) | (1 << MORK_RFOOTREF) | (1 << MORK_LFOOTREF) | (1 << MORK_RHANDREF) | (1 << MORK_LHANDREF) | (1 << MORK_LEYEREF) | (1 << MORK_REYEREF))

extern int numReferences[];
extern int* jointIDs[NUM_REFERENCED]; //mxd

extern void EnableRefPoints(LERPedReferences_t* ref_info, int mask);
extern void DisableRefPoints(LERPedReferences_t* ref_info, int mask);
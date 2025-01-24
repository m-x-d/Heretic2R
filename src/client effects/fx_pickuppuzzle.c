//
// fx_PuzzlePickup.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "items.h"

#define BOB_HEIGHT	6.0f
#define BOB_SPEED	ANGLE_10

typedef struct PuzzleModel
{
	char* model_name;
	struct model_s* model;
	float scale;
} PuzzleModel_t;

static PuzzleModel_t puzzle_models[] =
{
	{ "models/items/puzzles/townkey/tris.fm",		NULL, 1.5f },	// ITEM_TOWNKEY
	{ "models/items/puzzles/cog/tris.fm",			NULL, 1.0f },	// ITEM_COG
	{ "models/items/puzzles/shield/tris.fm",		NULL, 1.5f },	// ITEM_SHIELD
	{ "models/items/puzzles/potion/tris.fm",		NULL, 0.5f },	// ITEM_POTION
	{ "models/items/puzzles/plazajug/tris.fm",		NULL, 1.0f },	// ITEM_CONT
	{ "models/items/puzzles/jugfull/tris.fm",		NULL, 1.0f },	// ITEM_SLUMCONT
	{ "models/items/puzzles/crystalshard/tris.fm",	NULL, 1.75f },	// ITEM_CRYSTAL
	{ "models/items/puzzles/hivekey/tris.fm",		NULL, 1.0f },	// ITEM_CANKEY
	{ "models/items/puzzles/amulet/tris.fm",		NULL, 1.5f },	// ITEM_AMULET
	{ "models/items/puzzles/spear/tris.fm",			NULL, 1.0f },	// ITEM_SPEAR
	{ "models/items/puzzles/tcheckrikgem/tris.fm",	NULL, 1.5f },	// ITEM_GEM
	{ "models/items/puzzles/wheel/tris.fm",			NULL, 1.75f },	// ITEM_WHEEL
	{ "models/items/puzzles/oreunrefined/tris.fm",	NULL, 0.5f },	// ITEM_ORE
	{ "models/items/puzzles/orerefined/tris.fm",	NULL, 0.5f },	// ITEM_REF_ORE
	{ "models/items/puzzles/dungeonkey/tris.fm",	NULL, 0.5f },	// ITEM_DUNKEY
	{ "models/items/puzzles/cloudkey/tris.fm",		NULL, 1.5f },	// ITEM_CLOUDKEY
	{ "models/items/puzzles/hivekey/tris.fm",		NULL, 1.0f },	// ITEM_HIVEKEY
	{ "models/items/puzzles/hiveidol/tris.fm",		NULL, 1.0f },	// ITEM_HPSYM
	{ "models/items/puzzles/book/tris.fm",			NULL, 1.0f },	// ITEM_TOME
	{ "models/items/puzzles/townkey/tris.fm",		NULL, 1.5f },	// ITEM_TAVERNKEY
};

void PreCachePuzzleItems(void)
{
	for (int i = 0; i < ITEM_TOTAL; i++)
		puzzle_models[i].model = fxi.RegisterModel(puzzle_models[i].model_name);
}

// --------------------------------------------------------------

static qboolean FXPuzzlePickupThink(struct client_entity_s *self, centity_t *owner)
{
	// Rotate and bob
	VectorCopy(owner->current.origin, self->r.origin);
	self->r.origin[2] += (cos(self->SpawnData) * BOB_HEIGHT); 
	self->SpawnData += BOB_SPEED;

	return(true);
}

void FXPuzzlePickup(centity_t *owner, int type, int flags, vec3_t origin)
{
	client_entity_t		*ce;
	byte				tag;
	vec3_t angles;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_PUZZLE].formatString, &tag,&angles);

	flags &= ~CEF_OWNERS_ORIGIN;
	ce = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 50);
	VectorDegreesToRadians(angles, ce->r.angles);

	VectorCopy(ce->r.origin, ce->origin);
	ce->r.model = &puzzle_models[tag].model;
	ce->r.flags = RF_TRANSLUCENT | RF_GLOW;
	ce->r.scale = puzzle_models[tag].scale;

	ce->radius = 10.0;
	ce->alpha = 0.8;
	ce->Update = FXPuzzlePickupThink;

	if (tag == ITEM_TAVERNKEY)
		ce->r.skinnum = 1;

	else if (tag == ITEM_CANKEY)
		ce->r.skinnum = 1;

	AddEffect(owner, ce);
}

// end

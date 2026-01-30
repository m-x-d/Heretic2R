//
// fx_PuzzlePickup.c -- Named 'fx_pickuppuzzle.c' in original logic --mxd.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Vector.h"
#include "items.h"
#include "Utilities.h"

#define BOB_HEIGHT		6.0f
#define BOB_SPEED		ANGLE_10
#define ANIMATION_SPEED	50 //mxd

static struct model_s* pickup_halo_model; //mxd

typedef struct PuzzleModel
{
	char* model_name;
	struct model_s* model;
	float scale;
	float halo_scale; //mxd
	qboolean do_bob_effect; //mxd
} PuzzleModel_t;

static PuzzleModel_t puzzle_models[] =
{
	{ "models/items/puzzles/townkey/tris.fm",		NULL, 1.5f,  1.5f, true },	// ITEM_TOWNKEY
	{ "models/items/puzzles/cog/tris.fm",			NULL, 1.0f,  1.3f, true },	// ITEM_COG
	{ "models/items/puzzles/shield/tris.fm",		NULL, 1.5f,  2.0f, false },	// ITEM_SHIELD
	{ "models/items/puzzles/potion/tris.fm",		NULL, 0.5f,  1.0f, true },	// ITEM_POTION
	{ "models/items/puzzles/plazajug/tris.fm",		NULL, 1.0f,  1.0f, true },	// ITEM_CONT
	{ "models/items/puzzles/jugfull/tris.fm",		NULL, 1.0f,  1.0f, true },	// ITEM_SLUMCONT
	{ "models/items/puzzles/crystalshard/tris.fm",	NULL, 1.75f, 1.75f, true },	// ITEM_CRYSTAL
	{ "models/items/puzzles/hivekey/tris.fm",		NULL, 1.0f,  1.0f, true },	// ITEM_CANKEY
	{ "models/items/puzzles/amulet/tris.fm",		NULL, 1.5f,  1.5f, true },	// ITEM_AMULET
	{ "models/items/puzzles/spear/tris.fm",			NULL, 1.0f,  1.2f, false },	// ITEM_SPEAR
	{ "models/items/puzzles/tcheckrikgem/tris.fm",	NULL, 1.5f,  1.5f, true },	// ITEM_GEM
	{ "models/items/puzzles/wheel/tris.fm",			NULL, 1.75f, 1.75f, true },	// ITEM_WHEEL
	{ "models/items/puzzles/oreunrefined/tris.fm",	NULL, 0.5f,  1.0f, true },	// ITEM_ORE
	{ "models/items/puzzles/orerefined/tris.fm",	NULL, 0.5f,  1.0f, true },	// ITEM_REF_ORE
	{ "models/items/puzzles/dungeonkey/tris.fm",	NULL, 0.5f,  1.0f, true },	// ITEM_DUNKEY
	{ "models/items/puzzles/cloudkey/tris.fm",		NULL, 1.5f,  1.0f, true },	// ITEM_CLOUDKEY
	{ "models/items/puzzles/hivekey/tris.fm",		NULL, 1.0f,  1.0f, true },	// ITEM_HIVEKEY
	{ "models/items/puzzles/hiveidol/tris.fm",		NULL, 1.0f,  1.0f, true },	// ITEM_HPSYM
	{ "models/items/puzzles/book/tris.fm",			NULL, 1.0f,  1.0f, true },	// ITEM_TOME
	{ "models/items/puzzles/townkey/tris.fm",		NULL, 1.5f,  1.0f, true },	// ITEM_TAVERNKEY
};

void PreCachePuzzleItems(void)
{
	for (int i = 0; i < ITEM_TOTAL; i++)
		puzzle_models[i].model = fxi.RegisterModel(puzzle_models[i].model_name);

	pickup_halo_model = fxi.RegisterModel("sprites/Spells/bluball.sp2"); //mxd
}

static qboolean PuzzlePickupUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXPuzzlePickupThink' in original logic.
{
	VectorCopy(owner->origin, self->r.origin); //mxd. Use interpolated origin (to make items dropped by Drop_Item() fly smoothly).

	//mxd. Some puzzle items require exact positioning...
	if (!puzzle_models[self->puzzle_pickup_tag].do_bob_effect)
		return true;

	// Bob, but don't rotate.
	const int step = fx_time - self->nextThinkTime; //mxd
	const float lerp = (float)step / ANIMATION_SPEED; //mxd

	self->r.origin[2] += cosf(self->SpawnData) * BOB_HEIGHT;
	self->SpawnData += BOB_SPEED * lerp;

	if (self->SpawnData > ANGLE_360)
		self->SpawnData = fmodf(self->SpawnData, ANGLE_360); //mxd. Otherwise SpawnData will eventually (in ~8 minutes of runtime) cause floating point precision errors...

	return true;
}

void FXPuzzlePickup(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte tag;
	vec3_t angles;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_PICKUP_PUZZLE].formatString, &tag, &angles);

	const float bob_phase = GetPickupBobPhase(origin); //mxd

	// Create pickup model.
	flags &= ~CEF_OWNERS_ORIGIN;
	client_entity_t* ce = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 0); //mxd. next_think_time 50 in original logic. Set to 0, so self->nextThinkTime holds previous update time in AmmoPickupThink()...

	ce->radius = 10.0f;
	ce->r.model = &puzzle_models[tag].model;
	ce->r.scale = puzzle_models[tag].scale;
	ce->r.flags = RF_GLOW; //mxd. Remove RF_TRANSLUCENT flag and 0.8 alpha (looks broken with those enabled).

	if (tag == ITEM_CRYSTAL || tag == ITEM_GEM || tag == ITEM_POTION || tag == ITEM_CONT || tag == ITEM_SLUMCONT) //mxd
	{
		ce->r.flags |= (RF_TRANSLUCENT | RF_TRANS_ADD);
		ce->alpha = 0.7f;
	}

	if (tag == ITEM_TAVERNKEY || tag == ITEM_CANKEY)
		ce->r.skinnum = 1;

	VectorDegreesToRadians(angles, ce->r.angles);

	ce->puzzle_pickup_tag = tag; //mxd
	ce->SpawnData = bob_phase;
	ce->Update = PuzzlePickupUpdate;

	AddEffect(owner, ce);

	//mxd. Create pickup halo.
	client_entity_t* halo = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 0);

	halo->radius = 10.0f;
	halo->r.model = &pickup_halo_model;
	halo->r.flags = RF_TRANSLUCENT;
	halo->r.scale = puzzle_models[tag].halo_scale;
	halo->alpha = 0.15f;

	halo->puzzle_pickup_tag = tag;
	halo->SpawnData = bob_phase;
	halo->Update = PuzzlePickupUpdate;

	AddEffect(owner, halo);
}
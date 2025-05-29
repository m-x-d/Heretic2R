//
// fx_debris.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "fx_debris.h"
#include "fx_debris_local.h" //mxd
#include "fx_blood.h" //mxd
#include "fx_sparks.h" //mxd
#include "ce_DefaultMessageHandler.h"
#include "Particle.h"
#include "fx_dustpuff.h"
#include "Utilities.h"
#include "Angles.h"
#include "Random.h"
#include "Vector.h"
#include "q_Physics.h"
#include "ce_DLight.h"
#include "fx_smoke.h"
#include "g_playstats.h"

#pragma region ========================== Debris base info ==========================

typedef struct DebrisChunk
{
	char* modelName;
	int skinNum;
	struct model_s* model;
	float mass;
} DebrisChunk_t;

static int debris_chunk_offsets[NUM_MAT + 1] =
{
	0,	// Stone.
	4,	// Grey stone.
	8,	// Cloth.
	12,	// Metal.
	16,	// Flesh.
	22,	// Pottery.
	26,	// Glass.
	31,	// Leaf.
	34,	// Wood.
	38,	// Brown Stone.
	42,	// Nothing - just smoke.
	43,	// Insect chunks.

	51	// Total debris_chunks count.
};

static DebrisChunk_t debris_chunks[] =
{
	// Stone.
	{ "models/debris/stone/schunk1/tris.fm", 0, NULL, 3.0f },	// 0
	{ "models/debris/stone/schunk2/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/stone/schunk3/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/stone/schunk4/tris.fm", 0, NULL, 3.0f },

	// Grey stone.
	{ "models/debris/stone/schunk1/tris.fm", 0, NULL, 3.0f },	// 4
	{ "models/debris/stone/schunk2/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/stone/schunk3/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/stone/schunk4/tris.fm", 0, NULL, 3.0f },

	// Cloth.
	{ "models/debris/pottery/pot1/tris.fm", 0, NULL, 0.2f },	// 8
	{ "models/debris/pottery/pot1/tris.fm", 0, NULL, 0.2f },
	{ "models/debris/pottery/pot1/tris.fm", 0, NULL, 0.3f },
	{ "models/debris/pottery/pot1/tris.fm", 0, NULL, 0.4f },

	// Metal.
	{ "models/debris/metal/mchunk1/tris.fm", 0, NULL, 2.0f },	// 12
	{ "models/debris/metal/mchunk2/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/metal/mchunk3/tris.fm", 0, NULL, 4.0f },
	{ "models/debris/metal/mchunk4/tris.fm", 0, NULL, 5.0f },

	// Flesh.
	{ "models/debris/meat/chunk1/tris.fm", 0, NULL, 3.0f },		// 16
	{ "models/debris/meat/chunk2/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/meat/chunk3/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/meat/chunk4/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/meat/chunk5/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/meat/chunk6/tris.fm", 0, NULL, 3.0f },

	// Pottery.
	{ "models/debris/pottery/pot1/tris.fm", 0, NULL, 2.0f },	// 22
	{ "models/debris/pottery/pot2/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/pottery/pot3/tris.fm", 0, NULL, 2.5f },
	{ "models/debris/pottery/pot4/tris.fm", 0, NULL, 1.4f },

	// Glass - models need different skins.
	{ "models/debris/wood/splinter1/tris.fm", 0, NULL, 1.8f },	// 26
	{ "models/debris/wood/splinter2/tris.fm", 0, NULL, 2.0f },
	{ "models/debris/wood/splinter3/tris.fm", 0, NULL, 1.9f },
	{ "models/debris/wood/splinter4/tris.fm", 0, NULL, 1.6f },
	{ "models/debris/wood/splinter1/tris.fm", 0, NULL, 1.8f },

	// Leaf - invalid debris type.
	{ "models/debris/pottery/pot1/tris.fm", 0, NULL, 2.0f },	// 31
	{ "models/debris/pottery/pot1/tris.fm", 0, NULL, 2.0f },
	{ "models/debris/pottery/pot1/tris.fm", 0, NULL, 2.0f },

	// Wood chunks.
	{ "models/debris/wood/splinter1/tris.fm", 0, NULL, 1.8f },	// 34
	{ "models/debris/wood/splinter2/tris.fm", 0, NULL, 2.0f },
	{ "models/debris/wood/splinter3/tris.fm", 0, NULL, 1.9f },
	{ "models/debris/wood/splinter4/tris.fm", 0, NULL, 1.6f },

	// Brown stone.
	{ "models/debris/stone/schunk1/tris.fm", 1, NULL, 3.0f },	// 38
	{ "models/debris/stone/schunk2/tris.fm", 1, NULL, 3.0f },
	{ "models/debris/stone/schunk3/tris.fm", 1, NULL, 3.0f },
	{ "models/debris/stone/schunk4/tris.fm", 1, NULL, 3.0f },

	// Nothing - just smoke.
	{ "models/debris/meat/chunk6/tris.fm", 0, NULL, 3.0f },		// 42

	// Insect Chunks - fixme, use diff skins for diff bugs...
	{ "models/debris/pottery/pot1/tris.fm",  2, NULL, 2.0f },	// 43
	{ "models/debris/pottery/pot2/tris.fm",  2, NULL, 3.0f },
	{ "models/debris/pottery/pot3/tris.fm",  2, NULL, 2.5f },
	{ "models/debris/pottery/pot4/tris.fm",  2, NULL, 1.4f },
	{ "models/debris/insect/chunk1/tris.fm", 0, NULL, 2.0f },
	{ "models/debris/insect/chunk2/tris.fm", 0, NULL, 3.0f },
	{ "models/debris/insect/chunk3/tris.fm", 0, NULL, 2.5f },
	{ "models/debris/insect/chunk4/tris.fm", 0, NULL, 1.4f },
};

static float debris_elasticity[NUM_MAT] =
{
	1.3f,	// Stone.
	1.3f,	// Grey Stone.
	1.1f,	// Cloth.
	1.4f,	// Metal.
	1.2f,	// Flesh.
	1.3f,	// Pottery.
	1.3f,	// Glass.
	1.1f,	// Leaf.
	1.5f,	// Wood.
	1.3f,	// Brown Stone.
	1.0f,	// Nothing - just smoke.
	1.3f,	// Insect chunks.
};

#pragma endregion

void InitDebrisStatics(void)
{
	ce_class_statics[CID_DEBRIS].msgReceivers[MSG_COLLISION] = Debris_Collision;
}

void PreCacheDebris(void)
{
	for (uint i = 0; i < sizeof(debris_chunks) / sizeof(debris_chunks[0]); i++)
		debris_chunks[i].model = fxi.RegisterModel(debris_chunks[i].modelName);
}

static void DoFireTrail(client_entity_t* spawner)
{
	if (spawner->flags & CEF_FLAG7 && spawner->dlight != NULL && spawner->dlight->intensity <= 0.0f)
	{
		// Flame out.
		spawner->flags &= ~CEF_FLAG6;
		return;
	}

	const paletteRGBA_t color = { .c = 0xe5007fff };
	const int material = (spawner->SpawnInfo & SIF_FLAG_MASK);
	const qboolean is_flesh = (material == MAT_FLESH || material == MAT_INSECT || spawner->effectID == FX_BODYPART); //mxd
	const float master_scale = spawner->r.scale * (is_flesh ? 3.33f : 1.0f);
	const int flame_duration = (r_detail->value < DETAIL_NORMAL ? 700 : 1000);
	const int count = GetScaledCount(irand(2, 5), 0.3f);

	for (int i = 0; i < count; i++)
	{
		client_particle_t* flame = ClientParticle_new(irand(PART_16x16_FIRE1, PART_16x16_FIRE3), color, flame_duration);

		const float radius = spawner->r.scale * 2.0f;
		VectorSet(flame->origin, flrand(-radius, radius), flrand(-radius, radius), flrand(-4.0f, -2.0f) * spawner->r.scale);
		VectorAdd(flame->origin, spawner->r.origin, flame->origin);

		flame->scale = master_scale;
		VectorSet(flame->velocity, flrand(-1.0f, 1.0f), flrand(-1.0f, 1.0f), flrand(17.0f, 20.0f));
		flame->acceleration[2] = 32.0f * spawner->r.scale;
		flame->d_scale = flrand(-5.0f, -2.5f);
		flame->d_alpha = flrand(-200.0f, -160.0f);
		flame->duration = (int)((255.0f * (float)flame_duration) / -flame->d_alpha); // Time taken to reach zero alpha.
		flame->startTime = fxi.cl->time;
		flame->type |= PFL_ADDITIVE;

		AddParticleToList(spawner, flame);
	}
}

//mxd. Added to reduce code duplication.
static qboolean IsInWater(const vec3_t origin)
{
	// Not-very-perfect way of doing a pointcontents from the FX dll.
	vec3_t start;
	VectorCopy(origin, start);
	start[2] += 1.0f;

	trace_t trace;
	fxi.Trace(start, vec3_origin, vec3_origin, origin, 0, 0, &trace);

	return (trace.contents & MASK_WATER);
}

#pragma region ========================== Body Part spawn functions ==========================

void FXBodyPart(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	short frame;
	short body_part;
	byte damage;
	byte modelindex;
	byte owner_entnum;

	// Increase count on owner so that can have multiple effects?
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_BODYPART].formatString, &frame, &body_part, &damage, &modelindex, &owner_entnum);

	const centity_t* realowner = &fxi.server_entities[owner_entnum];

	float ke = (float)damage * 10000.0f;
	ke = max(10000.0f, ke);

	BodyPart_Spawn(realowner, body_part, origin, ke, frame, type, modelindex, flags, owner);
}

static void BodyPart_Spawn(const centity_t* owner, const int body_part, vec3_t origin, const float ke, const int frame, const int type, const byte modelindex, const int flags, centity_t* harpy)
{
	BodyPart_Throw(owner, body_part, origin, ke, frame, type, modelindex, flags, harpy);

	if (ke > 0.0f && type != FX_THROWWEAPON)
		DoBloodSplash(origin, 5, flags & CEF_FLAG8);
}

static void BodyPart_Throw(const centity_t* owner, const int body_part, vec3_t origin, float ke, const int frame, const int type, const byte modelindex, const int flags, centity_t* harpy) //mxd. Named 'FXBodyPart_Throw' in original logic.
{
	//FIXME: make sure parts have correct skins, even node 0!
	client_entity_t* gib = ClientEntity_new(type, 0, origin, NULL, 17); //flags sent as 0

	int material;
	if (type == FX_THROWWEAPON) // Not elastic enough for effect?
		material = MAT_METAL;
	else if (flags & CEF_FLAG8) // Insect material type.
		material = MAT_INSECT;
	else
		material = MAT_FLESH;

	gib->SpawnInfo = material;
	gib->classID = CID_DEBRIS; //FIXME: when do I run out of these? - ie, when is debris invalid?
	gib->msgHandler = CE_DefaultMsgHandler;

	if (modelindex == 255) //FIXME: these will tilt up and down with the player's torso!!!
	{
		// Player special model.
		gib->r.model = fxi.cl->clientinfo[owner->baseline.number - 1].model;
		if (gib->r.model == NULL)
			gib->r.model = fxi.cl->baseclientinfo.model;
	}
	else
	{
		gib->r.model = &fxi.cl->model_draw[modelindex];
	}

	gib->r.fmnodeinfo = FMNodeInfo_new();
	gib->r.frame = frame; // First frame should be parts frame of a flexmodel.
	gib->r.skinnum = owner->current.skinnum; // Need to copy base skin also.

	int node_num = 1;
	for (int whichnode = 1; whichnode <= 16384; whichnode *= 2, node_num++) // Bitwise.
	{
		if (body_part & whichnode)
		{
			gib->r.fmnodeinfo[node_num] = owner->current.fmnodeinfo[node_num]; // Copy skins and flags and colors.
			gib->r.fmnodeinfo[node_num].flags &= ~FMNI_NO_DRAW;
		}
		else
		{
			gib->r.fmnodeinfo[node_num].flags |= FMNI_NO_DRAW;
		}
	}

	// Turn off first node?
	if (modelindex != 255 || !(body_part & 1))
	{
		gib->r.fmnodeinfo[0].flags |= FMNI_NO_DRAW;
	}
	else
	{
		gib->r.fmnodeinfo[0] = owner->current.fmnodeinfo[0]; // Copy skins and flags and colors.
		gib->r.fmnodeinfo[0].flags &= ~FMNI_NO_DRAW;
	}

	gib->flags |= (CEF_CLIP_TO_WORLD | CEF_ABSOLUTE_PARTS);
	gib->r.skinnum = (owner->entity != NULL ? owner->entity->skinnum : 0);
	gib->radius = 2.0f;

	if (harpy != NULL) //HACK: harpy took it!
	{
		gib->flags |= CEF_OWNERS_ORIGIN;
		gib->Update = BodyPartAttachedUpdate;
		gib->updateTime = 50;
		AddEffect(harpy, gib);

		return;
	}

	const vec3_t dir = { 0.0f, 0.0f, 1.0f };
	VectorRandomCopy(dir, gib->velocity, 0.5f);

	if (ke == 0.0f)
	{
		ke = flrand(10.0f, 100.0f) * 10000.0f;
		gib->color.c = 0x00000000;
	}
	else
	{
		gib->color = color_white; //mxd
	}

	const int chunk_index = irand(debris_chunk_offsets[material], debris_chunk_offsets[material + 1] - 1);
	Vec3ScaleAssign(sqrtf(ke / debris_chunks[chunk_index].mass), gib->velocity);

	gib->acceleration[2] = GetGravity();

	gib->r.angles[0] = flrand(-ANGLE_180, ANGLE_180);
	gib->r.angles[1] = flrand(-ANGLE_90, ANGLE_90);

	gib->elasticity = debris_elasticity[MAT_FLESH];

	gib->Update = BodyPart_Update;
	gib->updateTime = 50;

	const int detail = (int)r_detail->value; //mxd

	if (detail == DETAIL_LOW)
		gib->LifeTime = fxi.cl->time + 1000;
	else if (detail == DETAIL_NORMAL)
		gib->LifeTime = fxi.cl->time + 3000;
	else if (detail == DETAIL_HIGH) //mxd. Same as DETAIL_NORMAL in original logic.
		gib->LifeTime = fxi.cl->time + 6000;
	else // DETAIL_UBERHIGH
		gib->LifeTime = fxi.cl->time + 10000;

	if ((flags & CEF_FLAG6) && !IsInWater(origin)) // On fire - add dynamic light.
	{
		if (!ref_soft && detail > DETAIL_NORMAL) //mxd. '== DETAIL_HIGH' in original logic (ignores DETAIL_UBERHIGH).
		{
			const paletteRGBA_t color = { .c = 0xe5007fff }; //TODO: randomize color a bit?

			gib->flags |= CEF_FLAG7; // Don't spawn blood too, just flames.
			gib->dlight = CE_DLight_new(color, 50.0f, -5.0f);
		}

		gib->flags |= CEF_FLAG6;
	}

	AddEffect(NULL, gib);
}

static qboolean BodyPartAttachedUpdate(struct client_entity_s* self, centity_t* owner)
{
	VectorCopy(owner->lerp_origin, self->r.origin);
	VectorSet(self->r.angles,
		owner->lerp_angles[PITCH] * -1.0f * ANGLE_TO_RAD,
		owner->lerp_angles[YAW] * ANGLE_TO_RAD,
		owner->lerp_angles[ROLL] * ANGLE_TO_RAD);

	if ((self->SpawnInfo & SIF_FLAG_MASK) == MAT_FLESH || (self->SpawnInfo & SIF_FLAG_MASK) == MAT_INSECT)
		DoBloodTrail(self, -1);

	if (self->flags & CEF_FLAG6) // On fire - do a fire trail.
		DoFireTrail(self);

	return true;
}

static qboolean BodyPart_Update(struct client_entity_s* self, centity_t* owner) //mxd. Named 'FXBodyPart_Update' in original logic.
{
	const int cur_time = fxi.cl->time;
	const float d_time = (float)(cur_time - self->lastThinkTime) / 1000.0f;

	if (cur_time > self->LifeTime)
	{
		self->d_alpha = flrand(-0.05f, -0.2f);
		self->Update = FXDebris_Vanish;

		return true;
	}

	for (int i = 0; i < 3; i++)
		self->r.angles[i] += d_time * ANGLE_360;

	self->lastThinkTime = cur_time;

	if ((self->SpawnInfo & SIF_FLAG_MASK) == MAT_FLESH || (self->SpawnInfo & SIF_FLAG_MASK) == MAT_INSECT)
		DoBloodTrail(self, 6);

	if (self->flags & CEF_FLAG6) // On fire - do a fire trail.
		DoFireTrail(self);

	return true;
}

#pragma endregion

#pragma region ========================== Debris spawn functions ==========================

// CEF_FLAG6 = on fire.
// CEF_FLAG7 = male insect skin on mat_insect (CEF_FLAG7 cleared out and set if has dynamic light for fire).
// CEF_FLAG8 = use reflection skin.

client_entity_t* FXDebris_Throw(const vec3_t origin, const int material, const vec3_t dir, const float ke, const float scale, const int flags, const qboolean altskin)
{
	const int chunk_index = irand(debris_chunk_offsets[material], debris_chunk_offsets[material + 1] - 1);

	client_entity_t* debris = ClientEntity_new(-1, 0, origin, NULL, 50);
	debris->SpawnInfo = material;
	debris->classID = CID_DEBRIS;
	debris->msgHandler = CE_DefaultMsgHandler;

	debris->r.model = &debris_chunks[chunk_index].model;
	debris->r.scale = scale;
	debris->r.angles[0] = flrand(-ANGLE_180, ANGLE_180);
	debris->r.angles[1] = flrand(-ANGLE_90, ANGLE_90);

	debris->flags |= CEF_CLIP_TO_WORLD | CEF_ABSOLUTE_PARTS;
	debris->radius = 5.0f;

	VectorRandomCopy(dir, debris->velocity, 0.5f);
	Vec3ScaleAssign(sqrtf(ke / debris_chunks[chunk_index].mass), debris->velocity);

	debris->acceleration[2] = GetGravity();

	debris->elasticity = debris_elasticity[material];
	debris->r.skinnum = debris_chunks[chunk_index].skinNum;

	if (material == MAT_FLESH || material == MAT_INSECT) // Flesh need a different update for blood.
	{
		debris->Update = FleshDebris_Update;
		if (altskin && chunk_index < 47) // Using multi-skinned pottery chunks.
			debris->r.skinnum = 1; // Male
	}
	else
	{
		if (material == MAT_GLASS)
		{
			debris->r.skinnum = 1;
			debris->r.flags |= RF_TRANSLUCENT;
		}

		debris->Update = Debris_Update;
	}

	// Debris lasts 10 seconds before it slowly goes away.
	debris->LifeTime = fxi.cl->time + 1000;

	if (flags & CEF_FLAG6) // On fire - add dynamic light.
	{
		if (flags & CEF_FLAG7) // High detail, non-ref_soft?
		{
			const paletteRGBA_t color = { .c = 0xe5007fff };

			debris->flags |= CEF_FLAG7; // Spawn blood too, not just flames.
			debris->dlight = CE_DLight_new(color, 50.0f, -5.0f);
		}

		debris->flags |= CEF_FLAG6;
	}

	if (flags & CEF_FLAG8) // Reflective.
		debris->r.flags |= RF_REFLECTION;

	AddEffect(NULL, debris);

	return debris;
}

// Flesh debris throws should be a separate effect. This would save quite a bit of code.
// num - number of chunks to spawn (dependent on size and mass).
// material - type of chunk to explode.
// dir - direction of debris (currently always 0.0 0.0 1.0).
// ke - kinetic energy (dependent of damage and number of chunks).
// mins - mins field of edict (so debris is spawned at base).
// scale - size of the spawned chunks (dependent on size).
void FXDebris_SpawnChunks(int type, int flags, const vec3_t origin, const int num, const int material, const vec3_t dir, const float ke, const vec3_t mins, const float scale, const qboolean altskin) //TODO: remove unused 'type' arg?
{
	if (flags & CEF_FLAG6) // On fire, check for highdetail, non-ref_soft.
	{
		if (IsInWater(origin))
			flags &= ~CEF_FLAG6; // In water - no flames, pal!
		else if (!ref_soft && (int)r_detail->value == DETAIL_HIGH) //TODO: and uberhigh?
			flags |= CEF_FLAG7; // Do dynamic light and blood trail.
	}

	for (int i = 0; i < num; i++)
	{
		vec3_t hold_origin;
		VectorCopy(origin, hold_origin);

		for (int c = 0; c < 3; c++)
			hold_origin[c] += flrand(-mins[c], mins[c]);

		if (material != MAT_NONE)
		{
			FXDebris_Throw(hold_origin, material, dir, ke, scale, flags, altskin);

			if (material == MAT_FLESH) // Flesh need a different update for blood.
				DoBloodSplash(hold_origin, 5, false);
			else if (material == MAT_INSECT)
				DoBloodSplash(hold_origin, 5, true);
			else if (irand(0, 1))
				CreateSinglePuff(hold_origin, 20.0f);
		}
		else // Nothing but smoke.
		{
			CreateSinglePuff(hold_origin, 20.0f);
		}
	}
}

static void Debris_SpawnFleshChunks(int type, int flags, vec3_t origin, const int num, const int material, vec3_t dir, const float ke, vec3_t mins, const float scale, const qboolean altskin) //TODO: remove unused 'type' arg?
{
	if (flags & CEF_FLAG6) // On fire, check for highdetail, non-ref_soft.
	{
		if (IsInWater(origin))
			flags &= ~CEF_FLAG6; // In water - no flames, pal!
		else if (!ref_soft && (int)r_detail->value == DETAIL_HIGH) //TODO: and uberhigh?
			flags |= CEF_FLAG7; // Do dynamic light and blood trail.
	}

	for (int i = 0; i < num; i++)
	{
		vec3_t holdorigin;
		VectorCopy(origin, holdorigin);

		for (int c = 0; c < 3; c++)
			holdorigin[c] += flrand(-mins[c], mins[c]);

		FXDebris_Throw(holdorigin, material, dir, ke, scale, flags, altskin);
		DoBloodSplash(holdorigin, 5, material == MAT_INSECT);
	}
}

void FXDebris(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	byte size;
	byte material;
	vec3_t mins;
	byte mag;

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_DEBRIS].formatString, &size, &material, mins, &mag);

	vec3_t dir;
	VectorSet(dir, 0.0f, 0.0f, 1.0f);

	Vec3ScaleAssign(mag, mins);

	float scale = (float)size * 10.0f;
	scale = sqrtf(scale) * 0.08f;

	int num;
	switch ((int)r_detail->value)
	{
		default:
		case DETAIL_LOW:		num = ClampI(size, 1, 5);  break;
		case DETAIL_NORMAL:		num = ClampI(size, 1, 12); break;
		case DETAIL_HIGH:		num = ClampI(size, 1, 16); break;
		case DETAIL_UBERHIGH:	num = ClampI(size, 1, 24); break;
	}

	const float ke = 40000.0f + ((float)size * 400.0f);

	FXDebris_SpawnChunks(type, flags, origin, num, material, dir, ke, mins, scale, false);
}

void FXFleshDebris(centity_t* owner, const int type, int flags, vec3_t origin)
{
	byte material;
	byte size;
	vec3_t mins;
	byte mag;

	qboolean altskin = false;

	if (flags & CEF_FLAG7)
	{
		// Male insect.
		material = MAT_INSECT;
		flags &= ~CEF_FLAG7;
		flags &= ~CEF_FLAG8;
		altskin = true;
	}
	else if (flags & CEF_FLAG8)
	{
		// Normal insect.
		material = MAT_INSECT;
		flags &= ~CEF_FLAG8;
	}
	else
	{
		material = MAT_FLESH;
	}

	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_FLESH_DEBRIS].formatString, &size, mins, &mag);

	vec3_t dir;
	VectorSet(dir, 0.0f, 0.0f, 1.0f);

	Vec3ScaleAssign(mag, mins);

	int num;
	switch ((int)r_detail->value)
	{
		default:
		case DETAIL_LOW:		num = ClampI(size, 1, 5);  break;
		case DETAIL_NORMAL:		num = ClampI(size, 1, 12); break;
		case DETAIL_HIGH:		num = ClampI(size, 1, 16); break;
		case DETAIL_UBERHIGH:	num = ClampI(size, 1, 24); break;
	}

	const float scale = (float)mag / 24.0f;

	Debris_SpawnFleshChunks(type, flags, origin, num, material, dir, 80000.0f, mins, scale, altskin);
}

#pragma endregion

#pragma region ========================== Debris message receivers ==========================

static void Debris_Collision(client_entity_t* self, CE_Message_t* msg)
{
	char* snd_name; //mxd

	if (!(self->flags & CEF_CLIP_TO_WORLD))
		return;

	trace_t* trace;
	ParseMsgParms(msg, "g", &trace);

	// Invalid trace or didn't hit the world?
	if (trace->startsolid || trace->allsolid || Vec3IsZeroEpsilon(trace->plane.normal) || trace->ent != (struct edict_s*)-1)
		return;

	if (trace->contents & CONTENTS_SOLID)
	{
		// Hit a solid surface, make noise and leave any decals.
		const int material = (self->SpawnInfo & SIF_FLAG_MASK); // Again, the SpawnInfo lower 2 bits are material types, >= 16 are flags - here we mask out those flags to get the actual materialtype.

		switch (material)
		{
			//TODO: find out what surface this is hitting?
			case MAT_METAL:
				if (self->effectID == FX_THROWWEAPON && irand(0, 2) == 0)
				{
					vec3_t dir;
					VectorSet(dir, 0.0f, 0.0f, 1.0f);
					GenericSparks(NULL, FX_SPARKS, 0, self->r.origin, dir);

					snd_name = (irand(0, 1) ? "misc/dropmetal1.wav" : "misc/dropmetal.wav");
					fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), 1.0f, ATTN_NORM, 0.0f);
				}
				else
				{
					snd_name = va("misc/drophvmtl%i.wav", irand(1, 3));
					fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), 1.0f, ATTN_STATIC, 0.0f);
				}
				// Need more hollow sounds for big metal.
				break;

			case MAT_WOOD:
				if (irand(0, 6) == 0)
				{
					snd_name = (irand(0, 1) ? "misc/dropwood1.wav" : "misc/dropwood.wav");
					fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), 1.0f, ATTN_STATIC, 0.0f);
				}
				break;

			case MAT_STONE:
			case MAT_GREYSTONE:
			case MAT_BROWNSTONE:
				if (irand(0, 6) == 0)
				{
					snd_name = (irand(0, 2) ? va("misc/boulder%i.wav", irand(1, 2)) : "misc/dropthing.wav");
					fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), 1.0f, ATTN_STATIC, 0.0f);
				}
				break;

			case MAT_FLESH: // Maybe slide? Wet sound?
			case MAT_INSECT:
			{
				const qboolean dark = (self->flags & CEF_FLAG6);
				const qboolean yellow = (material == MAT_INSECT);

				if (self->effectID == FX_BODYPART)
					DoBloodSplash(self->r.origin, irand(1, 3), yellow);

				if (self->effectID == FX_BODYPART || irand(0, 1))
				{
					snd_name = va("misc/fleshdrop%i.wav", irand(1, 3));
					fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), 1.0f, ATTN_STATIC, 0.0f);

					if (!(self->SpawnInfo & SIF_INWATER))
						ThrowBlood(self->r.origin, trace->plane.normal, dark, yellow, false);
				}
			} break;

			case MAT_GLASS:
				if (irand(0, 2) == 0)
				{
					snd_name = va("misc/dropglass%i.wav", irand(1, 2));
					fxi.S_StartSound(self->r.origin, -1, CHAN_AUTO, fxi.S_RegisterSound(snd_name), 1.0f, ATTN_STATIC, 0.0f);
				}
				break;

			default:
				break;
		}
	}

	// Don't bounce if velocity is small.
	if (trace->plane.normal[2] > GROUND_NORMAL && (Q_fabs(self->velocity[2]) < 100.0f || VectorLength(self->velocity) < 100.0f || trace->fraction < 0.075f))
	{
		// Set pitch so that chunks lie flat on ground.
		self->r.angles[PITCH] = ANGLE_90;

		BecomeStatic(self);

		self->d_alpha = flrand(-0.1f, -0.25f);
		self->Update = FXDebris_Vanish;

		return;
	}

	BounceVelocity(self->velocity, trace->plane.normal, self->velocity, self->elasticity);

	const float d_time = fxi.cls->frametime * trace->fraction;
	if (d_time > 0.0f) // The game might crash with a zero movement. --Pat
		Physics_MoveEnt(self, d_time, d_time * d_time * 0.5f, trace);
}

#pragma endregion

#pragma region ========================== Debris update ==========================

qboolean FXDebris_Vanish(struct client_entity_s* self, centity_t* owner)
{
	if (self->SpawnInfo & SIF_INLAVA)
		FXDarkSmoke(self->r.origin, flrand(0.2f, 0.5f), flrand(30.0f, 50.0f));

	if (self->alpha < 0.1f || self->r.scale < 0.1f)
	{
		if (self->flags & CEF_FLAG6)
		{
			// Let the smoke die out.
			self->alpha = 0.0f;
			self->r.scale = 0.0f;
			self->updateTime = 1000;
			self->Update = RemoveSelfAI; //mxd. FXDebris_Remove() in original logic.

			return true;
		}

		return false;
	}

	if (self->flags & CEF_FLAG6 && irand(0, 2) == 0) // On fire - do a fire trail.
	{
		if (flrand(0.0f, 0.3f) > self->alpha || flrand(0.0f, 0.3f) > self->r.scale) //BUGFIX: mxd. Original logic uses irand(0, 0.3) here.
		{
			self->dlight = NULL;
			self->flags &= ~CEF_FLAG6;
			self->d_alpha = -0.01f;
		}
		else
		{
			DoFireTrail(self); //FIXME: make them just smoke when still?
		}
	}

	return true;
}

static qboolean Debris_Update(struct client_entity_s* self, centity_t* owner)
{
	const int cur_time = fxi.cl->time;

	if (cur_time > self->LifeTime)
	{
		self->d_alpha = flrand(-0.05f, -0.2f);
		self->Update = FXDebris_Vanish;
	}
	else
	{
		const float d_time = (float)(cur_time - self->lastThinkTime) / 1000.0f;

		self->r.angles[0] += ANGLE_360 * d_time;
		self->r.angles[1] += ANGLE_360 * d_time;
		self->lastThinkTime = cur_time;

		if (self->flags & CEF_FLAG6) // On fire - do a fire trail.
			DoFireTrail(self);
	}

	return true;
}

static qboolean FleshDebris_Update(struct client_entity_s* self, centity_t* owner)
{
	const int cur_time = fxi.cl->time;

	if (self->flags & CEF_FLAG6) // On fire - do a fire trail.
	{
		DoFireTrail(self);

		if (self->flags & CEF_FLAG7)
			DoBloodTrail(self, 2);
	}
	else
	{
		DoBloodTrail(self, 2);
	}

	if (cur_time > self->LifeTime)
	{
		self->d_alpha = flrand(-0.05f, -0.2f);
		self->Update = FXDebris_Vanish;
	}
	else
	{
		const float d_time = (float)(cur_time - self->lastThinkTime) / 1000.0f;

		self->r.angles[0] += ANGLE_360 * d_time;
		self->r.angles[1] += ANGLE_360 * d_time;
		self->lastThinkTime = cur_time;
	}

	return true;
}

#pragma endregion
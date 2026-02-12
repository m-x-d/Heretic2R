//
// g_waterfx.c -- Water-related fx, including at least 1 fountain.
//
// Copyright 1998 Raven Software
//

#include "g_waterfx.h" //mxd
#include "g_obj.h" //mxd
#include "Vector.h"
#include "g_local.h"
#include "Random.h"

#pragma region ========================== env_water_drip ==========================

#define SF_YELLOW	1 //mxd

void EnvWaterDripThink(edict_t* self) //mxd. Named 'waterdrip_go' in original logic.
{
	const byte b_frame = ((self->spawnflags & SF_YELLOW) ? 1 : 0); //mxd
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_DRIPPER, CEF_BROADCAST, self->s.origin, "bb", self->count, b_frame);
	self->think = NULL;
}

void EnvWaterDripUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'waterdrip_use' in original logic.
{
	if (self->PersistantCFX > 0)
	{
		gi.RemoveEffects(&self->s, FX_REMOVE_EFFECTS);
		self->PersistantCFX = 0;
	}
	else
	{
		EnvWaterDripThink(self);
	}
}

// QUAKED env_water_drip (1 .5 0) (-4 -4 0) (4 4 4) YELLOW
// Spawns a drip of water which falls straight down.
// Spawnflags:
// YELLOW - Use a yellow drip. //TODO: doesn't work (waterdrop.sp2 has no sprite 1) --mxd //TODO: This really ought to be sent over as a flag (like CEF_FLAG6), but its a persistent effect, so not critical.
// Variables:
// count - Drips per minute (default 20).
void SP_env_water_drip(edict_t* self)
{
	if (self->count == 0)
		self->count = 20;

	self->solid = SOLID_NOT;
	self->s.effects |= EF_NODRAW_ALWAYS_SEND;
	
	self->use = EnvWaterDripUse;
	self->think = EnvWaterDripThink;
	self->nextthink = level.time + 4.0f;

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== env_water_fountain ==========================

#define SF_START_OFF	32

void EnvWaterFountainUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'fountain_use' in original logic.
{
	if (self->spawnflags & SF_START_OFF) // Enable effect.
	{
		const short s_drop = (short)(-self->delay * 8.0f); // At the time of creation of this effect, I thought positive z was down, hence the MINUS sign for the distance to fall.
		self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_FOUNTAIN, CEF_BROADCAST, self->s.origin, "vsb", self->s.angles, s_drop, 0);
		self->s.sound = (byte)gi.soundindex("ambient/fountainloop.wav");
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
		self->spawnflags &= ~SF_START_OFF;
	}
	else // Disable effect.
	{
		if (self->PersistantCFX > 0)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_WATER);
			self->PersistantCFX = 0;
		}

		gi.RemoveEffects(&self->s, FX_FOUNTAIN);
		self->spawnflags |= SF_START_OFF;
		self->s.sound = 0;
	}
}

// QUAKED env_water_fountain (1 .5 0) (-4 -4 0) (4 4 4) RED GREEN BLUE DARK DARKER START_OFF
// If targeted it can be turned on/off but it will start on unless START_OFF.
// Spawnflags:
// START_OFF - Fountain will be off until triggered.
// Variables:
// angles	- xyz velocity of spawned particles.
// delay	- The distance from emitter to ground (128 is OK, max is 256). 
void SP_env_water_fountain(edict_t* self)
{
	if (self->targetname != NULL)
		self->use = EnvWaterFountainUse;

	if (self->delay == 0.0f) //mxd. Set default delay. //TODO: use gi.trace() to set this up automatically?..
		self->delay = 64.0f;

	self->s.effects |= (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);
	gi.linkentity(self);

	if (!(self->spawnflags & SF_START_OFF)) // Enable?
	{
		self->spawnflags |= SF_START_OFF;
		EnvWaterFountainUse(self, NULL, NULL); //mxd
	}
}

#pragma endregion

#pragma region ========================== env_waterfall_base ==========================

// QUAKED env_waterfall_base (1 1 0) (-8 -8 -8) (8 8 8)
// Variables:
// angles - [x radius, yaw, y radius].
void SP_env_waterfall_base(edict_t* self)
{
	self->s.effects |= (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);

	gi.linkentity(self);

	const short xrad = (short)self->s.angles[0];
	const short yrad = (short)self->s.angles[2];
	const byte b_yaw = (byte)(self->s.angles[1] * DEG_TO_BYTEANGLE); //mxd. Use define.
	gi.CreatePersistantEffect(&self->s, FX_WATERFALLBASE, CEF_BROADCAST, self->s.origin, "bbb", xrad, yrad, b_yaw);
}

#pragma endregion

#pragma region ========================== obj_fishhead1, obj_fishhead2 ==========================

#define SF_NODRIP	1 //mxd

static void SpawnFishheadDrippers(const edict_t* self, const vec3_t start, const vec3_t end) //mxd. Added to reduce code duplication.
{
	static const float lerp_mins[] = { 0.45f, 0.7f };
	static const float lerp_maxs[] = { 0.65f, 1.0f };

	for (int i = 0; i < 2; i++)
	{
		vec3_t origin;
		VectorLerp(start, flrand(lerp_mins[i], lerp_maxs[i]), end, origin);

		VectorRotate(origin, self->s.angles[YAW], origin);
		Vec3ScaleAssign(self->s.scale, origin);
		Vec3AddAssign(self->s.origin, origin);

		gi.CreatePersistantEffect(NULL, FX_DRIPPER, 0, origin, "bb", self->count, 0); //mxd. Last arg was 2 in original logic (but waterdrop.sp2 only has single frame...).
	}
}

// QUAKED obj_fishhead1 (1 .5 0) (0 -76 -86) (136 76 86) NODRIP
// Large fish head fountain. No teeth in mouth and the fins on top are connected. Also spawns 4 drips frame 0.
// Spawnflags:
// NODRIP - Won't drip.
// Variables:
// count - number of drips per minute (default 20).
void SP_obj_fishhead1(edict_t* self)
{
	static const vec3_t jaw_ls = VEC3_SET(-2.59204f, -50.1f, -42.2f); // Jaw back, left side.
	static const vec3_t jaw_le = VEC3_SET(92.4f, -12.46f, -86.8f); // Jaw front, left side.
	static const vec3_t jaw_rs = VEC3_SET(-2.59204f, 50.0f, -42.2f); // Jaw back, right side.
	static const vec3_t jaw_re = VEC3_SET(92.4f, 12.41f, -86.8f); // Jaw front, right side.

	if (!(self->spawnflags & SF_NODRIP))
	{
		if (self->count == 0)
			self->count = 20;

		SpawnFishheadDrippers(self, jaw_ls, jaw_le); //mxd
		SpawnFishheadDrippers(self, jaw_rs, jaw_re); //mxd
	}

	self->s.modelindex = (byte)gi.modelindex("models/objects/fishheads/fishhead1/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	VectorSet(self->mins, 0.0f, -76.0f, -86.0f);
	VectorSet(self->maxs, 136.0f, 76.0f, 86.0f);

	ObjectInit(self, 100, 500, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_fishhead2 (1 .5 0) (0 -110 -118) (136 110 118) NODRIP
// Large fish head fountain. The mouth has teeth. The fins on top are not connected. Also spawns 4 drips frame 0.
// Spawnflags:
// NODRIP - Won't drip.
// Variables:
// count - number of drips per minute (default 20).
void SP_obj_fishhead2(edict_t* self)
{
	static const vec3_t jaw_ls = VEC3_SET(-2.49532f, -72.6f, -61.8f); // Jaw back, left side.
	static const vec3_t jaw_le = VEC3_SET(106.8f, -31.1f, -112.5f); // Jaw front, left side.
	static const vec3_t jaw_rs = VEC3_SET(-2.49532f, 72.6f, -61.8f); // Jaw back, right side.
	static const vec3_t jaw_re = VEC3_SET(106.8f, 32.8f, -112.5f); // Jaw front, right side.

	if (!(self->spawnflags & SF_NODRIP))
	{
		if (self->count == 0)
			self->count = 20;

		SpawnFishheadDrippers(self, jaw_ls, jaw_le); //mxd
		SpawnFishheadDrippers(self, jaw_rs, jaw_re); //mxd
	}

	self->s.modelindex = (byte)gi.modelindex("models/objects/fishheads/fishhead2/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	VectorSet(self->mins, 0.0f, -110.0f, -118.0f);
	VectorSet(self->maxs, 136.0f, 110.0f, 118.0f);

	ObjectInit(self, 100, 500, MAT_GREYSTONE, SOLID_BBOX); //mxd
}

#pragma endregion

#pragma region ========================== env_mist ==========================

// QUAKED env_mist (1 .5 0) (-64 -1 -32) (64 1 32)
// Variables:
// scale - Sets mist scale (max. 25).
void SP_env_mist(edict_t* self)
{
	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	VectorSet(self->mins, -5.0f, -5.0f, -5.0f);
	VectorSet(self->maxs, 5.0f, 5.0f, 5.0f);

	const byte b_scale = (byte)(self->s.scale * 10.0f);
	gi.CreatePersistantEffect(&self->s, FX_MIST, CEF_BROADCAST, self->s.origin, "b", b_scale);
	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== env_bubbler ==========================

// QUAKED env_bubbler (1 .5 0) (-4 -4 0) (4 4 4)
// Makes bubbles.
// Variables:
// count - Bubbles spawned per minute (default 120).
void SP_env_bubbler(edict_t* self)
{
	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;

	VectorSet(self->mins, -5.0f, -5.0f, -5.0f);
	VectorSet(self->maxs, 5.0f, 5.0f, 5.0f);

	if (self->count == 0)
		self->count = 120;

	gi.CreatePersistantEffect(&self->s, FX_BUBBLER, CEF_BROADCAST, self->s.origin, "b", self->count);
	gi.linkentity(self);
}

#pragma endregion
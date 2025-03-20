//
// g_waterfx.c -- Water-related fx, including at least 1 fountain.
//
// Copyright 1998 Raven Software
//

#include "g_obj.h" //mxd
#include "Vector.h"
#include "g_local.h"

#pragma region ========================== env_water_drip ==========================

#define SF_YELLOW	1 //mxd

static void EnvWaterDripThink(edict_t* self) //mxd. Named 'waterdrip_go' in original logic.
{
	const byte b_frame = ((self->spawnflags & SF_YELLOW) ? 1 : 0);
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_DRIPPER, CEF_BROADCAST, self->s.origin, "bb", self->count, b_frame);
	self->think = NULL;
}

static void EnvWaterDripUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'waterdrip_use' in original logic.
{
	if (self->PersistantCFX > 0)
	{
		gi.RemoveEffects(&self->s, 0);
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
// YELLOW - Use a yellow drip. //TODO: This really ought to be sent over as a flag (like CEF_FLAG6), but its a persistent effect, so not critical.
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

static void EnvWaterFountainUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'fountain_use' in original logic.
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

	const short xrad = (short)Q_ftol(self->s.angles[0]);
	const short yrad = (short)Q_ftol(self->s.angles[2]);
	const byte yaw = (byte)Q_ftol((self->s.angles[1] * 256.0f) / 360.0f);
	gi.CreatePersistantEffect(&self->s, FX_WATERFALLBASE, CEF_BROADCAST, self->s.origin, "bbb", xrad, yrad, yaw);
}

#pragma endregion

#pragma region ========================== obj_fishhead1, obj_fishhead2 ==========================

#define SF_NODRIP	1 //mxd

static void SpawnDripper(const edict_t* self, const vec3_t offset)
{
	vec3_t origin;
	VectorAdd(self->s.origin, offset, origin);
	gi.CreatePersistantEffect(NULL, FX_DRIPPER, 0, origin, "bb", self->count, 2);
}

/*QUAKED obj_fishhead1 (1 .5 0) (0 -76 -86) (136 76 86)  NODRIP
 Large fish head fountain. No teeth in mouth and the fins on top are connected. Also spawns 4 drips frame 0
-------  FIELDS  ------------------
NODRIP - won't drip
-----------------------------------
*/
void SP_obj_fishhead1 (edict_t *self)
{
	vec3_t		offset;

	if (!(self->spawnflags & 1))
	{
		if(!self->count)
			self->count = 20;

		VectorSet(offset, -20, -60, -50);
		SpawnDripper(self, offset);
		VectorSet(offset, 55, 30, -70);
		SpawnDripper(self, offset);
		VectorSet(offset, 0, 60, -70);
		SpawnDripper(self, offset);
		VectorSet(offset, 65, -7, -60);
		SpawnDripper(self, offset);
	}

	self->spawnflags |= SF_OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= SF_OBJ_NOPUSH;	// Cant push it

	VectorSet(self->mins, 0, -76, -86);
	VectorSet(self->maxs, 136, 76, 86);
	self->s.modelindex = gi.modelindex("models/objects/fishheads/fishhead1/tris.fm");

	ObjectInit(self,100,500,MAT_GREYSTONE,SOLID_BBOX);
}

/*QUAKED obj_fishhead2 (1 .5 0) (0 -110 -118) (136 110 118) NODRIP
Large fish head fountain. The mouth has teeth. The fins on top are not conntected. Also spawns 4 drips frame 0
-------  FIELDS  ------------------
NODRIP - won't drip
-----------------------------------
*/
void SP_obj_fishhead2 (edict_t *self)
{
	vec3_t		offset;

	if (!(self->spawnflags & 1))
	{
		if(!self->count)
			self->count = 20;

		VectorSet(offset, -20, -60, -50);
		SpawnDripper(self, offset);
		VectorSet(offset, 55, 30, -70);
		SpawnDripper(self, offset);
		VectorSet(offset, 0, 60, -70);
		SpawnDripper(self, offset);
		VectorSet(offset, 65, -7, -60);
		SpawnDripper(self, offset);
	}

	VectorSet(self->mins, 0, -110, -118);
	VectorSet(self->maxs, 136, 110, 118);
	self->s.modelindex = gi.modelindex("models/objects/fishheads/fishhead2/tris.fm");

	self->spawnflags |= SF_OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= SF_OBJ_NOPUSH;	// Cant push it
	self->takedamage = DAMAGE_NO;

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_NO;
	self->clipmask = MASK_MONSTERSOLID;

	BboxYawAndScale(self);



	gi.linkentity(self);
//	ObjectInit(self,100,500,MAT_GREYSTONE,SOLID_BBOX);
}

#pragma endregion

/*QUAK-ED obj_stalactite1 (1 .5 0) (-24 -24 -99) (24 24 99) DRIP  DARKSKIN
	
	A big long thick stalactite. These point down.

	DARKSKIN - if checked it uses the dark skin
	Also spawns a drip at the end
	Use the "count" field as number of drips per min
*/
void SP_obj_stalactite1(edict_t *self) 
{
	vec3_t	origin;

	if(self->spawnflags & 1)
	{
		if(!self->count)
			self->count = 20;

		VectorCopy(self->s.origin, origin);
		origin[2] += 200.0F;
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_DRIPPER, 0, origin, "bb", self->count, 2);
	}

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	
	VectorSet(self->mins, -24, -24, -99);
	VectorSet(self->maxs, 24, 24, 99);
	
	self->s.modelindex = gi.modelindex("models/objects/stalactite/stalact1/tris.fm");
	if (self->spawnflags & 2)
		self->s.skinnum = 1;

	gi.linkentity(self);
}


/*QUAK-ED obj_stalactite2 (1 .5 0) (-60 -60 -64) (60 60 64)  DRIP  DARKSKIN

	A big short stalactite. These point down.

	DARKSKIN - if checked it uses the dark skin
	Also spawns a drip at the end
	Use the "count" field as number of drips per min
*/
void SP_obj_stalactite2(edict_t *self)
{
	vec3_t	origin;

	if(self->spawnflags & 1)
	{
		if(!self->count)
			self->count = 20;

		VectorCopy(self->s.origin, origin);
		origin[2] += 128.0F;
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_DRIPPER, 0, origin, "bb", self->count, 2);
	}

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	
	VectorSet(self->mins,-60,-60,-64);
	VectorSet(self->maxs,60,60,64);
	
	self->s.modelindex = gi.modelindex("models/objects/stalactite/stalact2/tris.fm");
	if (self->spawnflags & 2)
		self->s.skinnum = 1;

	gi.linkentity(self);
}

/*QUAK-ED obj_stalactite3 (1 .5 0) (-23 -23 -98) (23 23 98)  DRIP  DARKSKIN

	A long pointy stalactite. These point down.

	DARKSKIN - if checked it uses the dark skin
	Also spawns a drip at the end
	Use the "count" field as number of drips per min
*/
void SP_obj_stalactite3(edict_t *self)
{
	vec3_t	origin;

	if(self->spawnflags & 1)
	{
		if(!self->count)
			self->count = 20;

		VectorCopy(self->s.origin, origin);
		origin[2] += 200.0F;
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_DRIPPER, 0, origin, "bb", self->count, 2);
	}

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	
	VectorSet(self->mins, -23, -23, -98);
	VectorSet(self->maxs, 23, 23, 98);
	
	self->s.modelindex = gi.modelindex("models/objects/stalactite/stalact3/tris.fm");
	if (self->spawnflags & 2)
		self->s.skinnum = 1;
	
	gi.linkentity(self);
}

/*QUAKED env_mist (1 .5 0) (-64 -1 -32) (64 1 32)
scale sets the scale
*/
void SP_env_mist(edict_t *self)
{
	byte		scale;

	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	scale = Q_ftol(self->s.scale * 10.0);
	VectorSet(self->mins, -5, -5, -5);
	VectorSet(self->maxs, 5, 5, 5);

	gi.CreatePersistantEffect(&self->s, FX_MIST, CEF_BROADCAST, self->s.origin, "b", scale);
	gi.linkentity(self);
}

/*QUAKED env_bubbler (1 .5 0) (-4 -4 0) (4 4 4)
Makes bubbles
---------KEYS--------
count - bubbles spawned per minute
*/
void SP_env_bubbler(edict_t *self)
{	
	if(!self->count)
		self->count = 120;
	VectorSet(self->mins, -5, -5, -5);
	VectorSet(self->maxs, 5, 5, 5);

	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	gi.CreatePersistantEffect(&self->s, FX_BUBBLER, CEF_BROADCAST, self->s.origin, "b", self->count);
	gi.linkentity(self);
}

// end


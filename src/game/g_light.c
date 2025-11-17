//
// g_light.c
//
// Copyright 1998 Raven Software
//

#include "g_light.h" //mxd
#include "g_combat.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_obj.h" //mxd
#include "FX.h"
#include "Utilities.h" //mxd
#include "Vector.h"
#include "g_local.h"

#pragma region ========================== Light support logic ==========================

void LightStaticsInit(void) //TODO: remove?
{
}

static void LightInit(edict_t* self)
{
	self->msgHandler = DefaultMsgHandler;

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_NO;

	BboxYawAndScale(self);

	gi.linkentity(self);
}

static void TorchInit(edict_t* self)
{
	// No targeted lights in deathmatch, because they cause global messages.
	if (self->targetname != NULL && DEATHMATCH)
	{
		G_FreeEdict(self);
	}
	else if (self->style >= 32)
	{
		self->use = TorchUse;
		self->think = TorchStart;
		self->nextthink = level.time + 1.5f; // If you don't wait a little they don't light right.
	}
}

void FlameDamagerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'fire_touch' in original logic.
{
	if (other->client != NULL && self->touch_debounce_time <= level.time)
	{
		self->touch_debounce_time = level.time + 1.0f;
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 0, DAMAGE_AVOID_ARMOR | DAMAGE_FIRE | DAMAGE_FIRE_LINGER, MOD_BURNT);
	}
}

// This creates an invisible entity that will burn the player if he's standing in the fire.
static void SpawnFlameDamager(edict_t* owner, const vec3_t origin) //mxd. Named 'create_fire_touch' in original logic.
{
	edict_t* damager = G_Spawn();

	damager->s.scale = owner->s.scale;
	damager->dmg = (int)(owner->s.scale * 3.0f);
	damager->spawnflags |= SF_OBJ_NOPUSH; // Used by ObjectInit() to set movetype...
	damager->movetype = PHYSICSTYPE_NONE;
	damager->touch = FlameDamagerTouch;

	VectorCopy(origin, damager->s.origin);
	VectorSet(damager->mins, -8.0f, -8.0f, -2.0f);
	VectorSet(damager->maxs, 8.0f, 8.0f, 14.0f);

	ObjectInit(damager, 2, 2, MAT_NONE, SOLID_TRIGGER);

	owner->enemy = damager;
}

void SpawnFlame(edict_t* self, const vec3_t origin)
{
	// NOTE - limit on scale is x8.
	const byte b_scale = (byte)(min(self->s.scale, 8.0f) * 32.0f);
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_FIRE, CEF_BROADCAST, origin, "b", b_scale);

	SpawnFlameDamager(self, origin);
}

#pragma endregion

#pragma region ========================== light ==========================

#define SF_LIGHT_START_OFF	1

void LightUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'light_use' in original logic.
{
	if (self->spawnflags & SF_LIGHT_START_OFF)
	{
		gi.configstring(CS_LIGHTS + self->style, "m");
		self->spawnflags &= ~SF_LIGHT_START_OFF;
	}
	else
	{
		gi.configstring(CS_LIGHTS + self->style, "a");
		self->spawnflags |= SF_LIGHT_START_OFF;
	}
}

// QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
// Non-displayed light. If targeted, will toggle between on and off.
// Default light value is 300.
// Default style is 0.
// Default _cone value is 10 (used to set size of light for spotlights)
void SP_light(edict_t* self)
{
	// No targeted lights in deathmatch, because they cause global messages.
	if (self->targetname == NULL || DEATHMATCH)
	{
		G_FreeEdict(self);
	}
	else if (self->style >= 32)
	{
		self->use = LightUse;
		const char* str = ((self->spawnflags & SF_LIGHT_START_OFF) ? "a" : "m"); //mxd
		gi.configstring(CS_LIGHTS + self->style, str);
	}
}

#pragma endregion

#pragma region ========================== env_fire ==========================

#define SF_ENV_FIRE_OFF			8
#define SF_ENV_FIRE_MOVEABLE	16
#define SF_ENV_FIRE_LIGHT_ON	32 //mxd

void EnvFireUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'fire_use' in original logic.
{
	if (self->spawnflags & SF_ENV_FIRE_OFF)
	{
		// NOTE - limit on scale is x8.
		const byte b_scale = (byte)(min(self->s.scale, 8.0f) * 32.0f);
		self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_FIRE, CEF_BROADCAST, self->s.origin, "b", b_scale);

		SpawnFlameDamager(self, self->s.origin);

		const char* snd_name = ((self->s.scale < 1.0f) ? "ambient/smallfire.wav" : "ambient/fireplace.wav"); //mxd
		self->s.sound = (byte)gi.soundindex(snd_name);
		self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;

		self->spawnflags &= ~SF_ENV_FIRE_OFF;
	}
	else
	{
		if (self->PersistantCFX > 0)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_FIRE);
			self->PersistantCFX = 0;
		}

		self->s.sound = 0;
		gi.RemoveEffects(&self->s, FX_FIRE);
		self->spawnflags |= SF_ENV_FIRE_OFF;

		G_FreeEdict(self->enemy); // Remove FireDamager ent.
	}
}

void EnvFireMoveThink(edict_t* self) //mxd. Named 'firemove_think' in original logic.
{
	const byte b_scale = (byte)(self->s.scale * 8.0f);
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_FIRE_ON_ENTITY, CEF_BROADCAST | CEF_OWNERS_ORIGIN, self->s.origin, "bbb", b_scale, 0, 1);

	self->think = NULL;
}

// QUAKED env_fire (1 .5 0) (0 -10 -24) (20 10 0)  x x x  FIRE_OFF MOVEABLE LIGHT_ON
// A fire about the size of a campfire. Triggerable.

// Spawnflags:
// FIRE_OFF - Fire will start off.
// MOVEABLE - Fire will move if given a velocity.
// LIGHT_ON - Fire will have light attached to it - if moveable, not required.

// Variables:
// scale - Size of flame (default 1) (no bigger than 8).
void SP_env_fire(edict_t* self)
{
	if (self->s.scale == 0.0f)
		self->s.scale = 1.0f;

	if (self->targetname != NULL)
	{
		self->use = EnvFireUse;
		edict_t* controller = G_Find(NULL, FOFS(target), self->targetname);

		if (controller != NULL && controller->materialtype == MAT_WOOD)
			controller->svflags |= SVF_ONFIRE; // Set it up to throw firey chunks.
	}

	if (self->spawnflags & SF_ENV_FIRE_MOVEABLE)
	{
		VectorSet(self->mins, -2.0f, -2.0f, -2.0f);
		VectorSet(self->maxs, 2.0f, 2.0f, 2.0f);

		self->mass = 250;
		self->friction = 0.0f;
		self->gravity = 0.0f;
		self->movetype = PHYSICSTYPE_FLY;

		self->model = NULL;
		self->solid = SOLID_NOT;
		self->clipmask = MASK_MONSTERSOLID;
	}
	else
	{
		VectorSet(self->mins, 0.0f, -10.0f, -24.0f);
		VectorSet(self->maxs, 20.0f, 10.0f, 0.0f);
	}

	self->s.effects |= (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);
	gi.linkentity(self);

	if (self->spawnflags & SF_ENV_FIRE_OFF)
		return;

	const char* snd_name = ((self->s.scale < 1.0f) ? "ambient/smallfire.wav" : "ambient/fireplace.wav"); //mxd
	self->s.sound = (byte)gi.soundindex(snd_name);
	self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;

	if (self->spawnflags & SF_ENV_FIRE_MOVEABLE)
	{
		self->think = EnvFireMoveThink;
		self->nextthink = level.time + 2.0f;
	}

	// Add a light or no?
	int fx_flags = CEF_BROADCAST;
	if (self->spawnflags & SF_ENV_FIRE_LIGHT_ON)
		fx_flags |= CEF_FLAG6;

	// NOTE - limit on scale is x8.
	const byte b_scale = (byte)(min(self->s.scale, 8.0f) * 32.0f);
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_FIRE, fx_flags, self->s.origin, "b", b_scale);

	SpawnFlameDamager(self, self->s.origin);
}

#pragma endregion

#pragma region ========================== Many variants of light_torch... ==========================

#define SF_TORCH_ANIMATE	2 //mxd
#define SF_TORCH_STARTOFF	8
#define SF_TORCH_NOHALO		16

void TorchUse(edict_t* self, edict_t* other, edict_t* activator)
{
	if (self->spawnflags & SF_TORCH_STARTOFF)
	{
		gi.configstring(CS_LIGHTS + self->style, "m");
		self->spawnflags &= ~SF_TORCH_STARTOFF;
	}
	else
	{
		gi.configstring(CS_LIGHTS + self->style, "a");
		self->spawnflags |= SF_TORCH_STARTOFF;
	}
}

void TorchStart(edict_t* self)
{
	const char* str = ((self->spawnflags & SF_TORCH_STARTOFF) ? "a" : "m"); //mxd
	gi.configstring(CS_LIGHTS + self->style, str);

	self->think = NULL;
}

// QUAKED light_walltorch (1 .5 0) (-16 -10 -12) (10 10 12) x ANIMATE x STARTOFF
// A torch that sticks out of a wall.
// Spawnflags:
// ANIMATE	- Places a flame on it.
// STARTOFF	- Light will start off if targeted (default is on).
void SP_light_walltorch(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/walltorch/tris.fm");
	self->s.sound = (byte)gi.soundindex("ambient/smallfire.wav");
	self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;

	VectorSet(self->mins, -16.0f, -10.0f, -12.0f);
	VectorSet(self->maxs, 10.0f, 10.0f, 12.0f);

	LightInit(self);

	if (self->spawnflags & SF_TORCH_ANIMATE) // Animate it.
	{
		const vec3_t hold_origin = VEC3_INITA(self->s.origin, 0.0f, 0.0f, 28.0f);
		SpawnFlame(self, hold_origin);
	}

	TorchInit(self);
}

// QUAKED light_floortorch (1 .5 0) (-14 -14 -17) (14 14 17) x ANIMATE x STARTOFF
// A stand for a torch that sits on the floor.
// Spawnflags:
// ANIMATE	- Places a flame on it.
// STARTOFF	- Light will start off if targeted (default is on).
void SP_light_floortorch(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/floortorch/tris.fm");
	self->s.sound = (byte)gi.soundindex("ambient/smallfire.wav");
	self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;

	VectorSet(self->mins, -14.0f, -14.0f, -18.0f);
	VectorSet(self->maxs, 14.0f, 14.0f, 18.0f);

	LightInit(self);

	if (self->spawnflags & SF_TORCH_ANIMATE) // Animate it.
	{
		const vec3_t hold_origin = VEC3_INITA(self->s.origin, 0.0f, 0.0f, 33.0f);
		SpawnFlame(self, hold_origin);
	}

	TorchInit(self);
}

// QUAKED light_torch1 (1 .5 0) (-4 -6 -5) (6 6 20) x x x STARTOFF NOHALO
// Wall torch that uses a blue gem.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
// NOHALO	- Turns off halo effect.
void SP_light_torch1(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/sinkcity/light-3/tris.fm");

	VectorSet(self->mins, -4.0f, -6.0f, -5.0f);
	VectorSet(self->maxs, 6.0f, 6.0f, 20.0f);

	LightInit(self);

	if (!(self->spawnflags & SF_TORCH_NOHALO))
	{
		vec3_t origin = VEC3_INIT(self->s.origin);

		vec3_t forward;
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(origin, 2.0f, forward, origin);

		origin[2] += 16.0f;

		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, CEF_FLAG6 | CEF_FLAG7, origin, "");
	}

	TorchInit(self);
}

// QUAKED light_gem2 (1 .5 0) (-1 -6 -8) (4 6 8) x x x STARTOFF NOHALO
// A yellow gem in an octogonal frame.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
// NOHALO	- Turns off halo effect.
// Variables:
// style	- 0 yellow light, 1 green light.
void SP_light_gem2(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/sinkcity/light-2/tris.fm");

	VectorSet(self->mins, -1.0f, -6.0f, -8.0f);
	VectorSet(self->maxs, 4.0f, 6.0f, 8.0f);

	if (self->style == 1)
		self->s.skinnum = 1;

	LightInit(self);

	if (!(self->spawnflags & SF_TORCH_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, CEF_FLAG6 | CEF_FLAG8, self->s.origin, "");

	TorchInit(self);
}

// QUAKED light_chandelier1 (1 .5 0) (-36 -36 -43) (34 34 43) x x x STARTOFF
// A big gold chandelier for the great hall.
// Spawnflags:
// STARTOFF - Light will start off if targeted (default is on).
void SP_light_chandelier1(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/chandelier/chan1/tris.fm");

	VectorSet(self->mins, -36.0f, -36.0f, -43.0f);
	VectorSet(self->maxs, 36.0f, 36.0f, 43.0f);

	LightInit(self);
	TorchInit(self);
}

// QUAKED light_chandelier2 (1 .5 0) (-18 -18 -40) (18 18 40) x ANIMATE x STARTOFF
// A very heavy chandelier that doesn't have a skin yet.
// Spawnflags:
// ANIMATE	- The flame flickers.
// STARTOFF	- Light will start off if targeted (default is on).
void SP_light_chandelier2(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -18.0f, -40.0f);
	VectorSet(self->maxs, 18.0f, 18.0f, 40.0f);

	SpawnClientAnim(self, FX_ANIM_CHANDELIER2, NULL);

	LightInit(self);
	TorchInit(self);
}

// QUAKED light_chandelier3 (1 .5 0) (-34 -34 -80) (34 34 0) x x x STARTOFF
// A thin gold chandelier.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
void SP_light_chandelier3(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/chandelier/chan3/tris.fm");

	VectorSet(self->mins, -34.0f, -34.0f, -80.0f);
	VectorSet(self->maxs, 34.0f, 34.0f, 0.0f);

	LightInit(self);
	TorchInit(self);
}

// QUAKED light_lantern1 (1 .5 0) (-28 -8 -22) (4 8 22) x x x STARTOFF NOHALO
// Lantern on a wooden arm.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
// NOHALO	- Turns off halo effect.
void SP_light_lantern1(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/lantern-1/tris.fm");

	VectorSet(self->mins, -28.0f, -8.0f, -22.0f);
	VectorSet(self->maxs, 4.0f, 8.0f, 22.0f);

	LightInit(self);

	if (!(self->spawnflags & SF_TORCH_NOHALO))
	{
		const vec3_t origin = VEC3_INITA(self->s.origin, 0.0f, 0.0f, -10.0f);
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");
	}

	TorchInit(self);
}

// QUAKED light_lantern2 (1 .5 0) (-6 -6 -24) (6 6 40) x x x STARTOFF NOHALO
// Lantern on a chain.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
// NOHALO	- Turns off halo effect.
void SP_light_lantern2(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/lantern-2/tris.fm");

	VectorSet(self->mins, -6.0f, -6.0f, -24.0f);
	VectorSet(self->maxs, 6.0f, 6.0f, 40.0f);

	LightInit(self);

	if (!(self->spawnflags & SF_TORCH_NOHALO))
	{
		const vec3_t origin = VEC3_INITA(self->s.origin, 0.0f, 0.0f, -10.0f);
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");
	}

	TorchInit(self);
}

// QUAKED light_lantern3 (1 .5 0) (-6 -6 -12) (6 6 11) x x x STARTOFF NOHALO
// Ceiling lantern.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
// NOHALO	- Turns off halo effect.
void SP_light_lantern3(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/lantern-3/tris.fm");

	VectorSet(self->mins, -6.0f, -6.0f, -12.0f);
	VectorSet(self->maxs, 6.0f, 6.0f, 11.0f);

	LightInit(self);

	if (!(self->spawnflags & SF_TORCH_NOHALO))
	{
		const vec3_t origin = VEC3_INITA(self->s.origin, 0.0f, 0.0f, -2.0f);
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");
	}

	TorchInit(self);
}

// QUAKED light_lantern4 (1 .5 0) (-18 -7 -7) (7 7 14) x x x STARTOFF NOHALO
// Wall lantern.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
// NOHALO	- Turns off halo effect.
void SP_light_lantern4(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/lantern-4/tris.fm");

	VectorSet(self->mins, -18.0f, -7.0f, -7.0f);
	VectorSet(self->maxs, 7.0f, 7.0f, 14.0f);

	LightInit(self);

	if (!(self->spawnflags & SF_TORCH_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, self->s.origin, "");

	TorchInit(self);
}

// QUAKED light_lantern5 (1 .5 0) (-7 -7 -7) (7 7 14) x x x STARTOFF NOHALO
// Lantern to place on a table.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
// NOHALO	- Turns off halo effect.
void SP_light_lantern5(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/lantern-4/tris.fm");

	VectorSet(self->mins, -7.0f, -7.0f, -7.0f);
	VectorSet(self->maxs, 7.0f, 7.0f, 14.0f);

	LightInit(self);

	self->s.frame = 1;

	if (!(self->spawnflags & SF_TORCH_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, self->s.origin, "");

	TorchInit(self);
}

// QUAKED light_buglight (1 .5 0) (-7 -7 -7) (7 7 25) x x x STARTOFF NOHALO
// A light shaped like a bug.
// Spawnflags:
// STARTOFF	- Light will start off if targeted (default is on).
// NOHALO	- Turns off halo effect.
void SP_light_buglight(edict_t* self)
{
	self->s.modelindex = (byte)gi.modelindex("models/objects/lights/bug/tris.fm");

	VectorSet(self->mins, -7.0f, -7.0f, -7.0f);
	VectorSet(self->maxs, 7.0f, 7.0f, 25.0f);

	LightInit(self);

	self->s.frame = 1;

	if (!(self->spawnflags & SF_TORCH_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, self->s.origin, "");

	TorchInit(self);
}

#pragma endregion
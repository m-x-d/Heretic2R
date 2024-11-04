//
// g_light.c
//
// Heretic II
// Copyright 1998 Raven Software
//

#include "fx.h"
#include "effectflags.h"
#include "g_local.h"
#include "g_misc.h"
#include "g_DefaultMessageHandler.h"
#include "vector.h"

#define LIGHT_STARTOFF	8

#define LIGHT_NOHALO	16

static void TorchUse (edict_t *self, edict_t *other, edict_t *activator);
static void TorchStart (edict_t *self);
void SpawnClientAnim(edict_t *self, byte type, char *sound);
void create_fire_touch (edict_t *owner,vec3_t origin);

void LightStaticsInit()
{

}

void LightInit(edict_t *self)
{
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_LIGHT;
	self->takedamage = DAMAGE_NO;

	BboxYawAndScale(self);

	gi.linkentity (self);
}

void TorchInit (edict_t *self)
{
	// no targeted lights in deathmatch, because they cause global messages
	if (self->targetname && deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (self->style >= 32)
	{
		self->use = TorchUse;

		self->nextthink = level.time + 1.5;		// If you don't wait a little they don't light right
		self->think = TorchStart;
	}
}

void fire_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	if (self->touch_debounce_time > level.time)
		return;

	self->touch_debounce_time = level.time + 1;

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 0, 
			DAMAGE_AVOID_ARMOR|DAMAGE_FIRE|DAMAGE_FIRE_LINGER, MOD_BURNT);
}

#define OBJ_NOPUSH			8

// This creates an entity that will burn the player if he's standing in the fire.

void create_fire_touch (edict_t *owner,vec3_t origin)
{
	edict_t *flame;

	// Now create the flame.

	flame = G_Spawn();

	VectorCopy(origin,flame->s.origin);

	flame->s.scale = owner->s.scale;
	flame->dmg = 3 * owner->s.scale;

	VectorSet(flame->mins, -8, -8, -2);
	VectorSet(flame->maxs, 8, 8, 14);

	flame->spawnflags |= OBJ_NOPUSH;

	flame->movetype = PHYSICSTYPE_NONE;
	flame->touch = fire_touch;

	ObjectInit(flame,2,2,MAT_NONE,SOLID_TRIGGER);

	owner->enemy = flame;
}

void SpawnFlame(edict_t *self,vec3_t origin)
{
	byte scale;

	// NOTE - LIMIT ON SCALE is x 8.
	
	if (self->s.scale >= 8.0)
		scale = 255;
	else
		scale = self->s.scale * 32;

	self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
								FX_FIRE,
								CEF_BROADCAST,
								origin,
								"b",scale);

	create_fire_touch (self,origin);
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF

	Non-displayed light.
	Default light value is 300.
	Default style is 0.
	If targeted, will toggle between on and off.
	Default _cone value is 10 (used to set size of light for spotlights)
*/

#define START_OFF	1

static void light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		gi.configstring (CS_LIGHTS+self->style, "m");
		self->spawnflags &= ~START_OFF;
	}
	else
	{
		gi.configstring (CS_LIGHTS+self->style, "a");
		self->spawnflags |= START_OFF;
	}
}

void SP_light (edict_t *self)
{
	// no targeted lights in deathmatch, because they cause global messages
	if (!self->targetname || deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (self->style >= 32)
	{
		self->use = light_use;
		if (self->spawnflags & START_OFF)
			gi.configstring (CS_LIGHTS+self->style, "a");
		else
			gi.configstring (CS_LIGHTS+self->style, "m");
	}
}

#define FIRE_OFF 8
#define FIRE_MOVEABLE 16

void fire_use (edict_t *self, edict_t *other, edict_t *activator)
{
	byte scale;
	if (self->spawnflags & FIRE_OFF)
	{
		// NOTE - LIMIT ON SCALE is x 8
		if (self->s.scale >= 8.0)
			scale = 255;
		else
			scale = self->s.scale * 32;

		self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
					FX_FIRE,
					CEF_BROADCAST,
					self->s.origin,
					"b",scale);

		create_fire_touch (self,self->s.origin);

		if (self->s.scale < 1)
			self->s.sound = gi.soundindex("ambient/smallfire.wav");
		else
			self->s.sound = gi.soundindex("ambient/fireplace.wav");
		self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;
		self->spawnflags &= ~FIRE_OFF;
	}
	else
	{
		if (self->PersistantCFX)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_FIRE);
			self->PersistantCFX = 0;
		}
		self->s.sound = 0;
		gi.RemoveEffects(&self->s, FX_FIRE);
		self->spawnflags |= FIRE_OFF;
		G_FreeEdict(self->enemy);

	}
}


void firemove_think(edict_t *self)
{
	byte scale;

 	scale = self->s.scale * 8;

	self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
				FX_FIRE_ON_ENTITY,
				CEF_BROADCAST | CEF_OWNERS_ORIGIN,
				self->s.origin,
				"bbb",scale,0,1);

	self->think = NULL;
}

/*QUAKED env_fire (1 .5 0) (0 -10 -24) (20 10 0)  INVULNERABLE ANIMATE EXPLODING  FIRE_OFF MOVEABLE LIGHT_ON
A fire about the size of a campfire. Triggerable.
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE -  N/A
EXPLODING - N/A
FIRE_OFF - fire will start off
MOVEABLE - fire will move if given a velocity
LIGHT_ON - fire will have light attached to it - if moveable, not required
-----------------------------------
scale - size of flame (default 1) (no bigger than 8)
*/
void SP_env_fire (edict_t *self)
{
	byte scale;
	int	flags;
	edict_t	*controller;


	if (!self->s.scale)
		self->s.scale = 1;

	if (self->targetname)
	{
		self->use = fire_use;
		controller = G_Find(NULL, FOFS(target), self->targetname);
		if(controller)
		{//set it up to throw firey chunks
			if(controller->materialtype == MAT_WOOD)
			{
				controller->svflags |= SVF_ONFIRE;
			}
		}
	}


	if (self->spawnflags & FIRE_MOVEABLE)
	{
		VectorSet(self->mins, -2, -2, -2);
		VectorSet(self->maxs, 2, 2, 2);
		self->mass = 250;
		self->friction = 0;
		self->gravity = 0;
		self->s.effects |= EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;
//		self->svflags |= SVF_ALWAYS_SEND;
		self->movetype = PHYSICSTYPE_FLY;

		self->model = 0;
		self->solid = SOLID_NOT;
		self->clipmask = MASK_MONSTERSOLID;

	}
	else
	{
		VectorSet(self->mins, 0, -10, -24);
		VectorSet(self->maxs, 20, 10, 0);
		self->s.effects |= EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;
	}

	gi.linkentity(self);

	if (self->spawnflags & FIRE_OFF)
		return;

	if (self->s.scale < 1)
		self->s.sound = gi.soundindex("ambient/smallfire.wav");
	else
		self->s.sound = gi.soundindex("ambient/fireplace.wav");
	self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;

	// NOTE - LIMIT ON SCALE is x 8
	if (self->s.scale >= 8.0)
	 	scale = 255;
	else
	 	scale = self->s.scale * 32;

	if (self->spawnflags & FIRE_MOVEABLE)
	{
		self->think = firemove_think;
		self->nextthink = level.time + 2;
	}

	// add a light or no ?
	flags = CEF_BROADCAST;
	if (self->spawnflags & 32)
		flags |= CEF_FLAG6;

	self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
						FX_FIRE,
						flags,
						self->s.origin,
						"b",scale);

	create_fire_touch (self,self->s.origin);
}

static void TorchUse (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & LIGHT_STARTOFF)
	{
		gi.configstring (CS_LIGHTS+self->style, "m");
		self->spawnflags &= ~LIGHT_STARTOFF;
	}
	else
	{
		gi.configstring (CS_LIGHTS+self->style, "a");
		self->spawnflags |= LIGHT_STARTOFF;
	}
}

static void TorchStart (edict_t *self)
{
	if (self->spawnflags & LIGHT_STARTOFF)
	{
		gi.configstring (CS_LIGHTS+self->style, "a");
	}
	else
	{
		gi.configstring (CS_LIGHTS+self->style, "m");
	}

	self->think = NULL;
}


/*QUAKED light_walltorch (1 .5 0) (-16 -10 -12) (10 10 12)  INVULNERABLE ANIMATE EXPLODING STARTOFF
A torch that sticks out of a wall
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - Places a flame on it
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
-----------------------------------
*/
void SP_light_walltorch (edict_t *self)
{
	vec3_t holdorigin;

	self->s.modelindex = gi.modelindex("models/objects/lights/walltorch/tris.fm");
	self->s.sound = gi.soundindex("ambient/smallfire.wav");
	self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;

	VectorSet(self->mins, -16, -10, -12);
	VectorSet(self->maxs, 10, 10, 12);

	LightInit(self);

	if (self->spawnflags & 2)	// Animate it
	{
		VectorCopy(self->s.origin,holdorigin);
		holdorigin[2] += 28;
		SpawnFlame(self,holdorigin);		
	}

	TorchInit(self);

}

/*QUAKED light_floortorch (1 .5 0) (-14 -14 -17) (14 14 17)  INVULNERABLE ANIMATE EXPLODING STARTOFF
A stand for a torch that sits on the floor.
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - Places a flame on it
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
-----------------------------------
*/
void SP_light_floortorch (edict_t *self)
{
	vec3_t holdorigin;

	self->s.modelindex = gi.modelindex("models/objects/lights/floortorch/tris.fm");
	self->s.sound = gi.soundindex("ambient/smallfire.wav");
	self->s.sound_data = (127 & ENT_VOL_MASK) | ATTN_STATIC;

	VectorSet(self->mins, -14, -14, -18);
	VectorSet(self->maxs, 14, 14, 18);

	LightInit(self);

	if (self->spawnflags & 2)	// Animate it
	{
		VectorCopy(self->s.origin,holdorigin);
		holdorigin[2] += 33;
		SpawnFlame(self,holdorigin);	
	}

	TorchInit(self);
}


/*QUAK-ED light_flame (1 .5 0) (-16 -16 0) (16 16 34)  INVULNERABLE ANIMATE EXPLODING  STARTOFF
A 30 frame flame.
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
-----------------------------------
*/
/*void SP_light_flame (edict_t *self)
{
	VectorSet(self->mins, -16, -16, -34);
	VectorSet(self->maxs, 16, 16, 0);

	self->nextthink = level.time + 2;
	self->think = flamethink;

	LightInit(self);

	TorchInit(self);
}
*/



/*QUAKED light_torch1 (1 .5 0) (-4 -6 -5) (6 6 20)  INVULNERABLE  ANIMATE   EXPLODING  STARTOFF  NOHALO
Wall torch that uses a blue gem
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
NOHALO - turns off halo effect
-----------------------------------
*/
void SP_light_torch1 (edict_t *self)
{
	vec3_t origin, vf;

	self->s.modelindex=gi.modelindex("models/objects/lights/sinkcity/light-3/tris.fm");

	VectorSet(self->mins, -4, -6, -5);
	VectorSet(self->maxs, 6, 6, 20);

	LightInit(self);

	VectorCopy(self->s.origin, origin);

	AngleVectors(self->s.angles, vf, NULL, NULL);
	VectorMA(origin, 2, vf, origin);

	origin[2] += 16;

	if (!(self->spawnflags & LIGHT_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, CEF_FLAG6|CEF_FLAG7, origin, "");

	TorchInit(self);
}

/*QUAKED light_gem2 (1 .5 0) (-1 -6 -8) (4 6 8)  INVULNERABLE  ANIMATE   EXPLODING  STARTOFF  NOHALO
A yellow gem in an octogonal frame
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
NOHALO - turns off halo effect							
-----------------------------------
style 
- 0 yellow light
- 1 green light

*/
void SP_light_gem2 (edict_t *self)
{
	vec3_t origin;

	self->s.modelindex = gi.modelindex("models/objects/lights/sinkcity/light-2/tris.fm");

	VectorSet(self->mins, -1, -6, -8);
	VectorSet(self->maxs, 4, 6, 8);

	if (self->style == 1)
		self->s.skinnum = 1;

	LightInit(self);

	VectorCopy(self->s.origin, origin);

	if (!(self->spawnflags & LIGHT_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, CEF_FLAG6|CEF_FLAG8, origin, "");

	TorchInit(self);
}

/*QUAKED light_chandelier1 (1 .5 0) (-36 -36 -43) (34 34 43)  INVULNERABLE ANIMATE EXPLODING STARTOFF
A big gold chandelier for the great hall
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
-----------------------------------
*/
void SP_light_chandelier1 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chandelier/chan1/tris.fm");

	VectorSet(self->mins, -36, -36,-43);
	VectorSet(self->maxs, 36, 36, 43);

	LightInit(self);
	TorchInit(self);
}


/*QUAKED light_chandelier2 (1 .5 0) (-18 -18 -40) (18 18 40)  INVULNERABLE ANIMATE EXPLODING  STARTOFF
A very heavy chandelier that doesn't have a skin yet.
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - The flame flickers
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
-----------------------------------
*/
void SP_light_chandelier2 (edict_t *self)
{
	VectorSet(self->mins, -18, -18, -40);
	VectorSet(self->maxs, 18, 18, 40);

	SpawnClientAnim(self, FX_ANIM_CHANDELIER2, NULL);

	LightInit(self);
	TorchInit(self);
}

/*QUAKED light_chandelier3 (1 .5 0) (-34 -34 -80) (34 34 0)  INVULNERABLE ANIMATE EXPLODING STARTOFF
A  thin gold chandelier 
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
-----------------------------------
*/
void SP_light_chandelier3 (edict_t *self)
{
	self->s.modelindex = gi.modelindex("models/objects/chandelier/chan3/tris.fm");

	VectorSet(self->mins, -34, -34,-80);
	VectorSet(self->maxs, 34, 34, 0);

	LightInit(self);

	TorchInit(self);
}

/*QUAKED light_lantern1 (1 .5 0) (-28 -8 -22) (4 8 22)  INVULNERABLE ANIMATE EXPLODING STARTOFF  NOHALO
lantern on a wooden arm
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
NOHALO - turns off halo effect
-----------------------------------
*/
void SP_light_lantern1 (edict_t *self)
{
	vec3_t origin;

	self->s.modelindex = gi.modelindex("models/objects/lights/lantern-1/tris.fm");

	VectorSet(self->mins, -28, -8,-22);
	VectorSet(self->maxs, 4, 8, 22);

	LightInit(self);

	VectorCopy(self->s.origin, origin);
	origin[2] -=10;

	if (!(self->spawnflags & LIGHT_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");

	TorchInit(self);
}

/*QUAKED light_lantern2 (1 .5 0) (-6 -6 -24) (6 6 40)  INVULNERABLE ANIMATE EXPLODING STARTOFF  NOHALO
Lanern on a chain
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
NOHALO - turns off halo effect
-----------------------------------
*/
void SP_light_lantern2 (edict_t *self)
{
	vec3_t origin;

	self->s.modelindex = gi.modelindex("models/objects/lights/lantern-2/tris.fm");

	VectorSet(self->mins, -6, -6,-24);
	VectorSet(self->maxs, 6, 6, 40);

	LightInit(self);

	VectorCopy(self->s.origin, origin);
	origin[2] -=10;

	if (!(self->spawnflags & LIGHT_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");

	TorchInit(self);
}

/*QUAKED light_lantern3 (1 .5 0) (-6 -6 -12) (6 6 11)  INVULNERABLE ANIMATE EXPLODING STARTOFF  NOHALO
Ceiling lantern
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
NOHALO - turns off halo effect
-----------------------------------
*/
void SP_light_lantern3 (edict_t *self)
{
	vec3_t origin;

	self->s.modelindex = gi.modelindex("models/objects/lights/lantern-3/tris.fm");

	VectorSet(self->mins, -6, -6,-12);
	VectorSet(self->maxs, 6, 6, 11);

	LightInit(self);

	VectorCopy(self->s.origin, origin);
	origin[2] -=2;

	if (!(self->spawnflags & LIGHT_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");

	TorchInit(self);
}

/*QUAKED light_lantern4 (1 .5 0) (-18 -7 -7) (7 7 14)  INVULNERABLE ANIMATE EXPLODING STARTOFF  NOHALO
Wall lantern
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
NOHALO - turns off halo effect
-----------------------------------
*/
void SP_light_lantern4 (edict_t *self)
{
	vec3_t origin;

	self->s.modelindex = gi.modelindex("models/objects/lights/lantern-4/tris.fm");

	VectorSet(self->mins, -18, -7,-7);
	VectorSet(self->maxs, 7, 7, 14);

	LightInit(self);

	VectorCopy(self->s.origin, origin);

	if (!(self->spawnflags & LIGHT_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");

	TorchInit(self);
}

/*QUAKED light_lantern5 (1 .5 0) (-7 -7 -7) (7 7 14)  INVULNERABLE ANIMATE EXPLODING STARTOFF NOHALO
Lantern to place on a table
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
NOHALO - turns off halo effect
-----------------------------------
*/
void SP_light_lantern5 (edict_t *self)
{
	vec3_t origin;

	self->s.modelindex = gi.modelindex("models/objects/lights/lantern-4/tris.fm");

	VectorSet(self->mins, -7, -7,-7);
	VectorSet(self->maxs,  7,  7, 14);

	LightInit(self);

	VectorCopy(self->s.origin, origin);
	self->s.frame = 1;

	if (!(self->spawnflags & LIGHT_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");

	TorchInit(self);
}



/*QUAKED light_buglight (1 .5 0) (-7 -7 -7) (7 7 25)  INVULNERABLE ANIMATE EXPLODING STARTOFF NOHALO
A light shaped like a bug
-------  FIELDS  ------------------
INVULNERABLE - N/A
ANIMATE - N/A
EXPLODING - N/A
STARTOFF - Light will start off if targeted (default is on)
NOHALO - turns off halo effect
-----------------------------------
*/
void SP_light_buglight (edict_t *self)
{
	vec3_t origin;

	self->s.modelindex = gi.modelindex("models/objects/lights/bug/tris.fm");

	VectorSet(self->mins, -7, -7,-7);
	VectorSet(self->maxs,  7,  7, 25);

	LightInit(self);

	VectorCopy(self->s.origin, origin);
	self->s.frame = 1;

	if (!(self->spawnflags & LIGHT_NOHALO))
		self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_HALO, 0, origin, "");

	TorchInit(self);
}


/*QUAKED env_sun1 (1 .5 0) (-12 -12 0) (12 12 38) INVULNERABLE ANIMATE  EXPLODING  
Places two suns in the world and attaches a lens flare to them.
One sun is blue, the other is yellow
*/
void SP_env_sun1 (edict_t *self)
{
	vec3_t origin;

	self->solid = SOLID_NOT;
	self->movetype = PHYSICSTYPE_NONE;
	
//	VectorSet(origin, 200, -100, 4000);

//	gi.CreatePersistantEffect(NULL, FX_LENSFLARE, CEF_FLAG7, origin, "bbbbf", -1, (byte) 64, (byte) 64, (byte) 146, 0.75);

	VectorSet(origin, 200, -100, 4000);
	gi.CreatePersistantEffect(NULL, FX_LENSFLARE, CEF_FLAG7 | CEF_FLAG6, origin, "bbbf",  (byte) 128, (byte) 108, (byte) 64, 0.75);
}
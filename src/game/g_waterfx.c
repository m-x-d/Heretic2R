// Water related fx, including
// 1. Fountain

#include "g_local.h"
#include "fx.h"
#include "random.h"
#include "vector.h"

#define OBJ_INVULNERABLE	1
#define OBJ_ANIMATE			2
#define OBJ_EXPLODING		4
#define OBJ_NOPUSH			8

void BboxYawAndScale(edict_t *self);

void waterdrip_go(edict_t *self)
{
	byte	frame;

	frame = 0;
	if(self->spawnflags & 1)
	{
		frame = 1;
	}

	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_DRIPPER, CEF_BROADCAST, 
		self->s.origin, "bb", self->count, frame);

	self->think = NULL;
}

void waterdrip_use (edict_t *self, edict_t *other, edict_t *activator)
{

	if (!self->PersistantCFX)
		waterdrip_go(self);
	else
	{
		gi.RemoveEffects(&self->s,0);
		self->PersistantCFX = 0;
	}
}

/*QUAKED env_water_drip (1 .5 0) (-4 -4 0) (4 4 4) YELLOW
Spawns a drip of water which falls straight down
---- SPAWN FLAGS ------
if YELLOW set.. uses a yellow drip
------- KEYS ----------
count - drips per minute (default 20)
*/

// This really ought to be sent over as a flag
// but its a persistant effect, so not critical

void SP_env_water_drip(edict_t *self)
{

	if(!self->count)
	{
		self->count = 20;
	}

	self->use = waterdrip_use;
	self->solid = SOLID_NOT;
	self->s.effects |= EF_NODRAW_ALWAYS_SEND;
	gi.linkentity(self);

	self->think = waterdrip_go;
	self->nextthink = level.time + 4;

}

#define FOUNTAIN_OFF 32

void fountain_use (edict_t *self, edict_t *other, edict_t *activator)
{
	byte		frame;
	short	drop;

	drop = -self->delay * 8.0;
	frame = 0;
	if (self->spawnflags & FOUNTAIN_OFF)
	{
		self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_FOUNTAIN, CEF_BROADCAST, self->s.origin, "vsb", self->s.angles, drop, frame);
		self->s.sound = gi.soundindex("ambient/fountainloop.wav");
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
		self->spawnflags &= ~FOUNTAIN_OFF;
	}
	else
	{
		if (self->PersistantCFX)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_WATER);
			self->PersistantCFX = 0;
		}
		gi.RemoveEffects(&self->s, FX_FOUNTAIN);
		self->spawnflags |= FOUNTAIN_OFF;
		self->s.sound = 0;
	}
}

/*QUAKED env_water_fountain (1 .5 0) (-4 -4 0) (4 4 4) RED GREEN BLUE DARK DARKER START_OFF
-------SPAWN FLAGS--------
START_OFF - fountain will be off until triggered

--------KEYS-----------
angles - xyz velocity of spawned particles
delay  - the distance from emitter to ground (128 is Okay, max is 256)

If targeted it can be turned on/off but it will start on unless START_OFF

*/


// At the time of creation of this effect, I thought +ve z was down
// Hence the MINUS sign for the distance to fall

void SP_env_water_fountain(edict_t *self)
{
	byte	frame;
	short	drop;

	if (self->targetname) 
	{
		self->use = fountain_use;
	}

	self->s.effects |= EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;
	gi.linkentity(self);

	if (self->spawnflags & FOUNTAIN_OFF)	// Start off
	{
		return;
	}

	drop = -self->delay * 8.0;
	frame = 0;
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_FOUNTAIN, CEF_BROADCAST, self->s.origin, "vsb", self->s.angles, drop, frame);
	self->s.sound = gi.soundindex("ambient/fountainloop.wav");
	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
}

void SpawnDripper(edict_t *self, vec3_t offset)
{
	vec3_t		origin;

	VectorAdd(self->s.origin, offset, origin);
	gi.CreatePersistantEffect(NULL, FX_DRIPPER, 0, origin, "bb", self->count, 2);
}

/*QUAKED env_waterfall_base (1 1 0) (-8 -8 -8) (8 8 8)
angles - this first field is the x radius
         second is the yaw
		 third is the y radius
*/

void SP_env_waterfall_base(edict_t *self)
{
	short	xrad, yrad;
	byte	yaw;

	self->s.effects |= EF_NODRAW_ALWAYS_SEND|EF_ALWAYS_ADD_EFFECTS;

	self->svflags = 0;

	xrad = Q_ftol(self->s.angles[0]);
	yrad = Q_ftol(self->s.angles[2]);
	yaw = Q_ftol((self->s.angles[1] * 256.0) / 360.0);

	gi.linkentity(self);
	gi.CreatePersistantEffect(&self->s, FX_WATERFALLBASE, CEF_BROADCAST, self->s.origin, "bbb", xrad, yrad, yaw);

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

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Cant push it

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

	self->spawnflags |= OBJ_INVULNERABLE;	// Always indestructible
	self->spawnflags |= OBJ_NOPUSH;	// Cant push it
	self->takedamage = DAMAGE_NO;

	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_NO;
	self->clipmask = MASK_MONSTERSOLID;

	BboxYawAndScale(self);



	gi.linkentity(self);
//	ObjectInit(self,100,500,MAT_GREYSTONE,SOLID_BBOX);
}

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


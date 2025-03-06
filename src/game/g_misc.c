//
// g_misc.c
//
// Copyright 1998 Raven Software
//

#include "g_misc.h"
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "g_items.h" //mxd
#include "m_harpy.h" //mxd
#include "m_stats.h"
#include "g_playstats.h"
#include "p_client.h" //mxd
#include "p_teleport.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "q_shared.h"
#include "g_local.h"

int AndoriaSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_SMALLFOUNTAIN,
	AS_LARGEFOUNTAIN,
	AS_SEWERWATER,
	AS_OUTSIDEWATERWAY,
	AS_WINDCHIME,
};

int CloudSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_CAULDRONBUBBLE,
	AS_WINDEERIE,
	AS_WINDNOISY,
	AS_WINDSOFTHI,
	AS_WINDSOFTLO,
	AS_WINDSTRONG1,
	AS_WINDSTRONG2,
	AS_WINDWHISTLE,
};

int HiveSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_GONG,
	AS_WINDEERIE,
	AS_WINDNOISY,
	AS_WINDSOFTHI,
	AS_WINDSOFTLO,
	AS_WINDSTRONG1,
	AS_WINDSTRONG2,
	AS_WINDWHISTLE,
};

int MineSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_MUDPOOL,
	AS_ROCKS,
	AS_WINDEERIE,
	AS_WINDSOFTLO,
	AS_CONVEYOR,
	AS_BUCKETCONVEYOR,
	AS_CAVECREAK,
};

int SilverSpringSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_FIRE,
	AS_WATERLAPPING,	
	AS_SEAGULLS,
	AS_OCEAN,
	AS_BIRDS,
	AS_CRICKETS,
	AS_FROGS,
	AS_CRYING,
	AS_MOSQUITOES,
	AS_BUBBLES,
	AS_BELL,
	AS_FOOTSTEPS,
	AS_MOANS,
	AS_SEWERDRIPS,
	AS_WATERDRIPS,
	AS_HEAVYDRIPS,
	AS_CAULDRONBUBBLE,
	AS_SPIT,
};

int SwampCanyonSoundID[AS_MAX] =
{
	AS_NOTHING,
	AS_BIRD1,
	AS_BIRD2,
	AS_HUGEWATERFALL,
	AS_MUDPOOL,
	AS_WINDEERIE,
	AS_WINDNOISY,
	AS_WINDSOFTHI,
	AS_WINDSOFTLO,
	AS_WINDSTRONG1,
	AS_WINDSTRONG2,
	AS_WINDWHISTLE,
};

#pragma region ========================== func_areaportal ==========================

static void FuncAreaportalUse(edict_t* ent, edict_t* other, edict_t* activator) //mxd. Named 'Use_Areaportal' in original logic.
{
	ent->count ^= 1; // Toggle state.
	gi.SetAreaPortalState(ent->style, ent->count);
}

// QUAKED func_areaportal (0 0 0) ?
// This is a non-visible object that divides the world into areas that are separated when this portal is not activated.
// Usually enclosed in the middle of a door.
void SP_func_areaportal(edict_t* ent)
{
	ent->use = FuncAreaportalUse;
	ent->count = 0; // Always start closed.
}

#pragma endregion

#pragma region ========================== Spawn debris logic ==========================

static void SpawnDebris(edict_t* self, float size, vec3_t origin)
{
	static const char* debris_sounds[NUM_MAT] = //mxd. Made local static.
	{
		"misc/breakstone.wav",	// MAT_STONE
		"misc/breakstone.wav",	// MAT_GREYSTONE
		"misc/tearcloth.wav",	// MAT_CLOTH
		"misc/metalbreak.wav",	// MAT_METAL
		"misc/fleshbreak.wav",	// MAT_FLESH
		"misc/potbreak.wav",	// MAT_POTTERY
		"misc/glassbreak2.wav",	// MAT_GLASS
		"misc/breakstone.wav",	// MAT_LEAF	FIXME
		"misc/breakwood.wav",	// MAT_WOOD
		"misc/breakstone.wav",	// MAT_BROWNSTONE
		"misc/bushbreak.wav",	// MAT_NONE
		NULL,					// MAT_INSECT
	};

	size /= 10.0f;
	byte b_size = (byte)(Clamp(size, 1.0f, 255.0f));

	vec3_t half_size;
	VectorScale(self->size, 0.5f, half_size);
	const byte b_mag = (byte)(Clamp(VectorLength(half_size), 1.0f, 255.0f));

	int fx_flags = 0;
	if (self->fire_damage_time > level.time || (self->svflags & SVF_ONFIRE))
		fx_flags |= CEF_FLAG6;

	if (self->materialtype == MAT_FLESH || self->materialtype == MAT_INSECT)
	{
		const int violence = ((blood_level != NULL) ? (int)blood_level->value : VIOLENCE_DEFAULT); //TODO: why blood_level NULL check? Inited in InitGame(), accessed without NULL check in G_RunFrame()...

		if (violence < VIOLENCE_NORMAL)
		{
			size /= 10.0f; //TODO: check this. Do we really need to divide by 10 AGAIN?
			b_size = (byte)(Clamp(size, 1.0f, 255.0f));
			gi.CreateEffect(NULL, FX_DEBRIS, fx_flags, origin, "bbdb", b_size, MAT_STONE, half_size, b_mag);
		}
		else
		{
			if (violence > VIOLENCE_NORMAL)
			{
				b_size *= (violence - VIOLENCE_NORMAL); //TODO: doesn't make much sense. Max violence configurable via menus is 3. Should be (violence - VIOLENCE_NORMAL + 1)?
				b_size = min(255, b_size);
			}

			if (self->materialtype == MAT_INSECT)
				fx_flags |= CEF_FLAG8;

			if (Q_stricmp(self->classname, "monster_tcheckrik_male") == 0) //mxd. stricmp -> Q_stricmp
				fx_flags |= CEF_FLAG7;//use male insect skin on chunks

			gi.CreateEffect(NULL, FX_FLESH_DEBRIS, fx_flags, origin, "bdb", b_size, half_size, b_mag);
		}
	}
	else
	{
		if (self->s.renderfx & RF_REFLECTION)
			fx_flags |= CEF_FLAG8;

		gi.CreateEffect(NULL, FX_DEBRIS, fx_flags, origin, "bbdb", b_size, (byte)self->materialtype, half_size, b_mag);
	}

	if (self->classID == CID_OBJECT && (strcmp(self->classname, "obj_larvabrokenegg") == 0 || strcmp(self->classname, "obj_larvaegg") == 0))
		self->materialtype = MAT_POTTERY;

	if (debris_sounds[self->materialtype] != NULL)
		gi.sound(self, CHAN_VOICE, gi.soundindex(debris_sounds[self->materialtype]), 2.0f, ATTN_NORM, 0.0f); //TODO: why 2.0 volume?
}

void BecomeDebris(edict_t* self)
{
	const int violence = ((blood_level != NULL) ? (int)blood_level->value : VIOLENCE_DEFAULT); //TODO: why blood_level NULL check? Inited in InitGame(), accessed without NULL check in G_RunFrame()...

	// Haven't yet thrown parts?
	if (violence > VIOLENCE_BLOOD && !(self->svflags & SVF_PARTS_GIBBED) && self->monsterinfo.dismember != NULL)
	{
		//FIXME: have a generic GibParts effect that throws flesh and several body parts - much cheaper?
		int num_limbs = irand(3, 10);

		if (violence > VIOLENCE_NORMAL)
			num_limbs *= (violence - VIOLENCE_NORMAL); //TODO: doesn't make much sense. Max violence configurable via menus is 3. Should be (violence - VIOLENCE_NORMAL + 1)?

		for (int i = 0; i < num_limbs; i++)
			if (self->svflags & SVF_MONSTER)
				self->monsterinfo.dismember(self, irand(80, 160), hl_MeleeHit | irand(hl_Head, hl_LegLowerRight)); //mxd. flrand() -> irand()

		self->svflags |= SVF_PARTS_GIBBED;
		self->think = BecomeDebris;
		self->nextthink = level.time + 0.1f;

		return;
	}

	// Set my message handler to the special message handler for dead entities.
	self->msgHandler = DeadMsgHandler;

	// What the hell is this??? //TODO: when is this used?
	if (self->spawnflags & 4 && !(self->svflags & SVF_MONSTER))
	{
		// Need to create an explosion effect for this.
		edict_t* attacker = ((self->owner != NULL) ? self->owner : self); //mxd
		T_DamageRadius(self, attacker, self, 60.0f, (float)self->dmg, (float)self->dmg / 2.0f, DAMAGE_NORMAL | DAMAGE_AVOID_ARMOR, MOD_DIED);
	}

	// A zero mass is well and truly illegal!
	if (self->mass < 0)
		gi.dprintf("ERROR: %s needs a mass to generate debris", self->classname);

	// Create a chunk-spitting client effect and remove me now that I've been chunked.
	float size;

	// This only yields 4, 8, 12, or 16 chunks, generally seems to yield 16.
	if ((self->svflags & SVF_MONSTER) && self->classID != CID_MOTHER)
	{
		size = VectorLength(self->size) * 100.0f;
	}
	else
	{
		// Set this brush up as if it were an object, so the debris will be thrown properly.
		// If I'm a BModel (and therefore don't have an origin), calculate one to use instead and slap that into my origin.
		if (Vec3IsZero(self->s.origin))
			VectorMA(self->absmin, 0.5f, self->size, self->s.origin);

		size = VectorLength(self->size) * 3.0f;

		if (self->solid == SOLID_BSP)
			size *= 3.0f;
		else if (self->classID == CID_MOTHER)
			size *= 10.0f;

		if (self->mass == 0)
			self->mass = (int)(size / 10.0f);
	}

	SpawnDebris(self, size, self->s.origin);

	self->s.modelindex = 0;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;

	G_SetToFree(self);
	self->nextthink = level.time + 2.0f;
}

void SprayDebris(const edict_t* self, const vec3_t spot, byte num_chunks, float damage) //TODO: remove unused arg.
{
	byte b_mat = (byte)self->materialtype;
	const byte b_mag = (byte)(Clamp(VectorLength(self->mins), 1.0f, 255.0f));

	if (b_mat == MAT_FLESH || b_mat == MAT_INSECT)
	{
		const int violence = ((blood_level != NULL) ? (int)blood_level->value : VIOLENCE_DEFAULT); //TODO: why blood_level NULL check? Inited in InitGame(), accessed without NULL check in G_RunFrame()...

		if (violence < VIOLENCE_NORMAL)
		{
			b_mat = MAT_STONE;
		}
		else if (violence > VIOLENCE_NORMAL)
		{
			num_chunks *= (violence - VIOLENCE_NORMAL); //TODO: doesn't make much sense. Max violence configurable via menus is 3. Should be (violence - VIOLENCE_NORMAL + 1)?
			num_chunks = min(255, num_chunks);
		}
	}

	if (b_mat == MAT_FLESH || b_mat == MAT_INSECT)
	{
		int fx_flags = 0;

		if (self->materialtype == MAT_INSECT)
		{
			fx_flags |= CEF_FLAG8;

			if (Q_stricmp(self->classname, "monster_tcheckrik_male") == 0) //mxd. stricmp -> Q_stricmp
				fx_flags |= CEF_FLAG7; // Use male insect skin on chunks.
		}

		if (self->fire_damage_time > level.time || (self->svflags & SVF_ONFIRE))
			fx_flags |= CEF_FLAG6;

		gi.CreateEffect(NULL, FX_FLESH_DEBRIS, fx_flags, spot, "bdb", num_chunks, self->mins, b_mag);
	}
	else
	{
		num_chunks = (byte)(Clamp((float)num_chunks / 100.0f, 1.0f, 255.0f));
		gi.CreateEffect(NULL, FX_DEBRIS, 0, spot, "bbdb", num_chunks, MAT_STONE, self->mins, b_mag);
	}
}

void DefaultObjectDieHandler(edict_t* self, G_Message_t* msg)
{
	edict_t* inflictor;
	ParseMsgParms(msg, "ee", &inflictor, &inflictor);

	G_UseTargets(self, inflictor);

	if (self->target_ent != NULL)
		BecomeDebris(self->target_ent);

	BecomeDebris(self);
}

#pragma endregion

#pragma region ========================== Body part / weapon throw logic ==========================

void ThrowBodyPart(const edict_t* self, const vec3_t* spot, const int body_part, float damage, const int frame) //TODO: change 'damage' arg type to int.
{
	// Add blood spew to sever loc and blood trail on flying part.
	if (damage > 0.0f)
	{
		damage = min(255.0f, damage);
		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/fleshbreak.wav"), 1.0f, ATTN_NORM, 0.0f);
	}

	vec3_t origin;
	VectorAdd(self->s.origin, *spot, origin);

	int	fx_flags = 0;

	if (self->fire_damage_time > level.time || self->svflags & SVF_ONFIRE)
		fx_flags = CEF_FLAG6;

	if (self->materialtype == MAT_INSECT)
		fx_flags |= CEF_FLAG8;

	if (give_head_to_harpy != NULL && take_head_from == self)
	{
		harpy_take_head(give_head_to_harpy, self, body_part, frame, fx_flags);
		SprayDebris(self, *spot, 5, damage);
	}
	else
	{
		gi.CreateEffect(NULL, FX_BODYPART, fx_flags, origin, "ssbbb", (short)frame, (short)body_part, (byte)damage, self->s.modelindex, self->s.number);
	}
}

void ThrowWeapon(const edict_t* self, const vec3_t* spot, const int body_part, float damage, const int frame) //TODO: change 'damage' arg type to int.
{
	damage = min(255.0f, damage);

	vec3_t origin;
	VectorAdd(self->s.origin, *spot, origin);

	gi.CreateEffect(NULL, FX_THROWWEAPON, 0, origin, "ssbbb", (short)frame, (short)body_part, (byte)damage, self->s.modelindex, self->s.number);
}

#pragma endregion

#pragma region ========================== path_corner ==========================

static void PathCornerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'path_corner_touch' in original logic.
{
	if (other->movetarget != self || other->enemy != NULL)
		return;

	if (self->pathtarget != NULL)
	{
		char* save_target = self->target;
		self->target = self->pathtarget;
		G_UseTargets(self, other);
		self->target = save_target;
	}

	edict_t* next = ((self->target != NULL) ? G_PickTarget(self->target) : NULL);

	if (next != NULL && (next->spawnflags & MSF_AMBUSH))
	{
		vec3_t v;
		VectorCopy(next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= other->mins[2];

		VectorCopy(v, other->s.origin);

		next = G_PickTarget(next->target);
	}

	other->goalentity = next;
	other->movetarget = next;

	if (self->wait > 0.0f)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
	else if (other->movetarget != NULL)
	{
		vec3_t v;
		VectorSubtract(other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = VectorYaw(v);
	}
	else
	{
		other->monsterinfo.pausetime = level.time + 100000000.0f;
		QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
	}
}

// QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
// Variables:
// target		- Target name of next path corner.
// pathtarget	- Used when an entity that has this path_corner targeted touches it angles - used to make the brush rotate.
//				  The brush MUST have an origin brush in it. It is an accumulative value, so if the value is 0 40 0 the brush rotate
//				  an additional 40 degrees along the y axis before it reaches this path corner.
// wait			- -1 makes trains stop until retriggered.
//				  -3 trains explode upon reaching this path corner.
// noise		- Wav file to play when corner is hit (trains only).
void SP_path_corner(edict_t* self)
{
	if (self->targetname == NULL)
	{
		gi.dprintf("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEdict(self);

		return;
	}

	if (st.noise != NULL)
		self->moveinfo.sound_middle = gi.soundindex(st.noise);

	self->solid = SOLID_TRIGGER;
	self->movetype = PHYSICSTYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->touch = PathCornerTouch;

	VectorSet(self->mins, -8.0f, -8.0f, -8.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 8.0f);

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== point_combat ==========================

#define SF_HOLD	1 //mxd

static void PointCombatTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'point_combat_touch' in original logic.
{
	if (other->movetarget != self)
		return; // Wasn't purposely heading for me.

	if (self->target != NULL)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);

		if (other->goalentity == NULL)
		{
			gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			other->movetarget = self;
		}

		self->target = NULL;
	}
	else if ((self->spawnflags & SF_HOLD) && !(other->flags & (FL_SWIM | FL_FLY)))
	{
		other->spawnflags |= MSF_FIXED; // Stay here forever for now.
		QPostMessage(other, MSG_STAND, PRI_DIRECTIVE, NULL);
	}

	if (other->movetarget == self)
	{
		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	}

	if (self->pathtarget != NULL)
	{
		edict_t* activator;
		char* save_target = self->target;
		self->target = self->pathtarget;

		if (other->enemy != NULL && other->enemy->client != NULL)
			activator = other->enemy;
		else if (other->oldenemy != NULL && other->oldenemy->client != NULL)
			activator = other->oldenemy;
		else if (other->activator != NULL && other->activator->client != NULL)
			activator = other->activator;
		else
			activator = other;

		G_UseTargets(self, activator);
		self->target = save_target;
	}
}

// QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) HOLD
// Makes this the target of a monster and it will head here when first activated before going after the activator.
// Spawnflags:
// HOLD - monster will stay here.
void SP_point_combat(edict_t* self)
{
	self->solid = SOLID_TRIGGER;
	self->svflags = SVF_NOCLIENT;
	self->touch = PointCombatTouch;

	VectorSet(self->mins, -8.0f, -8.0f, -16.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 16.0f);

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== info_null, info_notnull  ==========================

// QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
// Used as a positional target for spotlights, etc.
void SP_info_null(edict_t* self)
{
	G_FreeEdict(self);
}

// QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
// Used as a positional target for lightning.
void SP_info_notnull(edict_t* self)
{
	self->solid = SOLID_TRIGGER;
	self->movetype = PHYSICSTYPE_NONE;

	VectorCopy(self->s.origin, self->absmin);
	VectorCopy(self->s.origin, self->absmax);
}

#pragma endregion

#pragma region ========================== func_wall ==========================

#define SF_TRIGGER_SPAWN	1 //mxd
#define SF_TOGGLE			2 //mxd
#define SF_START_ON			4 //mxd
#define SF_ANIMATED			8 //mxd
#define SF_ANIMATED_FAST	16 //mxd

static void FuncWallUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'func_wall_use' in original logic.
{
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox(self);
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	gi.linkentity(self);

	if (!(self->spawnflags & SF_TOGGLE))
		self->use = NULL;
}

// QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
// This is just a solid wall if not inhibited.
// Spawnflags:
// TRIGGER_SPAWN	- The wall will not be present until triggered, it will then blink in to existence; it will kill anything that was in it's way.
// TOGGLE			- Only valid for TRIGGER_SPAWN walls. This allows the wall to be turned on and off.
// START_ON			- Only valid for TRIGGER_SPAWN walls. The wall will initially be present.
void SP_func_wall(edict_t* self)
{
	self->movetype = PHYSICSTYPE_PUSH;
	gi.setmodel(self, self->model);

	if (self->spawnflags & SF_ANIMATED)
		self->s.effects |= EF_ANIM_ALL;

	if (self->spawnflags & SF_ANIMATED_FAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	// Just a wall?
	if (!(self->spawnflags & (SF_TRIGGER_SPAWN | SF_TOGGLE | SF_START_ON)))
	{
		self->solid = SOLID_BSP;
		gi.linkentity(self);

		return;
	}

	// Must have TRIGGER_SPAWN flag.
	self->spawnflags |= SF_TRIGGER_SPAWN;

	// Yell if the spawnflags are odd.
	if ((self->spawnflags & SF_START_ON) && !(self->spawnflags & SF_TOGGLE))
	{
		gi.dprintf("func_wall START_ON without TOGGLE\n");
		self->spawnflags |= SF_TOGGLE;
	}

	self->use = FuncWallUse;

	if (self->spawnflags & SF_START_ON)
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== func_object ==========================

static void FuncObjectTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'func_object_touch' in original logic.
{
	// Only squash thing we fall on top of.
	if (plane != NULL && plane->normal[2] == 1.0f && other->takedamage != DAMAGE_NO)
		T_Damage(other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, DAMAGE_AVOID_ARMOR, MOD_DIED);
}

static void FuncObjectRelease(edict_t* self) //mxd. Named 'func_object_release' in original logic.
{
	self->movetype = PHYSICSTYPE_STEP;
	self->touch = FuncObjectTouch;
}

static void FuncObjectUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'func_object_use' in original logic.
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;

	KillBox(self);
	FuncObjectRelease(self);
}

#define SF_TRIGGER_SPAWN	1 //mxd
#define SF_ANIMATED			2 //mxd
#define SF_ANIMATED_FAST	4 //mxd

// QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
// This is solid bmodel that will fall if it's support it removed.
void SP_func_object(edict_t* self)
{
	gi.setmodel(self, self->model);

	VectorInc(self->mins);
	VectorDec(self->maxs);

	if (self->dmg == 0)
		self->dmg = 100;

	if (self->spawnflags == 0) //TODO: a strange way to check for SF_TRIGGER_SPAWN flag?.. Will also trigger when SF_ANIMATED or SF_ANIMATED_FAST are set!
	{
		self->solid = SOLID_BSP;
		self->think = FuncObjectRelease;
		self->nextthink = level.time + FRAMETIME * 2.0f;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->use = FuncObjectUse;
		self->svflags |= SVF_NOCLIENT;
	}

	if (self->spawnflags & SF_ANIMATED)
		self->s.effects |= EF_ANIM_ALL;

	if (self->spawnflags & SF_ANIMATED_FAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->movetype = PHYSICSTYPE_PUSH;
	self->clipmask = MASK_MONSTERSOLID;

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== item_spitter ==========================

#define SF_NOFLASH	1 //mxd

static void ItemSpitterUse(edict_t* self, edict_t* owner, edict_t* attacker) //mxd. Named 'ItemSpitterSpit' in original logic.
{
	if (self->target == NULL || self->style == 0)
		return;

	self->style = 0; // Mark spitter as used.

	const float delta = 360.0f / (float)self->count;
	vec3_t hold_angles = { owner->s.angles[0], 0.0f, owner->s.angles[2] };
	gitem_t* item = P_FindItemByClassname(self->target);

	for (int i = 0; i < self->count; i++)
	{
		edict_t* new_item = G_Spawn();

		vec3_t forward;
		AngleVectors(hold_angles, forward, NULL, NULL);

		VectorCopy(self->s.origin, new_item->s.origin);
		VectorMA(new_item->s.origin, self->dmg_radius, forward, new_item->s.origin);

		if (self->mass != 0)
			new_item->spawnflags |= self->mass;

		if (item == NULL) // Must be an object, not an item.
		{
			new_item->classname = ED_NewString(self->target);
			ED_CallSpawn(new_item);
		}
		else
		{
			new_item->movetype = PHYSICSTYPE_STEP;
			SpawnItem(new_item, item);
		}

		if (!(self->spawnflags & SF_NOFLASH))
			gi.CreateEffect(NULL, FX_PICKUP, 0, new_item->s.origin, "");

		hold_angles[YAW] += delta;
	}
}

// QUAKED item_spitter (0 .5 .8) (-4 -4 -4)  (4 4 4)	NOFLASH
// When targeted it will spit out an number of items in various directions

// Spawnflags:
// NOFLASH - No flash is created when item is 'spit out'.

// Variables:
// target		- Classname of item or object being spit out.
// count		- Number of items being spit out (default 1).
// radius		- Distance from item_spitter origin that items will be spawned.
// spawnflags2	- The spawnflags for the item being created.
void SP_item_spitter(edict_t* self)
{
	self->style = 1; // To show it hasn't been used yet.
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->count = max(1, self->count);
	self->dmg_radius = (float)st.radius;
	self->mass = st.spawnflags2; //TODO: Why is mass used to store spawnflags? Can't we just use spawnflags for that?..
	self->use = ItemSpitterUse;

	VectorSet(self->mins, -4.0f, -4.0f, -4.0f);
	VectorSet(self->maxs,  4.0f,  4.0f,  4.0f);

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== misc_update_spawner ==========================

// Update the spawner so that we will re-materialize in a different position.
static void MiscUpdateSpawnerTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'respawner_touch' in original version,
{
	// If we aren't a player, forget it.
	if (other->client == NULL)
		return;

	edict_t* spot = NULL;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_start")) != NULL)
	{
		if (game.spawnpoint[0] == 0 && spot->targetname == NULL)
			break;

		if (game.spawnpoint[0] == 0 || spot->targetname == NULL)
			continue;

		if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
			break;
	}

	if (spot == NULL)
	{
		if (game.spawnpoint[0] == 0)
			spot = G_Find(spot, FOFS(classname), "info_player_start"); // There wasn't a spawnpoint without a target, so use any.

		if (spot == NULL)
		{
			gi.error("Couldn't find spawn point %s\n", game.spawnpoint);
			return;
		}
	}

	VectorSet(spot->s.origin, self->mins[0] + self->size[0] / 2.0f, self->mins[1] + self->size[1] / 2.0f, self->mins[2] + self->size[2]);
	VectorCopy(self->s.angles, spot->s.angles);

	G_FreeEdict(self);
}

// QUAKED misc_update_spawner (.5 .5 .5) ?
// This creates the spawner update entity, which updates the spawner position when triggered.
void misc_update_spawner(edict_t* ent) //TODO: Rename to SP_misc_update_spawner?
{
	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = MiscUpdateSpawnerTouch;

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}

#pragma endregion

#pragma region ========================== misc_teleporter ==========================

#define SF_NO_MODEL				1
#define SF_DEATHMATCH_RANDOM	2
#define SF_START_OFF			4
#define SF_MULT_DEST			8

static void MiscTeleporterCreateEffect(edict_t* self) //mxd. Added to reduce code duplication.
{
	edict_t* effect = G_Spawn();

	VectorCopy(self->maxs, effect->maxs);
	VectorCopy(self->mins, effect->mins);
	effect->solid = SOLID_NOT;
	effect->s.effects |= (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS);
	self->enemy = effect;

	gi.linkentity(effect);

	vec3_t fx_origin;
	for (int i = 0; i < 3; i++)
		fx_origin[i] = ((self->maxs[i] - self->mins[i]) / 2.0f) + self->mins[i];

	if (!(self->spawnflags & SF_NO_MODEL))
		effect->PersistantCFX = gi.CreatePersistantEffect(&effect->s, FX_TELEPORT_PAD, CEF_BROADCAST, fx_origin, "");
}

static void MiscTeleporterDeactivate(edict_t* self, G_Message_t* msg) //mxd. Named 'Teleporter_Deactivate' in original logic.
{
	self->touch = NULL;

	// If there's an effect out there, kill it.
	if (self->enemy != NULL)
	{
		gi.RemoveEffects(&self->enemy->s, FX_TELEPORT_PAD);

		if (self->enemy->PersistantCFX > 0)
		{
			gi.RemovePersistantEffect(self->enemy->PersistantCFX, REMOVE_TELEPORT_PAD);
			self->enemy->PersistantCFX = 0;
		}

		self->enemy = NULL;
	}
}

static void MiscTeleporterActivate(edict_t* self, G_Message_t* msg) //mxd. Named 'Teleporter_Activate' in original logic.
{
	self->touch = teleporter_touch;

	// If there's no effect already, create a new one.
	if (self->enemy == NULL)
		MiscTeleporterCreateEffect(self); //mxd
}

void TeleporterStaticsInit(void) //TODO: rename to MiscTeleporterStaticsInit.
{
	classStatics[CID_TELEPORTER].msgReceivers[G_MSG_SUSPEND] = MiscTeleporterDeactivate;
	classStatics[CID_TELEPORTER].msgReceivers[G_MSG_UNSUSPEND] = MiscTeleporterActivate;
}

// QUAKED misc_teleporter (1 0 0) ? NO_MODEL DEATHMATCH_RANDOM START_OFF MULT_DEST
// This creates the teleporter disc that will send us places
// Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.

// Spawnflags:
// NO_MODEL				- Makes teleporter invisible.
// DEATHMATCH_RANDOM	- Makes the teleporter dump you at random spawn points in deathmatch.
// START_OFF			- Pad has no effect, and won't teleport you anywhere till its activated.
// MULT_DEST			- Pad is targeted at more than one destination.

// Variables:
// style - Number of destinations this pad has.
void SP_misc_teleporter(edict_t* ent)
{
	if (ent->target == NULL && (!(ent->spawnflags & SF_DEATHMATCH_RANDOM) || !DEATHMATCH)) //BUGFIX: '!ent->spawnflags & DEATHMATCH_RANDOM' in original logic.
	{
		gi.dprintf("teleporter without a target.\n");
		G_FreeEdict(ent);

		return;
	}

	ent->msgHandler = DefaultMsgHandler;
	ent->classID = CID_TELEPORTER;

	ent->movetype = PHYSICSTYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;

	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	// If we don't have multiple destinations - probably redundant.
	if (!(ent->spawnflags & SF_MULT_DEST))
		ent->style = 0;

	// If we want an effect on spawn, create it.
	if (!(ent->spawnflags & SF_START_OFF))
	{
		ent->touch = teleporter_touch;
		MiscTeleporterCreateEffect(ent); //mxd
	}
}

// QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
// Point teleporters at these.
void SP_misc_teleporter_dest(edict_t* ent)
{
	ent->s.skinnum = 0;
	ent->solid = SOLID_NOT;
	VectorSet(ent->mins, -32.0f, -32.0f, -24.0f);
	VectorSet(ent->maxs,  32.0f,  32.0f, -16.0f);

	gi.linkentity(ent);

	vec3_t end_pos;
	VectorCopy(ent->s.origin, end_pos);
	end_pos[2] -= 500.0f;

	trace_t tr;
	gi.trace(ent->s.origin, vec3_origin, vec3_origin, end_pos, NULL, CONTENTS_WORLD_ONLY | MASK_PLAYERSOLID, &tr);

	VectorCopy(tr.endpos, ent->last_org);
	ent->last_org[2] -= player_mins[2];
}

#pragma endregion

#pragma region ========================== misc_magic_portal ==========================

extern void use_target_changelevel(edict_t* self, edict_t* other, edict_t* activator); //TODO: move to g_target.h

static void MiscMagicPortalTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'misc_magic_portal_touch' in original logic.
{
	if (level.time < self->touch_debounce_time || other->client == NULL) // Not a player.
		return;

	edict_t* ent = NULL;
	ent = G_Find(ent, FOFS(targetname), self->target);

	if (ent != NULL)
		use_target_changelevel(ent, self, other);

	self->touch_debounce_time = level.time + 4.0f;
}

void misc_magic_portal_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (level.time < self->impact_debounce_time)
		return;
	
	if (self->solid == SOLID_NOT)
	{	// We aren't engaged yet.  Make solid and start the effect.
		self->solid = SOLID_TRIGGER;
		self->touch = MiscMagicPortalTouch;
		self->PersistantCFX = gi.CreatePersistantEffect(&self->s, FX_MAGIC_PORTAL, CEF_BROADCAST, self->s.origin, 
								"vbb", self->s.angles, (byte)self->style, (byte)self->count);
		self->s.effects &= ~EF_DISABLE_EXTRA_FX;
	}
	else
	{	// We were on, now turn it off.
		self->solid = SOLID_NOT;
		self->touch = NULL;
		// remove the persistant effect
		if (self->PersistantCFX)
		{
			gi.RemovePersistantEffect(self->PersistantCFX, REMOVE_PORTAL);
			self->PersistantCFX = 0;
		}

		self->s.effects |= EF_DISABLE_EXTRA_FX;
	}

	gi.linkentity(self);

	self->impact_debounce_time = level.time + 4.0;
}



#define START_OFF	1

/*QUAKED misc_magic_portal (1 .5 0) (-16 -16 -32) (16 16 32)  START_OFF
A magical glowing portal. Triggerable.
-------  FIELDS  ------------------
START_OFF - portal will start off
-----------------------------------
angles - manipulates the facing of the effect as normal.
style - 0-blue, 1-red, 2-green
count - Close after 1-255 seconds.  0 means stay until triggered.

In order to be functional as a world teleport,
  it must target a target_changelevel

*/
void SP_misc_magic_portal (edict_t *self)
{
	// Set up the basics.
	VectorSet(self->mins, -16, -16, -32);
	VectorSet(self->maxs, 16, 16, 32);
	self->s.scale = 1;
	self->mass = 250;
	self->friction = 0;
	self->gravity = 0;
	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;
	self->svflags |= SVF_ALWAYS_SEND;
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = NULL;

	self->use = misc_magic_portal_use;

	if (!self->spawnflags & START_OFF)
	{	// Set up the touch function, since this baby is live.
		misc_magic_portal_use(self, NULL, NULL);
	}

	gi.linkentity(self);
}

#pragma endregion

void flame_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->damage_debounce_time < level.time)
	{
		other->damage_debounce_time = level.time + 0.2;

		T_Damage (other, world, world, other->s.origin, vec3_origin, vec3_origin, 10, 0, DAMAGE_SPELL|DAMAGE_AVOID_ARMOR,MOD_DIED);//Fixme: propper params.
	}
}

void flame_think (edict_t *self)
{
//	self->s.frame++;
//	if (self->s.frame > 5)
//		self->s.frame = 0;

//	self->nextthink = level.time + FRAMETIME;
	self->nextthink = level.time + 200;
}


void soundambient_think(edict_t *self)
{
	byte	style,wait,attenuation,volume;

	attenuation = Q_ftol(self->attenuation);
	volume = Q_ftol(self->volume * 255);

	self->s.sound_data = (volume & ENT_VOL_MASK) | attenuation;

	// if its a looping sound, create it on this entity
	switch((int)(self->style))
	{
	case AS_FIRE:
		self->s.sound = gi.soundindex("ambient/fireplace.wav");
		break;
	case AS_WATERLAPPING:
		self->s.sound = gi.soundindex("ambient/waterlap.wav");
		break;
	case AS_OCEAN:	
		self->s.sound = gi.soundindex("ambient/ocean.wav");
		break;
	case AS_SMALLFOUNTAIN:
   		self->s.sound = gi.soundindex("ambient/smallfountain.wav");
		break;
	case AS_LARGEFOUNTAIN:
   		self->s.sound = gi.soundindex("ambient/fountainloop.wav");
		break;
	case AS_SEWERWATER:
   		self->s.sound = gi.soundindex("ambient/sewerflow.wav");
		break;
	case AS_OUTSIDEWATERWAY:
   		self->s.sound = gi.soundindex("ambient/river.wav");
		break;
	case AS_CAULDRONBUBBLE:
		self->s.sound = gi.soundindex("ambient/cauldronbubble.wav");
		break;
	case AS_HUGEWATERFALL:
		self->s.sound = gi.soundindex("ambient/hugewaterfall.wav");
		break;
	case AS_MUDPOOL:
		self->s.sound = gi.soundindex("ambient/mudpool.wav");
		break;
   	case AS_WINDEERIE:
   		self->s.sound = gi.soundindex("ambient/windeerie.wav");
		break;
   	case AS_WINDNOISY:
   		self->s.sound = gi.soundindex("ambient/windnoisy.wav");
		break;
   	case AS_WINDSOFTHI:
   		self->s.sound = gi.soundindex("ambient/windsofthi.wav");
		break;
   	case AS_WINDSOFTLO:
   		self->s.sound = gi.soundindex("ambient/windsoftlow.wav");
		break;
   	case AS_WINDSTRONG1:
   		self->s.sound = gi.soundindex("ambient/windstrong1.wav");
		break;
   	case AS_WINDSTRONG2:
   		self->s.sound = gi.soundindex("ambient/windstrong2.wav");
		break;
   	case AS_WINDWHISTLE:
   		self->s.sound = gi.soundindex("ambient/windwhistle.wav");
		break;
   	case AS_CONVEYOR:
   		self->s.sound = gi.soundindex("objects/conveyor.wav");
		break;
   	case AS_BUCKETCONVEYOR:
   		self->s.sound = gi.soundindex("objects/bucketconveyor.wav");
		break;
   	case AS_SPIT:
   		self->s.sound = gi.soundindex("objects/spit.wav");
		break;
	default:
		style = Q_ftol(self->style);
		wait = Q_ftol(self->wait);
		gi.CreatePersistantEffect(&self->s,
					FX_SOUND,
					CEF_BROADCAST | CEF_OWNERS_ORIGIN,
					self->s.origin,
					"bbbb",
					style,attenuation,volume,wait);
		break;
	}
	self->count = 1;	// This is just a flag to show it's on

	self->think = NULL;
}


void sound_ambient_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count)	// This is just a flag to show it's on
	{
		self->count = 0;
		gi.RemoveEffects(&self->s,0);
	}
	else
		soundambient_think(self);
}


void sound_ambient_init(edict_t *self)
{
	VectorSet(self->mins,-4,-4,-4);
	VectorSet(self->maxs,4,4,4);

	self->movetype = PHYSICSTYPE_NONE;

	if (self->attenuation <= 0.01)
		self->attenuation = 1;

	if (self->volume <= 0.01)
		self->volume = .5;

	if (self->wait<1)
		self->wait = 10;

	self->s.effects |= EF_NODRAW_ALWAYS_SEND;

	if (!(self->spawnflags & 2))
	{
		self->nextthink = level.time + 2.5;
		self->think = soundambient_think;
	}

	// if we are asked to do a sound of type zero, free this edict, since its obviously bogus
	if (!self->style)
	{
		gi.dprintf("Bogus ambient sound at x:%f y:%f z:%f\n",self->s.origin[0], self->s.origin[1],self->s.origin[2]); 
		G_SetToFree(self);
		return;
	}

	// if we are non local, clear the origin of this object
	if (self->spawnflags & 1)
	{
		VectorClear(self->s.origin);
	}	
	else
		// if we are here, then this ambient sound should have an origin
		assert(Vec3NotZero(self->s.origin));

	self->use = sound_ambient_use;

	gi.linkentity(self);
}




/*QUAKED sound_ambient_cloudfortress (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
Generates an ambient sound for cloud fortress levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style  
1 - Cauldron bubbling (looping sound)
2 - wind, low, eerie (looping)
3 - wind, low, noisy (looping)
4 - wind, high, soft (looping)
5 - wind, low, soft (looping)
6 - wind, low, strong (looping)
7 - wind, high, strong (looping)
8 - wind, whistling, strong (looping)

attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 - 
   2 - 
   3 - diminish very rapidly with distance 

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_cloudfortress (edict_t *self)
{
	sound_ambient_init(self);

	self->style = CloudSoundID[self->style];
}


/*QUAKED sound_ambient_mine (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL START_OFF
Generates an ambient sound for mine levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style  
1 - Mud pool bubbling (looping)
2 - Rocks falling (3 sounds)
3 - wind, low, eerie (looping)
4 - wind, low, soft (looping)
5 - conveyor belt (looping)
6 - bucket conveyor belt (looping)
7 - three different creaks of heavy timbers

attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 - 
   2 - 
   3 - diminish very rapidly with distance 

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_mine (edict_t *self)
{
	sound_ambient_init(self);

	self->style = MineSoundID[self->style];
}


/*QUAKED sound_ambient_hive (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL  START_OFF
Generates an ambient sound for hive levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style   
1 - gong
2 - wind, low, eerie (looping)
3 - wind, low, noisy (looping)
4 - wind, high, soft (looping)
5 - wind, low, soft (looping)
6 - wind, low, strong (looping)
7 - wind, high, strong (looping)
8 - wind, whistling, strong (looping)

attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 - 
   2 - 
   3 - diminish very rapidly with distance 

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_hive (edict_t *self)
{
	sound_ambient_init(self);

	self->style = HiveSoundID[self->style];
}


/*QUAKED sound_ambient_andoria (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL  START_OFF
Generates an ambient sound for andoria levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style  
1 - small fountain (constant loop)
2 - large fountain (constant loop)
3 - water running out of sewer (constant loop)
4 - rushing waterway outside (constant loop)
5 - wind chime 

attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 - 
   2 - 
   3 - diminish very rapidly with distance 

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_andoria (edict_t *self)
{
	sound_ambient_init(self);

	self->style = AndoriaSoundID[self->style];
}

/*QUAKED sound_ambient_swampcanyon (1 0 0) (-4 -4 0) (4 4 4) NON_LOCAL  START_OFF
Generates an ambient sound for swamp or canyon levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style   
1 - bird, quick, high pitch
2 - bird, low, medium pitch
3 - huge waterfall
4 - mud pool bubbling (looping)
5 - wind, low, eerie (looping)
6 - wind, low, noisy (looping)
7 - wind, high, soft (looping)
8 - wind, low, soft (looping)
9 - wind, low, strong (looping)
10 - wind, high, strong (looping)
11 - wind, whistling, strong (looping)


attenuation  (how quickly sound drops off from origin)
   0 - heard over entire level (default)
   1 - 
   2 - 
   3 - diminish very rapidly with distance 

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_swampcanyon (edict_t *self)
{
	self->style = SwampCanyonSoundID[self->style];
	sound_ambient_init(self);
}

/*QUAKED sound_ambient_silverspring (1 0 0) (-4 -4 -4) (4 4 4) NON_LOCAL  START_OFF
Generates an ambient sound for silverspring levels
-------  FLAGS  ------------------
NON_LOCAL - sound occurs everywhere in the level - attenuation is not operative with this type of sound
wait    amount of seconds to wait + or - 50% before spawning sound again (default is 10 seconds)
START_OFF - starts off, can be triggered on
-------  KEYS  ------------------
style :  
1 - fire (looping)
2 - water lapping (looping)
3 - seagulls (2 random calls)
4 - ocean
5 - birds (10 random bird calls)
6 - crickets (3 random chirps)
7 - frogs (2 random ribbets)
8 - distant women/children crying (4 total)
9 - mosquitoes (2 random sounds)
10 - bubbles 
11 - bell tolling
12 - footsteps (3 random sounds)
13 - moans/screams/coughing (5 random sounds)
14 - Sewer drips (3 random sounds)
15 - Water drips (3 random sounds)
16 - Solid drips - heavy liquid (3 random sounds)
17 - Cauldron bubbling (looping sound)
18 - Creaking for the spit that's holding the elf over a fire

attenuation  (how quickly sound drops off from origin)
0 - heard over entire level (default)
1 - 
2 - 
3 - diminish very rapidly with distance 

volume   range of .1 to 1   (default .5)
  0 - silent
  1 - full volume
-----------------------------------
*/
void SP_sound_ambient_silverspring (edict_t *self)
{
	self->style = SilverSpringSoundID[self->style];
	sound_ambient_init(self);
}

/*QUAKED misc_remote_camera (0 0.5 0.8) (-4 -4 -4) (4 4 4) ACTIVATING SCRIPTED NO_DELETE

	pathtarget	- holds the name of the camera's owner entity (if any).
	target		- holds the name of the entity to be looked at.
	spawnflags	- 1 only the activating client will see the remote camera view.
	            - 2 this is a scripted camera
				- 4 don't delete camera
*/

void remove_camera(edict_t *Self)
{
	if(Self->spawnflags&1)
	{
		// Just for the activator.

		Self->activator->client->RemoteCameraLockCount--;
	}
	else
	{
		// For all clients.

		int		i;
		edict_t *cl_ent;

		for(i=0;i<game.maxclients;i++)
		{
			cl_ent=g_edicts+1+i;
	
			if(!cl_ent->inuse)
				continue;
	
			cl_ent->client->RemoteCameraLockCount--;
		}
	}

	if(!(Self->spawnflags&4))
	{
		G_FreeEdict(Self);
	}
}

void misc_remote_camera_think(edict_t *Self)
{
	// ********************************************************************************************
	// Attempt to find my owner entity (i.e. what I'm fixed to). If nothing is found, then my
	// position will remain unchanged.
	// ********************************************************************************************

	if(Self->pathtarget)
		Self->enemy=G_Find(NULL,FOFS(targetname),Self->pathtarget);

	if(Self->enemy||(Self->spawnflags&2))
	{
		// I am attatched to another (possibly moving) entity, so update my position.

		if(Self->enemy)
		{
			VectorCopy(Self->enemy->s.origin,Self->s.origin);
		}

		// Update the position on client(s).

		if(Self->spawnflags&1)
		{
			// Just for the activator.

			if(Self->activator->client->RemoteCameraNumber==Self->s.number)
			{
				int	i;

				for(i=0;i<3;i++)
					Self->activator->client->ps.remote_vieworigin[i]=Self->s.origin[i]*8.0;
			}
		}	
		else
		{
			// For all clients.

			int		i;
			edict_t *cl_ent;
	
			for(i=0;i<game.maxclients;i++)
			{
				cl_ent=g_edicts+1+i;
		
				if(!cl_ent->inuse)
					continue;
		
				if(cl_ent->client->RemoteCameraNumber==Self->s.number)
				{
					int j;

					for(j=0;j<3;j++)
						cl_ent->client->ps.remote_vieworigin[j]=Self->s.origin[j]*8.0;
				}
			}
		}
	}

	// ********************************************************************************************
	// Find my target entity and then orientate myself to look at it.
	// ********************************************************************************************
	
	if(Self->targetEnt=G_Find(NULL,FOFS(targetname),Self->target))
	{
		// Calculate the angles from myself to my target.

		vec3_t	Forward;

		VectorSubtract(Self->targetEnt->s.origin,Self->s.origin,Forward);
		VectorNormalize(Forward);
		vectoangles(Forward,Self->s.angles);
		Self->s.angles[PITCH]=-Self->s.angles[PITCH];

		// Update the angles on client(s).

		if(Self->spawnflags&1)
		{
			// Just for the activator.

			if(Self->activator->client->RemoteCameraNumber==Self->s.number)
			{
				int	i;

				for(i=0;i<3;i++)
					Self->activator->client->ps.remote_viewangles[i]=Self->s.angles[i];
			}
		}
		else
		{
			// For all clients.

			int		i;
			edict_t *cl_ent;
	
			for(i=0;i<game.maxclients;i++)
			{
				cl_ent=g_edicts+1+i;
		
				if(!cl_ent->inuse)
					continue;

				if(cl_ent->client->RemoteCameraNumber==Self->s.number)
				{
					int j;
					
					for(j=0;j<3;j++)
						cl_ent->client->ps.remote_viewangles[j]=Self->s.angles[j];
				}
			}
		}
	}

	// Think again or remove myself?

	if (Self->spawnflags & 2)
	{
		Self->nextthink = level.time+FRAMETIME;
	}
	else
	{
		Self->delay-=0.1;

		if (Self->delay >= 0.0)
		{	
			Self->nextthink=level.time+0.1;
		}
		else
		{
			remove_camera(Self);
		}
	}
}

void Use_misc_remote_camera(edict_t *Self,edict_t *Other,edict_t *Activator)
{
	vec3_t	Forward;

	// ********************************************************************************************
	// If I am already active, just return, else flag that I am active.
	// ********************************************************************************************

	if(Self->count)
	{
		if(Self->spawnflags&2)
		{
			// I am a scripted camera, so free myself before returning.
	
			remove_camera(Self);
		}
		
		return;
	}

	Self->count=1;

	// ********************************************************************************************
	// Signal to client(s) that a remote camera view is active,
	// ********************************************************************************************

	if(Self->spawnflags&1)
	{
		// Signal to just the activator (i.e. person who was ultimately responsible for triggering the
		// remote camera) that their camera view has changed to a remote camera view..

		Self->activator=Activator;

		Self->activator->client->RemoteCameraLockCount++;

		Self->activator->client->RemoteCameraNumber=Self->s.number;
	}
	else
	{
		// Signal to all clients that their camera view has changed to a remote camera view..

		int		i;
		edict_t *cl_ent;
	
		for(i=0;i<game.maxclients;i++)
		{
			cl_ent=g_edicts+1+i;
		
			if(!cl_ent->inuse)
				continue;
	
			cl_ent->client->RemoteCameraLockCount++;

			cl_ent->client->RemoteCameraNumber=Self->s.number;
		}
	}

	// ********************************************************************************************
	// Attempt to find my owner entity (i.e. what I'm fixed to). If nothing is found, then I am a
	// static camera so set up my position here (it will remain unchanged hereafter).
	// ********************************************************************************************

	if(!Self->pathtarget)
	{
		// I am static, so set up my position (which will not change hereafter).

		if(Self->spawnflags&1)
		{
			// Just for the activator.

			int	i;

			Self->enemy=NULL;

			for(i=0;i<3;i++)
				Self->activator->client->ps.remote_vieworigin[i]=Self->s.origin[i]*8.0;
		}
		else
		{
			// For all clients.

			int		i,j;
			edict_t *cl_ent;
	
			Self->enemy=NULL;

			for(i=0;i<game.maxclients;i++)
			{
				cl_ent=g_edicts+1+i;
		
				if(!cl_ent->inuse)
					continue;
		
				for(j=0;j<3;j++)
					cl_ent->client->ps.remote_vieworigin[j]=Self->s.origin[j]*8.0;
			}
		}
	}
	else
	{
		Self->enemy=G_Find(NULL,FOFS(targetname),Self->pathtarget);

		if(Self->enemy||(Self->spawnflags&2))
		{
			// I am attatched to another (possibly moving) entity, so update my position.

			if(Self->enemy)
			{
				VectorCopy(Self->enemy->s.origin,Self->s.origin);
			}

			// Update the position on client(s).

			if(Self->spawnflags&1)
			{
				// Just for the activator.

				if(Self->activator->client->RemoteCameraNumber==Self->s.number)
				{
					int	i;

					for(i=0;i<3;i++)
						Self->activator->client->ps.remote_vieworigin[i]=Self->s.origin[i]*8.0;
				}
			}	
			else
			{
				// For all clients.

				int		i;
				edict_t *cl_ent;
		
				for(i=0;i<game.maxclients;i++)
				{
					cl_ent=g_edicts+1+i;
			
					if(!cl_ent->inuse)
						continue;
			
					if(cl_ent->client->RemoteCameraNumber==Self->s.number)
					{
						int j;

						for(j=0;j<3;j++)
							cl_ent->client->ps.remote_vieworigin[j]=Self->s.origin[j]*8.0;
					}
				}
			}
		}
	}
	// ********************************************************************************************
	// Find my target entity and then orientate myself to look at it.
	// ********************************************************************************************

	Self->targetEnt=G_Find(NULL,FOFS(targetname),Self->target);
	VectorSubtract(Self->targetEnt->s.origin,Self->s.origin,Forward);
	VectorNormalize(Forward);
	vectoangles(Forward,Self->s.angles);
	Self->s.angles[PITCH]=-Self->s.angles[PITCH];

	// Update the angles on client(s).

	if(Self->spawnflags&1)
	{
		// Just for the activator.

		if(Self->activator->client->RemoteCameraNumber==Self->s.number)
		{
			int	i;

			for(i=0;i<3;i++)
				Self->activator->client->ps.remote_viewangles[i]=Self->s.angles[i];
		}
	}
	else
	{
		// For all clients.

		int		i;
		edict_t *cl_ent;

		for(i=0;i<game.maxclients;i++)
		{
			cl_ent=g_edicts+1+i;
	
			if(!cl_ent->inuse)
				continue;

			if(cl_ent->client->RemoteCameraNumber==Self->s.number)
			{
				int j;
				
				for(j=0;j<3;j++)
					cl_ent->client->ps.remote_viewangles[j]=Self->s.angles[j];
			}
		}
	}

	// ********************************************************************************************
	// Setup next think stuff.
	// ********************************************************************************************
	
	Self->think=misc_remote_camera_think;
	Self->nextthink=level.time + FRAMETIME;
}

void SP_misc_remote_camera(edict_t *Self)
{
	Self->enemy=Self->targetEnt=NULL;

	if(!Self->target)
	{
		gi.dprintf("Object 'misc_remote_camera' without a target.\n");
		
		G_FreeEdict(Self);
		
		return;
	}
	
	Self->movetype = PHYSICSTYPE_NONE;
	Self->solid=SOLID_NOT;
	VectorSet(Self->mins,-4,-4,-4);
	VectorSet(Self->maxs,4,4,4);
	Self->count=0;

	Self->use=Use_misc_remote_camera;
	
	gi.linkentity(Self);
}

// Spawns a client model animation
// spawnflags & 2 is a designer flag whether to animate or not
// If the model is supposed to animate, the hi bit of the type is set
// If the model is static, then the default frame stored on the client is used
// Valid scale ranges from 1/50th to 5

void SpawnClientAnim(edict_t *self, byte type, char *sound)
{
	int		scale, skin;

	if (self->spawnflags & 2)	// Animate it
	{
		type |= 0x80;
		if(sound)
		{
			self->s.sound = gi.soundindex(sound);
			self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
		}
	}
	scale = (byte)(self->s.scale * 50);
	assert((scale > 0) && (scale < 255));
	skin = (byte)self->s.skinnum;

//	self->svflags |= SVF_ALWAYS_SEND;
	self->PersistantCFX = gi.CreatePersistantEffect(&self->s,
							FX_ANIMATE,
							CEF_BROADCAST,
							self->s.origin,
							"bbbv", type, scale, skin, self->s.angles);

	self->s.effects |= EF_ALWAYS_ADD_EFFECTS;
}

//A check to see if ent should reflect
qboolean EntReflecting(edict_t *ent, qboolean checkmonster, qboolean checkplayer)
{
	if(!ent)
	{
		return false;
	}

	if(checkmonster)
	{
		if(ent->svflags & SVF_MONSTER && ent->svflags & SVF_REFLECT)
		{
			return true;
		}
	}

	if(checkplayer)
	{
		if(ent->client)
		{
			if(ent->client->playerinfo.reflect_timer > level.time)
			{
				return true;
			}
			// possibly, we might want to reflect this if the player has gold armor
			else
			if((ent->client->playerinfo.pers.armortype == ARMOR_TYPE_GOLD) && (ent->client->playerinfo.pers.armor_count) && (irand(0,100) < 30))
				return true;

		}
	}

	return false;
}

/*void SkyFlyCheck(edict_t *self)
{
	if(self->s.origin[2]>3900)
		G_FreeEdict(self);
	else
		self->nextthink = level.time + 0.1;
}*/

void SkyFly (edict_t *self) //TODO: replace with G_SetToFree()?
{
/*	if(deathmatch->value)
	{*/
		G_SetToFree(self);
		return;
/*	}
//They're not being drawn, even after this is set- why not?  Did they stop?
	self->svflags |= SVF_ALWAYS_SEND;
	self->movetype = PHYSICSTYPE_NOCLIP;
	self->solid = SOLID_NOT;

	self->touch = NULL;
	self->isBlocked = NULL;
	self->isBlocking = NULL;
	self->bounced = NULL;
//or just remove self after a time
	self->think = SkyFlyCheck;
	self->nextthink = level.time + 0.1;
*/
}

void fire_spark_think (edict_t *self)
{
	if(self->delay && self->delay < level.time)
	{
		G_FreeEdict(self);
		return;
	}

	self->think = fire_spark_think;
	self->nextthink = level.time + 0.1;//self->wait;
}

void fire_spark_gone (edict_t *self, edict_t *other, edict_t *activator)
{
	self->use = NULL;
	gi.RemoveEffects(&self->s, FX_SPARKS);
	G_FreeEdict(self);
}

void fire_spark_use (edict_t *self, edict_t *other, edict_t *activator)
{
	gi.CreateEffect(&self->s, FX_SPARKS, CEF_FLAG6|CEF_FLAG7|CEF_FLAG8, self->s.origin, "d", vec3_up);

	self->use = fire_spark_gone;

	self->think = fire_spark_think;
	self->nextthink = level.time + 0.1;
}

/*QUAKED misc_fire_sparker (0 0 0) (-4 -4 0) (4 4 8) FIREBALL

  FIREBALL - more of a poofy fireball trail

  Fires of sparks when used...
  used a second time removes it

  "delay" - how long to live for... (default is forever)
*/

void SP_misc_fire_sparker (edict_t *self)
{

	if(self->spawnflags & 1)
		self->s.effects |= EF_MARCUS_FLAG1;

	self->svflags |= SVF_ALWAYS_SEND;
	self->solid = SOLID_NOT;
	self->movetype = PHYSICSTYPE_NOCLIP;
	self->clipmask = 0;

	self->use = fire_spark_use;
}

// end

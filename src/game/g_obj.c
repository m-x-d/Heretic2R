//
// g_obj.c
//
// Copyright 1998 Raven Software
//

#include <float.h> //mxd
#include "g_obj.h" //mxd
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_light.h" //mxd
#include "g_monster.h" //mxd
#include "g_playstats.h"
#include "m_move.h" //mxd
#include "m_plagueElf.h" //mxd. For PlagueElfDyingSound().
#include "m_plagueElf_anim.h"
#include "m_stats.h"
#include "FX.h"
#include "Random.h"
#include "Utilities.h" //mxd
#include "Vector.h"

#pragma region ========================== ObjectStaticsInit ==========================

void DefaultObjectDieHandler(edict_t* self, G_Message_t* msg) //mxd. Originally defined in g_misc.c
{
	edict_t* inflictor;
	G_ParseMsgParms(msg, "ee", &inflictor, &inflictor);

	G_UseTargets(self, inflictor);

	if (self->target_ent != NULL)
		BecomeDebris(self->target_ent);

	BecomeDebris(self);
}

void ObjectStaticsInit(void)
{
	classStatics[CID_OBJECT].msgReceivers[MSG_DEATH] = DefaultObjectDieHandler;
}

#pragma endregion

#pragma region ========================== LeverStaticsInit ==========================

static void ObjLeverUsed(edict_t* self, G_Message_t* msg) //mxd. Named 'lever_used' in original logic.
{
	self->activator = self->enemy;
}

void LeverStaticsInit(void)
{
	classStatics[CID_LEVER].msgReceivers[MSG_DEATH] = ObjLeverUsed;
}

#pragma endregion

#pragma region ========================== Utility functions ==========================

// It is assumed all bounding boxes for objects were initially implemented as if the objects yaw was 0.
void BboxYawAndScale(edict_t* self)
{
	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	// Get bbox corners rotated on XY axis.
	vec3_t rotated_corners[4];

	VectorMA(self->s.origin, self->mins[0], forward, rotated_corners[0]);
	VectorMA(rotated_corners[0], self->mins[1], right, rotated_corners[0]);

	VectorMA(self->s.origin, self->maxs[0], forward, rotated_corners[1]);
	VectorMA(rotated_corners[1], self->maxs[1], right, rotated_corners[1]);

	VectorMA(self->s.origin, self->mins[0], forward, rotated_corners[2]);
	VectorMA(rotated_corners[2], self->maxs[1], right, rotated_corners[2]);

	VectorMA(self->s.origin, self->maxs[0], forward, rotated_corners[3]);
	VectorMA(rotated_corners[3], self->mins[1], right, rotated_corners[3]);

	// Find min and max points.
	vec3_t min_point = { FLT_MAX, FLT_MAX, 0.0f };
	vec3_t max_point = { -FLT_MAX, -FLT_MAX, 0.0f }; //mxd. Because FLT_MIN is min normalized POSITIVE value...
	
	for (int i = 0; i < 4; i++)
	{
		min_point[0] = min(rotated_corners[i][0], min_point[0]);
		min_point[1] = min(rotated_corners[i][1], min_point[1]);

		max_point[0] = max(rotated_corners[i][0], max_point[0]);
		max_point[1] = max(rotated_corners[i][1], max_point[1]);
	}

	// Update ent bbox.
	const float scale = (self->s.scale == 0.0f ? 1.0f : self->s.scale);

	self->mins[0] = (min_point[0] - self->s.origin[0]) * scale;
	self->mins[1] = (min_point[1] - self->s.origin[1]) * scale;
	self->mins[2] *= scale;

	self->maxs[0] = (max_point[0] - self->s.origin[0]) * scale;
	self->maxs[1] = (max_point[1] - self->s.origin[1]) * scale;
	self->maxs[2] *= scale;
}

void PushableObjectTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'objpush_touch' in original logic.
{
	// FIXME: make player push?
	if (other->groundentity == NULL || other->groundentity == self)
		return;

	const float ratio = (float)other->mass / (float)self->mass;

	if (M_walkmove(self, other->s.angles[YAW], 20.0f * ratio * FRAMETIME) && self->pain_debounce_time < level.time)
	{
		// There are going to be more sounds to choose from, dependent on the mass of the object.
		gi.sound(self, CHAN_BODY, gi.soundindex("misc/barrelmove.wav"), 1.0f, ATTN_STATIC, 0.0f);
		self->pain_debounce_time = level.time + 1.2f;
	}
}

void ObjectInit(edict_t* self, const int health, const int mass, const MaterialID_t material_type, const int solid)
{
	self->solid = solid;
	self->msgHandler = DefaultMsgHandler;
	self->takedamage = ((self->spawnflags & SF_OBJ_INVULNERABLE) ? DAMAGE_NO : DAMAGE_YES);
	self->clipmask = MASK_MONSTERSOLID;

	if (self->health == 0)
		self->health = health;

	if (self->mass == 0)
		self->mass = ((mass == 0) ? 10 : mass); // Needs a mass if it breaks up.

	if (self->materialtype == 0)
		self->materialtype = material_type;

	BboxYawAndScale(self);

	if (!(self->spawnflags & SF_OBJ_NOPUSH)) // PUSHABLE.
	{
		self->movetype = PHYSICSTYPE_STOP;
		self->monsterinfo.aiflags = AI_NOSTEP;
		self->touch = PushableObjectTouch;

		self->think = M_DropToFloor;
		self->nextthink = level.time + FRAMETIME * 2.0f;
	}
	else
	{
		self->movetype = PHYSICSTYPE_NONE;
		gi.linkentity(self);
	}
}

#pragma endregion

#pragma region ========================== obj_banner, obj_banneronpole ==========================

// QUAKED obj_banner (1 .5 0) (-8 -44 -296) (8 44 0) x ANIMATE
// A really big banner.
// Spawnflags:
// ANIMATE - Makes it flutter in the breeze, just like a ...banner.
// Variables:
// skinnum - 0 : blue, 1 : red.
void SP_obj_banner(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -44.0f, -296.0f);
	VectorSet(self->maxs, 8.0f, 44.0f, 0.0f);

	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	SpawnClientAnim(self, FX_ANIM_BANNER, "ambient/bannerflap.wav");
	ObjectInit(self, 40, 200, MAT_CLOTH, SOLID_BBOX);
}

// QUAKED obj_banneronpole (1 .5 0) (-8 -28 -30) (8 28 30) x ANIMATE
// A banner on a pole sticking out of a wall.
// Spawnflags:
// ANIMATE - Makes it flutter in the breeze.
void SP_obj_banneronpole(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -28.0f, -30.0f);
	VectorSet(self->maxs, 8.0f, 28.0f, 30.0f);

	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	SpawnClientAnim(self, FX_ANIM_BANNERONPOLE, "ambient/bannerflap.wav");
	ObjectInit(self, 40, 200, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_barrel ==========================

void ObjBarrelExplodeThink(edict_t* self) //mxd. Named 'barrel_explode_think' in original logic.
{
	vec3_t origin;
	VectorCopy(self->s.origin, origin);

	AlertMonsters(self, self->owner, 3.0f, false);

	self->fire_damage_time = level.time + 1.0f;
	self->svflags |= SVF_ONFIRE;
	BecomeDebris(self);

	T_DamageRadiusFromLoc(origin, self->owner, self->owner, NULL, BARREL_EXPLODE_RADIUS, BARREL_EXPLODE_DMG_MAX, BARREL_EXPLODE_DMG_MIN, DAMAGE_NORMAL | DAMAGE_FIRE | DAMAGE_EXTRA_KNOCKBACK, MOD_BARREL);

	// Start the explosion.
	gi.CreateEffect(NULL, FX_BARREL_EXPLODE, CEF_BROADCAST, origin, "");

	G_SetToFree(self);
}

void ObjBarrelDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point) //mxd. Named 'barrel_explode_think' in original logic.
{
	self->think = ObjBarrelExplodeThink;
	self->nextthink = level.time + FRAMETIME;
	self->owner = attacker; // The one to get credit for this should be the one destroying the barrel.

	self->takedamage = DAMAGE_NO;
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->blocked = NULL;
	self->isBlocked = NULL;
	self->isBlocking = NULL;
	self->bounced = NULL;

	gi.linkentity(self);
}

// QUAKED obj_barrel (1 .5 0) (-12 -12 -19) (12 12 19) INVULNERABLE x EXPLODING NOPUSH
// A barrel.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// EXPLODING	- The barrel will explode and cause radius damage.
// NOPUSH		- Can't be moved by player.
void SP_obj_barrel(edict_t* self)
{
	VectorSet(self->mins, -12.0f, -12.0f, -19.0f);
	VectorSet(self->maxs, 12.0f, 12.0f, 19.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/barrel/normal/tris.fm");
	self->dmg = 10;

	ObjectInit(self, 10, 60, MAT_WOOD, SOLID_BBOX);

	if (self->spawnflags & SF_OBJ_EXPLODING)
	{
		// Set this up so we go through die rather than the message.
		self->classID = CID_NONE;
		self->die = ObjBarrelDie;
		self->s.skinnum = 1;
	}
}

#pragma endregion

#pragma region ========================== obj_broom, obj_chair1, obj_chair2, obj_chair3 ==========================

// QUAKED obj_broom (1 .5 0) (-2 -2 -25) (2 2 25) INVULNERABLE
// A broom.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_broom(edict_t* self)
{
	VectorSet(self->mins, -2.0f, -2.0f, -25.0f);
	VectorSet(self->maxs, 2.0f, 2.0f, 25.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/broom/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH;

	ObjectInit(self, 10, 40, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_chair1 (1 .5 0) (-12 -8 -26) (12 8 26)  INVULNERABLE x x NOPUSH
// A highback wooden chair with a triangle at the top.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_chair1(edict_t* self)
{
	VectorSet(self->mins, -12.0f, -8.0f, -26.0f);
	VectorSet(self->maxs, 12.0f, 8.0f, 26.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chairs/chair1/tris.fm");

	ObjectInit(self, 20, 50, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_chair2 (1 .5 0) (-18 -29 -30) (18 29 30)  INVULNERABLE
// A thick chair with slanted sides
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_chair2(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -29.0f, -30.0f);
	VectorSet(self->maxs, 18.0f, 29.0f, 30.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chairs/chair2/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH;

	ObjectInit(self, 20, 50, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_chair3 (1 .5 0) (-14 -21 -28) (14 21 28)
// A big stone throne.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_chair3(edict_t* self)
{
	VectorSet(self->mins, -14.0f, -21.0f, -28.0f);
	VectorSet(self->maxs, 14.0f, 21.0f, 28.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chairs/chair3/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 20, 50, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_chest1, obj_chest2, obj_chest3 ==========================

void ObjChest1AnimThink(edict_t* self) //mxd. Named 'chest1_anim' in original logic.
{
	if (self->s.frame < 10)
	{
		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void ObjChest1Use(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'chest1_use' in original logic.
{
	gi.sound(self, CHAN_VOICE, gi.soundindex("objects/chest.wav"), 1.0f, ATTN_NORM, 0.0f);
	self->think = ObjChest1AnimThink;
	self->nextthink = level.time + FRAMETIME;
}

// QUAKED obj_chest1 (1 .5 0) (-10 -18 -19) (10 18 19) INVULNERABLE x x NOPUSH
// A large chest with a snake carving on top. When used it opens its lid.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_chest1(edict_t* self)
{
	VectorSet(self->mins, -10.0f, -18.0f, -19.0f);
	VectorSet(self->maxs, 10.0f, 18.0f, 19.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chests/chest1/tris.fm");
	self->use = ObjChest1Use;

	ObjectInit(self, 60, 150, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_chest2 (1 .5 0) (-14 -17 -9) (14 17 9) INVULNERABLE
// A medium sized chest with the top open - for use in the mines.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_chest2(edict_t* self)
{
	VectorSet(self->mins, -14.0f, -17.0f, -9.0f);
	VectorSet(self->maxs, 14.0f, 17.0f, 9.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chests/chest2/tris.fm");
	self->spawnflags &= ~SF_OBJ_NOPUSH; //TODO: explicitly pushable. Why?

	ObjectInit(self, 60, 150, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_chest3 (1 .5 0) (-10 -17 -6) (10 17 6) INVULNERABLE
// A medium sized chest with the top closed - for use in the mines.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_chest3(edict_t* self)
{
	VectorSet(self->mins, -10.0f, -17.0f, -6.0f);
	VectorSet(self->maxs, 10.0f, 17.0f, 6.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chests/chest3/tris.fm");
	self->spawnflags &= ~SF_OBJ_NOPUSH; //TODO: explicitly pushable. Why?

	ObjectInit(self, 60, 150, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_cog1 ==========================

void ObjCog1AnimThink(edict_t* self) //mxd. Named 'cog1_anim' in original logic.
{
	if (++self->s.frame > 11)
		self->s.frame = 1;

	if (self->touch_debounce_time > level.time) // First time through reach anim.
	{
		self->nextthink = level.time + FRAMETIME * 2.0f;
	}
	else
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1.0f, ATTN_NORM, 0.0f);
		self->think = NULL;
	}
}

void ObjCog1Use(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'cog1_use' in original logic.
{
	gi.sound(self, CHAN_VOICE, gi.soundindex("items/cogsturn.wav"), 1.0f, ATTN_NORM, 0.0f);

	self->touch_debounce_time = level.time + (FRAMETIME * 30.0f);
	self->s.frame = 1;

	self->think = ObjCog1AnimThink;
	self->nextthink = level.time + FRAMETIME * 2.0f;
}

// QUAKED obj_cog1 (1 .5 0) (-8 -4 0) (8 4 20)
// A cog with spokes coming out the front of it.
void SP_obj_cog1(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -4.0f, 0.0f);
	VectorSet(self->maxs, 8.0f, 4.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/cogs/cog1/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->use = ObjCog1Use;

	ObjectInit(self, 40, 20, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_corpse1, obj_corpse2 ==========================

static void ObjCorpseInit(edict_t* self) //mxd. Named 'SpawnCorpse' in original logic.
{
	self->s.modelindex = (byte)gi.modelindex("models/monsters/plaguelf/tris.fm");
	self->style = ClampI(self->style, 0, 4); //mxd

	switch (self->style)
	{
		case 0:
			self->s.frame = FRAME_death13end;
			break;

		case 1:
			self->s.frame = FRAME_deathb13end;
			break;

		case 2:
			self->s.frame = FRAME_deathc13end;
			break;

		case 3:
			self->s.frame = FRAME_deathd13end;
			break;

		case 4:
			self->s.frame = FRAME_skewered;
			self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
			self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;
			break;
	}

	const int chance = irand(0, 3); //TODO: the gaff has 2x chance to show up (cause irand() max is inclusive). Also, not needed with style 4.

	if (chance == 0)
	{
		// Show the hammer.
		self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
	}
	else if (chance == 1)
	{
		// Show the hoe.
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
	}
	else
	{
		// Show the gaff (that hook thingie).
		self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
		self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
	}

	VectorSet(self->mins, -30.0f, -12.0f, -2.0f);
	VectorSet(self->maxs, 30.0f, 12.0f, 2.0f);

	self->spawnflags |= SF_OBJ_NOPUSH;	// Can't be pushed
	self->svflags |= SVF_DEADMONSTER; // Doesn't block walking.

	ObjectInit(self, 40, 60, MAT_FLESH, SOLID_BBOX);
}

// QUAKED obj_corpse1 (1 .5 0) (-30 -12 0) (30 12 5) INVULNERABLE
// Dead plague elf.
// Spawnflags:
// INVULNERABLE - It can't be hurt.
// Variables:
// style - (default 0)
//		0 - Both arms above head.
//		1 - On side.
//		2 - Arm over face.
//		3 - Arms out to side.
//		4 - Skewered.
void SP_obj_corpse1(edict_t* self)
{
	ObjCorpseInit(self);
}

// QUAKED obj_corpse2 (1 .5 0) (-30 -12 0) (30 12 5) INVULNERABLE
// Dead plague elf with a different skin
// Spawnflags:
// INVULNERABLE - It can't be hurt.
// Variables:
// style - (default 0)
//		0 - Both arms above head.
//		1 - On side.
//		2 - Arm over face.
//		3 - Arms out to side.
//		4 - Skewered.
void SP_obj_corpse2(edict_t* self)
{
	ObjCorpseInit(self);
	self->s.skinnum = 1;
}

#pragma endregion

#pragma region ========================== obj_dying_elf ==========================

void ObjDyingElfIdle(edict_t* self) //mxd. Named 'dying_elf_idle' in original logic.
{
	if (++self->s.frame > FRAME_fetal26)
		self->s.frame = FRAME_fetal1;

	if (irand(0, 50) == 0)
		PlagueElfDyingSound(self, DYING_ELF_IDLE_VOICE);

	self->nextthink = level.time + FRAMETIME;
}

void ObjDyingElfReachAnim(edict_t* self) //mxd. Named 'dying_elf_reach_anim' in original logic.
{
	if (self->touch_debounce_time < level.time) // First time through reach anim.
	{
		self->s.frame = FRAME_reach1;
		self->touch_debounce_time = level.time + (FRAMETIME * 60.0f);
	}
	else if (self->count == 0) // Reaching.
	{
		self->s.frame++;
		self->think = ObjDyingElfIdle;
	}

	if (self->s.frame > FRAME_reach38) // All done, stay down for a bit.
	{
		self->s.frame = FRAME_fetal1;
		self->think = ObjDyingElfIdle;
	}

	self->nextthink = level.time + FRAMETIME;
}

void ObjDyingElfTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'dying_elf_touch' in original logic.
{
	if (self->touch_debounce_time < level.time) // First time through reach anim.
	{
		if (irand(1, 3) == 1 || self->touch_debounce_time == -1.0f)
		{
			self->enemy = other;
			self->think = ObjDyingElfReachAnim;
			self->nextthink = level.time + FRAMETIME;

			if (self->enemy->client != NULL || (self->enemy->svflags & SVF_MONSTER))
				PlagueElfDyingSound(self, DYING_ELF_TOUCH_VOICE);
		}
		else
		{
			self->touch_debounce_time = level.time + (FRAMETIME * 20.0f);
		}
	}
}

void ObjDyingElfPain(edict_t* self, edict_t* other, float kick, int damage) //mxd. Named 'dying_elf_pain' in original logic.
{
	self->enemy = other;
	self->think = ObjDyingElfReachAnim;
	self->nextthink = level.time + FRAMETIME;

	if (self->enemy->client != NULL || (self->enemy->svflags & SVF_MONSTER))
		PlagueElfDyingSound(self, DYING_ELF_PAIN_VOICE);
}

void ObjDyingElfDie(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t point) //mxd. Named 'dying_elf_die' in original logic.
{
	gi.sound(self, CHAN_VOICE, gi.soundindex(va("monsters/plagueElf/death%i.wav", irand(1, 3))), 1.0f, ATTN_NORM, 0.0f);
	BecomeDebris(self);
}

// QUAKED obj_dying_elf (1 .5 0) (-30 -12 0) (30 12 5) INVULNERABLE
// Plague elf lying on the ground shaking.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// Variables:
// style - Skin to use.
void SP_obj_dying_elf(edict_t* self)
{
	VectorSet(self->mins, -32.0f, -32.0f, -2.0f);
	VectorSet(self->maxs, 32.0f, 32.0f, 6.0f);

	self->s.modelindex = (byte)gi.modelindex("models/monsters/plaguelf/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->movetype = PHYSICSTYPE_STEP; //TODO: ignored: ObjectInit() below will re-set this to PHYSICSTYPE_NONE.

	self->touch_debounce_time = -1.0f;
	self->touch = ObjDyingElfTouch;
	self->pain = ObjDyingElfPain;
	self->die = ObjDyingElfDie;

	ObjectInit(self, 40, 60, MAT_FLESH, SOLID_BBOX);

	// No weapons.
	self->s.fmnodeinfo[MESH__HOE].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__GAFF].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__HAMMER].flags |= FMNI_NO_DRAW;
	self->s.fmnodeinfo[MESH__HANDLE].flags |= FMNI_NO_DRAW;

	self->s.frame = FRAME_fetal1;
	self->think = ObjDyingElfIdle;
	self->nextthink = level.time + FRAMETIME;
}

#pragma endregion

#pragma region ========================== obj_sign1, obj_sign4 ==========================

// QUAKED obj_sign1 (1 .5 0) (-29 -4 -16) (29 4 16) INVULNERABLE
// A square sign coming out of a wall.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// Variables:
// style:
//		0 - Sign with a dragon.
//		1 - Sign with two steins. //TODO: skin does not exist!
//		2 - Sign with a fish.
void SP_obj_sign1(edict_t* self)
{
	VectorSet(self->mins, -29.0f, -4.0f, -16.0f);
	VectorSet(self->maxs,  29.0f,  4.0f,  16.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/signs/sign1/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.skinnum = ClampI(self->style, 0, 2); //mxd
	self->s.frame = 3;

	ObjectInit(self, 40, 150, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_sign4 (1 .5 0) (-8 -18 -29) (8 18 29) INVULNERABLE
// A square sign that is on top of a post. It is leaning badly. For the MINE or DESERT areas.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// Variables:
// style:
//		0 - Andorian skin.
//		1 - Tchecktrik skin.
void SP_obj_sign4(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -18.0f, -29.0f);
	VectorSet(self->maxs, 8.0f, 18.0f, 29.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/signs/sign4/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed
	self->s.skinnum = ClampI(self->style, 0, 1); //mxd

	ObjectInit(self, 40, 150, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_stalactite1, obj_stalactite2, obj_stalactite3 ==========================

#define SF_DRIP		1 //mxd
#define SF_DARKSKIN	2 //mxd

static void StalactiteInitDripper(edict_t* self, const vec3_t tip) //mxd. Added to reduce code duplication.
{
	if (!(self->spawnflags & SF_DRIP))
		return;

	if (self->count == 0)
		self->count = 20;

	//mxd. Rotate, scale and move tip coords to world pos.
	vec3_t origin;
	VectorRotate(tip, self->s.angles[YAW], origin);
	Vec3ScaleAssign(self->s.scale, origin);
	Vec3AddAssign(self->s.origin, origin);

	self->PersistantCFX = gi.CreatePersistantEffect(NULL, FX_DRIPPER, 0, origin, "bb", self->count, 0); //mxd. Last arg was 2 in original logic (but waterdrop.sp2 only has single frame...).
}

// QUAKED obj_stalactite1 (1 .5 0) (-24 -24 -99) (24 24 99) DRIP DARKSKIN
// A big long thick stalactite. These point down.
// Spawnflags:
// DRIP		- Spawn drips at the end. //TODO: why do we need both spawnflag and field for this?..
// DARKSKIN	- Use the dark skin.
// Variables:
// count - number of drips per minute.
void SP_obj_stalactite1(edict_t* self) //mxd. Originally located in g_waterfx.c.
{
	static const vec3_t tip = { -7.0273f, -2.9505f, -99.938f }; //mxd. Vertex coord of stalagmite's tip.

	StalactiteInitDripper(self, tip); //BUGFIX: mxd. original logic sets origin[2] to 200 (which is way above the model). 

	self->s.modelindex = (byte)gi.modelindex("models/objects/stalactite/stalact1/tris.fm");
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;

	if (self->spawnflags & SF_DARKSKIN)
		self->s.skinnum = 1;

	VectorSet(self->mins, -24.0f, -24.0f, -99.0f);
	VectorSet(self->maxs,  24.0f,  24.0f,  99.0f);

	gi.linkentity(self);
}

// QUAKED obj_stalactite2 (1 .5 0) (-60 -60 -64) (60 60 64) DRIP DARKSKIN
// A big short stalactite. These point down.
// Spawnflags:
// DRIP		- Spawn drips at the end. //TODO: why do we need both spawnflag and field for this?..
// DARKSKIN	- Use the dark skin.
// Variables:
// count - number of drips per minute.
void SP_obj_stalactite2(edict_t* self)
{
	static const vec3_t tip = { 1.9282f, 11.5995f, -64.439f }; //mxd. Vertex coord of stalagmite's tip.

	StalactiteInitDripper(self, tip); //BUGFIX: mxd. original logic sets origin[2] to 128 (which is way above the model). 

	self->s.modelindex = (byte)gi.modelindex("models/objects/stalactite/stalact2/tris.fm");
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;

	if (self->spawnflags & SF_DARKSKIN)
		self->s.skinnum = 1;

	VectorSet(self->mins, -60.0f, -60.0f, -64.0f);
	VectorSet(self->maxs,  60.0f,  60.0f,  64.0f);

	gi.linkentity(self);
}

// QUAKED obj_stalactite3 (1 .5 0) (-23 -23 -98) (23 23 98) DRIP DARKSKIN
// A long pointy stalactite. These point down.
// Spawnflags:
// DRIP		- Spawn drips at the end. //TODO: why do we need both spawnflag and field for this?..
// DARKSKIN	- Use the dark skin.
// Variables:
// count - number of drips per minute.
void SP_obj_stalactite3(edict_t* self)
{
	static const vec3_t tip = { -0.5409f, 12.404f, -99.2525f }; //mxd. Vertex coord of stalagmite's tip.

	StalactiteInitDripper(self, tip); //BUGFIX: mxd. original logic sets origin[2] to 200 (which is way above the model). 

	self->s.modelindex = (byte)gi.modelindex("models/objects/stalactite/stalact3/tris.fm");
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;

	if (self->spawnflags & SF_DARKSKIN)
		self->s.skinnum = 1;

	VectorSet(self->mins, -23.0f, -23.0f, -98.0f);
	VectorSet(self->maxs,  23.0f,  23.0f,  98.0f);

	gi.linkentity(self);
}

#pragma endregion

#pragma region ========================== obj_stalagmite1, obj_stalagmite2, obj_stalagmite3 ==========================

#undef SF_DARKSKIN //mxd (UGH!)
#define SF_DARKSKIN	8 //mxd

// QUAKED obj_stalagmite1 (1 .5 0) (-32 -32 -200) (32 32 0) INVULNERABLE x x DARKSKIN
// A big long thick stalagmite. These point up.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// DARKSKIN		- If checked, it uses the dark skin.
void SP_obj_stalagmite1(edict_t* self)
{
	VectorSet(self->mins, -32.0f, -32.0f, -200.0f);
	VectorSet(self->maxs, 32.0f, 32.0f, 0.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/stalagmite/smite1/tris.fm"); //TODO: model doesn't exist.
	self->s.skinnum = ((self->spawnflags & SF_DARKSKIN) ? 1 : 0); //mxd

	//TODO: SF_DARKSKIN will be interpreted by ObjectInit() as OBJ_NOPUSH.
	ObjectInit(self, 200, 300, MAT_BROWNSTONE, SOLID_BBOX);
}

#undef SF_DARKSKIN //mxd (UGH!)
#define SF_DARKSKIN	1 //mxd

// QUAKED obj_stalagmite2 (1 .5 0) (-32 -32 -128) (32 32 0) DARKSKIN x x NOPUSH
// A big squat stalagmite. These point up.
// Spawnflags:
// DARKSKIN - If checked, it uses the dark skin.
// NOPUSH	- Can't be moved by player.
void SP_obj_stalagmite2(edict_t* self)
{
	VectorSet(self->mins, -32.0f, -32.0f, -128.0f);
	VectorSet(self->maxs, 32.0f, 32.0f, 0.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/stalagmite/smite2/tris.fm"); //TODO: model doesn't exist.
	self->s.skinnum = ((self->spawnflags & SF_DARKSKIN) ? 1 : 0); //mxd

	//TODO: SF_DARKSKIN will be interpreted by ObjectInit() as OBJ_INVULNERABLE.
	ObjectInit(self, 200, 300, MAT_BROWNSTONE, SOLID_BBOX);
}

// QUAKED obj_stalagmite3 (1 .5 0) (-32 -32 -200) (32 32 0) DARKSKIN x x NOPUSH
// A long pointy stalagmite. These point up.
// Spawnflags:
// DARKSKIN - If checked, it uses the dark skin.
// NOPUSH	- Can't be moved by player.
void SP_obj_stalagmite3(edict_t* self)
{
	VectorSet(self->mins, -16.0f, -16.0f, -200.0f);
	VectorSet(self->maxs, 16.0f, 16.0f, 0.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/stalagmite/smite3/tris.fm"); //TODO: model doesn't exist.
	self->s.skinnum = ((self->spawnflags & SF_DARKSKIN) ? 1 : 0); //mxd

	//TODO: SF_DARKSKIN will be interpreted by ObjectInit() as OBJ_INVULNERABLE.
	ObjectInit(self, 200, 200, MAT_BROWNSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ==== obj_statue_corvus, obj_statue_dolphin1, obj_statue_dolphin2, obj_statue_dolphin3, obj_statue_dolphin4, obj_statue_guardian ====

// QUAKED obj_statue_corvus (1 .5 0) (-16 -16 0) (16 16 32)
// A statue of Corvus.
void SP_obj_statue_corvus(edict_t* self)
{
	VectorSet(self->mins, -16.0f, -16.0f, 0.0f);
	VectorSet(self->maxs, 16.0f, 16.0f, 128.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/corvus/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);
}

#define SF_VULNERABLE	1 //mxd

// QUAKED obj_statue_dolphin1 (1 .5 0) (-68 -22 -30) (68 22 30) VULNERABLE
// The dolphin on a wall. Head turned to the left.
// Spawnflags:
// VULNERABLE - It can be hurt - default it can't be.
void SP_obj_statue_dolphin1(edict_t* self)
{
	VectorSet(self->mins, -68.0f, -22.0f, -30.0f);
	VectorSet(self->maxs, 68.0f, 22.0f, 30.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/dolphin/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH;

	if (self->spawnflags & SF_VULNERABLE)
		self->spawnflags &= ~SF_OBJ_INVULNERABLE; // Can be destroyed.
	else
		self->spawnflags |= SF_OBJ_INVULNERABLE; // Can't be destroyed.

	ObjectInit(self, 300, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_statue_dolphin2 (1 .5 0) (-17 -20 -70) (17 20 70)
// The dolphin on all fours.
void SP_obj_statue_dolphin2(edict_t* self)
{
	VectorSet(self->mins, -17.0f, -20.0f, -70.0f);
	VectorSet(self->maxs, 17.0f, 20.0f, 70.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/dolphin/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = 1;

	ObjectInit(self, 300, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_statue_dolphin3 (1 .5 0) (-17 -20 -70) (17 20 70)
// The dolphin up on two legs.
void SP_obj_statue_dolphin3(edict_t* self)
{
	VectorSet(self->mins, -17.0f, -20.0f, -70.0f);
	VectorSet(self->maxs, 17.0f, 20.0f, 70.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/dolphin/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = 3;

	ObjectInit(self, 300, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_statue_dolphin4 (1 .5 0) (-63 -22 -37) (63 22 37)
// The dolphin on a wall. Head turned to the right.
void SP_obj_statue_dolphin4(edict_t* self)
{
	VectorSet(self->mins, -63.0f, -22.0f, -37.0f);
	VectorSet(self->maxs, 63.0f, 22.0f, 37.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/dolphin/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = 2;

	ObjectInit(self, 300, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_statue_guardian (1 .5 0) (-100 -64 0) (64 64 128)
// A big statue of a fish guy on his haunches holding a spear.
void SP_obj_statue_guardian(edict_t* self)
{
	VectorSet(self->mins, -100.0f, -64.0f, 0.0f);
	VectorSet(self->maxs, 64.0f, 64.0f, 128.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/guardian/tris.fm"); //TODO: has unused second skin. Pick it randomly?
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 400, 300, MAT_METAL, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_table1, obj_table2, obj_throne,  ==========================

// QUAKED obj_table1 (1 .5 0) (-28 -54 -18) (28 54 18) INVULNERABLE x x NOPUSH
// A large wooden dining table with two legs.
// Spawnflags:
// INVULNERABLE - It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_table1(edict_t* self)
{
	VectorSet(self->mins, -28.0f, -54.0f, -18.0f);
	VectorSet(self->maxs, 28.0f, 54.0f, 18.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/tables/table1/tris.fm");

	ObjectInit(self, 40, 100, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_table2 (1 .5 0) (-28 -54 -17) (28 54 17) INVULNERABLE
// A grey stone table.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_table2(edict_t* self)
{
	VectorSet(self->mins, -28.0f, -54.0f, -17.0f);
	VectorSet(self->maxs, 28.0f, 54.0f, 17.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/tables/table2/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH;

	ObjectInit(self, 80, 150, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_throne (1 .5 0) (-20 -22 -44) (20 22 44) INVULNERABLE
// A highbacked throne.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_throne(edict_t* self)
{
	VectorSet(self->mins, -20.0f, -22.0f, -44.0f);
	VectorSet(self->maxs, 20.0f, 22.0f, 44.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chairs/throne/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH;

	ObjectInit(self, 150, 200, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_kettle, obj_cauldron, obj_firepot ==========================

// QUAKED obj_kettle (1 .5 0) (-8 -8 0) (8 8 10) INVULNERABLE x x NOPUSH
// A kettle.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_kettle(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, 0.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 10.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/kettle/tris.fm");

	ObjectInit(self, 40, 100, MAT_METAL, SOLID_BBOX);
}

// QUAKED obj_cauldron (1 .5 0) (-22 -22 -10) (22 22 10) INVULNERABLE ANIMATE
// A metal cauldron filled with green liquid.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// ANIMATE		- Play ambient/cauldronbubble sound.
void SP_obj_cauldron(edict_t* self)
{
	VectorSet(self->mins, -22.0f, -22.0f, -10.0f);
	VectorSet(self->maxs, 22.0f, 22.0f, 10.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/caldrn/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	if (self->spawnflags & SF_OBJ_ANIMATE)	// Animate it.
	{
		self->s.sound = (byte)gi.soundindex("ambient/cauldronbubble.wav");
		self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;
	}

	ObjectInit(self, 60, 100, MAT_METAL, SOLID_BBOX);
}

// QUAKED obj_firepot (1 .5 0) (-18 -18 -12) (18 18 12) x ANIMATE
// A grey stone firepot.
// Spawnflags:
// ANIMATE - Spawn fire fx.
void SP_obj_firepot(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -18.0f, -12.0f);
	VectorSet(self->maxs, 18.0f, 18.0f, 12.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/firepot/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.sound = (byte)gi.soundindex("ambient/fireplace.wav"); //TODO: play only when OBJ_ANIMATE is set?
	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;

	ObjectInit(self, 140, 100, MAT_GREYSTONE, SOLID_BBOX);

	if (self->spawnflags & SF_OBJ_ANIMATE)	// Animate it.
	{
		vec3_t hold_origin;
		VectorCopy(self->s.origin, hold_origin);
		hold_origin[2] += 30.0f; //TODO: hovers above firepot. Spawn a bit lower?

		SpawnFlame(self, hold_origin);
	}
}

#pragma endregion

#pragma region ========================== obj_statue_duckbill1, obj_statue_duckbill2 ==========================

// QUAKED obj_statue_duckbill1 (1 .5 0) (-67 -24 -51) (67 24 51)
// The duckbilled thing - tail to the left.
void SP_obj_statue_duckbill1(edict_t* self)
{
	VectorSet(self->mins, -67.0f, -24.0f, -51.0f);
	VectorSet(self->maxs, 67.0f, 24.0f, 51.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/duckbill/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 150, 100, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_statue_duckbill2 (1 .5 0) (-67 -24 -50) (67 24 50)
// The duckbilled thing - tail to the right.
void SP_obj_statue_duckbill2(edict_t* self)
{
	VectorSet(self->mins, -67.0f, -24.0f, -50.0f); //TODO: why z-difference between this and obj_statue_duckbill1?..
	VectorSet(self->maxs, 67.0f, 24.0f, 50.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/duckbill/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = 1;

	ObjectInit(self, 150, 100, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_seasonglobe ==========================

#define GLOBE_ENABLED	1.0f //mxd
#define GLOBE_DISABLED	0.0f //mxd

void ObjSeasonglobeBottomThink(edict_t* self) //mxd. Named 'globebottom_turn' in original logic.
{
	M_ChangeYaw(self);

	const float current = anglemod(self->s.angles[YAW]);
	const float ideal = self->ideal_yaw;

	if (fabsf(ideal - current) < 0.1f)
	{
		self->s.angles[YAW] = ceilf(ideal);

		if (self->s.angles[YAW] == 225.0f && self->enemy->s.angles[YAW] == 45.0f)
			G_UseTargets(self, self);

		self->monsterinfo.idle_time = GLOBE_DISABLED;
		self->think = NULL;
	}
	else
	{
		self->nextthink = level.time + FRAMETIME;
	}
}

void ObjSeasonglobeTopThink(edict_t* self) //mxd. Named 'globetop_turn' in original logic.
{
	M_ChangeYaw(self);

	const float current = anglemod(self->s.angles[YAW]);
	const float ideal = self->ideal_yaw;

	if (fabsf(ideal - current) < 0.1f)
	{
		self->s.angles[YAW] = ceilf(ideal);

		if (self->s.angles[YAW] == 270.0f) // Because they want it to swing all the way around.
		{
			self->ideal_yaw = 45.0f;
			self->nextthink = level.time + FRAMETIME;
		}
		else
		{
			if (self->s.angles[YAW] == 45.0f && self->enemy->s.angles[YAW] == 225.0f)
				G_UseTargets(self, self);

			self->monsterinfo.idle_time = GLOBE_DISABLED;
			self->think = NULL;
		}
	}
	else
	{
		self->nextthink = level.time + FRAMETIME;
	}
}

void ObjSeasonglobeBottomUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'globebottom_use' in original logic.
{
	if (self->monsterinfo.idle_time == GLOBE_ENABLED)
		return;

	gi.sound(self, CHAN_BODY, gi.soundindex("objects/globebottomstart.wav"), 1.0f, ATTN_NORM, 0.0f);

	self->monsterinfo.idle_time = GLOBE_ENABLED;
	self->ideal_yaw = 225.0f;

	self->think = ObjSeasonglobeBottomThink;
	self->nextthink = level.time + FRAMETIME;
}

void ObjSeasonglobeTopUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'globetop_use' in original logic.
{
	if (self->monsterinfo.idle_time == GLOBE_ENABLED)
		return;

	gi.sound(self, CHAN_BODY, gi.soundindex("objects/globetop.wav"), 1.0f, ATTN_NORM, 0.0f);

	self->monsterinfo.idle_time = GLOBE_ENABLED;
	self->ideal_yaw = 270.0f;

	self->think = ObjSeasonglobeTopThink;
	self->nextthink = level.time + FRAMETIME;
}

// QUAKED obj_seasonglobe (1 .5 0) (-80 -80 0) (80 80 100)
// The globe thingy. //mxd. Uses hardcoded targetnames and angles for top and bottom parts...
void SP_obj_seasonglobe(edict_t* bottom)
{
	// Setup bottom part.
	VectorSet(bottom->mins, -80.0f, -80.0f, 0.0f);
	VectorSet(bottom->maxs, 80.0f, 80.0f, 320.0f);

	bottom->s.modelindex = (byte)gi.modelindex("models/objects/globe/globebottom/tris.fm");
	bottom->s.frame = 1;
	bottom->s.scale = 1.75f;
	bottom->movetype = PHYSICSTYPE_NONE;
	bottom->solid = SOLID_BBOX;

	BboxYawAndScale(bottom);
	bottom->targetname = "globebottom";
	bottom->use = ObjSeasonglobeBottomUse;
	bottom->yaw_speed = 2.5f;
	VectorSet(bottom->s.angles, 0.0f, 90.0f, 0.0f);

	gi.linkentity(bottom);

	// Setup top part.
	edict_t* top = G_Spawn();

	VectorSet(top->mins, -80.0f, -80.0f, 0.0f);
	VectorSet(top->maxs, 80.0f, 80.0f, 100.0f);

	VectorCopy(bottom->s.origin, top->s.origin);
	top->s.origin[2] += 36.0f;

	top->s.modelindex = (byte)gi.modelindex("models/objects/globe/globetop/tris.fm");
	top->s.frame = 1;
	top->s.scale = 1.75f;
	top->movetype = PHYSICSTYPE_NONE;
	top->solid = SOLID_BBOX;

	BboxYawAndScale(top);
	top->targetname = "globetop";
	top->use = ObjSeasonglobeTopUse;
	top->yaw_speed = 2.5f;
	VectorSet(top->s.angles, 0.0f, 120.0f, 0.0f);
	top->target = bottom->target;

	gi.linkentity(top);

	bottom->enemy = top;
	top->enemy = bottom;
}

#pragma endregion

#pragma region ========================== obj_stein, obj_scroll, obj_fountain_fish, obj_statue_boulderfish ==========================

// QUAKED obj_stein (1 .5 0) (-2 -2 -3) (2 2 3) INVULNERABLE x x NOPUSH
// A beer stein.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_stein(edict_t* self)
{
	VectorSet(self->mins, -2.0f, -2.0f, -3.0f);
	VectorSet(self->maxs, 2.0f, 2.0f, 3.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/stein/tris.fm");

	ObjectInit(self, 15, 10, MAT_METAL, SOLID_BBOX);
}

// QUAKED obj_scroll (1 .5 0) (-2 -18 -3) (2 18 3) INVULNERABLE x x NOPUSH
// A paper scroll.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_scroll(edict_t* self)
{
	VectorSet(self->mins, -2.0f, -18.0f, -3.0f);
	VectorSet(self->maxs, 2.0f, 18.0f, 3.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/scroll/tris.fm");

	ObjectInit(self, 10, 50, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_fountain_fish (1 .5 0) (-52 -34 -48) (52 34 48)
// A two-headed fish fountain.
void SP_obj_fountain_fish(edict_t* self)
{
	VectorSet(self->mins, -52.0f, -34.0f, -48.0f);
	VectorSet(self->maxs, 52.0f, 34.0f, 48.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/fountainfish/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 40, 50, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_statue_boulderfish (1 .5 0) (-26 -16 -27) (26 16 27) INVULNERABLE
// A statue of a fish. The one which raises up a boulder.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_statue_boulderfish(edict_t* self)
{
	VectorSet(self->mins, -26.0f, -16.0f, -27.0f);
	VectorSet(self->maxs, 26.0f, 16.0f, 27.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/boulderfish/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH;

	ObjectInit(self, 200, 150, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_pottedplant, obj_plant1, obj_plant2, obj_plant3 ==========================

// QUAKED obj_pottedplant (1 .5 0) (-20 -20 -30) (20 20 30) INVULNERABLE x x NOPUSH
// A potted plant with ferns.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_pottedplant(edict_t* self)
{
	VectorSet(self->mins, -20.0f, -20.0f, -30.0f);
	VectorSet(self->maxs, 20.0f, 20.0f, 30.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/plant/tris.fm");

	ObjectInit(self, 20, 50, MAT_POTTERY, SOLID_BBOX);
}

// QUAKED obj_plant1 (1 .5 0) (-8 -8 -24) (8 8 24) INVULNERABLE
// A clump of tall, thin, plants.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_plant1(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -24.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 24.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/plant1/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.effects |= EF_CAMERA_NO_CLIP;

	ObjectInit(self, 20, 50, MAT_LEAF, SOLID_NOT);
}

// QUAKED obj_plant2 (1 .5 0) (-20 -20 -10) (20 20 20) INVULNERABLE
// A plant with broad leaves.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_plant2(edict_t* self)
{
	VectorSet(self->mins, -20.0f, -20.0f, -10.0f);
	VectorSet(self->maxs, 20.0f, 20.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/plant2/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.effects |= EF_CAMERA_NO_CLIP;

	ObjectInit(self, 20, 50, MAT_LEAF, SOLID_NOT);
}

// QUAKED obj_plant3 (1 .5 0) (-8 -8 -12) (8 8 12) INVULNERABLE
// A group of ferns.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// Variables:
// style - Fern skin (0 - 2).
void SP_obj_plant3(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -12.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 12.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/plant3/tris.fm");
	self->s.skinnum = ClampI(self->style, 0, 2); //mxd
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.effects |= EF_CAMERA_NO_CLIP;

	ObjectInit(self, 20, 50, MAT_LEAF, SOLID_NOT);
}

#pragma endregion

#pragma region ========================== obj_treetop, obj_tree, obj_tree2, obj_tree3, obj_treetall, obj_treefallen ==========================

// QUAKED obj_treetop (1 .5 0) (-176 -176 -125) (176 176 125)
// A canopy for a tree.
void SP_obj_treetop(edict_t* self)
{
	VectorSet(self->mins, -176.0f, -176.0f, -125.0f);
	VectorSet(self->maxs, 176.0f, 176.0f, 125.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/treetop/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 40, 50, MAT_WOOD, SOLID_NOT);
}

// QUAKED obj_tree (1 .5 0) (-100 -100 -120) (100 100 120)
// A tree for Matt's level. //mxd. Unfinished texture.
void SP_obj_tree(edict_t* self)
{
	VectorSet(self->mins, -100.0f, -100.0f, -120.0f);
	VectorSet(self->maxs, 100.0f, 100.0f, 120.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/tree/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 40, 50, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_tree2 (1 .5 0) (-50 -50 -286) (50 50 286)
// A tall spikey tree for the swamps.
void SP_obj_tree2(edict_t* self)
{
	VectorSet(self->mins, -50.0f, -50.0f, -286.0f);
	VectorSet(self->maxs, 50.0f, 50.0f, 286.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/tree2/trunk2/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 40, 50, MAT_WOOD, SOLID_BBOX);

	// Spawn moss. //mxd. Why is this a separate model?..
	edict_t* moss = G_Spawn();

	VectorCopy(self->s.origin, moss->s.origin);
	VectorCopy(self->s.angles, moss->s.angles);

	moss->s.modelindex = (byte)gi.modelindex("models/objects/plants/tree2/moss2/tris.fm");
	moss->movetype = PHYSICSTYPE_NONE;
	moss->solid = SOLID_NOT;
	moss->s.scale = self->s.scale;
	moss->s.renderfx |= RF_TRANSLUCENT;
	BboxYawAndScale(moss);
	gi.linkentity(moss);

	self->target_ent = moss;
}

// QUAKED obj_tree3 (1 .5 0) (-50 -50 -286) (50 50 286)
// A tall spikey tree with big roots on the bottom.
void SP_obj_tree3(edict_t* self)
{
	VectorSet(self->mins, -50.0f, -50.0f, -286.0f);
	VectorSet(self->maxs, 50.0f, 50.0f, 286.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/tree3/trunk3/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 40, 50, MAT_WOOD, SOLID_BBOX);

	// Spawn moss. //mxd. Why is this a separate model?..
	edict_t* moss = G_Spawn();

	VectorCopy(self->s.origin, moss->s.origin);
	VectorCopy(self->s.angles, moss->s.angles);

	moss->s.modelindex = (byte)gi.modelindex("models/objects/plants/tree3/moss3/tris.fm");
	moss->movetype = PHYSICSTYPE_NONE;
	moss->solid = SOLID_NOT;
	moss->s.scale = self->s.scale;
	moss->s.renderfx |= RF_TRANSLUCENT;
	BboxYawAndScale(moss);
	gi.linkentity(moss);

	self->target_ent = moss;
}

// QUAKED obj_treetall (1 .5 0) (-46 -46 -340) (46 46 340)
// A very tall tree.
void SP_obj_treetall(edict_t* self)
{
	VectorSet(self->mins, -46.0f, -46.0f, -340.0f);
	VectorSet(self->maxs, 46.0f, 46.0f, 340.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/talltree/trunk1/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 40, 50, MAT_WOOD, SOLID_BBOX);

	// Spawn moss. //mxd. Why is this a separate model?..
	edict_t* moss = G_Spawn();

	VectorCopy(self->s.origin, moss->s.origin);
	VectorCopy(self->s.angles, moss->s.angles);

	moss->s.modelindex = (byte)gi.modelindex("models/objects/plants/talltree/moss1/tris.fm");
	moss->movetype = PHYSICSTYPE_NONE;
	moss->solid = SOLID_NOT;
	moss->s.renderfx |= RF_TRANSLUCENT;
	moss->s.scale = self->s.scale;
	BboxYawAndScale(moss);
	gi.linkentity(moss);

	self->target_ent = moss;
}

// QUAKED obj_treefallen (1 .5 0) (-24 -62 -35) (24 62 35)
// A cactus tree that is leaning as if it had over. Meant to be partially submerged in water or muck.
void SP_obj_treefallen(edict_t* self)
{
	VectorSet(self->mins, -24.0f, -62.0f, -35.0f);
	VectorSet(self->maxs, 24.0f, 62.0f, 35.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/cactus/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = 1;

	ObjectInit(self, 40, 50, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region == obj_shovel, obj_woodpile, obj_fishtrap, obj_bench, obj_bucket, obj_ropechain, obj_wheelbarrow, obj_wheelbarrowdamaged, obj_urn, obj_bigcrystal ==

// QUAKED obj_shovel (1 .5 0) (-8 -8 -20) (8 8 20)  INVULNERABLE
// A shovel.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_shovel(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -20.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/shovel/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 20, 40, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_woodpile (1 .5 0) (-12 -20 -7) (12 20 7) INVULNERABLE
// A pile of chopped wood.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_woodpile(edict_t* self)
{
	VectorSet(self->mins, -12.0f, -20.0f, -7.0f);
	VectorSet(self->maxs, 12.0f, 20.0f, 7.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/wood/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH;	// Can't be pushed.
	self->s.scale = 2.0f;

	ObjectInit(self, 100, 150, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_fishtrap (1 .5 0) (-14 -28 -13) (14 28 13) INVULNERABLE x x NOPUSH
// A fishtrap.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_fishtrap(edict_t* self)
{
	VectorSet(self->mins, -14.0f, -28.0f, -13.0f);
	VectorSet(self->maxs, 14.0f, 28.0f, 13.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/fishtrap/tris.fm");

	ObjectInit(self, 30, 100, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_bench (1 .5 0) (-10 -21 -10) (10 21 10) INVULNERABLE
// A stone bench to sit on.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_bench(edict_t* self)
{
	VectorSet(self->mins, -10.0f, -22.0f, -10.0f);
	VectorSet(self->maxs, 10.0f, 22.0f, 10.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chairs/bench/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 3, 4, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_bucket (1 .5 0) (-8 -8 -9) (8 8 10) INVULNERABLE
// A bucket.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_bucket(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -9.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 10.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/bucket/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 3, 4, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_ropechain (1 .5 0) (-20 -20 -14) (20 20 14)
// A pile of rope or chain
// Variables:
// skinnum - 0: rope, 1: chain.
void SP_obj_ropechain(edict_t* self)
{
	VectorSet(self->mins, -20.0f, -20.0f, -14.0f);
	VectorSet(self->maxs, 20.0f, 20.0f, 14.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/rope/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	if (self->s.skinnum == 0)
		ObjectInit(self, 80, 100, MAT_WOOD, SOLID_BBOX);
	else
		ObjectInit(self, 160, 100, MAT_METAL, SOLID_BBOX);
}

// QUAKED obj_wheelbarrow (1 .5 0) (-37 -20 -21) (37 20 21) INVULNERABLE x x NOPUSH
// A wheelbarrow.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_wheelbarrow(edict_t* self)
{
	VectorSet(self->mins, -37.0f, -20.0f, -21.0f);
	VectorSet(self->maxs, 37.0f, 20.0f, 21.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/wheelbarrow/tris.fm");

	ObjectInit(self, 60, 100, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_wheelbarrowdamaged (1 .5 0) (-38 -26 -20) (38 26 20) INVULNERABLE
// A wheelbarrow on it's side missing a wheel.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_wheelbarrowdamaged(edict_t* self)
{
	VectorSet(self->mins, -38.0f, -26.0f, -20.0f);
	VectorSet(self->maxs, 38.0f, 26.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/wheelbarrow/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.frame = 1;

	ObjectInit(self, 60, 100, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_urn (1 .5 0) (-8 -8 -27) (8 8 30) INVULNERABLE x x NOPUSH
// An urn.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_urn(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -27.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 30.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/urn/tris.fm");

	ObjectInit(self, 50, 100, MAT_POTTERY, SOLID_BBOX);
}

// QUAKED obj_bigcrystal (1 .5 0) (-35 -35 -50) (35 35 50)
// A big circular crystal which rotates.
// Variables:
// speed - Rate of rotation.
void SP_obj_bigcrystal(edict_t* self)
{
	VectorSet(self->mins, -35.0f, -35.0f, -50.0f);
	VectorSet(self->maxs, 35.0f, 35.0f, 50.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/crystals/bigcrystal/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 350, 200, MAT_GREYSTONE, SOLID_BBOX);

	self->avelocity[YAW] = self->speed;
	self->movetype = PHYSICSTYPE_FLY;
	self->gravity = 0.0f;
}

#pragma endregion

#pragma region ========================== obj_moss1, obj_moss2, obj_moss3, obj_moss4, obj_moss5 ==========================

// QUAKED obj_moss1 (1 .5 0) (-4 -10 -40) (4 10 40)
void SP_obj_moss1(edict_t* self)
{
	VectorSet(self->mins, -1.0f, -10.0f, -40.0f);
	VectorSet(self->maxs, 1.0f, 10.0f, 40.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/moss/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 10, 10, MAT_LEAF, SOLID_NOT);
}

// QUAKED obj_moss2 (1 .5 0) (-4 -9 -40) (4 9 40)
void SP_obj_moss2(edict_t* self)
{
	VectorSet(self->mins, -1.0f, -9.0f, -40.0f);
	VectorSet(self->maxs, 1.0f, 9.0f, 40.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/moss/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.skinnum = 1;

	ObjectInit(self, 10, 10, MAT_LEAF, SOLID_NOT);
}

// QUAKED obj_moss3 (1 .5 0) (-4 -15 -40) (4 15 40) INVULNERABLE
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_moss3(edict_t* self)
{
	VectorSet(self->mins, -1.0f, -15.0f, -40.0f);
	VectorSet(self->maxs, 1.0f, 15.0f, 40.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/moss/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.skinnum = 2;
	self->s.renderfx |= RF_TRANSLUCENT; //TODO: not needed? Makes fx effects and transparent surfaces visible through non-transparent parts.

	ObjectInit(self, 10, 10, MAT_LEAF, SOLID_NOT);
}

// QUAKED obj_moss4 (1 .5 0) (-4 -12 -40) (4 12 40) INVULNERABLE
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_moss4(edict_t* self)
{
	VectorSet(self->mins, -1.0f, -12.0f, -40.0f);
	VectorSet(self->maxs, 1.0f, 12.0f, 40.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/moss/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.skinnum = 3;
	self->s.renderfx |= RF_TRANSLUCENT; //TODO: not needed? Makes fx effects and transparent surfaces visible through non-transparent parts.

	ObjectInit(self, 10, 10, MAT_LEAF, SOLID_NOT);
}

// QUAKED obj_moss5 (1 .5 0) (-4 -10 -40) (4 10 40) INVULNERABLE
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_moss5(edict_t* self)
{
	VectorSet(self->mins, -1.0f, -10.0f, -40.0f);
	VectorSet(self->maxs, 1.0f, 10.0f, 40.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/moss/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.skinnum = 4;
	self->s.renderfx |= RF_TRANSLUCENT; //TODO: not needed? Makes fx effects and transparent surfaces visible through non-transparent parts.

	ObjectInit(self, 10, 10, MAT_LEAF, SOLID_NOT);
}

#pragma endregion

#pragma region ==================== obj_floor_candelabrum, obj_statue_dragonhead, obj_statue_dragon, obj_flagonpole ====================

// QUAKED obj_floor_candelabrum (1 .5 0) (-8 -8 -35) (8 8 35) x ANIMATE x NOPUSH
// A floor candelabrum.
// Spawnflags:
// ANIMATE	- Play flame animation. //mxd. Otherwise, the flames are still there, but not animated...
// NOPUSH	- Can't be moved by player.
void SP_obj_floor_candelabrum(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -35.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 35.0f);

	self->spawnflags |= SF_OBJ_INVULNERABLE; // Can't be destroyed.

	SpawnClientAnim(self, FX_ANIM_CANDELABRUM, NULL);
	ObjectInit(self, 40, 60, MAT_METAL, SOLID_BBOX);
}

// QUAKED obj_statue_dragonhead (1 .5 0) (-76 -28 -46) (76 28 46)
// A statue of a dragon head (very scary!).
void SP_obj_statue_dragonhead(edict_t* self)
{
	VectorSet(self->mins, -76.0f, -28.0f, -46.0f);
	VectorSet(self->maxs, 76.0f, 28.0f, 46.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/dragonhead/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 200, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_statue_dragon (1 .5 0) (-53 -33 -72) (53 33 72)
// A statue of a dragon.
// Variables:
// style - 0: dragon looking left (default); 1: dragon looking right.
void SP_obj_statue_dragon(edict_t* self)
{
	VectorSet(self->mins, -53.0f, -33.0f, -72.0f);
	VectorSet(self->maxs, 53.0f, 33.0f, 72.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/dragon/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = (short)ClampI(self->style, 0, 1); //mxd

	ObjectInit(self, 200, 400, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_flagonpole (1 .5 0) (-8 -8 0) (8 8 60) x ANIMATE
// A flag on a pole.
// Spawnflags:
// ANIMATE	- Play animation.
void SP_obj_flagonpole(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -28.0f, -30.0f);
	VectorSet(self->maxs, 8.0f, 28.0f, 30.0f);

	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	SpawnClientAnim(self, FX_ANIM_FLAGONPOLE, "ambient/bannerflap.wav");
	ObjectInit(self, 40, 200, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_lever1, obj_lever2, obj_lever3 ==========================

void ObjLeverDownThink(edict_t* self) //mxd. Merged lever1downthink(), lever2downthink() and lever3downthink() from original logic.
{
	if (self->s.frame < self->obj_lever_num_frames)
	{
		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void ObjLeverUpThink(edict_t* self) //mxd. Merged lever1upthink(), lever2upthink() and lever3upthink() from original logic.
{
	if (self->s.frame > 0)
	{
		self->s.frame--;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void ObjLeverUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Merged lever1_use(), lever2_use() and lever3_use() from original logic.
{
	if (self->s.frame == 0 || self->s.frame == self->obj_lever_num_frames) //mxd. Rewritten, so G_UseTargets() is only called when not playing animation.
	{
		gi.sound(self, CHAN_BODY, gi.soundindex(va("objects/lever%i.wav", self->obj_lever_index)), 1.0f, ATTN_NORM, 0.0f);
		self->think = (self->s.frame == 0 ? ObjLeverDownThink : ObjLeverUpThink);
		self->nextthink = level.time + FRAMETIME; //BUGFIX, kinda. mxd. Was set outside of frame checks in original logic.

		G_UseTargets(self, activator);
	}
}

// QUAKED obj_lever1 (1 .5 0) (-6 -14 -17) (6 14 17)
// A wooden floor lever that is triggerable.
void SP_obj_lever1(edict_t* self)
{
	VectorSet(self->mins, -6.0f, -14.0f, -17.0f);
	VectorSet(self->maxs,  6.0f,  14.0f,  17.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/levers/lever1/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->obj_lever_index = 1; //mxd
	self->obj_lever_num_frames = 5; //mxd
	self->use = ObjLeverUse;

	ObjectInit(self, 150, 125, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_lever2 (1 .5 0) (-14 -14 -9) (14 14 9)
// A wooden wheel lever that is triggerable.
void SP_obj_lever2(edict_t* self)
{
	VectorSet(self->mins, -14.0f, -14.0f, -9.0f);
	VectorSet(self->maxs,  14.0f,  14.0f,  9.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/levers/lever2/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->obj_lever_index = 2; //mxd
	self->obj_lever_num_frames = 4; //mxd. Model has 7 frames, but frame 0 orientation matches frame 7 orientation, making it hard to distinguish them...
	self->use = ObjLeverUse;

	ObjectInit(self, 150, 125, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_lever3 (1 .5 0) (-4 -6 -16) (4 6 16)
// A lever for the wall... and it's triggerable.
void SP_obj_lever3(edict_t* self)
{
	VectorSet(self->mins, -6.0f, -4.0f, -16.0f);
	VectorSet(self->maxs,  6.0f,  4.0f,  16.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/levers/lever3/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->obj_lever_index = 3; //mxd
	self->obj_lever_num_frames = 5; //mxd
	self->use = ObjLeverUse;

	ObjectInit(self, 150, 125, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_bush1, obj_bush2 ==========================

// QUAKED obj_bush1 (1 .5 0) (-34 -34 -19) (34 34 19)
// A dome-shaped bush.
void SP_obj_bush1(edict_t* self)
{
	// The bounding box in the QUAKED comment is bigger to show the designers the true size of the objects.
	VectorSet(self->mins, -16.0f, -16.0f, -19.0f);
	VectorSet(self->maxs, 16.0f, 16.0f, 19.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/bush1/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 25, 25, MAT_WOOD, SOLID_NOT);

	//mxd. Skip non-functional bush_touch() logic. //TODO: re-implement (would require a new sound, "objects/bush.wav" is not present in .paks)?
}

// QUAKED obj_bush2 (1 .5 0) (-56 -56 -40) (56 56 40)
// A larger dome-shaped bush.
void SP_obj_bush2(edict_t* self)
{
	// The bounding box in the QUAKED comment is bigger to show the designers the true size of the objects.
	VectorSet(self->mins, -16.0f, -16.0f, -40.0f);
	VectorSet(self->maxs, 16.0f, 16.0f, 40.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/bush2/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 25, 50, MAT_WOOD, SOLID_BBOX);

	//mxd. Skip non-functional bush_touch() logic. //TODO: re-implement (would require a new sound, "objects/bush.wav" is not present in .paks)?
}

#pragma endregion

#pragma region ========================== obj_cactus, obj_cactus3, obj_cactus4 ==========================

// Cactus will hurt player.
void ObjCactusTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'cactus_touch' in original logic.
{
	if (other->client != NULL && self->touch_debounce_time <= level.time)
	{
		self->touch_debounce_time = level.time + 5.0f;
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 1, 0, DAMAGE_AVOID_ARMOR, MOD_DIED);
	}
}

// QUAKED obj_cactus (1 .5 0) (-18 -18 -44) (18 18 44) INVULNERABLE
// A cactus. Hurts the player 1 pt every five seconds he is pushes against it.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_cactus(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -18.0f, -44.0f);
	VectorSet(self->maxs, 18.0f, 18.0f, 44.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/cactus/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 50, 125, MAT_NONE, SOLID_BBOX);

	self->touch_debounce_time = level.time;
	self->touch = ObjCactusTouch;
}

// QUAKED obj_cactus3 (1 .5 0) (-14 -14 -32) (14 14 32) INVULNERABLE
// A cactus. Hurts the player 1 pt every five seconds he is pushes against it.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_cactus3(edict_t* self)
{
	VectorSet(self->mins, -14.0f, -14.0f, -32.0f);
	VectorSet(self->maxs, 14.0f, 14.0f, 32.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/cactus3/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 50, 125, MAT_NONE, SOLID_BBOX);

	self->touch_debounce_time = level.time;
	self->touch = ObjCactusTouch;
}

void ObjCactus4CloseThink(edict_t* self) //mxd. Named 'cactus_close' in original logic.
{
	//mxd. Rewritten logic to remove think callback on animation finish.
	if (--self->s.frame >= 0)
		self->nextthink = level.time + FRAMETIME;
	else
		self->think = NULL;
}

void ObjCactus4OpenThink(edict_t* self) //mxd. Named 'cactus_open' in original logic.
{
	//mxd. Rewritten logic to remove think callback on animation finish.
	self->s.frame = min(16, self->s.frame + 4);

	if (self->s.frame < 16)
		self->nextthink = level.time + FRAMETIME;
	else
		self->think = NULL;
}

void ObjCactus4Use(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'cactus4_use' in original logic.
{
	if (self->s.frame == 0 || self->s.frame == 16)
	{
		gi.sound(self, CHAN_BODY, gi.soundindex("objects/lever1.wav"), 1.0f, ATTN_NORM, 0.0f); //TODO: could use a better sound...
		self->think = (self->s.frame == 0 ? ObjCactus4OpenThink : ObjCactus4CloseThink);
		self->nextthink = level.time + FRAMETIME;
	}
}

// QUAKED obj_cactus4 (1 .5 0) (-11 -11 -11) (11 11 11) INVULNERABLE
// A small cactus that open up or closes. Is triggerable.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_cactus4(edict_t* self)
{
	VectorSet(self->mins, -11.0f, -11.0f, -11.0f);
	VectorSet(self->maxs, 11.0f, 11.0f, 11.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/cactus4/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_NONE, SOLID_BBOX);

	self->touch_debounce_time = level.time;
	self->use = ObjCactus4Use;
	self->touch = ObjCactusTouch;
}

#pragma endregion

#pragma region == obj_basket, obj_claybowl, obj_clayjar, obj_gorgonbones, obj_grass, obj_swampflat_top, obj_swampflat_bottom, obj_treestump, obj_jawbone ==

// QUAKED obj_basket (1 .5 0) (-13 -13 -21) (13 13 21) INVULNERABLE x x NOPUSH
// A tall basket with a lid on it.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_basket(edict_t* self)
{
	VectorSet(self->mins, -13.0f, -13.0f, -21.0f);
	VectorSet(self->maxs, 13.0f, 13.0f, 21.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/basket/tris.fm");

	ObjectInit(self, 50, 70, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_claybowl (1 .5 0) (-6 -6 -2) (6 6 2) INVULNERABLE x x NOPUSH
// A bowl made of clay.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_claybowl(edict_t* self)
{
	VectorSet(self->mins, -6.0f, -6.0f, -2.0f);
	VectorSet(self->maxs, 6.0f, 6.0f, 2.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/claybowl/tris.fm");

	ObjectInit(self, 5, 5, MAT_POTTERY, SOLID_BBOX);
}

// QUAKED obj_clayjar (1 .5 0) (-15 -15 -24) (15 15 24) INVULNERABLE x x NOPUSH
// A big honking urn made of clay.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_clayjar(edict_t* self)
{
	VectorSet(self->mins, -15.0f, -15.0f, -24.0f);
	VectorSet(self->maxs, 15.0f, 15.0f, 24.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/clayjar/tris.fm");

	ObjectInit(self, 25, 125, MAT_POTTERY, SOLID_BBOX);
}

// QUAKED obj_gorgonbones (1 .5 0) (-18 -38 -9) (18 38 1) INVULNERABLE
// The bones of a dead gorgon.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_gorgonbones(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -38.0f, -9.0f);
	VectorSet(self->maxs, 18.0f, 38.0f, 1.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/bones/gorgon/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 50, 125, MAT_NONE, SOLID_BBOX);
}

// QUAKED obj_grass (1 .5 0) (-8 -8 -10) (8 8 10) INVULNERABLE
// A clump of grass.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_grass(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -10.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 10.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/grass/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.effects |= EF_CAMERA_NO_CLIP;

	ObjectInit(self, 50, 125, MAT_WOOD, SOLID_NOT); //TODO: should use MAT_LEAF instead?
}

// QUAKED obj_swampflat_top (1 .5 0) (0 -100 -48) (2 100 48)
// A flat poly to be used on the outer edge of swamp levels. Vegetation growing up.
void SP_obj_swampflat_top(edict_t* self)
{
	VectorSet(self->mins, 0.0f, -100.0f, -48.0f);
	VectorSet(self->maxs, 2.0f, 100.0f, 48.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/swampflat/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_swampflat_bottom (1 .5 0) (0 -100 -48) (2 100 48)
// A flat poly to be used on the outer edge of swamp levels. Vegetation hanging down.
void SP_obj_swampflat_bottom(edict_t* self)
{
	VectorSet(self->mins, 0.0f, -100.0f, -48.0f);
	VectorSet(self->maxs, 2.0f, 100.0f, 48.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/swampflat/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.skinnum = 1; //TODO: ignored: model has only skin 0. Should set angles[ROLL] to 180 instead?

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_treestump (1 .5 0) (-18 -18 -16) (18 18 16)
// A short tree stump.
void SP_obj_treestump(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -18.0f, -16.0f);
	VectorSet(self->maxs, 18.0f, 18.0f, 16.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/treestump/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.skinnum = 1; //TODO: ignored: model has only skin 0.

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_jawbone (1 .5 0) (-11 -11 -12) (11 11 12) INVULNERABLE x x NOPUSH
// The jaws of a fish.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_jawbone(edict_t* self)
{
	VectorSet(self->mins, -11.0f, -11.0f, -12.0f);
	VectorSet(self->maxs, 11.0f, 11.0f, 12.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/bones/jaws/tris.fm");
	self->s.effects |= EF_CAMERA_NO_CLIP;
	self->s.skinnum = 1; //TODO: ignored: model has only skin 0.

	ObjectInit(self, 25, 125, MAT_NONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_barrel_metal, obj_barrel_explosive ==========================

// QUAKED obj_barrel_metal (1 .5 0) (-11 -12 -18) (11 12 18) INVULNERABLE x x NOPUSH
// A metal barrel.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_barrel_metal(edict_t* self)
{
	VectorSet(self->mins, -11.0f, -12.0f, -18.0f);
	VectorSet(self->maxs, 11.0f, 12.0f, 18.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/barrel/metal/tris.fm");
	self->s.skinnum = 1; //TODO: ignored: model has only skin 0.

	ObjectInit(self, 75, 125, MAT_METAL, SOLID_BBOX);
}

// QUAKED obj_barrel_explosive (1 .5 0) (-11 -12 -18) (11 12 18) INVULNERABLE x x NOPUSH
// A barrel that explodes and does damage.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_barrel_explosive(edict_t* self) //TODO: explosive barrel doesn't explode...
{
	VectorSet(self->mins, -11.0f, -12.0f, -18.0f);
	VectorSet(self->maxs, 11.0f, 12.0f, 18.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/barrel/normal/tris.fm");
	self->s.skinnum = 1;

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_gascan, obj_pipe1, obj_pipe2, obj_pipewheel ==========================

// QUAKED obj_gascan (1 .5 0) (-8 -9 -13) (8 9 13) INVULNERABLE x x NOPUSH
// A metal gas can.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_gascan(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -9.0f, -13.0f);
	VectorSet(self->maxs, 8.0f, 9.0f, 13.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/barrel/gascan/tris.fm");
	self->s.skinnum = 1; //TODO: ignored: model has only skin 0.

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX); //TODO: should be MAT_METAL?
}

// QUAKED obj_pipe1 (1 .5 0) (-11 -24 -7) (11 24 7) INVULNERABLE x x NOPUSH
// A section of pipe with 90 degree bend in it.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_pipe1(edict_t* self)
{
	VectorSet(self->mins, -11.0f, -24.0f, -7.0f);
	VectorSet(self->maxs, 11.0f, 24.0f, 7.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pipes/pipe1/tris.fm");
	self->s.skinnum = 1; //TODO: ignored: model has only skin 0.

	ObjectInit(self, 50, 125, MAT_WOOD, SOLID_BBOX); //TODO: should be MAT_METAL?
}

// QUAKED obj_pipe2 (1 .5 0) (-6 -25 -4) (6 25 4) INVULNERABLE x x NOPUSH
// A straight section of pipe.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_pipe2(edict_t* self)
{
	VectorSet(self->mins, -6.0f, -25.0f, -4.0f);
	VectorSet(self->maxs, 6.0f, 25.0f, 4.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pipes/pipe2/tris.fm");
	self->s.skinnum = 1; //TODO: ignored: model has only skin 0.

	ObjectInit(self, 50, 125, MAT_WOOD, SOLID_BBOX); //TODO: should be MAT_METAL?
}

// QUAKED obj_pipewheel (1 .5 0) (-14 -14 -12) (14 14 12) INVULNERABLE
// A shutoff valve for pipe.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_pipewheel(edict_t* self)
{
	VectorSet(self->mins, -14.0f, -14.0f, -12.0f);
	VectorSet(self->maxs, 14.0f, 14.0f, 12.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pipes/pipewheel/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH;
	self->s.skinnum = 1; //TODO: ignored: model has only skin 0.

	ObjectInit(self, 50, 125, MAT_WOOD, SOLID_BBOX); //TODO: should be MAT_METAL?
}

#pragma endregion

#pragma region ========================== obj_minecart, obj_minecart2, obj_minecart3 ==========================

// QUAKED obj_minecart (1 .5 0) (-18 -29 -20) (18 29 20) INVULNERABLE x x NOPUSH
// A full mine cart used on the mine levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_minecart(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -29.0f, -20.0f);
	VectorSet(self->maxs, 18.0f, 29.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/carts/mine/tris.fm");

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX); //TODO: should be MAT_METAL?
}

// QUAKED obj_minecart2 (1 .5 0) (-18 -29 -20) (18 29 20) INVULNERABLE x x NOPUSH
// An empty mine cart used on the mine levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_minecart2(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -29, -20.0f);
	VectorSet(self->maxs, 18.0f, 29.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/carts/mine/tris.fm");
	self->s.frame = 20;

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX); //TODO: should be MAT_METAL?
}

// QUAKED obj_minecart3 (1 .5 0) (-18 -29 -15) (18 29 20) INVULNERABLE x x NOPUSH
// A busted mine cart used on the mine levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_minecart3(edict_t* self)
{
	VectorSet(self->mins, -18.0f, -29.0f, -15.0f);
	VectorSet(self->maxs, 18.0f, 29.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/carts/mine/tris.fm");
	self->s.frame = 40;

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX); //TODO: should be MAT_METAL?
}

#pragma endregion

#pragma region ========================== obj_andwallhanging, obj_pick ==========================

// QUAKED obj_andwallhanging (1 .5 0) ( 0 -19 -24) (4 19 24) INVULNERABLE x x NOPUSH
// A circular Andorian wall hanging.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_andwallhanging(edict_t* self)
{
	VectorSet(self->mins, 0.0f, -19.0f, -24.0f);
	VectorSet(self->maxs, 4.0f, 19.0f, 24.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/andwallhang/tris.fm");

	ObjectInit(self, 75, 100, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_pick (1 .5 0) ( -12 -13 -2) (12 13 2) INVULNERABLE x x NOPUSH
// A tool that is a pick.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_pick(edict_t* self)
{
	VectorSet(self->mins, -12.0f, -13.0f, -2.0f);
	VectorSet(self->maxs, 12.0f, 13.0f, 2.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/tools/pick/tris.fm");

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_metalchunk1, obj_metalchunk2, obj_metalchunk3 ==========================

// QUAKED obj_metalchunk1 (1 .5 0) ( -10 -26 -4) (10 26 4) INVULNERABLE x x NOPUSH
// A chunk of twisted metal.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_metalchunk1(edict_t* self)
{
	VectorSet(self->mins, -10.0f, -26.0f, -4.0f);
	VectorSet(self->maxs, 10.0f, 26.0f, 4.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pipes/metalchunks/tris.fm");

	ObjectInit(self, 75, 125, MAT_METAL, SOLID_BBOX);
}

// QUAKED obj_metalchunk2 (1 .5 0) ( -10 -26 -7) (10 26 7) INVULNERABLE x x NOPUSH
// Another chunk of twisted metal.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_metalchunk2(edict_t* self)
{
	VectorSet(self->mins, -10.0f, -26.0f, -7.0f);
	VectorSet(self->maxs, 10.0f, 26.0f, 7.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pipes/metalchunks/tris.fm");
	self->s.frame = 1;

	ObjectInit(self, 75, 125, MAT_METAL, SOLID_BBOX);
}

// QUAKED obj_metalchunk3 (1 .5 0) ( -9 -30 -4) (9 30 4) INVULNERABLE x x NOPUSH
// Another chunk of twisted metal.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_metalchunk3(edict_t* self)
{
	VectorSet(self->mins, -9.0f, -30.0f, -4.0f);
	VectorSet(self->maxs, 9.0f, 30.0f, 4.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pipes/metalchunks/tris.fm");
	self->s.frame = 2;

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX); //TODO: should be MAT_METAL?
}

#pragma endregion

#pragma region ========================== obj_rocks1, obj_rocks2 ==========================

// QUAKED obj_rocks1 (1 .5 0) ( -12 -13 -4) (12 13 4) INVULNERABLE
// A bunch of rocks together. For the MINE levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_rocks1(edict_t* self)
{
	VectorSet(self->mins, -12.0f, -13.0f, -4.0f);
	VectorSet(self->maxs, 12.0f, 13.0f, 4.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/rocks/rock1/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_rocks2 (1 .5 0) ( -9 -30 -4) (9 30 4) INVULNERABLE
// A big rock. For the MINE levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_rocks2(edict_t* self)
{
	VectorSet(self->mins, -34.0f, -40.0f, -19.0f);
	VectorSet(self->maxs, 34.0f, 40.0f, 19.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/rocks/rock2/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_hivepriestessssymbol ==========================

void ObjHivePriestessSymbolThink(edict_t* self) //mxd. Named 'symbolthink' in original logic.
{
	M_DropToFloor(self); // Clears self->think callback!

	if (self->touch_debounce_time >= level.time)
	{
		self->think = ObjHivePriestessSymbolThink;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->nextthink = THINK_NEVER; //mxd. Use define.
	}
}

void ObjHivePriestessSymbolUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'hivepriestesssymbol_use' in original logic.
{
	self->s.modelindex = (byte)gi.modelindex("models/items/puzzles/hiveidol/tris.fm");
	self->solid = SOLID_BBOX;

	// So it can lower alongside lowering platform in hivepriestess.bsp...
	self->velocity[2] = -1.0f;
	self->touch_debounce_time = level.time + 5.0f;

	self->think = ObjHivePriestessSymbolThink;
	self->nextthink = level.time + FRAMETIME;
}

// QUAKED obj_hivepriestessssymbol (1 .5 0) ( -4 -4 -13) (4 4 13) x x x NOPUSH
// The Hive Priestess Symbol was created originally by T'jektaluck back in 14567 AH (After Hive).
// It was stolen by Matt Pinkston in 20054. He was promptly captured and fed Zots until he died from sugar overload.
// Starts off invisible, appears when used. For the HIVE level.
// Spawnflags:
// NOPUSH - Can't be moved by player.
void SP_obj_hivepriestessssymbol(edict_t* self) //TODO: rename to 'SP_obj_hivepriestesssymbol'?..
{
	VectorSet(self->mins, -4.0f, -4.0f, -13.0f);
	VectorSet(self->maxs, 4.0f, 4.0f, 13.0f);

	self->spawnflags |= SF_OBJ_INVULNERABLE; // Can't be destroyed.
	self->use = ObjHivePriestessSymbolUse;

	ObjectInit(self, 75, 125, MAT_GREYSTONE, SOLID_NOT);
}

#pragma endregion

#pragma region ========================== obj_queenthrone, obj_queenchair ==========================

// QUAKED obj_queenthrone (1 .5 0) ( -40 -56 -49) (40 56 49)
// A throne for the queen. For the HIVE levels.
void SP_obj_queenthrone(edict_t* self)
{
	VectorSet(self->mins, -40.0f, -56.0f, -49.0f);
	VectorSet(self->maxs, 40.0f, 56.0f, 49.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chairs/queen/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 75, 125, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_queenchair (1 .5 0) ( -30 -28 -31) (30 28 31) INVULNERABLE
// A chair to go by the queen throne. For the HIVE levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_queenchair(edict_t* self)
{
	VectorSet(self->mins, -30.0f, -28.0f, -31.0f);
	VectorSet(self->maxs, 30.0f, 28.0f, 31.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/chairs/smallchair/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ====================== obj_larvaegg, obj_larvabrokenegg, obj_cocoon, obj_cocoonopen, obj_venusflytrap ======================

// QUAKED obj_larvaegg (1 .5 0) ( -6 -14 -6) (6 14 6) INVULNERABLE
// An egg for the larva. For the HIVE levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_larvaegg(edict_t* self)
{
	VectorSet(self->mins, -6.0f, -14.0f, -6.0f);
	VectorSet(self->maxs, 6.0f, 14.0f, 6.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/eggs/eggs/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_GLASS, SOLID_BBOX); //TODO: should use MAT_INSECT instead?
}

// QUAKED obj_larvabrokenegg (1 .5 0) ( -6 -7 -5) (6 7 5) INVULNERABLE
// A broken egg for the larva. For the HIVE levels
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_larvabrokenegg(edict_t* self)
{
	VectorSet(self->mins, -6.0f, -7.0f, -5.0f);
	VectorSet(self->maxs, 6.0f, 7.0f, 5.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/eggs/brokenegg/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_GLASS, SOLID_BBOX); //TODO: should use MAT_INSECT instead?
}

// QUAKED obj_cocoon (1 .5 0) ( -20 -20 -36) (20 20 36) INVULNERABLE
// A cocoon which hangs from the ceiling. For the HIVE levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_cocoon(edict_t* self)
{
	VectorSet(self->mins, -20.0f, -20.0f, -36.0f);
	VectorSet(self->maxs, 20.0f, 20.0f, 36.0f);

	// Always animate and can't be pushed.
	self->spawnflags |= (SF_OBJ_NOPUSH | SF_OBJ_ANIMATE);
	SpawnClientAnim(self, FX_ANIM_COCOON, NULL);

	ObjectInit(self, 75, 125, MAT_INSECT, SOLID_BBOX);
}

// QUAKED obj_cocoonopen (1 .5 0) ( -20 -20 -34) (20 20 34) INVULNERABLE
// An open cocoon which hangs from the ceiling. For the HIVE levels.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_cocoonopen(edict_t* self)
{
	VectorSet(self->mins, -20.0f, -20.0f, -34.0f);
	VectorSet(self->maxs, 20.0f, 20.0f, 34.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/eggs/cocoon/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->s.frame = 20;

	ObjectInit(self, 75, 125, MAT_INSECT, SOLID_BBOX);
}

// QUAKED obj_venusflytrap (1 .5 0) ( -20 -20 -24) (20 20 24) INVULNERABLE
// The venus flytrap - a viscous beast of a plant. //TODO: model has 20 unused frames of animation. Could be repurposed as idle animation.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_venusflytrap(edict_t* self)
{
	VectorSet(self->mins, -20.0f, -20.0f, -24.0f);
	VectorSet(self->maxs, 20.0f, 20.0f, 24.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/plants/venus/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_LEAF, SOLID_BBOX);

	// Spawn leaves.
	edict_t* leaves = G_Spawn();

	leaves->s.modelindex = (byte)gi.modelindex("models/objects/plants/v-plant/tris.fm");
	VectorCopy(self->s.origin, leaves->s.origin);
	leaves->s.origin[2] -= 16.0f;
	leaves->s.angles[YAW] = self->s.angles[YAW]; //BUGFIX: mxd. Was set after calling BboxYawAndScale() in original logic.
	leaves->movetype = PHYSICSTYPE_NONE;
	leaves->solid = SOLID_NOT;
	BboxYawAndScale(leaves);
	gi.linkentity(leaves);

	self->target_ent = leaves;
}

#pragma endregion

#pragma region ========================== obj_statue_techeckriktomb, obj_statue_techeckrikright, obj_statue_techeckrikleft ==========================

#define SF_NOGEM	16 //mxd

void ObjStatueTecheckrikTombUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'tomb_use' in original logic.
{
	self->s.frame = (short)(!self->s.frame); // Flip between 2 frames of animation. //TODO: would be nice to disable frame interpolation on this (would require implementing new FX_ flag)...
}

// QUAKED obj_statue_techeckriktomb (1 .5 0) ( -41 -11 -14) (41 11 14)
// The statue of the techeckrik laying down.
void SP_obj_statue_techeckriktomb(edict_t* self)
{
	VectorSet(self->mins, -41.0f, -11.0f, -14.0f);
	VectorSet(self->maxs, 41.0f, 11.0f, 14.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/tomb/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = 1; // Staff in hand.
	self->use = ObjStatueTecheckrikTombUse;

	ObjectInit(self, 75, 125, MAT_GREYSTONE, SOLID_BBOX);
}

void ObjStatueTecheckrikUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'tcheckrik_use' in original logic.
{
	//BUGFIX: mxd. Fixed incorrect frame set when triggering obj_statue_techeckrikright multiple times, or even the first time if SF_NOGEM spawnflag was not set.
	if (self->s.frame == 1 || self->s.frame == 3)
		self->s.frame--;
}

// QUAKED obj_statue_techeckrikright (1 .5 0) ( -26 -40 -50) (26 40 50) x x x x NOGEM
// The statue of the techeckrik with his arms raised up to the right.
// Spawnflags:
// NOGEM - No gem in the chest (puzzle).
void SP_obj_statue_techeckrikright(edict_t* self)
{
	VectorSet(self->mins, -26.0f, -40.0f, -50.0f);
	VectorSet(self->maxs, 26.0f, 40.0f, 50.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/tcheckrik/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = (short)((self->spawnflags & SF_NOGEM) ? 3 : 2);
	self->use = ObjStatueTecheckrikUse;

	ObjectInit(self, 75, 125, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_statue_techeckrikleft (1 .5 0) ( -26 -40 -50) (26 40 50) x x x x NOGEM
// The statue of the techeckrik with his arms raised up to the left.
// Spawnflags:
// NOGEM - No gem in the chest (puzzle).
void SP_obj_statue_techeckrikleft(edict_t* self)
{
	VectorSet(self->mins, -26.0f, -40.0f, -50.0f);
	VectorSet(self->maxs, 26.0f, 40.0f, 50.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/tcheckrik/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = (short)((self->spawnflags & SF_NOGEM) ? 1 : 0);
	self->use = ObjStatueTecheckrikUse;

	ObjectInit(self, 75, 125, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_spellbook ==========================

void ObjSpellbookAnimThink(edict_t* self) //mxd. Named 'spellbook_anim' in original logic.
{
	if (--self->s.frame >= 0)
	{
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		gi.sound(self, CHAN_BODY, gi.soundindex("misc/spbkcls.wav"), 1.0f, ATTN_NORM, 0.0f);
		self->think = NULL;
	}
}

void ObjSpellbookUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'spellbook_use' in original logic.
{
	self->think = ObjSpellbookAnimThink;
	self->nextthink = level.time + FRAMETIME;

	G_FreeEdict(self->target_ent); // Remove beam. //TODO: make it fade-out before deleting it?
	gi.sound(self, CHAN_BODY, gi.soundindex("misc/spbook.wav"), 1.0f, ATTN_NORM, 0.0f);
}

// QUAKED obj_spellbook (1 .5 0) ( -14 -14 -35) (14 14 40)
// A spell book that closes when triggered.
void SP_obj_spellbook(edict_t* self)
{
	VectorSet(self->mins, -14.0f, -14.0f, -35.0f);
	VectorSet(self->maxs, 14.0f, 14.0f, 40.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/spellbook/book/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = 20; // Book is open.
	self->use = ObjSpellbookUse;

	ObjectInit(self, 75, 125, MAT_NONE, SOLID_BBOX);

	// Spawn beam.
	edict_t* beam = G_Spawn(); //TODO: add RF_TRANS_ADD renderfx flag?

	beam->s.modelindex = (byte)gi.modelindex("models/objects/spellbook/beam/tris.fm"); //TODO: model has weird 10-frames animation. 
	VectorCopy(self->s.origin, beam->s.origin);
	beam->s.origin[2] += 2.0f;
	beam->s.angles[YAW] = self->s.angles[YAW]; //BUGFIX: mxd. Set after calling BboxYawAndScale() in original logic.
	beam->movetype = PHYSICSTYPE_NONE;
	beam->solid = SOLID_NOT;
	BboxYawAndScale(beam);
	gi.linkentity(beam);

	self->target_ent = beam;
}

#pragma endregion

#pragma region ========================== obj_skullpole, obj_pot1, obj_pot2, obj_bottle1, obj_jug1 ==========================

// QUAKED obj_skullpole (1 .5 0) ( -10 -10 -47) (10 10 47)
// A pole with skulls on it.
void SP_obj_skullpole(edict_t* self)
{
	VectorSet(self->mins, -10.0f, -10.0f, -47.0f);
	VectorSet(self->maxs, 10.0f, 10.0f, 47.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/flags/totempole/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 75, 125, MAT_POTTERY, SOLID_BBOX);
}

// QUAKED obj_pot1 (1 .5 0) ( -3 -8 -8) (3 8 8) INVULNERABLE
// A cooking pot which hangs from a wall.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_pot1(edict_t* self)
{
	VectorSet(self->mins, -3.0f, -8.0f, -8.0f);
	VectorSet(self->maxs, 3.0f, 8.0f, 8.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/pot/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_POTTERY, SOLID_BBOX);
}

// QUAKED obj_pot2 (1 .5 0) ( -7 -7 -3) (7 7 3) INVULNERABLE x x NOPUSH
// A cooking pot which lays flat on a table.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_pot2(edict_t* self)
{
	VectorSet(self->mins, -7.0f, -7.0f, -3.0f);
	VectorSet(self->maxs, 7.0f, 7.0f, 3.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/pot2/tris.fm");

	ObjectInit(self, 75, 125, MAT_POTTERY, SOLID_BBOX);
}

// QUAKED obj_bottle1 (1 .5 0) ( -3 -3 -7) (3 3 7) INVULNERABLE x x NOPUSH
// A bottle that looks like the one Jeannie came out of in "I Dream of Jeannie".
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_bottle1(edict_t* self)
{
	VectorSet(self->mins, -3.0f, -3.0f, -7.0f);
	VectorSet(self->maxs, 3.0f, 3.0f, 7.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/jars/bottle/tris.fm");

	ObjectInit(self, 75, 125, MAT_POTTERY, SOLID_BBOX);
}

// QUAKED obj_jug1 (1 .5 0) ( -6 -6 -6) (6 6 6) INVULNERABLE x x NOPUSH
// A wine jug.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_jug1(edict_t* self)
{
	VectorSet(self->mins, -6.0f, -6.0f, -6.0f);
	VectorSet(self->maxs, 6.0f, 6.0f, 6.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/jars/jug/tris.fm");

	ObjectInit(self, 75, 125, MAT_POTTERY, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_torture_table, obj_torture_wallring ==========================

// QUAKED obj_torture_table (1 .5 0) ( -46 -14 -14) (46 14 14)
// A table useful for wringing confessions from your broken and pitiful enemies.
// style - The frame of animation for model (0: Table is down, 1: Table is in upright position).
void SP_obj_torture_table(edict_t* self)
{
	VectorSet(self->mins, -46.0f, -14.0f, -14.0f);
	VectorSet(self->maxs, 46.0f, 14.0f, 14.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/table/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = (short)ClampI(self->style, 0, 1); //mxd

	ObjectInit(self, 75, 125, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_torture_wallring (1 .5 0) ( -2 -4 -6) (2 4 6) INVULNERABLE
// A ring that hangs from a wall.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_torture_wallring(edict_t* self)
{
	VectorSet(self->mins, -2.0f, -4.0f, -6.0f);
	VectorSet(self->maxs, 2.0f, 4.0f, 6.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/wallring/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 75, 125, MAT_METAL, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_statue_tchecktrik_bust ==========================

void ObjStatueTchecktrikBustUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'statue_tchecktrik_bust_use' in original logic.
{
	self->s.frame = (short)(!self->s.frame); // Flip between 2 frames of animation. //TODO: would be nice to disable frame interpolation on this (would require implementing new FX_ flag)...
}

// QUAKED obj_statue_tchecktrik_bust (1 .5 0) (-8 -12 -15) (8 12 15)
// A bust of a tchecktrik. When used, a necklace appears aroundit's neck.
// style:
//		0 - No necklace until used.
//		1 - Necklace until used.
void SP_obj_statue_tchecktrik_bust(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -12.0f, -15.0f);
	VectorSet(self->maxs, 8.0f, 12.0f, 15.0f);

	self->s.modelindex = (byte)gi.modelindex("models/items/puzzles/tchecktrikbust/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->s.frame = (short)(self->style == 0 ? 1 : 0);
	self->use = ObjStatueTchecktrikBustUse;

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_statue_sithraguard ==========================

void ObjStatueSsithraGuardThink(edict_t* self) //mxd. Named 'statue_sithraguard_think' in original logic.
{
	if (self->s.frame < 20)
	{
		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
	}
}

void ObjStatueSsithraGuardUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'statue_sithraguard_use' in original logic.
{
	if (self->s.frame > 0) //mxd. Add retrigger safeguard.
		return;

	self->think = ObjStatueSsithraGuardThink;
	self->nextthink = level.time + FRAMETIME;

	gi.sound(self, CHAN_BODY, gi.soundindex("items/statuearm.wav"), 1.0f, ATTN_NORM, 0.0f);

	// Spawn shield.
	edict_t* shield = G_Spawn();

	VectorCopy(self->s.origin, shield->s.origin);
	VectorCopy(self->s.angles, shield->s.angles);

	shield->s.modelindex = (byte)gi.modelindex("models/objects/statue/sithshield/tris.fm");
	shield->s.scale = self->s.scale;
	shield->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	shield->movetype = PHYSICSTYPE_NONE;
	shield->solid = SOLID_NOT;

	BboxYawAndScale(shield);
	gi.linkentity(shield);
}

// QUAKED obj_statue_sithraguard (1 .5 0) (-22 -20 -57) (22 20 57)
// A statue of a ssithra guard with spear extended. When used the guard pulls his arm back.
void SP_obj_statue_sithraguard(edict_t* self) //TODO: rename to 'SP_obj_statue_ssithraguard'?
{
	VectorSet(self->mins, -22.0f, -20.0f, -57.0f);
	VectorSet(self->maxs, 22.0f, 20.0f, 57.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/sithraguard/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->use = ObjStatueSsithraGuardUse;

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_torture_ironmaiden ==========================

//mxd
static vec3_t ironmaiden_opened_mins = { -28.0f, -48.0f, -49.0f };
static vec3_t ironmaiden_opened_maxs = {  28.0f,  48.0f,  49.0f };
static vec3_t ironmaiden_closed_mins = { -18.0f, -18.0f, -49.0f };
static vec3_t ironmaiden_closed_maxs = {  18.0f,  18.0f,  49.0f };

void ObjTortureIronmaidenOpen(edict_t* self) //mxd. Named 'ironmaiden_open' in original logic.
{
	//mxd. Switch bbox size. Not done in original logic.
	if (self->s.frame == 10)
	{
		VectorCopy(ironmaiden_opened_mins, self->mins);
		VectorCopy(ironmaiden_opened_maxs, self->maxs);
		gi.linkentity(self);

		//mxd. Originally called on frame 9. Originally used "items/ironmaiden.wav" sound.
		gi.sound(self, CHAN_BODY, gi.soundindex("items/statuearm.wav"), 1.0f, ATTN_NORM, 0.0f);
	}

	if (self->s.frame > 0)
	{
		self->s.frame--;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = NULL;
		self->touch = ObjTortureIronmaidenTouch;
	}
}

void ObjTortureIronmaidenClose(edict_t* self) //mxd. Named 'ironmaiden_close' in original logic.
{
	//mxd. Switch bbox size. Done in ObjTortureIronmaidenUse() in original logic.
	if (self->s.frame == 0)
	{
		VectorCopy(ironmaiden_closed_mins, self->mins);
		VectorCopy(ironmaiden_closed_maxs, self->maxs);
		gi.linkentity(self); //mxd. Not called in original logic.

		//mxd. Originally called in ObjTortureIronmaidenUse().
		gi.sound(self, CHAN_BODY, gi.soundindex("items/ironmaiden.wav"), 1.0f, ATTN_NORM, 0.0f);
	}

	if (self->s.frame < 10)
	{
		self->s.frame = min(10, self->s.frame + 5); //mxd. Close 5x faster.
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->think = ObjTortureIronmaidenOpen;
		self->nextthink = level.time + FRAMETIME * 30.0f; // Open up in 3 seconds.
	}
}

void ObjTortureIronmaidenUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'ironmaiden_use' in original logic.
{
	self->touch = NULL;
	self->think = ObjTortureIronmaidenClose;
	self->nextthink = level.time + FRAMETIME;
}

// Called only when in opened state.
void ObjTortureIronmaidenTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'ironmaiden_touch' in original logic.
{
	if (other->client == NULL || self->touch_debounce_time > level.time) //TODO: don't skip for non-players (remove other->client check)?
		return;

	self->touch_debounce_time = level.time + 3.0f; //mxd. +5.0 in original logic. Changed to match delay in ObjTortureIronmaidenClose().

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);

	vec3_t source;
	VectorMA(self->s.origin, 48.0f, forward, source); //mxd. scale:44 in original logic.

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, source, self, MASK_PLAYERSOLID, &trace);

	if (trace.ent == other)
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 5, 0, DAMAGE_AVOID_ARMOR, MOD_DIED);

	ObjTortureIronmaidenUse(self, NULL, NULL);
}

// QUAKED obj_torture_ironmaiden (1 .5 0) (-18 -18 -49) (18 18 49)
// An iron maiden that closes when used.
void SP_obj_torture_ironmaiden(edict_t* self)
{
	VectorCopy(ironmaiden_opened_mins, self->mins);
	VectorCopy(ironmaiden_opened_maxs, self->maxs);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/ironmaiden/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.
	self->use = ObjTortureIronmaidenUse;
	self->touch = ObjTortureIronmaidenTouch;

	ObjectInit(self, 250, 200, MAT_METAL, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_torture_rack, obj_torture_bed, obj_statue_saraphbust ==========================

// QUAKED obj_torture_rack (1 .5 0) (-22 -46 -19) (22 46 19)
// A torture rack.
void SP_obj_torture_rack(edict_t* self)
{
	VectorSet(self->mins, -22.0f, -46.0f, -19.0f);
	VectorSet(self->maxs, 22.0f, 46.0f, 19.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/rack/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 250, 200, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_torture_bed (1 .5 0) (-21 -43 -94) (21 43 94)
// A bed of spikes that falls from the ceiling.
void SP_obj_torture_bed(edict_t* self)
{
	VectorSet(self->mins, -21.0f, -43.0f, -94.0f);
	VectorSet(self->maxs, 21.0f, 43.0f, 94.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/bed/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 250, 200, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_statue_saraphbust (1 .5 0) (-10 -20 -24) (10 20 24) INVULNERABLE
// A bust of a saraph.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_statue_saraphbust(edict_t* self)
{
	VectorSet(self->mins, -10.0f, -20.0f, -24.0f);
	VectorSet(self->maxs, 10.0f, 20.0f, 24.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/saraphbust/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_biotank ==========================

void ObjBiotankContentsAnimThink(edict_t* self) //mxd. Named 'fish_anim' in original logic.
{
#define FISHBOB_HEIGHT	0.1f
#define FISHBOB_SPEED	ANGLE_10

	M_ChangeYaw(self);

	self->s.origin[2] += cosf(self->moveinfo.current_speed) * FISHBOB_HEIGHT;
	self->moveinfo.current_speed += FISHBOB_SPEED;
	self->s.frame = (short)((self->s.frame < self->count) ? self->s.frame + 1 : 0);

	self->nextthink = level.time + FRAMETIME;
}

void ObjBiotankTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'biotank_touch' in original logic.
{
	self->target_ent->ideal_yaw = anglemod(other->s.angles[YAW] + 180.0f);
}

static void ObjBiotankAddGlass(const edict_t* self, const float scale_forward, const float scale_right, const float yaw_offset) //mxd. Added to reduce code duplication.
{
	edict_t* glass = G_Spawn();

	glass->s.modelindex = (byte)gi.modelindex("models/objects/labs/bioglass2/tris.fm");

	VectorSet(glass->mins, -1.0f, -1.0f, -1.0f);
	VectorSet(glass->maxs, 1.0f, 1.0f, 1.0f);

	VectorCopy(self->s.origin, glass->s.origin);

	vec3_t forward;
	vec3_t right;
	AngleVectors(self->s.angles, forward, right, NULL);

	if (scale_forward != 0.0f)
		VectorMA(glass->s.origin, self->s.scale * scale_forward, forward, glass->s.origin);

	if (scale_right != 0.0f)
		VectorMA(glass->s.origin, self->s.scale * scale_right, right, glass->s.origin);

	VectorCopy(self->s.angles, glass->s.angles);
	glass->s.scale = self->s.scale;
	glass->s.angles[YAW] += yaw_offset;
	glass->s.renderfx |= RF_TRANSLUCENT;
	glass->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(glass, 250, 200, MAT_GLASS, SOLID_NOT);
}

// QUAKED obj_biotank (1 .5 0) (-20 -33 -52) (20 33 52)
// A biotank.
// Variables:
// style - What's in the biotank:
//		0 - Empty.
//		1 - ET's head.
//		2 - Hairless Critter.
//		3 - Three fishes.
//		4 - Wasp.
void SP_obj_biotank(edict_t* self)
{
	VectorSet(self->mins, -21.0f, -34.0f, -52.0f);
	VectorSet(self->maxs, 21.0f, 34.0f, 52.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/labs/biotank/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);

	// Spawn biotank glasses.
	ObjBiotankAddGlass(self, 0.0f, -8.0f, 90.0f); //mxd
	ObjBiotankAddGlass(self, 0.0f, 26.0f, 270.0f); //mxd
	ObjBiotankAddGlass(self, -17.0f, 9.0f, 0.0f); //mxd
	ObjBiotankAddGlass(self, 17.0f, 9.0f, 180.0f); //mxd

	// Spawn biotank contents.
	if (self->style == 0)
		return;

	edict_t* fish = G_Spawn();

	VectorCopy(self->s.origin, fish->s.origin);
	fish->s.origin[2] -= 8.0f;
	fish->movetype = PHYSICSTYPE_FLY;
	fish->solid = SOLID_NOT;
	fish->gravity = 0.0f;

	fish->yaw_speed = 4.0f;
	fish->ideal_yaw = self->s.angles[YAW];

	vec3_t right;
	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorMA(fish->s.origin, 9.0f, right, fish->s.origin);

	if (self->style == 1)
	{
		fish->s.modelindex = (byte)gi.modelindex("models/objects/labs/labfish/tris.fm");
		fish->count = 30;
		self->s.sound = (byte)gi.soundindex("objects/slowbubbles.wav");
	}
	else if (self->style == 2)
	{
		fish->s.modelindex = (byte)gi.modelindex("models/objects/labs/labfish2/tris.fm");
		fish->count = 60;
		self->s.sound = (byte)gi.soundindex("objects/slowbubbles.wav");
	}
	else if (self->style == 3)
	{
		fish->s.modelindex = (byte)gi.modelindex("models/objects/labs/labfish3/tris.fm");
		fish->count = 100;
		self->s.sound = (byte)gi.soundindex("objects/fastbubbles.wav");
	}
	else
	{
		fish->s.modelindex = (byte)gi.modelindex("models/monsters/bee/tris.fm");
		fish->count = 60;
		fish->s.scale = 2.0f;
		self->s.sound = (byte)gi.soundindex("objects/fastbubbles.wav");
	}

	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;

	BboxYawAndScale(fish);
	fish->s.angles[YAW] = self->s.angles[YAW];

	gi.linkentity(fish);

	fish->think = ObjBiotankContentsAnimThink;
	fish->nextthink = level.time + FRAMETIME;

	self->target_ent = fish;
	self->touch = ObjBiotankTouch;
}

#pragma endregion

#pragma region ========================== obj_tapper, obj_wallringplaque, obj_hangingdude ==========================

// QUAKED obj_tapper (1 .5 0) (-2 -5 -2) (2 5 2) INVULNERABLE
// A tapper for a keg.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_tapper(edict_t* self)
{
	VectorSet(self->mins, -2.0f, -5.0f, -2.0f);
	VectorSet(self->maxs, 2.0f, 5.0f, 2.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/tapper/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_NOT);
}

// QUAKED obj_wallringplaque (1 .5 0) (-3 -20 -55) (3 20 55)
// An iron plaque with rings hanging from it. Great for hanging half corpses from.
void SP_obj_wallringplaque(edict_t* self)
{
	VectorSet(self->mins, -3.0f, -20.0f, -55.0f);
	VectorSet(self->maxs, 3.0f, 20.0f, 55.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/plaque/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_hangingdude (1 .5 0) (-3 -20 -55) (3 20 55) INVULNERABLE
// Half a dude hanging from a wall ring plaque.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_hangingdude(edict_t* self)
{
	VectorSet(self->mins, -3.0f, -20.0f, -55.0f);
	VectorSet(self->maxs, 3.0f, 20.0f, 55.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/guy1/tris.fm");
	self->movetype = PHYSICSTYPE_NONE;
	self->solid = SOLID_BBOX;
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.
	self->targetname = "guy1"; //TODO: why hardcoded targetname?

	BboxYawAndScale(self);
	gi.linkentity(self);

	// Spawn plaque.
	edict_t* plaque = G_Spawn();

	VectorSet(plaque->mins, -3.0f, -20.0f, -55.0f);
	VectorSet(plaque->maxs, 3.0f, 20.0f, 55.0f);

	VectorCopy(self->s.origin, plaque->s.origin);
	VectorCopy(self->s.angles, plaque->s.angles);

	plaque->s.modelindex = (byte)gi.modelindex("models/objects/torture/plaque/tris.fm");
	plaque->movetype = PHYSICSTYPE_NONE;
	plaque->solid = SOLID_BBOX;
	plaque->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	BboxYawAndScale(plaque);
	gi.linkentity(plaque);
}

#pragma endregion

#pragma region ========================== obj_frypan, obj_eggpan, obj_nest, obj_choppeddude ==========================

// QUAKED obj_frypan (1 .5 0) (-1 -3 -10) (1 3 10) INVULNERABLE
// A pan which is hanging on a nail.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_frypan(edict_t* self)
{
	VectorSet(self->mins, -1.0f, -3.0f, -10.0f);
	VectorSet(self->maxs, 1.0f, 3.0f, 10.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/frypan/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_eggpan (1 .5 0) (-4 -10 -1) (4 10 1) INVULNERABLE
// A pan which is laying flat like it was on a stove.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_eggpan(edict_t* self)
{
	VectorSet(self->mins, -4.0f, -10.0f, -1.0f);
	VectorSet(self->maxs, 4.0f, 10.0f, 1.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/pots/eggpan/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_nest (1 .5 0) (-25 -25 -4) (25 25 4) INVULNERABLE
// A big bird nest.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_nest(edict_t* self)
{
	VectorSet(self->mins, -25.0f, -25.0f, -4.0f);
	VectorSet(self->maxs, 25.0f, 25.0f, 4.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/nest/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 250, 200, MAT_GREYSTONE, SOLID_BBOX);
}

// QUAKED obj_choppeddude (1 .5 0) (-15 -40 -8) (15 40 8) INVULNERABLE
// A guy who's all chopped up and laying on a table.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_choppeddude(edict_t* self)
{
	VectorSet(self->mins, -15.0f, -40.0f, -8.0f);
	VectorSet(self->maxs, 15.0f, 40.0f, 8.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/guy2/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 250, 200, MAT_FLESH, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_lab_parts_container, obj_eyeball_jar, obj_lab_tray ==========================

// QUAKED obj_lab_parts_container (1 .5 0) (-8 -8 -11) (8 8 11) INVULNERABLE x x NOPUSH
// A container of moving body parts.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_lab_parts_container(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -11.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 11.0f);

	self->spawnflags |= SF_OBJ_ANIMATE;

	SpawnClientAnim(self, FX_ANIM_LABPARTSCONTAINER, NULL);
	ObjectInit(self, 40, 200, MAT_GLASS, SOLID_BBOX);
}

// QUAKED obj_eyeball_jar (1 .5 0) (-13 -13 -18) (13 13 18) INVULNERABLE
// A jar full of eyeballs.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_eyeball_jar(edict_t* self)
{
	VectorSet(self->mins, -13.0f, -13.0f, -18.0f);
	VectorSet(self->maxs, 13.0f, 13.0f, 18.0f);

	self->spawnflags |= (SF_OBJ_NOPUSH | SF_OBJ_ANIMATE);

	SpawnClientAnim(self, FX_ANIM_EYEBALLJAR, NULL);
	ObjectInit(self, 50, 200, MAT_GLASS, SOLID_BBOX);
}

// QUAKED obj_lab_tray (1 .5 0) (-8 -8 -5) (8 8 5) INVULNERABLE
// A tray with a beating heart and some tools on it.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_lab_tray(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -5.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 5.0f);

	self->spawnflags |= (SF_OBJ_NOPUSH | SF_OBJ_ANIMATE);

	SpawnClientAnim(self, FX_ANIM_LABTRAY, NULL);
	ObjectInit(self, 40, 200, MAT_FLESH, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_hanging_ogle ==========================

void ObjHangingOgleMoanThink(edict_t* self) //mxd. Named 'ogle_moan' in original logic.
{
	gi.sound(self, CHAN_VOICE, gi.soundindex(va("monsters/ogle/oglemoan%i.wav", irand(1, 2))), 1.0f, ATTN_IDLE, 0.0f);
	self->nextthink = level.time + flrand(3.0f, 10.0f); //mxd. irand() in original logic.
}

// QUAKED obj_hanging_ogle (1 .5 0) (-8 -16 -34) (8 16 34) INVULNERABLE
// Poor little hanging ogle.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_hanging_ogle(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -16.0f, -34.0f);
	VectorSet(self->maxs, 8.0f, 16.0f, 34.0f);

	self->spawnflags |= (SF_OBJ_NOPUSH | SF_OBJ_ANIMATE);

	SpawnClientAnim(self, FX_ANIM_HANGING_OGLE, NULL);
	ObjectInit(self, 100, 200, MAT_FLESH, SOLID_BBOX);

	self->think = ObjHangingOgleMoanThink;
	self->nextthink = level.time + flrand(3.0f, 10.0f); //mxd. irand() in original logic.

	// Spawn plaque.
	edict_t* plaque = G_Spawn();

	VectorCopy(self->s.origin, plaque->s.origin);
	plaque->s.origin[2] += 27.0f;

	vec3_t forward;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(plaque->s.origin, -9.0f, forward, plaque->s.origin);

	VectorCopy(self->s.angles, plaque->s.angles);

	plaque->s.modelindex = (byte)gi.modelindex("models/objects/torture/plaque2/tris.fm");
	plaque->movetype = PHYSICSTYPE_NONE;
	plaque->solid = SOLID_NOT;
	plaque->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	BboxYawAndScale(plaque);
	gi.linkentity(plaque);

	self->target_ent = plaque; // Set, so plaque is removed when ogle is killed. //TODO: don't remove? 
}

#pragma endregion

#pragma region ========================== obj_ring_plaque2, obj_statue_sariph, obj_pushcart ==========================

// QUAKED obj_ring_plaque2 (1 .5 0) (-2 -24 -20) (2 24 20)
// More rings mounted into a wall plate.
void SP_obj_ring_plaque2(edict_t* self)
{
	VectorSet(self->mins, -2.0f, -24.0f, -20.0f);
	VectorSet(self->maxs, 2.0f, 24.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/torture/plaque2/tris.fm");
	self->spawnflags |= (SF_OBJ_INVULNERABLE | SF_OBJ_NOPUSH); // Can't be destroyed or pushed.

	ObjectInit(self, 250, 200, MAT_FLESH, SOLID_BBOX);
}

// QUAKED obj_statue_sariph (1 .5 0) (-13 -16 -41) (13 16 41) INVULNERABLE
// Big statue in armor carrying an axe.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
void SP_obj_statue_sariph(edict_t* self)
{
	VectorSet(self->mins, -13.0f, -16.0f, -41.0f);
	VectorSet(self->maxs, 13.0f, 16.0f, 41.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/statue/sariph/tris.fm");
	self->spawnflags |= SF_OBJ_NOPUSH; // Can't be pushed.

	ObjectInit(self, 250, 200, MAT_STONE, SOLID_BBOX);
}

// QUAKED obj_pushcart (1 .5 0) (-13 -16 -41) (13 16 41) INVULNERABLE x x NOPUSH
// A push cart for the ogles to push around.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_pushcart(edict_t* self)
{
	VectorSet(self->mins, -14.0f, -30.0f, -17.0f);
	VectorSet(self->maxs, 14.0f, 30.0f, 17.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/carts/pushcart/tris.fm");

	ObjectInit(self, 250, 200, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_bookopen, obj_bookclosed ==========================

// QUAKED obj_bookopen (1 .5 0) (-8 -16 -2) (8 16 2) INVULNERABLE x x NOPUSH
// An open book.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_bookopen(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -16.0f, -2.0f);
	VectorSet(self->maxs, 8.0f, 16.0f, 2.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/books/bookopen/tris.fm");

	ObjectInit(self, 250, 200, MAT_WOOD, SOLID_BBOX);
}

// QUAKED obj_bookclosed (1 .5 0) (-8 -8 -2) (8 8 2) INVULNERABLE x x NOPUSH
// A closed book standing up.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_bookclosed(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -2.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 2.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/books/bookclosed/tris.fm");

	ObjectInit(self, 250, 200, MAT_WOOD, SOLID_BBOX);
}

#pragma endregion

#pragma region ========================== obj_web ==========================

// QUAKED obj_web (1 .5 0) (-2 -18 -20) (2 18 20) INVULNERABLE x x NOPUSH
// A cobweb.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_web(edict_t* self) //TODO: has 20 unused frames of idle animation.
{
	VectorSet(self->mins, -2.0f, -18.0f, -20.0f);
	VectorSet(self->maxs, 2.0f, 18.0f, 20.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/web/web/tris.fm");
	self->s.renderfx |= RF_TRANSLUCENT;

	ObjectInit(self, 250, 200, MAT_WOOD, SOLID_NOT);
}

#pragma endregion

#pragma region ========================== obj_larva ==========================

#define LARVA_FAST_TWITCH 16

void ObjLarvaAnimThink(edict_t* self) //mxd. Named 'larva_anim' in original logic.
{
	if (self->s.frame < self->count)
		self->s.frame += (short)((self->spawnflags & LARVA_FAST_TWITCH) ? 2 : 1); //mxd. Some squirm faster.
	else
		self->s.frame = 0;

	self->nextthink = level.time + FRAMETIME;
}

void ObjLarvaTouch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf) //mxd. Named 'larva_touch' in original logic.
{
	if (other->client != NULL)
		T_Damage(self, other, other, vec3_origin, self->s.origin, vec3_origin, 10, 0, DAMAGE_AVOID_ARMOR, MOD_DIED);
}

// QUAKED obj_larva (1 .5 0) (-8 -8 -2) (8 8 2) INVULNERABLE x x NOPUSH
// A squirming larva.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_larva(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -12.0f, -2.0f);
	VectorSet(self->maxs, 8.0f, 12.0f, 2.0f);

	self->s.modelindex = (byte)gi.modelindex("models/objects/eggs/larva/tris.fm");

	ObjectInit(self, 2, 100, MAT_INSECT, SOLID_BBOX);

	self->movetype = PHYSICSTYPE_STEP;
	self->count = 19;

	self->think = ObjLarvaAnimThink;
	self->nextthink = level.time + FRAMETIME * flrand(2.0f, 7.0f); // Don't all start at the same time.

	self->s.sound = (byte)gi.soundindex("misc/larva.wav");
	self->s.sound_data = (255 & ENT_VOL_MASK) | ATTN_STATIC;

	self->svflags = SVF_ALLOW_AUTO_TARGET;

	if (irand(0, 1) == 1)
		self->spawnflags |= LARVA_FAST_TWITCH;

	self->touch = ObjLarvaTouch;
}

#pragma endregion

#pragma region ========================== obj_bloodsplat ==========================

// QUAKED obj_bloodsplat (1 .5 0) (-8 -8 -2) (8 8 2) INVULNERABLE x x NOPUSH
// A red blood splat.
// Spawnflags:
// INVULNERABLE	- It can't be hurt.
// NOPUSH		- Can't be moved by player.
void SP_obj_bloodsplat(edict_t* self)
{
	VectorSet(self->mins, -8.0f, -8.0f, -2.0f);
	VectorSet(self->maxs, 8.0f, 8.0f, 2.0f);

	self->s.modelindex = (byte)gi.modelindex("sprites/fx/bsplat.sp2");
	self->flags |= (RF_FIXED | RF_ALPHA_TEXTURE);

	ObjectInit(self, 2, 100, MAT_FLESH, SOLID_NOT);
}

#pragma endregion
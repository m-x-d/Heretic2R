//
// g_debris.c -- mxd. Part of g_misc.c in original logic.
//
// Copyright 1998 Raven Software
//

#include "g_debris.h" //mxd
#include "g_DefaultMessageHandler.h"
#include "g_monster.h"
#include "m_harpy.h" //mxd
#include "g_playstats.h"
#include "p_client.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Vector.h"
#include "g_local.h"

#pragma region ========================== Spawn debris logic ==========================

static void SpawnDebris(edict_t* self, float size, const vec3_t origin)
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
	int debris_size = ClampI((int)size, 1, 255);

	vec3_t half_size;
	VectorScale(self->size, 0.5f, half_size);
	const byte b_mag = (byte)(Clamp(VectorLength(half_size), 1.0f, 255.0f));

	int fx_flags = 0;
	if (self->fire_damage_time > level.time || (self->svflags & SVF_ONFIRE))
		fx_flags |= CEF_FLAG6;

	if (self->materialtype == MAT_FLESH || self->materialtype == MAT_INSECT)
	{
		if (BLOOD_LEVEL == VIOLENCE_NONE) //mxd. Original logic does blood_level NULL check before using it. //mxd. 'BLOOD_LEVEL < VIOLENCE_NORMAL' in original logic.
		{
			debris_size = max(1, debris_size / 10); //TODO: check this. Do we really need to divide by 10 AGAIN?
			gi.CreateEffect(NULL, FX_DEBRIS, fx_flags, origin, "bbdb", (byte)debris_size, MAT_STONE, half_size, b_mag);
		}
		else
		{
			if (BLOOD_LEVEL > VIOLENCE_BLOOD) //mxd. 'else if (BLOOD_LEVEL > VIOLENCE_NORMAL)' in original logic (doesn't make much sense. Max violence configurable via menus is 3).
				debris_size = min(255, debris_size * BLOOD_LEVEL);  //mxd. 'BLOOD_LEVEL - VIOLENCE_NORMAL' in original logic.

			if (self->materialtype == MAT_INSECT)
				fx_flags |= CEF_FLAG8;

			if (Q_stricmp(self->classname, "monster_tcheckrik_male") == 0) //mxd. stricmp -> Q_stricmp
				fx_flags |= CEF_FLAG7; // Use male insect skin on chunks.

			gi.CreateEffect(NULL, FX_FLESH_DEBRIS, fx_flags, origin, "bdb", (byte)debris_size, half_size, b_mag);
		}
	}
	else
	{
		if (self->s.renderfx & RF_REFLECTION)
			fx_flags |= CEF_FLAG8;

		gi.CreateEffect(NULL, FX_DEBRIS, fx_flags, origin, "bbdb", (byte)debris_size, (byte)self->materialtype, half_size, b_mag);
	}

	// Play different debris sound.
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
		self->nextthink = level.time + FRAMETIME; //mxd. Use define.

		return;
	}

	// Set my message handler to the special message handler for dead entities.
	self->msgHandler = DeadMsgHandler;

	// What the hell is this??? //TODO: explosion logic for func_train with wait:-3 (see FuncTrainWait())? Then why spawnflags check?..
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
	self->dead_state = DEAD_DEAD;

	G_SetToFree(self);
}

void SprayDebris(const edict_t* self, const vec3_t spot, int num_chunks) //mxd. 'byte num_chunks' in original logic. Removed unused 'damage' arg.
{
	byte b_mat = (byte)self->materialtype;
	const byte b_mag = (byte)(Clamp(VectorLength(self->mins), 1.0f, 255.0f));

	if (b_mat == MAT_FLESH || b_mat == MAT_INSECT)
	{
		if (BLOOD_LEVEL == VIOLENCE_NONE) //mxd. Original logic does blood_level NULL check before using it. //mxd. 'BLOOD_LEVEL < VIOLENCE_NORMAL' in original logic.
			b_mat = MAT_STONE;
		else if (BLOOD_LEVEL > VIOLENCE_BLOOD) //mxd. 'else if (BLOOD_LEVEL > VIOLENCE_NORMAL)' in original logic (doesn't make much sense. Max violence configurable via menus is 3).
			num_chunks = min(255, num_chunks * BLOOD_LEVEL);  //mxd. 'BLOOD_LEVEL - VIOLENCE_NORMAL' in original logic.
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

		gi.CreateEffect(NULL, FX_FLESH_DEBRIS, fx_flags, spot, "bdb", (byte)num_chunks, self->mins, b_mag);
	}
	else
	{
		num_chunks = max(1, num_chunks / 100);
		gi.CreateEffect(NULL, FX_DEBRIS, 0, spot, "bbdb", (byte)num_chunks, MAT_STONE, self->mins, b_mag);
	}
}

#pragma endregion

#pragma region ========================== Body part / weapon throw logic ==========================

void ThrowBodyPart(edict_t* self, const vec3_t spot, const int body_part, const int damage, const int frame) //mxd. Changed 'spot' arg type (from vec3_t*), changed 'damage' arg type (from float).
{
	// Add blood spew to sever loc and blood trail on flying part.
	if (damage > 0)
		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/fleshbreak.wav"), 1.0f, ATTN_NORM, 0.0f);

	vec3_t origin;
	VectorAdd(self->s.origin, spot, origin);

	int	fx_flags = 0;

	if (self->fire_damage_time > level.time || self->svflags & SVF_ONFIRE)
		fx_flags = CEF_FLAG6;

	if (self->materialtype == MAT_INSECT)
		fx_flags |= CEF_FLAG8;

	if (harpy_head_carrier != NULL && harpy_head_source == self)
	{
		HarpyTakeHead(harpy_head_carrier, self, body_part, frame, fx_flags);
		SprayDebris(self, spot, 5);
	}
	else
	{
		const byte b_damage = (byte)min(255, damage);
		gi.CreateEffect(NULL, FX_BODYPART, fx_flags, origin, "ssbbb", (short)frame, (short)body_part, b_damage, self->s.modelindex, self->s.number);
	}
}

void ThrowWeapon(const edict_t* self, const vec3_t spot, const int body_part, const int damage, const int frame) //mxd. Changed 'spot' arg type (from vec3_t*), changed 'damage' arg type (from float).
{
	vec3_t origin;
	VectorAdd(self->s.origin, spot, origin);

	const byte b_damage = (byte)min(255, damage);
	gi.CreateEffect(NULL, FX_THROWWEAPON, 0, origin, "ssbbb", (short)frame, (short)body_part, b_damage, self->s.modelindex, self->s.number);
}

#pragma endregion
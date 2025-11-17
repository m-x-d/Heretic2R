//
// g_func_MonsterSpawner.c -- Originally part of g_func.c
//
// Copyright 1998 Raven Software
//

#include "g_func_MonsterSpawner.h"
#include "g_ClassStatics.h"
#include "mg_ai.h"
#include "mg_guide.h"
#include "m_stats.h"
#include "Random.h"
#include "Vector.h"

#pragma region ========================== CID to classname ==========================

typedef enum MonsterSpawnerID_e
{
	MS_NOTHING = 0,
	MS_RAT,
	MS_PLAGUEELF,
	MS_SPREADER,
	MS_GORGON,
	MS_CHKROKTK,
	MS_TCHEKRIK_MALE,
	MS_TCHEKRIK_FEMALE,
	MS_TCHEKRIK_MOTHERS,
	MS_HIGH_PRIESTESS,
	MS_OGLE,
	MS_SERAPH_OVERLORD,
	MS_SERAPH_GUARD,
	MS_ASSASSIN,
	MS_MORCALAVIN,
	MS_DRANOR,
	MS_SIDHE_GUARD,
	MS_SIERNAN,
	MS_SSITHRA_SCOUT,
	MS_SSITHRA_VICTIM,
	MS_MSSITHRA,
	MS_HARPY,
	MS_FISH,
	MS_CHICKEN,
	MS_SSITHRA,
	MS_GKROKON,
	MS_RAT_GIANT,
	MS_PALACE_PLAGUE_GUARD,
	MS_INVISIBLE_PALACE_PLAGUE_GUARD,

	MS_MAX
} MonsterSpawnerID_t;

static char* monster_spawner_classnames[] =
{
	"monster_nothing",
	"monster_rat",				// MS_RAT
	"monster_plagueElf",		// MS_PLAGUEELF
	"monster_spreader",			// MS_SPREADER
	"monster_gorgon",			// MS_GORGON
	"monster_rat",				// MS_CHKROKTK
	"monster_tcheckrik_male",	// TCHEKRIK_MALE
	"monster_tcheckrik_female",	// TCHEKRIK_FEMALE
	"monster_plagueElf",		// TCHEKRIK_MOTHERS
	"monster_highPriestess",	// HIGH_PRIESTESS
	"monster_ogle",				// MS_OGLE
	"monster_seraph_overlord",	// MS_SERAPH_OVERLORD
	"monster_seraph_guard",		// MS_SERAPH_GUARD
	"monster_assassin",			// MS_ASSASSIN
	"monster_morcalavin",		// MS_MORCALAVIN
	"character_dranor",			// MS_DRANOR
	"monster_plagueElf",		// MS_SIDHE_GUARD
	"character_siernan1",		// MS_SIERNAN
	"character_ssithra_scout",	// MS_SSITHRA_SCOUT
	"character_ssithra_victim", // MS_SSITHRA_VICTIM
	"monster_mssithra",			// MS_MSSITHRA
	"monster_harpy",			// MS_HARPY
	"monster_fish",				// MS_FISH
	"monster_chicken",			// MS_CHICKEN
	"monster_ssithra",			// MS_SSITHRA
	"monster_gkrokon",			// MS_GKROKON
	"monster_rat_giant",		// MS_RAT_GIANT
	"monster_palace_plague_guard", //MS_PALACE_PLAGUE_GUARD,
	"monster_palace_plague_guard_invisible", //MS_INVISIBLE_PALACE_PLAGUE_GUARD,
};

static int cid_for_spawner_style[] =
{
	CID_NONE,
	CID_RAT,
	CID_PLAGUEELF,
	CID_SPREADER,
	CID_GORGON,
	CID_GKROKON,
	CID_TCHECKRIK,
	CID_TCHECKRIK,
	CID_MOTHER,
	CID_HIGHPRIESTESS,
	CID_OGLE,
	CID_SERAPH_OVERLORD,
	CID_SERAPH_GUARD,
	CID_ASSASSIN,
	CID_MORK,
	CID_DRANOR,
	CID_PLAGUEELF,
	CID_C_SIERNAN1,
	CID_SSITHRA_SCOUT,
	CID_SSITHRA_VICTIM,
	CID_MSSITHRA,
	CID_HARPY,
	CID_FISH,
	CID_CHICKEN,
	CID_SSITHRA,
	CID_GKROKON,
	CID_RAT,
	CID_PLAGUEELF,
	CID_PLAGUEELF,
};

#pragma endregion

#pragma region ========================== func_monsterspawner ==========================

#define SF_ONDEATH		1 //mxd
#define SF_RANDOMBUOY	2 //mxd
#define SF_PEACEFUL		4 //mxd //TODO: not implemented.

void FuncMonsterSpawnerGo(edict_t* self) //mxd. Named 'monsterspawner_go' in original logic.
{
	if (self->count <= 0)
	{
		self->think = NULL;
		return;
	}

	const vec3_t hold_origin = VEC3_INITA(self->s.origin, 0.0f, 0.0f, -8.0f);

	trace_t trace;
	gi.trace(self->s.origin, self->mins, self->maxs, hold_origin, self, MASK_MONSTERSOLID, &trace);

	if (trace.fraction != 1.0f)
		return;

	edict_t* monster = G_Spawn();
	monster->classname = ED_NewString(monster_spawner_classnames[self->style]);

	// Copy my designer-modified fields to the monster to override defaults.
	monster->classID = cid_for_spawner_style[self->style];
	monster->mintel = ((self->mintel > 0) ? self->mintel : MaxBuoysForClass[monster->classID]);
	monster->melee_range = self->melee_range;
	monster->missile_range = self->missile_range;
	monster->min_missile_range = self->min_missile_range;
	monster->bypass_missile_chance = self->bypass_missile_chance;
	monster->jump_chance = self->jump_chance;
	monster->wakeup_distance = self->wakeup_distance;

	monster->s.scale = self->s.scale;

	VectorScale(STDMinsForClass[monster->classID], monster->s.scale, monster->mins);
	VectorScale(STDMaxsForClass[monster->classID], monster->s.scale, monster->maxs);

	if (self->maxrange > 0.0f)
	{
		//H2_BUGFIX: mxd. Original logic sets angle[PITCH], which can result in monsters getting stuck in floor or ceiling when spawning.
		const vec3_t angle = VEC3_SET(0.0f, flrand(0.0f, 359.0f), 0.0f);

		vec3_t forward;
		AngleVectors(angle, forward, NULL, NULL);
		VectorMA(self->s.origin, self->maxrange, forward, monster->s.origin);
	}
	else if (self->spawnflags & SF_RANDOMBUOY)
	{
		edict_t* victim = NULL;

		// STEP 0: Who are we after?
		if (self->enemy != NULL && self->enemy->client != NULL)
			victim = self->enemy;

		if (victim == NULL)
			victim = level.sight_client;

		if (victim == NULL) // No players.
		{
			G_FreeEdict(monster);
			return;
		}

		for (int num_attempts = 0; ; num_attempts++)
		{
			if (num_attempts == 100) // Avoid infinite loops.
			{
				G_FreeEdict(monster);
				return; // Can't find any buoys close enough.
			}

			// STEP 1: Pick a random buoy.
			const buoy_t* start_buoy = &level.buoy_list[irand(0, level.active_buoys - 1)];

			// STEP 2: Make sure the buoy is within a certain range of the player (500).
			vec3_t buoy_dist;
			VectorSubtract(start_buoy->origin, victim->s.origin, buoy_dist);

			if (VectorLengthSquared(buoy_dist) > 250000) // More than 500 away.
				continue;

			// STEP 3: Make sure the buoy is not visible to the player (unless assassin).
			if (monster->classID != CID_ASSASSIN && MG_IsVisiblePos(victim, start_buoy->origin))
				continue;

			// STEP 4: If the player_buoy is defined, pick it, if not, find player's buoy.
			buoy_t* end_buoy;
			if (level.player_buoy[victim->s.number] > NULL_BUOY) // Could use player_last_buoy, but may not be reliable, and don't want to spend the time checking.
			{
				end_buoy = &level.buoy_list[level.player_buoy[victim->s.number]];
			}
			else
			{
				const int end_buoy_index = MG_SetFirstBuoy(victim);
				if (end_buoy_index == NULL_BUOY)
				{
					G_FreeEdict(monster);
					return; // Can't find a buoy for player.
				}

				end_buoy = &level.buoy_list[end_buoy_index];
			}

			// STEP 5: Make sure the buoy is within 1/2 the mintel (no more than 10) buoys of the player's buoy.
			const byte o_mintel = (byte)ClampI(monster->mintel, 3, 7);
			monster->lastbuoy = NULL_BUOY;
			monster->mintel = (byte)(ceilf((float)monster->mintel * 0.5f));

			const qboolean next_buoy_found = (FindNextBuoy(monster, start_buoy->id, end_buoy->id) != NULL); //mxd
			monster->mintel = o_mintel;

			if (!next_buoy_found) // Can't make connection within 1/2 mintel steps.
				continue;

			// STEP 6: Make sure nothing blocking is standing there.
			monster->clipmask = MASK_MONSTERSOLID;

			// STEP 7: OK, put them there.
			if (MG_MonsterAttemptTeleport(monster, start_buoy->origin, true)) // ignore_los since we checked above and can't see monster at this point yet.
				break;
		}
	}
	else
	{
		VectorCopy(self->s.origin, monster->s.origin);
	}

	VectorCopy(self->s.angles, monster->s.angles);
	ED_CallSpawn(monster);

	if (--self->count > 0)
		monster->owner = self;

	if (self->enemy != NULL && !(self->spawnflags & SF_ONDEATH))
	{
		// Was activated.
		if (self->enemy->client != NULL) //TODO: skip if SF_PEACEFUL is set?
			monster->enemy = self->enemy; // monster_start_go will check their enemy and do a FoundTarget.

		self->enemy = NULL;
	}

	if (self->count > 0 && !(self->spawnflags & SF_ONDEATH)) // This ! was inside quotes, ! is before & in order of operations.
	{
		self->think = FuncMonsterSpawnerGo;
		self->nextthink = level.time + self->wait;
	}
	else
	{
		self->think = NULL;
	}
}

void FuncMonsterSpawnerUse(edict_t* self, edict_t* other, edict_t* activator) //mxd. Named 'monsterspawner_use' in original logic. 
{
	self->enemy = activator;
	FuncMonsterSpawnerGo(self);
}

// QUAKED func_monsterspawner (0 .5 .8) (-8 -8 -8) (8 8 8) ONDEATH RANDOMBUOY PEACEFUL
// Triggerable monster spawner.

// Spawnflags:
// ONDEATH		- The next monster will not spawn until the current one is dead.
// RANDOMBUOY	- The monster will be teleported to a random buoy that the player cannot see.
// PEACEFUL		- Monsters are NOT spawned angry at ent that triggered spawner (monsters will spawn angry only if the spawner was triggered by a player).

// Variables:
// count	- Number of monsters to spawn before stopping (default 1).
// distance	- Radius which monster can spawn from monsterspawner.
// style	- Type of monster to spawn (see MonsterSpawnerID_t).
// wait		- Time to wait between spawnings (default 10).

// The following fields can be applied to the func_monsterspawner so that the monsters spawned from it will have the values you give them.
// The defaults are the monster's normal defaults. Monsters who do not use buoys may not use all of these fields (ie: rats, harpies, fish, imps, etc.).

// mintel		- Monster intelligence - this basically tells a monster how many buoys away an enemy has to be for it to give up.
// melee_range	- How close the player has to be, maximum, for the monster to go into melee.
//				  If this is zero, the monster will never melee. If it is negative, the monster will try to keep this distance from the player.
//				  If the monster has a backup, he'll use it if too close, otherwise, a negative value here means the monster will just stop running at the player at this distance.
//				Examples:
//					melee_range = 60 - monster will start swinging it player is closer than 60.
//					melee_range = 0 - monster will never do a melee attack.
//					melee_range = -100 - monster will never do a melee attack and will back away (if it has that ability) when player gets too close.
// missile_range			- Maximum distance the player can be from the monster to be allowed to use it's ranged attack.
// min_missile_range		- Minimum distance the player can be from the monster to be allowed to use it's ranged attack.
// bypass_missile_chance	- Chance that a monster will NOT fire it's ranged attack, even when it has a clear shot.
//							  This, in effect, will make the monster come in more often than hang back and fire.
//							  A percentage (0 = always fire/never close in, 100 = never fire/always close in).- must be whole number.
// jump_chance				- Every time the monster has the opportunity to jump, what is the chance (out of 100) that he will... (100 = jump every time) - must be whole number.
// wakeup_distance			- How far (max) the player can be away from the monster before it wakes up.
//							  This just means that if the monster can see the player, at what distance should the monster actually notice him and go for him.
void SP_func_monsterspawner(edict_t* self)
{
	self->solid = SOLID_NOT;
	self->maxrange = (float)st.distance;
	self->enemy = NULL;

	if (self->style <= MS_NOTHING || self->style >= MS_MAX)
		gi.dprintf("func_monsterspawner with a bad style of %d at %s\n", self->style, vtos(self->s.origin));

	if (self->count == 0)
		self->count = 1;

	if (self->wait == 0.0f)
		self->wait = 10.0f;

	if (self->s.scale == 0.0f)
		self->s.scale = 1.0f; // Transferred to spawned monster --mxd.

	if (self->targetname != NULL)
	{
		self->use = FuncMonsterSpawnerUse;
	}
	else
	{
		self->think = FuncMonsterSpawnerGo;
		self->nextthink = level.time + self->wait;
	}

	gi.linkentity(self);
}

#pragma endregion

// QUAKED monster_chkroktk (1 .5 0) (-16 -16 -26) (16 16 26) AMBUSH ASLEEP
// Temp entity until the actual code is written for monster.
void SP_monster_chkroktk(edict_t* self)
{
	self->style = MS_CHKROKTK;
	FuncMonsterSpawnerGo(self);
}

// QUAKED character_sidhe_guard (1 .5 0) (-16 -16 -26) (16 16 26) AMBUSH ASLEEP
// Temp entity until the actual code is written for monster.
void SP_character_sidhe_guard(edict_t* self)
{
	self->style = MS_SIDHE_GUARD;
	FuncMonsterSpawnerGo(self);
}
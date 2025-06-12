//
// g_main.c
//
// Copyright 1998 Raven Software
//

#include "g_main.h" //mxd
#include "cl_strings.h"
#include "g_ai.h" //mxd
#include "g_combat.h" //mxd
#include "g_cmds.h" //mxd
#include "g_Physics.h"
#include "g_playstats.h"
#include "g_ResourceManagers.h" //mxd
#include "g_save.h" //mxd
#include "g_Skeletons.h"
#include "g_spawnf.h" //mxd
#include "p_anims.h"
#include "p_client.h" //mxd
#include "p_hud.h" //mxd
#include "p_view.h" //mxd
#include "sc_Main.h" //mxd
#include "FX.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "g_local.h"

#define GAME_DECLSPEC	__declspec(dllexport)

game_locals_t game;
level_locals_t level;
game_import_t gi;
game_export_t globals;
spawn_temp_t st;

int sm_meat_index;

edict_t* g_edicts;

cvar_t* deathmatch;
cvar_t* coop;
cvar_t* dmflags;
cvar_t* advancedstaff;
cvar_t* skill;
cvar_t* fraglimit;
cvar_t* timelimit;
cvar_t* password;
cvar_t* maxclients;
cvar_t* maxentities;
cvar_t* sv_maplist;

cvar_t* dedicated;
cvar_t* filterban;

cvar_t* sv_maxvelocity; //TODO: turn into define? Missile SFX becomes desynched with missile ent when sv_maxvelocity < missile velocity.
cvar_t* sv_gravity;
cvar_t* sv_friction;

cvar_t* sv_cheats;
cvar_t* sv_nomonsters = NULL;
cvar_t* sv_freezemonsters;

cvar_t* checkanim; //TODO: unused. Remove.
cvar_t* allowillegalskins;

cvar_t* deactivate_buoys;
cvar_t* anarchy;
cvar_t* impact_damage;
cvar_t* cheating_monsters;
cvar_t* singing_ogles;

cvar_t* flood_msgs;
cvar_t* flood_persecond;
cvar_t* flood_waitdelay;
cvar_t* flood_killdelay;
cvar_t* no_runshrine;
cvar_t* no_tornado;
cvar_t* no_irondoom;
cvar_t* no_phoenix;
cvar_t* no_morph;
cvar_t* no_shield;
cvar_t* no_teleport;

cvar_t* dm_no_bodies;

cvar_t* player_dll;

cvar_t* sv_cinematicfreeze;
cvar_t* sv_jumpcinematic;
cvar_t* blood_level;
cvar_t* log_file_name;
cvar_t* log_file_header;
cvar_t* log_file_footer;
cvar_t* log_file_line_header;

static void G_RunFrame(void);

static void ShutdownGame(void)
{
	gi.dprintf("==== ShutdownGame ====\n");

	ShutdownScripts(true);

	if (game.entitiesSpawned)
	{
		ClearMessageQueues();

		edict_t* ent = &g_edicts[0];
		for (int i = 0; i < game.maxentities; i++, ent++)
		{
			SLList_Des(&ent->msgQ.msgs);
			G_FreeEdict(ent);
		}

		G_ReleaseResourceManagers();
		game.entitiesSpawned = false;
	}

	FreeLevelMessagesBuffer(); //mxd

	gi.FreeTags(TAG_LEVEL);
	gi.FreeTags(TAG_GAME);

	P_Freelib(); // Free the player lib.
}

GAME_DECLSPEC game_export_t* GetGameAPI(const game_import_t* import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;

	globals.numSkeletalJoints = MAX_ARRAYED_SKELETAL_JOINTS;
	globals.skeletalJoints = skeletalJoints;
	globals.jointNodes = jointNodes;

	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;
	globals.ConstructEntities = ConstructEntities;
	globals.CheckCoopTimeout = CheckCoopTimeout;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	//mxd. Original dlls compatibility check...
	assert(globals.edict_size == 1680);

	memset(&game, 0, sizeof(game));

	return &globals;
}

static void ClientEndServerFrames(void)
{
	// Calculate the player views now that all pushing and damage has been added.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		edict_t* ent = &g_edicts[i + 1];
		if (ent->inuse && ent->client != NULL)
			ClientEndServerFrame(ent);
	}
}

// Returns the created target changelevel.
static edict_t* CreateTargetChangeLevel(char* map)
{
	edict_t* ent = G_Spawn();

	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;

	return ent;
}

// The timelimit or fraglimit has been exceeded.
static void EndDMLevel(void)
{
	static const char* delimiters = " ,\n\r";

	// Stay on same level flag.
	if (DMFLAGS & DF_SAME_LEVEL)
	{
		BeginIntermission(CreateTargetChangeLevel(level.mapname));
		return;
	}

	// See if it's in the map list.
	if (sv_maplist->string[0] != 0)
	{
		char* maplist = _strdup(sv_maplist->string); //mxd. strdup -> _strdup
		char* first = NULL;
		char* ptr = NULL; //mxd
		char* map = strtok_s(maplist, delimiters, &ptr); //mxd. strtok -> strtok_s

		while (map != NULL)
		{
			if (Q_stricmp(map, level.mapname) == 0)
			{
				// It's in the list, go to the next one.
				map = strtok_s(NULL, delimiters, &ptr); //mxd. strtok -> strtok_s

				char* dest_map; //mxd
				if (map != NULL)
					dest_map = map;
				else if (first != NULL) // End of list, go to first one.
					dest_map = first;
				else // If there isn't a first one, go to the same level.
					dest_map = level.mapname;

				BeginIntermission(CreateTargetChangeLevel(dest_map));
				free(maplist);

				return;
			}

			if (first == NULL)
				first = map;

			map = strtok_s(NULL, delimiters, &ptr); //mxd. strtok -> strtok_s
		}

		free(maplist);
	}

	// Go to a specific map.
	if (level.nextmap[0] != 0)
	{
		BeginIntermission(CreateTargetChangeLevel(level.nextmap));
		return;
	}

	// Search for a changelevel.
	edict_t* ent = G_Find(NULL, FOFS(classname), "target_changelevel");
	if (ent != NULL)
	{
		BeginIntermission(ent);
		return;
	}

	// The map designer didn't include a changelevel, so create a fake ent that goes back to the same level.
	BeginIntermission(CreateTargetChangeLevel(level.mapname));
}

static void CheckDMRules(void)
{
	if (level.intermissiontime > 0.0f || !DEATHMATCH)
		return;

	if (timelimit->value > 0.0f && level.time >= timelimit->value * 60.0f)
	{
		gi.Obituary(PRINT_HIGH, GM_TIMELIMIT, 0, 0);
		EndDMLevel();

		return;
	}

	if (FRAGLIMIT == 0)
		return;

	for (int i = 0; i < MAXCLIENTS; i++)
	{
		if (!g_edicts[i + 1].inuse)
			continue;

		const gclient_t* cl = &game.clients[i];
		if (cl->resp.score >= FRAGLIMIT)
		{
			gi.Obituary(PRINT_HIGH, GM_FRAGLIMIT, 0, 0);
			EndDMLevel();

			return;
		}
	}
}

static void ExitLevel(void)
{
	char command[256];
	Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.changemap);

	gi.AddCommandString(command);
	level.changemap = NULL;
	level.exitintermission = false;
	level.intermissiontime = 0;

	ClientEndServerFrames();
	ClearMessageQueues();
}

void CheckContinuousAutomaticEffects(edict_t* self)
{
	// Only used for fire damage for now.
	if (self->fire_damage_time > level.time)
	{
		vec3_t check_point;
		VectorCopy(self->s.origin, check_point);
		check_point[2] += self->mins[2] + self->size[2] * 0.5f;

		if (gi.pointcontents(check_point) & (CONTENTS_WATER | CONTENTS_SLIME)) // Not lava.
		{
			//FIXME: make hiss and smoke too.
			gi.dprintf("%s fire doused\n", self->classname);
			self->fire_damage_time = 0.0f;
			self->s.effects &= ~EF_ON_FIRE; // Use this to instead notify the fire to stop.
			gi.CreateEffect(NULL, FX_ENVSMOKE, CEF_FLAG6, check_point, "");

			return;
		}

		if (self->health <= 0)
			return;

		edict_t* damager = (self->fire_damage_enemy != NULL ? self->fire_damage_enemy : world);

		if (self->client != NULL)
		{
			// Take less damage than a monster.
			if (!((byte)(level.time * 10) & 7))
				T_Damage(self, damager, damager, vec3_origin, self->s.origin, vec3_origin, 1, 0, DAMAGE_BURNING, MOD_BURNT);
		}
		else // For monsters.
		{
			// Only account for damage every .4 second.
			if (!((byte)(level.time * 10) & 3))
				T_Damage(self, damager, damager, vec3_origin, self->s.origin, vec3_origin, irand(FIRE_LINGER_DMG_MIN, FIRE_LINGER_DMG_MAX), 0, DAMAGE_BURNING, MOD_BURNT);
		}

		if (self->client == NULL)
			return;

		const playerinfo_t* pi = &self->client->playerinfo; //mxd
		if (pi->lowerseq == ASEQ_ROLLDIVEF_W || pi->lowerseq == ASEQ_ROLLDIVEF_R || pi->lowerseq == ASEQ_ROLL_FROM_FFLIP ||
			pi->upperseq == ASEQ_ROLLDIVEF_W || pi->upperseq == ASEQ_ROLLDIVEF_R || pi->upperseq == ASEQ_ROLL_FROM_FFLIP ||
			pi->lowerseq == ASEQ_ROLL_L || pi->lowerseq == ASEQ_ROLL_R || pi->lowerseq == ASEQ_ROLL_B ||
			pi->upperseq == ASEQ_ROLL_L || pi->upperseq == ASEQ_ROLL_R || pi->upperseq == ASEQ_ROLL_B)
		{
			float waterlevel = (float)self->waterlevel / 5.0f;
			if (self->watertype & CONTENTS_LAVA)
				waterlevel = 0.0f;

			self->fire_damage_time -= 0.15f + waterlevel * 0.5f; // Stop, drop and roll!
		}
	}
	else if (self->fire_damage_time > 0.0f)
	{
		self->fire_damage_time = 0.0f;
		self->s.effects &= ~EF_ON_FIRE; // Notify the fire to stop.
	}
}

static void EntityThink(edict_t* self)
{
	// See if anything is happening to us we need to update...
	CheckContinuousAutomaticEffects(self);

#ifdef _DEBUG
	const float think_time = self->nextthink;
#endif

	// Not used for guides anymore, but nice for effects like tinting/fading, etc. that should continue while the entity is doing other stuff.
	if (self->pre_think != NULL && self->next_pre_think > 0.0f && self->next_pre_think < level.time)
		self->pre_think(self);

	if (ThinkTime(self))
	{
		self->think(self);

		// We expect that either ent is removed, ent->think callback is cleared or ent nextthink time is updated as a result of calling ent->think().
		assert(!self->inuse || self->think == NULL || think_time != self->nextthink); //TODO: shouldn't we check for nextthink > think_time or nextthink > level.time instead? 
	}
}

static void EntityPostThink(edict_t* self)
{
	// For effects that rely on accurate physics info.
	if (self->post_think != NULL && self->next_post_think > 0.0f && self->next_post_think < level.time)
		self->post_think(self);
}

static void SetNumPlayers(void)
{
	game.num_clients = 0;

	edict_t* ent = &g_edicts[0];
	for (int i = 0; i < MAX_CLIENTS; i++, ent++)
		if (ent != NULL && ent->client != NULL)
			game.num_clients++;
}

static void UpdatePlayerBuoys(void)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (level.player_buoy[i] == NULL_BUOY) //mxd. Inverted check.
			continue;

		edict_t* ent = &g_edicts[0];
		for (int j = 0; j < globals.num_edicts; j++, ent++)
		{
			if (i != ent->s.number - 1)
				continue;

			// If player hasn't moved, don't clear this.
			vec3_t v;
			VectorSubtract(level.buoy_list[level.player_buoy[i]].origin, ent->s.origin, v);

			if (VectorLengthSquared(v) > 576) // 24 squared.
			{
				// Save it so monsters can check this first- FIXME: should this expire?
				level.player_last_buoy[i] = level.player_buoy[i];

				// This is for monsters following buoys - only the first monster who's searching for the player has to do a buoy connection
				// to him this frame, the rest can use this - reset each frame.
				level.player_buoy[i] = NULL_BUOY;
			}

			break;
		}
	}
}

// Advances the world by 0.1 seconds.
static void G_RunFrame(void)
{
	if (DEATHMATCH || COOP)
		ClampI(BLOOD_LEVEL, VIOLENCE_NONE, VIOLENCE_NORMAL);

	// Update server ticks.
	level.framenum++;
	level.time = (float)level.framenum * FRAMETIME;

	// Choose a client for monsters to target this frame. Only targets one client for all monsters.
	AI_SetSightClient();

	// Exit intermissions.
	if (level.exitintermission)
	{
		ExitLevel();
		return;
	}

	// Update any joints that need to be
	UpdateSkeletons();

	// Keep track of player buoys.
	if (DEATHMATCH)
		SetNumPlayers(); // For shrines and pick-ups.
	else
		UpdatePlayerBuoys();

	// Treat each object in turn. Even the world gets a chance to think.
	edict_t* ent = &g_edicts[0];
	for (int i = 0; i < globals.num_edicts; i++, ent++)
	{
		if (SV_CINEMATICFREEZE && (ent->svflags & SVF_MONSTER) && !ent->monsterinfo.c_mode)
			continue;

		// If entity not in use - don't process.
		if (!ent->inuse)
			continue;

		//mxd. Skip non-functional pvs_cull logic. Didn't cause performance problems in 1998, so can be skipped today sa well...

		level.current_entity = ent;

		if (ent->msgHandler != NULL) // Eventually this check won't be needed.
			ProcessMessages(ent);

		if (ent->flags & FL_SUSPENDED)
			continue;

		// Remember original origin.
		VectorCopy(ent->s.origin, ent->s.old_origin);

		// Make sure the entity still has something to stand on.
		if (ent->groundentity != NULL)
		{
			// Check for the groundentity being freed.
			if (!ent->groundentity->inuse)
			{
				ent->groundentity = NULL;
			}
			else if (ent->groundentity->linkcount != ent->groundentity_linkcount)
			{
				// If the ground entity moved, make sure we are still on it.
				ent->groundentity = NULL;

				if (ent->svflags & SVF_MONSTER)
					CheckEntityOn(ent);
			}
		}

		if (i > 0 && i <= MAXCLIENTS)
		{
			ClientBeginServerFrame(ent);

			// OK, we need to hack in some bits here - the player's think function never appears to get called.
			// We need the think function for when the player is a chicken, in order to keep track of how long he should remain a chicken.
			if (ent->flags & FL_CHICKEN) // We're set as a chicken.
				EntityThink(ent);

			continue;
		}

		// Use new physics for everything except flymissile (and movetype none).
		// The scripts work using the new physics now.
		if (ent->movetype != MOVETYPE_FLYMISSILE)
		{
			EntityThink(ent);

			assert(ent->movetype < NUM_PHYSICSTYPES);

			if (!ent->inuse)
				continue;

			EntityPhysics(ent);
			EntityPostThink(ent);
		}
		else // Use old physics for missiles (for compatibility).
		{
			G_RunEntity(ent);
		}
	}

	ProcessScripts();
	CheckDMRules(); // See if it is time to end a deathmatch.
	ClientEndServerFrames(); // Build the playerstate_t structures for all players.

	assert(Vec3IsZero(vec3_origin));
}
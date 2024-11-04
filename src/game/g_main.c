#include "g_local.h"
#include "g_Skeletons.h"
#include "ArrayedList.h"
#include "vector.h"
#include "g_Physics.h"
#include "g_volume_effect.h"
#include "PrimitiveDisplayHack.h"
#include "q_shared.h"
#include "timing.h"
#include "fx.h"
#include "Utilities.h"
#include "random.h"
#include "g_playstats.h"
#include "p_anims.h"
#include "cl_strings.h"

//Used for monster_speeds timing stuff
#ifdef _DEVEL
__int64	begin_monster_time, end_monster_time;
__int64	begin_ent_time, end_ent_time;
__int64	next_update, all, mt, cur_all, cur_mt;
int		num_monsters, num_ents;
#endif //_DEVEL

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
qboolean MonsterAdvanceFrame=false;

//int	snd_fry;

edict_t	*g_edicts;

cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*advancedstaff;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*password;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*sv_maplist;
cvar_t	*g_select_empty;

cvar_t	*dedicated;
cvar_t	*filterban;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;
cvar_t	*sv_friction;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;
cvar_t	*sv_nomonsters=NULL;
cvar_t	*sv_freezemonsters;

cvar_t	*autorotate;
cvar_t	*blood;

cvar_t	*checkanim;
cvar_t	*allowillegalskins;

cvar_t	*pvs_cull;

cvar_t	*showbuoys;
cvar_t	*showlitebuoys;
cvar_t	*mgai_debug;
cvar_t	*deactivate_buoys;
cvar_t	*anarchy;
cvar_t	*impact_damage;
cvar_t	*cheating_monsters;
cvar_t	*singing_ogles;

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;
cvar_t	*flood_killdelay;
cvar_t	*no_runshrine;
cvar_t	*no_tornado;
cvar_t	*no_irondoom;
cvar_t	*no_phoenix;
cvar_t	*no_morph;
cvar_t	*no_shield;
cvar_t	*no_teleport;

cvar_t	*game_test;
cvar_t	*dm_no_bodies;

cvar_t	*player_dll;

cvar_t	*sv_cinematicfreeze;
cvar_t	*sv_jumpcinematic;
cvar_t	*blood_level;
cvar_t	*log_file_name;
cvar_t	*log_file_header;
cvar_t	*log_file_footer;
cvar_t	*log_file_line_header;

void SpawnEntities (char *mapname, char *entities, char *spawnpoint, qboolean loadgame);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void RunEntity (edict_t *ent);
void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);
void InitGame (void);
void G_RunFrame (void);
void ConstructEntities(void);
void CheckCoopTimeout(qboolean BeenHereBefore);
void ClearMessageQueues();

//===================================================================

/*
=================
ShutdownGame
=================
*/
void ShutdownGame (void)
{
	void G_ReleaseResourceManagers();

	edict_t *ent;
	int i;

	gi.dprintf ("==== ShutdownGame ====\n");

	ShutdownScripts(true);

	if(game.entitiesSpawned)
	{
		ClearMessageQueues();

		for(i = 0, ent = g_edicts; i < game.maxentities ; ++i, ++ent)
		{
			SLList_Des(&ent->msgQ.msgs);
			G_FreeEdict(ent);
		}

		G_ReleaseResourceManagers();
	}
	game.entitiesSpawned = false;

	gi.FS_FreeFile(messagebuf);

	gi.FreeTags(TAG_LEVEL);
	gi.FreeTags(TAG_GAME);

	P_Freelib();	// free the player lib
}



game_export_t *GetGameAPI (game_import_t *import)
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
	globals.CheckCoopTimeout= CheckCoopTimeout;

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

	memset(&game, 0, sizeof(game));

	return &globals;
}

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		ClientEndServerFrame (ent);
	}

}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	edict_t		*ent;
	char *s, *t, *f;
	static const char *seps = " ,\n\r";

	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		BeginIntermission (CreateTargetChangeLevel (level.mapname) );
		return;
	}

	// see if it's in the map list
	if (*sv_maplist->string)
	{
		s = strdup(sv_maplist->string);
		f = NULL;
		t = strtok(s, seps);
		while (t != NULL)
		{
			if (Q_stricmp(t, level.mapname) == 0)
			{
				// it's in the list, go to the next one
				t = strtok(NULL, seps);
				if (t == NULL)
				{ // end of list, go to first one
					if (f == NULL) // there isn't a first one, same level
						BeginIntermission (CreateTargetChangeLevel (level.mapname) );
					else
						BeginIntermission (CreateTargetChangeLevel (f) );
				}
				else
				{
					BeginIntermission (CreateTargetChangeLevel (t) );
				}
				free(s);
				return;
			}
			if (!f)
			{
				f = t;
			}
			t = strtok(NULL, seps);
		}
		free(s);
	}

	if (level.nextmap[0]) // go to a specific map
		BeginIntermission (CreateTargetChangeLevel (level.nextmap) );
	else
	{	// search for a changelevel
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			return;
		}
		BeginIntermission (ent);
	}
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i;
	gclient_t	*cl;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;

	if (timelimit->value)
	{
		if (level.time >= timelimit->value*60)
		{
			gi.Obituary (PRINT_HIGH, GM_TIMELIMIT, 0, 0);
			EndDMLevel ();
			return;
		}
	}

	if (fraglimit->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{
				gi.Obituary (PRINT_HIGH, GM_FRAGLIMIT, 0, 0);
				EndDMLevel ();
				return;
			}
		}
	}
}


/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;

	ClientEndServerFrames ();

	ClearMessageQueues();
}

void CheckContinuousAutomaticEffects(edict_t *self)
{//only used for fire damage for now
	edict_t *damager;
	vec3_t	checkpoint;

	if(self->fire_damage_time > level.time)
	{
		VectorCopy(self->s.origin, checkpoint);
		checkpoint[2] += self->mins[2];
		checkpoint[2] += self->size[2] * 0.5;
		if(gi.pointcontents(checkpoint) & (CONTENTS_WATER|CONTENTS_SLIME))			// Not lava
		{//FIXME: make hiss and smoke too
			gi.dprintf("%s fire doused\n", self->classname);
			self->fire_damage_time = 0;
//			gi.RemoveEffects(&self->s, FX_FIRE_ON_ENTITY);//turn off CFX too
			self->s.effects &= ~EF_ON_FIRE;			// Use this to instead notify the fire to stop.
			gi.CreateEffect(NULL,
					FX_ENVSMOKE,
					CEF_FLAG6,
					checkpoint,
					"");
			return;
		}

		if(self->health <= 0)
			return;

		if(self->fire_damage_enemy)
			damager = self->fire_damage_enemy;
		else
			damager = world;

		if (self->client)
		{	// Take less damage than a monster.
			if (!(((byte)(level.time*10))&0x07))
			{
				T_Damage(self, damager, damager, vec3_origin, self->s.origin, vec3_origin, 
							1, 0, DAMAGE_BURNING,MOD_BURNT);
			}
		}
		else	// For monsters
		{	// Only account for damage every .4 second.
			if (!(((byte)(level.time*10))&0x03))
			{
				T_Damage(self, damager, damager, vec3_origin, self->s.origin, vec3_origin, 
							irand(FIRE_LINGER_DMG_MIN, FIRE_LINGER_DMG_MAX), 0, DAMAGE_BURNING,MOD_BURNT);
			}
			//tint it darker brown as goes on?  How to get back? no, scorched art would look better
		}

		if(self->client)
		{
			if(self->client->playerinfo.lowerseq == ASEQ_ROLLDIVEF_W || self->client->playerinfo.lowerseq == ASEQ_ROLLDIVEF_R || self->client->playerinfo.lowerseq == ASEQ_ROLL_FROM_FFLIP ||
				self->client->playerinfo.upperseq == ASEQ_ROLLDIVEF_W || self->client->playerinfo.upperseq == ASEQ_ROLLDIVEF_R || self->client->playerinfo.upperseq == ASEQ_ROLL_FROM_FFLIP ||
				self->client->playerinfo.lowerseq == ASEQ_ROLL_L || self->client->playerinfo.lowerseq == ASEQ_ROLL_R || self->client->playerinfo.lowerseq == ASEQ_ROLL_B ||
				self->client->playerinfo.upperseq == ASEQ_ROLL_L || self->client->playerinfo.upperseq == ASEQ_ROLL_R || self->client->playerinfo.upperseq == ASEQ_ROLL_B)
			{
				float waterlevel;

				waterlevel = self->waterlevel/5.0;
				if (self->watertype & CONTENTS_LAVA)
					waterlevel = 0;
				self->fire_damage_time -= (0.15 + (waterlevel*0.5));//stop, drop and roll!
			}
		}
	}
	else if(self->fire_damage_time>0)
	{
		self->fire_damage_time = 0;
//		gi.RemoveEffects(&self->s, FX_FIRE_ON_ENTITY);//turn off CFX too
		self->s.effects &= ~EF_ON_FIRE;		// Use this to instead notify the fire to stop.
		return;
	}

}

static void EntityThink(edict_t *self)
{
	float	thinktime;

	//see if anything is happening to us we need to update...
	CheckContinuousAutomaticEffects(self);

	thinktime = self->nextthink;

	if(self->pre_think && self->next_pre_think > 0.0f && self->next_pre_think < level.time)
	{//not used for guides anymore, but nice for effects
		//like tinting/fading, etc that should continue
		//while the entity is doing other stuff
		self->pre_think(self);
	}
	if(!ThinkTime(self))
	{
		return;
	}
	self->think(self);

	assert(!self->inuse || !self->think || thinktime != self->nextthink);
	//NOTENOTE: This is a Quake oldy... it's common practice to do this!
	/*assert(self->nextthink == -1);*/
}

static void EntityPostThink(edict_t *self)
{
	if(self->post_think && self->next_post_think > 0.0f && self->next_post_think < level.time)
	{//for effects that rely on accurate physics info
		self->post_think (self);
	}
}

static void SetNumPlayers (void)
{
	int			i;
	edict_t		*ent;

	ent = g_edicts;
	game.num_clients = 0;
	for(i = 0; i < MAX_CLIENTS; i++, ent++)
	{//If player hasn't moved, don't clear this
		if(ent)
		{
			if(ent->client)
			{
				game.num_clients++;
			}
		}
	}
}

static void UpdatePlayerBuoys (void)
{
	qboolean	dont_null;
	int			i, j;
	edict_t		*ent;
	vec3_t		v;

	for(i = 0; i<MAX_CLIENTS; i++)
	{	
		if(level.player_buoy[i] > NULL_BUOY)
		{
			ent = g_edicts;
			for(j = 0; j < globals.num_edicts; j++, ent++)
			{//If player hasn't moved, don't clear this
				if(ent->s.number - 1 == i)
				{
					VectorSubtract(level.buoy_list[level.player_buoy[i]].origin, ent->s.origin, v);
					if(VectorLengthSquared(v) > 576)//24 squared
						dont_null = false;
					else
						dont_null = true;
					break;
				}
			}
		}

		if(!dont_null)
		{
			level.player_last_buoy[i] = level.player_buoy[i];//save it so monsters can check this first- FIXME: should this expire?
			level.player_buoy[i] = NULL_BUOY;//this is for monsters following buoys- only the first monster who's searching for the player has to do a buoy connection to him this frame, the rest can use this- reset each frame
		}
	}
}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame (void)
{
	void		UpdateSkeletons();
	int			i;
	edict_t		*ent;

//	void (*update)(edict_t *self);				//old crap causing buoy crash

	qboolean SV_RunThink(edict_t *ent);

	if(deathmatch->value || coop->value)
		Clamp(blood_level->value, VIOLENCE_NONE, VIOLENCE_NORMAL);

	// Update server ticks
	level.framenum++;
	level.time = level.framenum * FRAMETIME;

	// choose a client for monsters to target this frame
	// Only targets one client for all monsters
	AI_SetSightClient ();

	// exit intermissions
	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}

	// Update any joints that need to be
	UpdateSkeletons();

	//Keep track of player buoys
	if(!deathmatch->value)
		UpdatePlayerBuoys();
	else
		SetNumPlayers();//for shrines and pick-ups

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = g_edicts;
	for(i = 0; i < globals.num_edicts; i++, ent++)
	{

		if (sv_cinematicfreeze->value)
		{
			if ((ent->svflags & SVF_MONSTER)  && (!ent->monsterinfo.c_mode))
				continue;
		}

		// If entity not in use - don`t process
		if(!ent->inuse)
		{
#ifdef BBOX_DISPLAY_HACK
			DisableBBoxDisplay(i);
#endif
			continue;
		}

		//
		// Don`t let monster think or move if its not in the PVS and not hunting
		//

		// If the ent is a monster (but not a  cinematic monster) and the culling is active...

		if ((ent->svflags & SVF_MONSTER) && (!ent->monsterinfo.c_mode) && pvs_cull->value)
		{
			// Ent cannot be hunting an enemy or moving to a goalentity.

			if (!ent->enemy && !ent->goalentity && level.sight_client)
			{
				int		j;
				edict_t *client_ent;

				// If not in our PVS, we don't care.

				for(j=0;j<maxclients->value;j++)
				{
					client_ent=g_edicts+1+j;	
					
					if(client_ent->inuse)
					{
//						if (!gi.inPVS(ent->s.origin, level.sight_client->s.origin))
						if (!gi.inPVS(ent->s.origin, client_ent->s.origin))
						{
							continue;
						}
					}
				}
			}
		}

#ifdef BBOX_DISPLAY_HACK
		EnableBBoxDisplay(i);
#endif
		level.current_entity = ent;

#ifndef G_TRANSITION
		assert(ent->msgHandler);
#else
		if(ent->msgHandler)	// eventually this check wont be needed
#endif // G_TRANSITION
		{
			ProcessMessages(ent);
		}

		if(ent->flags & FL_SUSPENDED)
		{
			continue;
		}

		// Remember original origin
		VectorCopy(ent->s.origin, ent->s.old_origin);

		// Make sure the entity still has something to stand on
		if(ent->groundentity)
		{
			// check for the groundentity being freed
			if(!ent->groundentity->inuse)
			{
				ent->groundentity = NULL;
			}
			else if(ent->groundentity->linkcount != ent->groundentity_linkcount)
			{	// if the ground entity moved, make sure we are still on it
				ent->groundentity = NULL;

				if(ent->svflags & SVF_MONSTER)
				{
					CheckEntityOn(ent);
				}
			}
		}

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			// ok, we need to hack in some bits here - the players think function never appears to get called. Why, I don't know
			// kinda defies the point of having a think based system if your not going to use it. Still, never mind.
			// we need the think function for when the player is a chicken, in order to keep track of how long he should remain a chicken

			if (ent->flags & FL_CHICKEN)	// We're set as a chicken
				EntityThink(ent);

			continue;
		}

		// Use new physics for everything except flymissile (and movetype none)
		// The scripts work using the new physics now
		if(ent->movetype != MOVETYPE_FLYMISSILE)
		{
			EntityThink(ent);

			assert(ent->movetype < NUM_PHYSICSTYPES);

			if(!ent->inuse)
			{
				continue;
			}

			EntityPhysics(ent);
			EntityPostThink(ent);
		}
		else
		// Use old physics for missiles (for compatibility)
		{
			G_RunEntity(ent);
		}
	}

	// If the monsters are frozen, we wanted a single frame advance.
	if (MonsterAdvanceFrame)
	{
		MonsterAdvanceFrame = false;
	}

	ProcessScripts ();

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	assert(Vec3IsZero(vec3_origin));
}


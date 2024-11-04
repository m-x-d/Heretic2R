
#include "g_local.h"
#include "g_Skeletons.h"
#include "ArrayedList.h"
#include "Message.h"
#include "q_ClientServer.h"
#include "q_Physics.h"
#include "g_playstats.h"
#include "utilities.h"
#include "p_anims.h"
#include "FX.h"

extern void	InitPlayerinfo(edict_t *ent);
extern void LoadPersistantEffects(FILE *f);
extern void SavePersistantEffects(FILE *f);
extern player_export_t	playerExport;	// interface to player DLL.

field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"model", FOFS(model), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"accel", FOFS(accel), F_FLOAT},
	{"decel", FOFS(decel), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"scripttarget", FOFS(scripttarget), F_LSTRING},
	{"pathtarget", FOFS(pathtarget), F_LSTRING},
	{"jumptarget", FOFS(jumptarget), F_LSTRING},
	{"deathtarget", FOFS(deathtarget), F_LSTRING},
	{"killtarget", FOFS(killtarget), F_LSTRING},
	{"combattarget", FOFS(combattarget), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"text_msg", FOFS(text_msg), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"time", FOFS(time), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"skinnum", FOFS(s.skinnum), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},
	{"materialtype", FOFS(materialtype), F_INT},
	{"scale", FOFS(s.scale), F_FLOAT},
	{"color", FOFS(s.color), F_RGBA},
	{"absLight", FOFS(s.absLight), F_RGB},
	{"frame", FOFS(s.frame), F_INT},
	{"mintel", FOFS(mintel), F_INT},
	{"melee_range", FOFS(melee_range), F_FLOAT},
	{"missile_range", FOFS(missile_range), F_FLOAT},
	{"min_missile_range", FOFS(min_missile_range), F_FLOAT},
	{"bypass_missile_chance", FOFS(bypass_missile_chance), F_INT},
	{"jump_chance", FOFS(jump_chance), F_INT},
	{"wakeup_distance", FOFS(wakeup_distance), F_FLOAT},
	{"c_mode", FOFS(monsterinfo.c_mode), F_INT, F_INT},
	{"homebuoy", FOFS(homebuoy), F_LSTRING},
	{"wakeup_target", FOFS(wakeup_target), F_LSTRING},
	{"pain_target", FOFS(pain_target), F_LSTRING},

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},
	{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},
	{"rotate", STOFS(rotate), F_INT, FFL_SPAWNTEMP},
	{"target2", FOFS(target2), F_LSTRING},
	{"pathtargetname",  FOFS(pathtargetname), F_LSTRING},
	{"zangle", STOFS(zangle), F_FLOAT, FFL_SPAWNTEMP},
	{"file", STOFS(file), F_LSTRING, FFL_SPAWNTEMP},
	{"radius", STOFS(radius), F_INT, FFL_SPAWNTEMP},
	{"offensive", STOFS(offensive), F_INT, FFL_SPAWNTEMP},
	{"defensive", STOFS(defensive), F_INT, FFL_SPAWNTEMP},
	{"spawnflags2", STOFS(spawnflags2), F_INT, FFL_SPAWNTEMP},
	{"cooptimeout", STOFS(cooptimeout), F_INT, FFL_SPAWNTEMP},

	{"script", STOFS(script), F_LSTRING, FFL_SPAWNTEMP},
	{"parm1", STOFS(parms[0]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm2", STOFS(parms[1]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm3", STOFS(parms[2]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm4", STOFS(parms[3]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm5", STOFS(parms[4]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm6", STOFS(parms[5]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm7", STOFS(parms[6]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm8", STOFS(parms[7]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm9", STOFS(parms[8]), F_LSTRING, FFL_SPAWNTEMP},
	{"parm10", STOFS(parms[9]), F_LSTRING, FFL_SPAWNTEMP},

};

// -------- just for savegames ----------
// all pointer fields should be listed here, or savegames
// won't work properly (they will crash and burn).
// this wasn't just tacked on to the fields array, because
// these don't need names, we wouldn't want map fields using
// some of these, and if one were accidentally present twice
// it would double swizzle (fuck) the pointer.

field_t		savefields[] =
{
	{"", FOFS(classname), F_LSTRING},
	{"", FOFS(target), F_LSTRING},
	{"", FOFS(target2), F_LSTRING},
	{"", FOFS(targetname), F_LSTRING},
	{"", FOFS(scripttarget), F_LSTRING},
	{"", FOFS(killtarget), F_LSTRING},
	{"", FOFS(team), F_LSTRING},
	{"", FOFS(pathtarget), F_LSTRING},
	{"", FOFS(deathtarget), F_LSTRING},
	{"", FOFS(combattarget), F_LSTRING},
	{"", FOFS(model), F_LSTRING},
	{"", FOFS(map), F_LSTRING},
	{"", FOFS(message), F_LSTRING},
	{"", FOFS(client), F_CLIENT},
	{"", FOFS(item), F_ITEM},
	{"", FOFS(goalentity), F_EDICT},
	{"", FOFS(movetarget), F_EDICT},
	{"", FOFS(enemy), F_EDICT},
	{"", FOFS(oldenemy), F_EDICT},
	{"", FOFS(activator), F_EDICT},
#ifdef G_TRANSITION
	{"", FOFS(groundentity), F_EDICT},
#endif
	{"", FOFS(teamchain), F_EDICT},
	{"", FOFS(teammaster), F_EDICT},
	{"", FOFS(owner), F_EDICT},
	{"", FOFS(mynoise), F_EDICT},
	{"", FOFS(mynoise2), F_EDICT},
	{"", FOFS(target_ent), F_EDICT},
	{"", FOFS(chain), F_EDICT},
	{"", FOFS(blockingEntity), F_EDICT},
	{"", FOFS(last_buoyed_enemy), F_EDICT},
	{"", FOFS(placeholder), F_EDICT},
	{"", FOFS(fire_damage_enemy), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		levelfields[] =
{
	{"", LLOFS(changemap), F_LSTRING},
	{"", LLOFS(sight_client), F_EDICT},
	{"", LLOFS(sight_entity), F_EDICT},
	{NULL, 0, F_INT}
};

field_t		bouyfields[] =
{
	{"", BYOFS(pathtarget), F_LSTRING},
	{"", BYOFS(target), F_LSTRING},
	{"", BYOFS(targetname), F_LSTRING},
	{"", BYOFS(jump_target), F_LSTRING},
	{NULL, 0, F_INT}
};


field_t		clientfields[] =
{
	{"", CLOFS(playerinfo.pers.weapon), F_ITEM},
	{"", CLOFS(playerinfo.pers.lastweapon), F_ITEM},
	{"", CLOFS(playerinfo.pers.defence), F_ITEM},
	{"", CLOFS(playerinfo.pers.lastdefence), F_ITEM},
	{"", CLOFS(playerinfo.pers.newweapon), F_ITEM},
						 
	{NULL, 0, F_INT}
};

trig_message_t	message_text[MAX_MESSAGESTRINGS];
unsigned		*messagebuf;

int LoadTextFile(char *name, char **addr)
{
	int		length;
	char	*buffer;

	length = gi.FS_LoadFile(name, &buffer);
	if(length <= 0)
	{
		Sys_Error("Unable to load %s", name);
		return(0);
	}
	*addr = (char *)gi.TagMalloc(length + 1, 0);
	memcpy(*addr, buffer, length);
	*(*addr + length) = 0;
	gi.FS_FreeFile(buffer);

	return(length + 1);
}

void Load_Strings(void)
{
	char	*buffer;
	int		i,length;
	char *p,*startp,*return_p;

	length = LoadTextFile ("levelmsg.txt", &buffer);
	messagebuf = (unsigned *) buffer;
	startp = buffer;
	p =0;
	for (i=1; p < (buffer + length) ;++i)
	{			
		if (i> MAX_MESSAGESTRINGS)
		{
			Com_Printf ("Too many strings\n");
			return;
		}

		// Read in string up to return
		return_p = strchr(startp,13);	// Search for return characters 13 10
		if (!return_p)	// At end of file
		{
			break;
		}
		else
			*return_p = 0;

		// Search string for #
		p = strchr(startp,'#');	// Search for # which signifies a wav file
		if ((p) && (p < return_p))
		{
			*p = 0;
			message_text[i].wav = ++p;	// Save stuff after #
		}

		// Save stuff before #
		message_text[i].string = startp;

		do
		{
			p = strchr(startp,'@');	// Search for # which signifies a wav file
			if (p)
				*p = '\n';
		} while(p);

		return_p +=2;	// Hop over 13 10 
		startp = return_p;	// Advance to next string

	} 
}

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is begun
============
*/
void InitGame (void)
{
	void G_InitResourceManagers();

	gi.dprintf ("==== InitGame ====\n");

	G_InitResourceManagers();

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these.

	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", MAX_VELOCITY_STRING, 0);

	sv_gravity = gi.cvar ("sv_gravity", GRAVITY_STRING, 0);		// GRAVITY FOR ALL GAMES
	sv_friction = gi.cvar ("sv_friction", FRICTION_STRING, 0);		// FRICTION FOR ALL GAMES

	// Noset vars.

	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	// Latched vars.

	sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	gi.cvar ("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	
	skill = gi.cvar ("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", G_MAX_ENTITIES, CVAR_LATCH);

	sv_nomonsters = gi.cvar ("nomonsters", "0", CVAR_SERVERINFO|CVAR_LATCH);
	sv_freezemonsters = gi.cvar ("freezemonsters", "0", 0);

	// Change anytime vars.

	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	advancedstaff = gi.cvar ("advancedstaff", "1", CVAR_SERVERINFO);

	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar ("password", "", CVAR_USERINFO);
	filterban = gi.cvar ("filterban", "1", 0);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	autorotate = gi.cvar("autorotate", "0", 0);
	blood = gi.cvar("blood", "1", 0);

	checkanim = gi.cvar("checkanim", "0", 0);
	allowillegalskins = gi.cvar("allowillegalskins", "0", CVAR_ARCHIVE);

	pvs_cull = gi.cvar("pvs_cull", "1", 0);

	showbuoys = gi.cvar("showbuoys", "0", 0);
	showlitebuoys = gi.cvar("showlitebuoys", "0", 0);
	mgai_debug = gi.cvar("mgai_debug", "0", 0);
	deactivate_buoys = gi.cvar("deactivate_buoys", "0", 0);
	anarchy = gi.cvar("anarchy", "0", 0);
	impact_damage = gi.cvar("impact_damage", "1", 0);
	cheating_monsters = gi.cvar("cheating_monsters", "1", 0);
	singing_ogles = gi.cvar("singing_ogles", "0", 0);
	no_runshrine = gi.cvar("no_runshrine", "0", 0);
	no_tornado = gi.cvar("no_tornado", "0", 0);
	no_teleport = gi.cvar("no_teleport","0",0);
	no_phoenix = gi.cvar("no_phoenix","0",0);
	no_irondoom = gi.cvar("no_irondoom","0",0);
	no_morph = gi.cvar("no_morph","0",0);
	no_shield = gi.cvar("no_shield","0",0);

	game_test = gi.cvar("game_test", "0", 0);
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);
	flood_killdelay = gi.cvar ("flood_killdelay", "10", 0);
	sv_maplist = gi.cvar ("sv_maplist", "", 0);

	player_dll = Cvar_Get("player_dll", DEFAULT_PLAYER_LIB, 0);

	sv_cinematicfreeze = gi.cvar("sv_cinematicfreeze", "0", 0);
	sv_jumpcinematic = gi.cvar("sv_jumpcinematic", "0", 0);
	log_file_name = gi.cvar("log_file_name", "", CVAR_ARCHIVE);
	log_file_footer = gi.cvar("log_file_footer", "", CVAR_ARCHIVE);
	log_file_header = gi.cvar("log_file_header", "", CVAR_ARCHIVE);
	log_file_line_header = gi.cvar("log_file_line_header", "", CVAR_ARCHIVE);

	blood_level = gi.cvar ("blood_level", VIOLENCE_DEFAULT_STR, CVAR_ARCHIVE);
	dm_no_bodies = gi.cvar ("dm_no_bodies", "0", CVAR_ARCHIVE);

	gi.cvar("flash_screen", "1", 0);

	P_Load(player_dll->string);

	// ********************************************************************************************
	// Initialise the inventory items.
	// ********************************************************************************************
	
	// Server side only elements.

	G_InitItems();
	
	// ********************************************************************************************
	// Initialise hep messages.
	// ********************************************************************************************

	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "No help message1");
	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "No help message2");

	// ********************************************************************************************
	// Initialize all entities for this game.
	// ********************************************************************************************

	game.maxentities = maxentities->value;
	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// ********************************************************************************************
	// Initialize all clients for this game.
	// ********************************************************************************************

	game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients+1;

	level.cinActive = false;

	Load_Strings();
}

//=========================================================

void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:
		if ( *(char **)p )
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = len;
		break;
	case F_EDICT:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = index;
		break;
	case F_CLIENT:
		if ( *(gclient_t **)p == NULL)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = index;
		break;
	case F_ITEM:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(gitem_t **)p - playerExport.p_itemlist;
		*(int *)p = index;
		break;

	default:
		gi.error ("WriteEdict: unknown field type");
	}
}

void WriteField2 (FILE *f, field_t *field, byte *base)
{
	int			len;
	void		*p;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
	case F_GSTRING:
		if ( *(char **)p )
		{
			len = strlen(*(char **)p) + 1;
			fwrite (*(char **)p, len, 1, f);
		}
		break;
	}
}

void ReadField (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
		len = *(int *)p;
		if (!len)
			*(char **)p = NULL;
		else
		{
			*(char **)p = gi.TagMalloc (len, TAG_LEVEL);
			fread (*(char **)p, len, 1, f);
		}
		break;
	case F_GSTRING:
		len = *(int *)p;
		if (!len)
			*(char **)p = NULL;
		else
		{
			*(char **)p = gi.TagMalloc (len, TAG_GAME);
			fread (*(char **)p, len, 1, f);
		}
		break;
	case F_EDICT:
		index = *(int *)p;
		if ( index == -1 )
			*(edict_t **)p = NULL;
		else
			*(edict_t **)p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *(int *)p;
		if ( index == -1 )
			*(gclient_t **)p = NULL;
		else
			*(gclient_t **)p = &game.clients[index];
		break;
	case F_ITEM:
		index = *(int *)p;
		if ( index == -1 )
			*(gitem_t **)p = NULL;
		else
			*(gitem_t **)p = &playerExport.p_itemlist[index];
		break;

	default:
		gi.error ("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field=clientfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=clientfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient (FILE *f, gclient_t *client)
{
	field_t		*field;

	fread (client, sizeof(*client), 1, f);

	for (field=clientfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)client);
	}
}

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void WriteGame (char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];
	PerEffectsBuffer_t	*peffect;

	SaveClientData ();

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	memset (str, 0, sizeof(str));
	strcpy (str, __DATE__);
	fwrite (str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite (&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i=0 ; i<game.maxclients ; i++)
		WriteClient (f, &game.clients[i]);

	SaveScripts(f, true);

	// this is a bit bogus - search through the client effects and kill all the FX_PLAYER_EFFECTS before saving, since they will be re-created
	// upon players re-joining the game after a load anyway.
	peffect = (PerEffectsBuffer_t*) gi.Persistant_Effects_Array;
	for (i=0; i<MAX_PERSISTANT_EFFECTS; i++, peffect++)
	{
		if (peffect->fx_num == FX_PLAYER_PERSISTANT)
			peffect->numEffects = 0;
	}

	// save all the current persistant effects
	fwrite (gi.Persistant_Effects_Array, (sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS), 1, f);

	fclose (f);

	// this is a bit bogus - search through the client effects and renable all FX_PLAYER_EFFECTS
	peffect = (PerEffectsBuffer_t*) gi.Persistant_Effects_Array;
	for (i=0; i<MAX_PERSISTANT_EFFECTS; i++, peffect++)
	{
		if (peffect->fx_num == FX_PLAYER_PERSISTANT)
			peffect->numEffects = 1;
	}


}

void ReadGame (char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];


	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	fread (str, sizeof(str), 1, f);
	if (strcmp (str, __DATE__))
	{
		fclose (f);
		gi.error ("Savegame from an older version.\n");
		return;
	}

	gi.FreeTags (TAG_GAME);

	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread (&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i=0 ; i<game.maxclients ; i++)
		ReadClient (f, &game.clients[i]);

	LoadScripts(f, true);

	fclose (f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field=savefields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=savefields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals (FILE *f)
{
	field_t		*field;
	level_locals_t		temp;
	cvar_t *r_farclipdist;
	cvar_t *r_fog;
	cvar_t *r_fog_density;
	int			i;

	// set up some console vars as level save variables
	r_farclipdist = gi.cvar("r_farclipdist", FAR_CLIP_DIST, 0);
	level.far_clip_dist_f = r_farclipdist->value;
	r_fog = Cvar_Get ("r_fog", "0", 0);
	level.fog = r_fog->value;
	r_fog_density = Cvar_Get ("r_fog_density", "0", 0);
	level.fog_density = r_fog_density->value;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field=levelfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	for (i = 0; i< level.active_buoys; i++)
	{
		// change the pointers to lengths or indexes
		for (field=bouyfields ; field->name ; field++)
		{
			WriteField1 (f, field, (byte *)&temp.buoy_list[i]);
		}
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=levelfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)&level);
	}

	for (i = 0; i< level.active_buoys; i++)
	{
		// change the pointers to lengths or indexes
		for (field=bouyfields ; field->name ; field++)
		{
			WriteField2 (f, field, (byte *)&level.buoy_list[i]);
		}
	}

}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	SinglyLinkedList_t msgs;
	char *temp;
	void *s;

	if(ent->s.clientEffects.buf)
	{
		temp = ent->s.clientEffects.buf; // buffer needs to be stored to be cleared by the engine
	}
	else
	{
		temp = NULL;
	}

	msgs = ent->msgQ.msgs;

	s=ent->Script;
	fread (ent, sizeof(*ent), 1, f);
	ent->Script=s;

	ent->s.clientEffects.buf = temp;

	ent->msgQ.msgs = msgs;
	ent->last_alert = NULL;

/*
	// Only clients need skeletons - these are set up when all else is done. -MW.

	if(ent->s.skeletalType != SKEL_NULL)
	{
		CreateSkeleton(ent->s.skeletalType);
	}
*/
	for (field=savefields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)ent);
	}
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals (FILE *f)
{
	field_t		*field;
	char		temp[20];
	int			i;

	fread (&level, sizeof(level), 1, f);

	for (field=levelfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)&level);
	}

	for (i = 0; i< level.active_buoys; i++)
	{
		// change the pointers to lengths or indexes
		for (field=bouyfields ; field->name ; field++)
		{
			ReadField (f, field, (byte *)&level.buoy_list[i]);
		}
	}


	// set those console vars we should 
	sprintf(temp, "%f", level.far_clip_dist_f);
	gi.cvar_set("r_farclipdist", temp);
	sprintf(temp, "%f", level.fog);
	gi.cvar_set("r_fog", temp);
	sprintf(temp, "%f", level.fog_density);
	gi.cvar_set("r_fog_density", temp);

	// these are pointers and should be reset.
	level.alert_entity = NULL;
	level.last_alert = NULL;
	for (i=0; i<MAX_ALERT_ENTS; i++)
	{
		level.alertents[i].inuse = false;
		level.alertents[i].prev_alert = NULL;
		level.alertents[i].next_alert = NULL;
	}
}


/*
=================
WriteLevel

=================
*/
void WriteLevel (char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
	void	*base;
	PerEffectsBuffer_t	*peffect;

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite (&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = (void *)InitGame;
	fwrite (&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals (f);

	// write out all the configstrings
//	fwrite (sv.configstrings, sizeof(sv.configstrings), 1, f);

	// write out all the entities
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];
	
		// we don't want to not save player entities, even if they are not in use, since when we go from
		// level to a level we've already been to, there maybe monsters that are targeting the player,
		// and they have problems if they are targeted at a player that has no data in them, even if the player is
		// not inuse.
		if (!ent->inuse && !ent->client)
			continue;

		fwrite (&i, sizeof(i), 1, f);
		WriteEdict (f, ent);
	}
	i = -1;
	fwrite (&i, sizeof(i), 1, f);

	SaveScripts(f, false);

	// this is a bit bogus - search through the client effects and kill all the FX_PLAYER_EFFECTS before saving, since they will be re-created
	// upon players re-joining the game after a load anyway.
	peffect = (PerEffectsBuffer_t*) gi.Persistant_Effects_Array;
	for (i=0; i<MAX_PERSISTANT_EFFECTS; i++, peffect++)
	{
		if (peffect->fx_num == FX_PLAYER_PERSISTANT)
			peffect->numEffects = 0;
	}

	// save all the current persistant effects
	fwrite (gi.Persistant_Effects_Array, (sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS), 1, f);

	fclose (f);

	// this is a bit bogus - search through the client effects and renable all FX_PLAYER_EFFECTS
	peffect = (PerEffectsBuffer_t*) gi.Persistant_Effects_Array;
	for (i=0; i<MAX_PERSISTANT_EFFECTS; i++, peffect++)
	{
		if (peffect->fx_num == FX_PLAYER_PERSISTANT)
			peffect->numEffects = 1;

	} 
}


/*
=================
ReadLevel

SpawnEntities will allready have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.
=================
*/
void ReadLevel (char *filename)
{
	void ClearMessageQueues();

	int		entnum;
	FILE	*f;
	int		i;
	void	*base;
	edict_t	*ent;

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

//	ClearMessageQueues();

	// Free any dynamic memory allocated by loading the level base state.

	gi.FreeTags (TAG_LEVEL);

	// Wipe all the entities.

	memset (g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));

	globals.num_edicts = maxclients->value+1;

	// Check edict size.

	fread (&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose (f);
		gi.error ("ReadLevel: mismatched edict size");
	}

	fread (&base, sizeof(base), 1, f);
	if (base != (void *)InitGame)
	{
		fclose (f);
		gi.error ("ReadLevel: function pointers have moved - file was saved on different version.");
	}

	// Load the level locals.

	ReadLevelLocals (f);

	// Load all the entities.

	while (1)
	{
		if (fread (&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose (f);
			gi.error ("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum+1;

		ent = &g_edicts[entnum];
		ReadEdict (f, ent);

		// Let the server rebuild world links for this ent.

		ent->last_alert = NULL;
		memset (&ent->area, 0, sizeof(ent->area));

		// NOTE NOTE 
		// Missiles must be linked in specially.  G_LinkMissile links as a SOLID_NOT, even though the entity is SOLID_BBOX
		if (ent->movetype == MOVETYPE_FLYMISSILE && ent->solid == SOLID_BBOX)
		{	
			G_LinkMissile (ent);
		}
		else
		{
			gi.linkentity (ent);
		}
		
		// Force the monsters just loaded to point at the right anim.

		if((ent->classID > 0) && (!Cid_init[ent->classID]) && (ent->classID < NUM_CLASSIDS))	 	// Need to call once per level that item is on
		{
			classStaticsInits[ent->classID]();	
			Cid_init[ent->classID] = -1;
		}

		if ( ((ent->classname) && (*ent->classname)) && strcmp(ent->classname, "player") && ent->classID && classStatics[ent->classID].resInfo && ent->curAnimID)
			SetAnim(ent, ent->curAnimID);
	}

	LoadScripts(f, false);

  	// Load up all the persistant effects and fire them off.

	fread (gi.Persistant_Effects_Array, (sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS), 1, f);
	gi.ClearPersistantEffects();

	fclose (f);

	// Mark all clients as unconnected.

	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = &g_edicts[i+1];
		ent->client = game.clients + i;
		ent->client->playerinfo.pers.connected = false;
		InitPlayerinfo(ent);
		SetupPlayerinfo(ent);
		P_PlayerBasicAnimReset(&ent->client->playerinfo);
	}

	// Do any load time things at this point.

	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// Fire any cross-level triggers.

		if (ent->classname)
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;
	}
}

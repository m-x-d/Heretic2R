#include "g_local.h"
#include "g_Skeletons.h"
#include "ArrayedList.h"
#include "PrimitiveDisplayHack.h"
#include "q_Physics.h"
#include "q_shared.h"

void ED_CallSpawn (edict_t *ent);
void Cvar_SetValue (char *var_name, float value);

extern char *dm_statusbar;
extern char *single_statusbar;


typedef struct
{
	char	*level_name;
	int		default_preset;
} eax_level_info_t;

#define MAX_CURRENT_LEVELS	29

eax_level_info_t	eax_level_info[MAX_CURRENT_LEVELS] = 
{
	{"ssdocks",			EAX_CITY_AND_SEWERS},
	{"sswarehouse",		EAX_CITY_AND_SEWERS},
	{"sstown",			EAX_CITY_AND_ALLEYS},
	{"sspalace",		EAX_CITY_AND_ALLEYS},
	{"dmireswamp",		EAX_CITY_AND_SEWERS},
	{"andhealer",		EAX_CITY_AND_SEWERS},
	{"andplaza",		EAX_CITY_AND_SEWERS},
	{"andslums",		EAX_CITY_AND_SEWERS},
	{"andacademic",		EAX_CITY_AND_SEWERS},
	{"kellcaves",		EAX_ALL_STONE},
	{"canyon",			EAX_ALL_STONE},
	{"hive1",			EAX_ALL_STONE},
	{"hive2",			EAX_ALL_STONE},
	{"gauntlet",		EAX_ALL_STONE},
	{"hivetrialpit",	EAX_ARENA},
	{"hivepriestess",	EAX_ALL_STONE},
	{"oglemine1",		EAX_ALL_STONE},
	{"oglemine2",		EAX_ALL_STONE},
	{"dungeon",			EAX_CITY_AND_ALLEYS},
	{"cloudhub",		EAX_CITY_AND_ALLEYS},
	{"cloudlabs",		EAX_CITY_AND_ALLEYS},
	{"cloudquarters",	EAX_CITY_AND_ALLEYS},
	{"cloudsanctum",	EAX_CITY_AND_ALLEYS},
	{"tutorial",		EAX_CITY_AND_ALLEYS},
	{"tutorial2",		EAX_CITY_AND_ALLEYS},
	{"dmandoria",		EAX_CITY_AND_ALLEYS},
	{"dmhive",			EAX_ALL_STONE},
	{"dmcanyon",		EAX_ALL_STONE},
	{"dmcloud",			EAX_CITY_AND_ALLEYS},
};


/*
=============
ED_NewString
=============
*/
char *ED_NewString (char *string)
{
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = gi.TagMalloc (l, TAG_LEVEL);

	new_p = newb;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}
	
	return newb;
}

/*
===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============
*/
void ED_ParseField (char *key, char *value, edict_t *ent)
{
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;
	int color[4];

	for (f=fields ; f->name ; f++)
	{
		if (!Q_stricmp(f->name, key))
		{	// found it
			if (f->flags & FFL_SPAWNTEMP)
				b = (byte *)&st;
			else
				b = (byte *)ent;

			switch (f->type)
			{
			case F_LSTRING:
				*(char **)(b+f->ofs) = ED_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			case F_IGNORE:
				break;
			case F_RGBA:
				sscanf(value, "%i %i %i %i", color, color + 1, color + 2, color + 3);
				((byte *)(b+f->ofs))[0] = color[0];
				((byte *)(b+f->ofs))[1] = color[1];
				((byte *)(b+f->ofs))[2] = color[2];
				((byte *)(b+f->ofs))[3] = color[3];
				break;
			case F_RGB:
				sscanf(value, "%i %i %i", color, color + 1, color + 2);
				((byte *)(b+f->ofs))[0] = color[0];
				((byte *)(b+f->ofs))[1] = color[1];
				((byte *)(b+f->ofs))[2] = color[2];
				break;
			}
			return;
		}
	}
#ifdef _DEVEL
	gi.dprintf ("%s is not a field\n", key);
#endif
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
char *ED_ParseEdict (char *data, edict_t *ent)
{
	qboolean	init;
	char		keyname[256];
	char		*com_token;

	init = false;
	memset (&st, 0, sizeof(st));

// go through all the dictionary pairs
	while (1)
	{	
	// parse key
		com_token = COM_Parse (&data);
		if (com_token[0] == '}')
			break;
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		strncpy (keyname, com_token, sizeof(keyname)-1);
		
	// parse value	
		com_token = COM_Parse (&data);
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			gi.error ("ED_ParseEntity: closing brace without data");

		init = true;	

	// keynames with a leading underscore are used for utility comments,
	// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		ED_ParseField (keyname, com_token, ent);
	}

	if (!init)
		memset (ent, 0, sizeof(*ent));

	return data;
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams (void)
{
	edict_t	*e, *e2, *chain;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for (i=1, e=g_edicts+i ; i < globals.num_edicts ; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		chain = e;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < globals.num_edicts ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

#ifdef _DEVEL
	gi.dprintf ("%i teams with %i entities\n", c, c2);
#endif
}

void ConstructEntities(void)
{
	edict_t *ent;
	int		i;

	// Create message queues for entites.

	for(i = 0, ent = g_edicts; i < maxentities->value ; i++, ent++)
	{
		SLList_DefaultCon(&ent->msgQ.msgs);

#ifdef BBOX_DISPLAY_HACK
		InitBBoxDisplayInfo(i, ent->mins, ent->maxs);
#endif

		ent->s.skeletalType = SKEL_NULL;
	}

	// Allocate skeletons for clients only.

	for (i=0 ; i<game.maxclients ; i++)
	{
		edict_t *ent;
		ent=&globals.edicts[1+i];
		ent->s.skeletalType=SKEL_CORVUS;
		ent->s.rootJoint = CreateSkeleton(ent->s.skeletalType);
	}

	game.entitiesSpawned = true;
}

void CheckCoopTimeout(qboolean BeenHereBefore)
{
	// Reset to zero cooptimeout if we've already been to the current level (no cinematic to see).

	if(BeenHereBefore)
		Cvar_SetValue("sv_cooptimeout",0);
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
int loadingBaseEnts;

void SpawnEntities (char *mapname, char *entities, char *spawnpoint, qboolean loadgame)
{
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	float		skill_level;

	loadingBaseEnts = loadgame;

	skill_level = floor (skill->value);
	if (skill_level < 0)
		skill_level = 0;
	if (skill_level > 3)
		skill_level = 3;
	if (skill->value != skill_level)
		gi.cvar_forceset("skill", va("%f", skill_level));

	SaveClientData ();

	ShutdownScripts(false);
	gi.FreeTags (TAG_LEVEL);

	memset (&level, 0, sizeof(level));
	memset (g_edicts, 0, game.maxentities * sizeof (g_edicts[0]));

	memset(skeletalJoints, 0, sizeof(skeletalJoints));
	memset(jointNodes, 0, sizeof(jointNodes));
	memset(classStatics, 0, sizeof(classStatics));
	memset(Cid_init, 0, sizeof(Cid_init));

	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);
	strncpy (game.spawnpoint, spawnpoint, sizeof(game.spawnpoint)-1);

	// Set client fields on player ents.

	for (i=0 ; i<game.maxclients ; i++)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;

	while (1)
	{
		// parse the opening brace	
		com_token = COM_Parse (&entities);
		if (!entities)
			break;
		if (com_token[0] != '{')
			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if (!ent)
			ent = g_edicts;
		else
			ent = G_Spawn ();
		entities = ED_ParseEdict (entities, ent);
		
		// remove things (except the world) from different skill levels or deathmatch
		if (ent != g_edicts)
		{
			if (deathmatch->value)
			{
				if ( ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
				{
					G_FreeEdict (ent);	
					inhibit++;
					continue;
				}
			}
			else
			{
				if (((coop->value) && (ent->spawnflags & SPAWNFLAG_NOT_COOP)) ||
					((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					((skill->value >= 2) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
					)
					{
						G_FreeEdict (ent);	
						inhibit++;
						continue;
					}
			}
			
			// Check if it's a monster and if we're nomonster here...
			if (sv_nomonsters && sv_nomonsters->value && strstr(ent->classname, "monster_"))
			{
#ifdef _DEVEL
				gi.dprintf("monster '%s' not spawned.\n", ent->classname);
#endif
				G_FreeEdict (ent);
				inhibit++;
				continue;
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
	}	

#ifdef _DEVEL
	gi.dprintf ("%i entities inhibited\n", inhibit);
#endif
	G_FindTeams ();
}

//===================================================================

/*QUAKED worldspawn (0 0 0) ? NOBODIES

Only used for the world.

NOBODIES - In DM, no bodies will be left behind by players- for maps with large amounts of visibility

"sky"			environment map name:

	andoria
	desert
	hive
	sky1	- Night Sky
	storm
	swamp
	town

"skyaxis"		vector axis for rotating sky
"skyrotate"		speed of rotation in degrees/second
"sounds"		music cd track number
"gravity"		800 is default gravity
"message"		text to print at user logon
"skinnum"		plague level for corvus: 0-2
"cooptimeout"	time to wait (in seconds) for all clients to have joined a map in coop (default is 0).
"scale" 	EAX environment type for this map.

 0 EAX_GENERIC,
 1 EAX_ALL_STONE,
 2 EAX_ARENA,
 3 EAX_CITY_AND_SEWERS,
 4 EAX_CITY_AND_ALLEYS,
 5 EAX_FOREST,
 6 EAX_PSYCHOTIC,

"offensive"		starting offensive weapons (flag bits): 

  1		- swordstaff
  2		- fireball
  4		- hellstaff
  8		- magic missile array
  16	- red-rain bow
  32	- sphere of annihlation
  64	- phoenix bow
  128	- mace balls
  256	- firewall

"defensive"		starting defensive weapons (flag bits):

  1		- ring of repulsion
  2		- lightning shield
  4		- teleport
  8		- morph ovum
  16	- meteor barrier

*/

void SP_worldspawn (edict_t *ent)
{
	int	i;

	ent->movetype = PHYSICSTYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true;			// since the world doesn't use G_Spawn()
	ent->s.modelindex = 1;		// world model is always index 1

	//---------------

	// Reserve some spots for dead player bodies.

	InitBodyQue ();
	
	if((ent->spawnflags & 1) && (deathmatch->value || coop->value))
		level.body_que = -1;

	// Set configstrings for items.

	SetItemNames ();

	if (st.nextmap)
		strcpy (level.nextmap, st.nextmap);

	// Make some data visible to the server.

	if (ent->message && ent->message[0])
	{
		gi.configstring (CS_LEVEL_NUMBER, ent->message );
		gi.configstring (CS_NAME, message_text[atoi(ent->message)].string);
		strncpy (level.level_name, ent->message, sizeof(level.level_name));
		gi.dprintf("Unique Level Index : %d\n", atoi(ent->message));
	}
	else
	{
		if(ent->text_msg)
			gi.configstring (CS_NAME, ent->text_msg);
		strncpy (level.level_name, level.mapname, sizeof(level.level_name));
		gi.dprintf("Warning : No Unique Level Index\n");
	}

	// this is a tremendous hack, but given the state of the code at this point, there is no other way to do this.
	for (i=0 ; i<MAX_CURRENT_LEVELS;i++)
	{
		// search through all the currently defined world maps, looking for names, so we can set
		// the EAX default sound type for this level.
		if (!stricmp(eax_level_info[i].level_name,  level.mapname))
		{
			Cvar_SetValue("EAX_default", (float)eax_level_info[i].default_preset);	
			break;
		}
	}
	
	// if we didn't find it in the current level list, lets just set it to generic
	if (i == MAX_CURRENT_LEVELS)
		Cvar_SetValue("EAX_default", ent->s.scale);
	// just in case
	ent->s.scale = 0;

	if (st.sky && st.sky[0])
		gi.configstring (CS_SKY, st.sky);
	else
		gi.configstring (CS_SKY, "desert");

	gi.configstring (CS_SKYROTATE, va("%f", st.skyrotate) );

	gi.configstring (CS_SKYAXIS, va("%f %f %f",
		st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]) );

	gi.configstring (CS_CDTRACK, va("%i", ent->sounds) );

	gi.configstring (CS_MAXCLIENTS, va("%i", (int)(maxclients->value) ) );

	// Status bar program.

	if (deathmatch->value)
		gi.configstring (CS_STATUSBAR, dm_statusbar);
	else
		gi.configstring (CS_STATUSBAR, single_statusbar);

	// Starting weapons for players entering a coop game.

	level.offensive_weapons=(!st.offensive)?0:st.offensive;
	level.defensive_weapons=(!st.defensive)?0:st.defensive;

	// Save away cooptimeout so it is accessible to the server (SV_) functions.

	Cvar_SetValue("sv_cooptimeout",(!st.cooptimeout)?0:st.cooptimeout);	

	//---------------

	// GRAVITY for all games.

	if (!st.gravity)
		gi.cvar_set("sv_gravity", "675");
	else
		gi.cvar_set("sv_gravity", st.gravity);

	// FRICTION for all games.

	sv_friction = gi.cvar ("sv_friction", FRICTION_STRING, 0);

	//
	// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
	//

	// 0 normal
	gi.configstring(CS_LIGHTS+0, "m");
	
	// 1 FLICKER (first variety)
	gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
	
	// 2 SLOW STRONG PULSE
	gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	
	// 3 CANDLE (first variety)
	gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	
	// 4 FAST STROBE
	gi.configstring(CS_LIGHTS+4, "mamamamamama");
	
	// 5 GENTLE PULSE 1
	gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	
	// 6 FLICKER (second variety)
	gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
	
	// 7 CANDLE (second variety)
	gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
	
	// 8 CANDLE (third variety)
	gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
	// 9 SLOW STROBE (fourth variety)
	gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
	
	// 10 FLUORESCENT FLICKER
	gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
	
	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	gi.configstring(CS_LIGHTS+63, "a");
}

// end
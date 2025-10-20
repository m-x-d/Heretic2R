//
// g_spawnf.c
//
// Copyright 1998 Raven Software
//

#include "g_spawnf.h" //mxd
#include "g_items.h" //mxd
#include "g_save.h" //mxd
#include "g_Skeletons.h"
#include "p_client.h" //mxd
#include "p_hud.h" //mxd
#include "q_Physics.h"
#include "sc_Main.h" //mxd
#include "qcommon.h"
#include "g_local.h"

typedef struct
{
	char* level_name;
	int default_preset;
} eax_level_info_t;

#define MAX_CURRENT_LEVELS	29

static eax_level_info_t eax_level_info[MAX_CURRENT_LEVELS] =
{
	{ "ssdocks",		EAX_CITY_AND_SEWERS },
	{ "sswarehouse",	EAX_CITY_AND_SEWERS },
	{ "sstown",			EAX_CITY_AND_ALLEYS },
	{ "sspalace",		EAX_CITY_AND_ALLEYS },
	{ "dmireswamp",		EAX_CITY_AND_SEWERS },
	{ "andhealer",		EAX_CITY_AND_SEWERS },
	{ "andplaza",		EAX_CITY_AND_SEWERS },
	{ "andslums",		EAX_CITY_AND_SEWERS },
	{ "andacademic",	EAX_CITY_AND_SEWERS },
	{ "kellcaves",		EAX_ALL_STONE },
	{ "canyon",			EAX_ALL_STONE },
	{ "hive1",			EAX_ALL_STONE },
	{ "hive2",			EAX_ALL_STONE },
	{ "gauntlet",		EAX_ALL_STONE },
	{ "hivetrialpit",	EAX_ARENA },
	{ "hivepriestess",	EAX_ALL_STONE },
	{ "oglemine1",		EAX_ALL_STONE },
	{ "oglemine2",		EAX_ALL_STONE },
	{ "dungeon",		EAX_CITY_AND_ALLEYS },
	{ "cloudhub",		EAX_CITY_AND_ALLEYS },
	{ "cloudlabs",		EAX_CITY_AND_ALLEYS },
	{ "cloudquarters",	EAX_CITY_AND_ALLEYS },
	{ "cloudsanctum",	EAX_CITY_AND_ALLEYS },
	{ "tutorial",		EAX_CITY_AND_ALLEYS },
	{ "tutorial2",		EAX_CITY_AND_ALLEYS },
	{ "dmandoria",		EAX_CITY_AND_ALLEYS },
	{ "dmhive",			EAX_ALL_STONE },
	{ "dmcanyon",		EAX_ALL_STONE },
	{ "dmcloud",		EAX_CITY_AND_ALLEYS },
};

char* ED_NewString(const char* string)
{
	const int len = (int)strlen(string) + 1;
	char* buffer = gi.TagMalloc(len, TAG_LEVEL);
	char* buf_p = buffer;

	for (int i = 0; i < len; i++)
	{
		if (i < len - 1 && string[i] == '\\')
		{
			i++;

			if (string[i] == 'n')
				*buf_p++ = '\n';
			else
				*buf_p++ = '\\';
		}
		else
		{
			*buf_p++ = string[i];
		}
	}

	return buffer;
}

// Takes a key/value pair and sets the binary values in an edict.
static void ED_ParseField(char* key, const char* value, edict_t* ent)
{
	for (const field_t* f = &fields[0]; f->name != NULL; f++)
	{
		if (Q_stricmp(f->name, key) != 0)
			continue;

		// Found it.
		byte* b = ((f->flags & FFL_SPAWNTEMP) ? (byte*)&st : (byte*)ent);

		switch (f->type)
		{
			case F_LSTRING:
				*(char**)(b + f->ofs) = ED_NewString(value);
				break;

			case F_VECTOR:
			{
				vec3_t vec;
				sscanf_s(value, "%f %f %f", &vec[0], &vec[1], &vec[2]); //mxd. sscanf -> sscanf_s
				((float*)(b + f->ofs))[0] = vec[0];
				((float*)(b + f->ofs))[1] = vec[1];
				((float*)(b + f->ofs))[2] = vec[2];
			} break;

			case F_INT:
				*(int*)(b + f->ofs) = Q_atoi(value);
				break;

			case F_FLOAT:
				*(float*)(b + f->ofs) = (float)strtod(value, NULL); //mxd. atof -> strtod
				break;

			case F_ANGLEHACK:
			{
				const float v = (float)strtod(value, NULL); //mxd. atof -> strtod
				((float*)(b + f->ofs))[0] = 0.0f;
				((float*)(b + f->ofs))[1] = v;
				((float*)(b + f->ofs))[2] = 0.0f;
			} break;

			case F_RGBA:
			{
				int color[4];
				sscanf_s(value, "%i %i %i %i", color, color + 1, color + 2, color + 3); //mxd. sscanf -> sscanf_s
				((byte*)(b + f->ofs))[0] = (byte)color[0];
				((byte*)(b + f->ofs))[1] = (byte)color[1];
				((byte*)(b + f->ofs))[2] = (byte)color[2];
				((byte*)(b + f->ofs))[3] = (byte)color[3];
			} break;

			case F_RGB:
			{
				int color[3];
				sscanf_s(value, "%i %i %i", color, color + 1, color + 2); //mxd. sscanf -> sscanf_s
				((byte*)(b + f->ofs))[0] = (byte)color[0];
				((byte*)(b + f->ofs))[1] = (byte)color[1];
				((byte*)(b + f->ofs))[2] = (byte)color[2];
			} break;

			default:
				break;
		}

		return;
	}

	gi.dprintf("%s is not a field\n", key);
}

// Parses an edict out of the given string, returning the new position. 'ent' should be a properly initialized empty edict.
static char* ED_ParseEdict(char* data, edict_t* ent)
{
	qboolean ent_initialized = false;
	memset(&st, 0, sizeof(st));

	// Go through all the dictionary pairs.
	while (true)
	{
		// Parse key.
		const char* com_token = COM_Parse(&data);
		if (com_token[0] == '}')
			break;

		if (data == NULL)
			gi.error("ED_ParseEntity: EOF without closing brace");

		char keyname[256];
		strncpy_s(keyname, sizeof(keyname), com_token, sizeof(keyname) - 1); //mxd. strncpy -> strncpy_s

		// Parse value.
		com_token = COM_Parse(&data);

		if (data == NULL)
			gi.error("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			gi.error("ED_ParseEntity: closing brace without data");

		ent_initialized = true;

		// Keynames with a leading underscore are used for utility comments, and are immediately discarded.
		if (keyname[0] != '_')
			ED_ParseField(keyname, com_token, ent);
	}

	if (!ent_initialized)
		memset(ent, 0, sizeof(*ent));

	return data;
}

// Chain together all entities with a matching team field.
// All but the first will have the FL_TEAMSLAVE flag set.
// All but the last will have the teamchain field set to the next one.
static void G_FindTeams(void)
{
	int num_teams = 0;
	int num_ents = 0;

	edict_t* e = &g_edicts[1];
	for (int i = 1; i < globals.num_edicts; i++, e++)
	{
		if (!e->inuse || e->team == NULL || (e->flags & FL_TEAMSLAVE))
			continue;

		edict_t* chain = e;
		e->teammaster = e;

		num_teams++;
		num_ents++;

		edict_t* e2 = &g_edicts[i + 1];
		for (int j = i + 1; j < globals.num_edicts; j++, e2++)
		{
			if (!e2->inuse || e2->team == NULL || (e2->flags & FL_TEAMSLAVE))
				continue;

			if (strcmp(e->team, e2->team) == 0)
			{
				num_ents++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	gi.dprintf("%i teams with %i entities\n", num_teams, num_ents);
}

void ConstructEntities(void)
{
	// Create message queues for entities.
	edict_t* ent = &g_edicts[0];
	for (int i = 0; i < (int)maxentities->value; i++, ent++)
	{
		SLList_DefaultCon(&ent->msgQ.msgs);
		ent->s.skeletalType = SKEL_NULL;
	}

	// Allocate skeletons for clients only.
	for (int i = 0; i < game.maxclients; i++)
	{
		edict_t* cl = &globals.edicts[i + 1];
		cl->s.skeletalType = SKEL_CORVUS;
		cl->s.rootJoint = (short)CreateSkeleton(cl->s.skeletalType);
	}

	game.entitiesSpawned = true;
}

void CheckCoopTimeout(const qboolean been_here_before)
{
	// Reset to zero cooptimeout if we've already been to the current level (no cinematic to see).
	if (been_here_before)
		Cvar_SetValue("sv_cooptimeout", 0.0f);
}

// Creates a server's entity / program execution context by parsing textual entity definitions out of an ent file.
void SpawnEntities(const char* map_name, char* entities, const char* spawn_point, qboolean loadgame) //TODO: remove unused 'loadgame' arg?
{
	float skill_level = floorf(skill->value);
	skill_level = Clamp(skill_level, SKILL_EASY, SKILL_VERYHARD);

	if (skill->value != skill_level)
		gi.cvar_forceset("skill", va("%f", skill_level));

	SaveClientData();

	ShutdownScripts(false);
	gi.FreeTags(TAG_LEVEL);

	memset(&level, 0, sizeof(level));
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));

	memset(skeletalJoints, 0, sizeof(skeletalJoints));
	memset(jointNodes, 0, sizeof(jointNodes));
	memset(classStatics, 0, sizeof(classStatics));
	memset(classStaticsInitialized, false, sizeof(classStaticsInitialized));

	strncpy_s(level.mapname, sizeof(level.mapname), map_name, sizeof(level.mapname) - 1); //mxd. strncpy -> strncpy_s
	strncpy_s(game.spawnpoint, sizeof(game.spawnpoint), spawn_point, sizeof(game.spawnpoint) - 1); //mxd. strncpy -> strncpy_s

	// Set client fields on player ents.
	for (int i = 0; i < game.maxclients; i++)
		g_edicts[i + 1].client = &game.clients[i];

	edict_t* ent = NULL;
	int num_inhibited = 0;

	while (true)
	{
		// Parse the opening brace.
		char* com_token = COM_Parse(&entities);
		if (entities == NULL)
			break;

		if (com_token[0] != '{')
			gi.error("ED_LoadFromFile: found %s when expecting {", com_token);

		if (ent == NULL)
			ent = &g_edicts[0];
		else
			ent = G_Spawn();

		entities = ED_ParseEdict(entities, ent);

		// Remove things (except the world) from different skill levels or deathmatch.
		if (ent != world)
		{
			if (DEATHMATCH)
			{
				if (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
				{
					G_FreeEdict(ent);
					num_inhibited++;

					continue;
				}
			}
			else
			{
				if ((COOP && (ent->spawnflags & SPAWNFLAG_NOT_COOP)) ||
					(SKILL == SKILL_EASY && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					(SKILL == SKILL_MEDIUM && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					(SKILL >= SKILL_HARD && (ent->spawnflags & SPAWNFLAG_NOT_HARD)))
				{
					G_FreeEdict(ent);
					num_inhibited++;

					continue;
				}
			}

			// Check if it's a monster and if we're nomonster here...
			if ((int)sv_nomonsters->value && strstr(ent->classname, "monster_"))
			{
				G_FreeEdict(ent);
				num_inhibited++;

				continue;
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY | SPAWNFLAG_NOT_MEDIUM | SPAWNFLAG_NOT_HARD | SPAWNFLAG_NOT_COOP | SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn(ent);
	}

	memset(&st, 0, sizeof(st)); // YQ2 BUGFIX: in case the last entity in the entstring has spawntemp fields.
	gi.dprintf("%i entities inhibited\n", num_inhibited);
	G_FindTeams();
}

#pragma region ========================== worldspawn ==========================

#define SF_NOBODIES	1 //mxd

// QUAKED worldspawn (0 0 0) ? NOBODIES
// Only used for the world.

// Spawnflags:
// NOBODIES - In DM, no bodies will be left behind by players - for maps with large amounts of visibility.

// Variables:
// sky			- Environment map name: andoria, desert, hive, sky1 (night sky), storm, swamp, town.
// skyaxis		- Vector axis for rotating sky.
// skyrotate	- Speed of rotation in degrees/second.
// sounds		- Music cd track number.
// gravity		- 800 is default gravity.
// message		- Text to print at user logon.
// skinnum		- Plague level for corvus: 0-2.
// cooptimeout	- Time to wait (in seconds) for all clients to have joined a map in coop (default is 0).
// scale		- EAX environment type for this map:
//		0 EAX_GENERIC,
//		1 EAX_ALL_STONE,
//		2 EAX_ARENA,
//		3 EAX_CITY_AND_SEWERS,
//		4 EAX_CITY_AND_ALLEYS,
//		5 EAX_FOREST,
//		6 EAX_PSYCHOTIC,
// offensive	- Starting offensive weapons (flag bits):
//		1	- Swordstaff.
//		2	- Fireball.
//		4	- Hellstaff.
//		8	- Magic missile array.
//		16	- Red-rain bow.
//		32	- Sphere of annihilation.
//		64	- Phoenix bow.
//		128	- Mace balls.
//		256	- Firewall.
// defensive	- Starting defensive weapons (flag bits):
//		1	- Ring of repulsion.
//		2	- Lightning shield.
//		4	- Teleport.
//		8	- Morph ovum.
//		16	- Meteor barrier.
void SP_worldspawn(edict_t* ent)
{
	ent->movetype = PHYSICSTYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true; // Since the world doesn't use G_Spawn().
	ent->s.modelindex = 1; // World model is always index 1.

	// Reserve some spots for dead player bodies.
	InitBodyQue();

	if ((ent->spawnflags & SF_NOBODIES) && (DEATHMATCH || COOP))
		level.body_que = -1;

	// Set configstrings for items.
	SetItemNames();

	if (st.nextmap != NULL)
		strcpy_s(level.nextmap, sizeof(level.nextmap), st.nextmap); //mxd. strcpy -> strcpy_s

	// Make some data visible to the server.
	if (ent->message != NULL && ent->message[0] != 0)
	{
		gi.configstring(CS_LEVEL_NUMBER, ent->message);
		gi.configstring(CS_NAME, message_text[Q_atoi(ent->message)].string);

		strncpy_s(level.level_name, sizeof(level.level_name), ent->message, sizeof(level.level_name)); //mxd. strcpy -> strcpy_s
		gi.dprintf("Unique Level Index : %d\n", Q_atoi(ent->message));
	}
	else
	{
		if (ent->text_msg != NULL)
			gi.configstring(CS_NAME, ent->text_msg);

		strncpy_s(level.level_name, sizeof(level.level_name), level.mapname, sizeof(level.level_name)); //mxd. strcpy -> strcpy_s
		gi.dprintf("Warning : No Unique Level Index\n");
	}

	// This is a tremendous hack, but given the state of the code at this point, there is no other way to do this.
	int eax_preset_index;
	for (eax_preset_index = 0; eax_preset_index < MAX_CURRENT_LEVELS; eax_preset_index++)
	{
		// Search through all the currently defined world maps, looking for names, so we can set the EAX default sound type for this level.
		if (Q_stricmp(eax_level_info[eax_preset_index].level_name, level.mapname) == 0)
		{
			Cvar_SetValue("EAX_default", (float)eax_level_info[eax_preset_index].default_preset);
			break;
		}
	}

	// If we didn't find it in the current level list, lets just set it to generic.
	if (eax_preset_index == MAX_CURRENT_LEVELS)
		Cvar_SetValue("EAX_default", ent->s.scale);

	// Just in case.
	ent->s.scale = 0.0f;

	if (st.sky != NULL && st.sky[0] != 0)
		gi.configstring(CS_SKY, st.sky);
	else
		gi.configstring(CS_SKY, "desert");

	gi.configstring(CS_SKYROTATE, va("%f", st.skyrotate));
	gi.configstring(CS_SKYAXIS, va("%f %f %f", st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]));
	gi.configstring(CS_CDTRACK, va("%i", ent->sounds));
	gi.configstring(CS_MAXCLIENTS, va("%i", MAXCLIENTS));

	// Status bar program.
	gi.configstring(CS_STATUSBAR, (DEATHMATCH ? dm_statusbar : single_statusbar));

	// Starting weapons for players entering a coop game.
	level.offensive_weapons = st.offensive;
	level.defensive_weapons = st.defensive;

	// Save away cooptimeout so it is accessible to the server (SV_) functions.
	Cvar_SetValue("sv_cooptimeout", (float)st.cooptimeout);

	// Gravity for all games.
	gi.cvar_set("sv_gravity", (st.gravity != NULL ? st.gravity : GRAVITY_STRING)); //mxd. Use define.

	// Friction for all games.
	sv_friction = gi.cvar("sv_friction", FRICTION_STRING, 0);

	// Setup light animation tables. 'a' is total darkness, 'z' is double-bright.
	gi.configstring(CS_LIGHTS + 0, "m");													// 0 - normal.
	gi.configstring(CS_LIGHTS + 1, "mmnmmommommnonmmonqnmmo");								// 1 FLICKER (first variety).
	gi.configstring(CS_LIGHTS + 2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");	// 2 SLOW STRONG PULSE.
	gi.configstring(CS_LIGHTS + 3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");					// 3 CANDLE (first variety).
	gi.configstring(CS_LIGHTS + 4, "mamamamamama");											// 4 FAST STROBE.
	gi.configstring(CS_LIGHTS + 5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");					// 5 GENTLE PULSE 1.
	gi.configstring(CS_LIGHTS + 6, "nmonqnmomnmomomno");									// 6 FLICKER (second variety).
	gi.configstring(CS_LIGHTS + 7, "mmmaaaabcdefgmmmmaaaammmaamm");							// 7 CANDLE (second variety).
	gi.configstring(CS_LIGHTS + 8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");			// 8 CANDLE (third variety).
	gi.configstring(CS_LIGHTS + 9, "aaaaaaaazzzzzzzz");										// 9 SLOW STROBE (fourth variety).
	gi.configstring(CS_LIGHTS + 10, "mmamammmmammamamaaamammma");							// 10 FLUORESCENT FLICKER.
	gi.configstring(CS_LIGHTS + 11, "abcdefghijklmnopqrrqponmlkjihgfedcba");				// 11 SLOW PULSE NOT FADE TO BLACK.

	// Styles 32-62 are assigned by the light program for switchable lights.
	gi.configstring(CS_LIGHTS + 63, "a"); // 63 testing.
}

#pragma endregion
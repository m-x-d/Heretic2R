//
// g_save.c
//
// Copyright 1998 Raven Software
//

#include "g_save.h" //mxd
#include "g_items.h" //mxd
#include "g_playstats.h"
#include "g_ResourceManagers.h" //mxd
#include "p_client.h" //mxd
#include "p_dll.h" //mxd
#include "p_view.h" //mxd
#include "q_ClientServer.h"
#include "q_Physics.h"
#include "FX.h"
#include "Message.h"
#include "Utilities.h"
#include "g_local.h"

#define MAX_MESSAGESTRINGS	1000

#pragma region ========================== SAVE FIELDS ==========================

const field_t fields[] =
{
	{ "classname",		FOFS(classname),	F_LSTRING,	0 },
	{ "origin",			FOFS(s.origin),		F_VECTOR,	0 },
	{ "model",			FOFS(model),		F_LSTRING,	0 },
	{ "spawnflags",		FOFS(spawnflags),	F_INT,		0 },
	{ "speed",			FOFS(speed),		F_FLOAT,	0 },
	{ "accel",			FOFS(accel),		F_FLOAT,	0 },
	{ "decel",			FOFS(decel),		F_FLOAT,	0 },
	{ "target",			FOFS(target),		F_LSTRING,	0 },
	{ "targetname",		FOFS(targetname),	F_LSTRING,	0 },
	{ "scripttarget",	FOFS(scripttarget),	F_LSTRING,	0 },
	{ "pathtarget",		FOFS(pathtarget),	F_LSTRING,	0 },
	{ "jumptarget",		FOFS(jumptarget),	F_LSTRING,	0 },
	{ "deathtarget",	FOFS(deathtarget),	F_LSTRING,	0 },
	{ "killtarget",		FOFS(killtarget),	F_LSTRING,	0 },
	{ "combattarget",	FOFS(combattarget),	F_LSTRING,	0 },
	{ "message",		FOFS(message),		F_LSTRING,	0 },
	{ "text_msg",		FOFS(text_msg),		F_LSTRING,	0 },
	{ "team",			FOFS(team),			F_LSTRING,	0 },
	{ "wait",			FOFS(wait),			F_FLOAT,	0 },
	{ "delay",			FOFS(delay),		F_FLOAT,	0 },
	{ "time",			FOFS(time),			F_FLOAT,	0 },
	{ "random",			FOFS(random),		F_FLOAT,	0 },
	{ "style",			FOFS(style),		F_INT,		0 },
	{ "count",			FOFS(count),		F_INT,		0 },
	{ "health",			FOFS(health),		F_INT,		0 },
	{ "skinnum",		FOFS(s.skinnum),	F_INT,		0 },
	{ "sounds",			FOFS(sounds),		F_INT,		0 },
	{ "light",			0,					F_IGNORE,	0 },
	{ "dmg",			FOFS(dmg),			F_INT,		0 },
	{ "angles",			FOFS(s.angles),		F_VECTOR,	0 },
	{ "angle",			FOFS(s.angles),		F_ANGLEHACK,0 },
	{ "mass",			FOFS(mass),			F_INT,		0 },
	{ "volume",			FOFS(volume),		F_FLOAT,	0 },
	{ "attenuation",	FOFS(attenuation),	F_FLOAT,	0 },
	{ "map",			FOFS(map),			F_LSTRING,	0 },
	{ "materialtype",	FOFS(materialtype),	F_INT,		0 },
	{ "scale",			FOFS(s.scale),		F_FLOAT,	0 },
	{ "color",			FOFS(s.color),		F_RGBA,		0 },
	{ "absLight",		FOFS(s.absLight),	F_RGB,		0 },
	{ "frame",			FOFS(s.frame),		F_INT,		0 },
	{ "mintel",			FOFS(mintel),		F_INT,		0 },
	{ "melee_range",	FOFS(melee_range),	F_FLOAT,	0 },
	{ "missile_range",			FOFS(missile_range),			F_FLOAT,	0 },
	{ "min_missile_range",		FOFS(min_missile_range),		F_FLOAT,	0 },
	{ "bypass_missile_chance",	FOFS(bypass_missile_chance),	F_INT,		0 },
	{ "jump_chance",			FOFS(jump_chance),				F_INT,		0 },
	{ "wakeup_distance",		FOFS(wakeup_distance),			F_FLOAT,	0 },
	{ "c_mode",			FOFS(monsterinfo.c_mode),	F_INT,		0 }, //BUGFIX, kinda: 'F_INT, F_INT' in original logic.
	{ "homebuoy",		FOFS(homebuoy),				F_LSTRING,	0 },
	{ "wakeup_target",	FOFS(wakeup_target),		F_LSTRING,	0 },
	{ "pain_target",	FOFS(pain_target),			F_LSTRING,	0 },

	// Temporary spawn vars - only valid when the spawn function is called.
	{ "lip",			STOFS(lip),				F_INT,		FFL_SPAWNTEMP },
	{ "distance",		STOFS(distance),		F_INT,		FFL_SPAWNTEMP },
	{ "height",			STOFS(height),			F_INT,		FFL_SPAWNTEMP },
	{ "noise",			STOFS(noise),			F_LSTRING,	FFL_SPAWNTEMP },
	{ "pausetime",		STOFS(pausetime),		F_FLOAT,	FFL_SPAWNTEMP },
	{ "item",			STOFS(item),			F_LSTRING,	FFL_SPAWNTEMP },
	{ "gravity",		STOFS(gravity),			F_LSTRING,	FFL_SPAWNTEMP },
	{ "sky",			STOFS(sky),				F_LSTRING,	FFL_SPAWNTEMP },
	{ "skyrotate",		STOFS(skyrotate),		F_FLOAT,	FFL_SPAWNTEMP },
	{ "skyaxis",		STOFS(skyaxis),			F_VECTOR,	FFL_SPAWNTEMP },
	{ "minyaw",			STOFS(minyaw),			F_FLOAT,	FFL_SPAWNTEMP },
	{ "maxyaw",			STOFS(maxyaw),			F_FLOAT,	FFL_SPAWNTEMP },
	{ "minpitch",		STOFS(minpitch),		F_FLOAT,	FFL_SPAWNTEMP },
	{ "maxpitch",		STOFS(maxpitch),		F_FLOAT,	FFL_SPAWNTEMP },
	{ "nextmap",		STOFS(nextmap),			F_LSTRING,	FFL_SPAWNTEMP },
	{ "rotate",			STOFS(rotate),			F_INT,		FFL_SPAWNTEMP },
	{ "target2",		FOFS(target2),			F_LSTRING,	0 },
	{ "pathtargetname",	FOFS(pathtargetname),	F_LSTRING,	0 },
	{ "zangle",			STOFS(zangle),			F_FLOAT,	FFL_SPAWNTEMP },
	{ "file",			STOFS(file),			F_LSTRING,	FFL_SPAWNTEMP },
	{ "radius",			STOFS(radius),			F_INT,		FFL_SPAWNTEMP },
	{ "offensive",		STOFS(offensive),		F_INT,		FFL_SPAWNTEMP },
	{ "defensive",		STOFS(defensive),		F_INT,		FFL_SPAWNTEMP },
	{ "spawnflags2",	STOFS(spawnflags2),		F_INT,		FFL_SPAWNTEMP },
	{ "cooptimeout",	STOFS(cooptimeout),		F_INT,		FFL_SPAWNTEMP },

	{ "script",			STOFS(script),			F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm1",			STOFS(parms[0]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm2",			STOFS(parms[1]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm3",			STOFS(parms[2]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm4",			STOFS(parms[3]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm5",			STOFS(parms[4]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm6",			STOFS(parms[5]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm7",			STOFS(parms[6]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm8",			STOFS(parms[7]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm9",			STOFS(parms[8]),		F_LSTRING,	FFL_SPAWNTEMP },
	{ "parm10",			STOFS(parms[9]),		F_LSTRING,	FFL_SPAWNTEMP },

	{ NULL, 0,	F_INT,	0 } //BUGFIX: mxd. Added terminator field.
};

// -------- Just for savegames ----------
// All pointer fields should be listed here, or savegames won't work properly (they will crash and burn).
// This wasn't just tacked on to the fields array, because these don't need names, we wouldn't want map fields
// using some of these, and if one were accidentally present twice, it would double swizzle (fuck) the pointer.
static const field_t savefields[] =
{
	{ "", FOFS(classname),			F_LSTRING,	0 },
	{ "", FOFS(target),				F_LSTRING,	0 },
	{ "", FOFS(target2),			F_LSTRING,	0 },
	{ "", FOFS(targetname),			F_LSTRING,	0 },
	{ "", FOFS(scripttarget),		F_LSTRING,	0 },
	{ "", FOFS(killtarget),			F_LSTRING,	0 },
	{ "", FOFS(team),				F_LSTRING,	0 },
	{ "", FOFS(pathtarget),			F_LSTRING,	0 },
	{ "", FOFS(deathtarget),		F_LSTRING,	0 },
	{ "", FOFS(combattarget),		F_LSTRING,	0 },
	{ "", FOFS(model),				F_LSTRING,	0 },
	{ "", FOFS(map),				F_LSTRING,	0 },
	{ "", FOFS(message),			F_LSTRING,	0 },
	{ "", FOFS(client),				F_CLIENT,	0 },
	{ "", FOFS(item),				F_ITEM,		0 },
	{ "", FOFS(goalentity),			F_EDICT,	0 },
	{ "", FOFS(movetarget),			F_EDICT,	0 },
	{ "", FOFS(enemy),				F_EDICT,	0 },
	{ "", FOFS(oldenemy),			F_EDICT,	0 },
	{ "", FOFS(activator),			F_EDICT,	0 },
	{ "", FOFS(groundentity),		F_EDICT,	0 },
	{ "", FOFS(teamchain),			F_EDICT,	0 },
	{ "", FOFS(teammaster),			F_EDICT,	0 },
	{ "", FOFS(owner),				F_EDICT,	0 },
	{ "", FOFS(mynoise),			F_EDICT,	0 },
	{ "", FOFS(mynoise2),			F_EDICT,	0 },
	{ "", FOFS(target_ent),			F_EDICT,	0 },
	{ "", FOFS(chain),				F_EDICT,	0 },
	{ "", FOFS(blockingEntity),		F_EDICT,	0 },
	{ "", FOFS(last_buoyed_enemy),	F_EDICT,	0 },
	{ "", FOFS(placeholder),		F_EDICT,	0 },
	{ "", FOFS(fire_damage_enemy),	F_EDICT,	0 },

	{ NULL, 0, F_INT, 0 }
};

static const field_t levelfields[] =
{
	{ "", LLOFS(changemap),		F_LSTRING,	0 },
	{ "", LLOFS(sight_client),	F_EDICT,	0 },
	{ "", LLOFS(sight_entity),	F_EDICT,	0 },

	{ NULL, 0, F_INT, 0 }
};

static const field_t bouyfields[] =
{
	{ "", BYOFS(pathtarget),	F_LSTRING,	0 },
	{ "", BYOFS(target),		F_LSTRING,	0 },
	{ "", BYOFS(targetname),	F_LSTRING,	0 },
	{ "", BYOFS(jump_target),	F_LSTRING,	0 },

	{ NULL, 0, F_INT, 0 }
};

static const field_t clientfields[] =
{
	{ "", CLOFS(playerinfo.pers.weapon),		F_ITEM,	0 },
	{ "", CLOFS(playerinfo.pers.lastweapon),	F_ITEM,	0 },
	{ "", CLOFS(playerinfo.pers.defence),		F_ITEM,	0 },
	{ "", CLOFS(playerinfo.pers.lastdefence),	F_ITEM,	0 },
	{ "", CLOFS(playerinfo.pers.newweapon),		F_ITEM,	0 },

	{ NULL, 0, F_INT, 0 }
};

#pragma endregion

trig_message_t message_text[MAX_MESSAGESTRINGS];
static char* message_buf; //mxd. Made local.

static int LoadTextFile(char* name, char** addr)
{
	char* buffer;
	const int length = gi.FS_LoadFile(name, (void**)&buffer);

	if (length <= 0)
	{
		Sys_Error("Unable to load %s", name);
		return 0;
	}

	*addr = (char*)gi.TagMalloc(length + 1, 0);
	memcpy(*addr, buffer, length);
	*(*addr + length) = 0;
	gi.FS_FreeFile(buffer);

	return length + 1;
}

static void LoadStrings(void)
{
	const int length = LoadTextFile("levelmsg.txt", &message_buf);

	char* start_ptr = message_buf;
	char* p = NULL;

	for (int i = 1; p < message_buf + length; i++)
	{
		if (i > MAX_MESSAGESTRINGS)
		{
			Com_Printf("Too many strings\n");
			return;
		}

		// Read in string up to return.
		char* return_ptr = strchr(start_ptr, '\r');

		if (return_ptr == NULL) // At end of file.
			break;

		*return_ptr = 0;

		// Search string for #
		p = strchr(start_ptr, '#');	// Search for '#', which signifies a wav file.

		if (p != NULL && p < return_ptr)
		{
			*p = 0;
			message_text[i].wav = ++p; // Save stuff after '#'.
		}

		// Save stuff before #
		message_text[i].string = start_ptr;

		do
		{
			p = strchr(start_ptr, '@'); // Replace '@' markers with newlines.
			if (p != NULL)
				*p = '\n';
		} while (p != NULL);

		return_ptr += 2; // Skip '\r\n'. 
		start_ptr = return_ptr; // Advance to next string.
	}
}

void FreeLevelMessagesBuffer(void) //mxd
{
	gi.FS_FreeFile(message_buf);
}

// This will be called when the dll is first loaded, which only happens when a new game is begun.
void InitGame(void)
{
	gi.dprintf("==== InitGame ====\n");

	G_InitResourceManagers();

	//FIXME: sv_ prefix is wrong for these.
	sv_maxvelocity = gi.cvar("sv_maxvelocity", MAX_VELOCITY_STRING, 0);
	sv_gravity = gi.cvar("sv_gravity", GRAVITY_STRING, 0); //mxd. Needs to be inited here, otherwise SetupPlayerinfo() will crash. Inited again in SP_worldspawn().
	//sv_friction = gi.cvar("sv_friction", FRICTION_STRING, 0); //mxd. Inited again in SP_worldspawn().

	// Noset vars.
	dedicated = gi.cvar("dedicated", "0", CVAR_NOSET);

	// Latched vars.
	sv_cheats = gi.cvar("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamedate", __DATE__, CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar("coop", "0", CVAR_LATCH);

	skill = gi.cvar("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar("maxentities", G_MAX_ENTITIES, CVAR_LATCH);

	sv_nomonsters = gi.cvar("nomonsters", "0", CVAR_SERVERINFO | CVAR_LATCH);
	sv_freezemonsters = gi.cvar("freezemonsters", "0", 0);

	// Change anytime vars.
	dmflags = gi.cvar("dmflags", "0", CVAR_SERVERINFO);
	advancedstaff = gi.cvar("advancedstaff", "1", CVAR_SERVERINFO);

	fraglimit = gi.cvar("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar("password", "", CVAR_USERINFO);
	filterban = gi.cvar("filterban", "1", 0);

	checkanim = gi.cvar("checkanim", "0", 0);
	allowillegalskins = gi.cvar("allowillegalskins", "0", CVAR_ARCHIVE);

	deactivate_buoys = gi.cvar("deactivate_buoys", "0", 0);
	anarchy = gi.cvar("anarchy", "0", 0);
	impact_damage = gi.cvar("impact_damage", "1", 0);
	cheating_monsters = gi.cvar("cheating_monsters", "1", 0);
	singing_ogles = gi.cvar("singing_ogles", "0", 0);
	no_runshrine = gi.cvar("no_runshrine", "0", 0);
	no_tornado = gi.cvar("no_tornado", "0", 0);
	no_teleport = gi.cvar("no_teleport", "0", 0);
	no_phoenix = gi.cvar("no_phoenix", "0", 0);
	no_irondoom = gi.cvar("no_irondoom", "0", 0);
	no_morph = gi.cvar("no_morph", "0", 0);
	no_shield = gi.cvar("no_shield", "0", 0);

	flood_msgs = gi.cvar("flood_msgs", "4", 0);
	flood_persecond = gi.cvar("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar("flood_waitdelay", "10", 0);
	flood_killdelay = gi.cvar("flood_killdelay", "10", 0);
	sv_maplist = gi.cvar("sv_maplist", "", 0);

	player_dll = Cvar_Get("player_dll", DEFAULT_PLAYER_LIB, 0);

	sv_cinematicfreeze = gi.cvar("sv_cinematicfreeze", "0", 0);
	sv_jumpcinematic = gi.cvar("sv_jumpcinematic", "0", 0);
	log_file_name = gi.cvar("log_file_name", "", CVAR_ARCHIVE);
	log_file_footer = gi.cvar("log_file_footer", "", CVAR_ARCHIVE);
	log_file_header = gi.cvar("log_file_header", "", CVAR_ARCHIVE);
	log_file_line_header = gi.cvar("log_file_line_header", "", CVAR_ARCHIVE);

	blood_level = gi.cvar("blood_level", VIOLENCE_DEFAULT_STR, CVAR_ARCHIVE);
	dm_no_bodies = gi.cvar("dm_no_bodies", "0", CVAR_ARCHIVE);

	gi.cvar("flash_screen", "1", 0);

	P_Load(player_dll->string);

	// Initialise the inventory items.
	G_InitItems(); // Server-side only elements.

	// Initialize all entities for this game.
	game.maxentities = (int)maxentities->value;
	g_edicts = gi.TagMalloc(game.maxentities * (int)sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// Initialize all clients for this game.
	game.maxclients = MAXCLIENTS;
	game.clients = gi.TagMalloc(game.maxclients * (int)sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients + 1;

	LoadStrings();
}

#pragma region ========================== FIELDS IO ==========================

static void ConvertField(const field_t* field, byte* base) //mxd. Named 'WriteField1' in original logic.
{
	int len;
	int index;

	void* p = &base[field->ofs];

	switch (field->type)
	{
		case F_INT:
		case F_FLOAT:
		case F_ANGLEHACK:
		case F_VECTOR:
		case F_IGNORE:
			break;

		case F_LSTRING: // Convert string pointer to string length.
		case F_GSTRING:
			if (*(char**)p != NULL)
				len = (int)strlen(*(char**)p) + 1;
			else
				len = 0;
			*(int*)p = len;
			break;

		case F_EDICT: // Convert edict pointer to edict index.
			if (*(edict_t**)p != NULL)
				index = *(edict_t**)p - g_edicts;
			else
				index = -1;
			*(int*)p = index;
			break;

		case F_CLIENT: // Convert client pointer to client index.
			if (*(gclient_t**)p != NULL)
				index = *(gclient_t**)p - game.clients;
			else
				index = -1;
			*(int*)p = index;
			break;

		case F_ITEM: // Convert item pointer to item index.
			if (*(edict_t**)p != NULL)
				index = *(gitem_t**)p - playerExport.p_itemlist;
			else
				index = -1;
			*(int*)p = index;
			break;

		default:
			gi.error("WriteEdict: unknown field type");
			break;
	}
}

static void WriteField(FILE* f, const field_t* field, byte* base) //mxd. Named 'WriteField2' in original logic.
{
	void* p = &base[field->ofs];

	if ((field->type == F_LSTRING || field->type == F_GSTRING) && *(char**)p != NULL)
	{
		const int len = (int)strlen(*(char**)p) + 1;
		fwrite(*(char**)p, len, 1, f);
	}
}

static void ReadField(FILE* f, const field_t* field, byte* base)
{
	int len;
	int index;

	void* p = &base[field->ofs];

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
			len = *(int*)p;
			if (len == 0)
			{
				*(char**)p = NULL;
			}
			else
			{
				const int tag = (field->type == F_LSTRING ? TAG_LEVEL : TAG_GAME); //mxd
				*(char**)p = gi.TagMalloc(len, tag);
				fread(*(char**)p, len, 1, f);
			}
			break;

		case F_EDICT:
			index = *(int*)p;
			if (index == -1)
				*(edict_t**)p = NULL;
			else
				*(edict_t**)p = &g_edicts[index];
			break;

		case F_CLIENT:
			index = *(int*)p;
			if (index == -1)
				*(gclient_t**)p = NULL;
			else
				*(gclient_t**)p = &game.clients[index];
			break;

		case F_ITEM:
			index = *(int*)p;
			if (index == -1)
				*(gitem_t**)p = NULL;
			else
				*(gitem_t**)p = &playerExport.p_itemlist[index];
			break;

		default:
			gi.error("ReadEdict: unknown field type");
			break;
	}
}

#pragma endregion

#pragma region ========================== CLIENT IO ==========================

// All pointer variables (except function pointers) must be handled specially.
static void WriteClient(FILE* f, gclient_t* client)
{
	// All of the ints, floats, and vectors stay as they are.
	gclient_t temp = *client;

	// Change the pointers to lengths or indexes.
	for (const field_t* field = clientfields; field->name != NULL; field++)
		ConvertField(field, (byte*)&temp);

	// Write the block.
	fwrite(&temp, sizeof(temp), 1, f);

	// Now write any allocated data following the edict.
	for (const field_t* field = clientfields; field->name != NULL; field++)
		WriteField(f, field, (byte*)client);
}

// All pointer variables (except function pointers) must be handled specially.
static void ReadClient(FILE* f, gclient_t* client)
{
	fread(client, sizeof(*client), 1, f);

	for (const field_t* field = clientfields; field->name != NULL; field++)
		ReadField(f, field, (byte*)client);
}

#pragma endregion

#pragma region ========================== GAME IO ==========================

// This will be called whenever the game goes to a new level and when the user explicitly saves the game.
// Game information include cross-level data, like multi-level triggers, and all client states.
// A single player death will automatically restore from the last save position.
void WriteGame(char* filename, const qboolean autosave)
{
	SaveClientData();

	FILE* f;
	if (fopen_s(&f, filename, "wb") != 0) //mxd. fopen -> fopen_s
	{
		gi.error("Couldn't open %s", filename);
		return;
	}

	char str[16] = { 0 };
	strcpy_s(str, sizeof(str), __DATE__); //mxd. strcpy -> strcpy_s
	fwrite(str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite(&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (int i = 0; i < game.maxclients; i++)
		WriteClient(f, &game.clients[i]);

	SaveScripts(f, true);

	// This is a bit bogus - search through the client effects and kill all the FX_PLAYER_EFFECTS before saving,
	// since they will be re-created upon players re-joining the game after a load anyway.
	PerEffectsBuffer_t* fx_buf = gi.Persistant_Effects_Array;
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, fx_buf++)
		if (fx_buf->fx_num == FX_PLAYER_PERSISTANT)
			fx_buf->numEffects = 0;

	// Save all the current persistent effects.
	fwrite(gi.Persistant_Effects_Array, (sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS), 1, f);
	fclose(f);

	// This is a bit bogus - search through the client effects and re-enable all FX_PLAYER_EFFECTS.
	fx_buf = (PerEffectsBuffer_t*)gi.Persistant_Effects_Array;
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, fx_buf++)
		if (fx_buf->fx_num == FX_PLAYER_PERSISTANT)
			fx_buf->numEffects = 1;
}

void ReadGame(char* filename)
{
	FILE* f;
	if (fopen_s(&f, filename, "rb") != 0) //mxd. fopen -> fopen_s
	{
		gi.error("Couldn't open %s", filename);
		return;
	}

	char str[16];
	fread(str, sizeof(str), 1, f);

	if (strcmp(str, __DATE__) != 0)
	{
		fclose(f);
		gi.error("Savegame from an older version (expected '%s', got '%s').\n", __DATE__, str);

		return;
	}

	gi.FreeTags(TAG_GAME);

	g_edicts = gi.TagMalloc(game.maxentities * (int)sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread(&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc(game.maxclients * (int)sizeof(game.clients[0]), TAG_GAME);

	for (int i = 0; i < game.maxclients; i++)
		ReadClient(f, &game.clients[i]);

	LoadScripts(f, true);

	fclose(f);
}

#pragma endregion

#pragma region ========================== EDICT IO ==========================

// All pointer variables (except function pointers) must be handled specially.
static void WriteEdict(FILE* f, edict_t* ent)
{
	// All of the ints, floats, and vectors stay as they are.
	edict_t temp = *ent;

	// Change the pointers to lengths or indexes.
	for (const field_t* field = savefields; field->name != NULL; field++)
		ConvertField(field, (byte*)&temp);

	// Write the block.
	fwrite(&temp, sizeof(temp), 1, f);

	// Now write any allocated data following the edict.
	for (const field_t* field = savefields; field->name != NULL; field++)
		WriteField(f, field, (byte*)ent);
}

// All pointer variables (except function pointers) must be handled specially.
static void ReadEdict(FILE* f, edict_t* ent)
{
	byte* fx_buf = ent->s.clientEffects.buf; // Buffer needs to be stored to be cleared by the engine.
	const SinglyLinkedList_t msgs = ent->msgQ.msgs;
	void* script = ent->Script;

	fread(ent, sizeof(*ent), 1, f);

	ent->Script = script;
	ent->s.clientEffects.buf = fx_buf;
	ent->msgQ.msgs = msgs;
	ent->last_alert = NULL;

	for (const field_t* field = savefields; field->name != NULL; field++)
		ReadField(f, field, (byte*)ent);
}

#pragma endregion

#pragma region ========================== LEVEL LOCALS IO ==========================

// All pointer variables (except function pointers) must be handled specially.
static void WriteLevelLocals(FILE* f)
{
	// Set up some console vars as level save variables.
	const cvar_t* r_farclipdist = gi.cvar("r_farclipdist", FAR_CLIP_DIST, 0);
	level.far_clip_dist_f = r_farclipdist->value;

	const cvar_t* r_fog = Cvar_Get("r_fog", "0", 0);
	level.fog = r_fog->value;

	const cvar_t* r_fog_density = Cvar_Get("r_fog_density", "0", 0);
	level.fog_density = r_fog_density->value;

	// All of the ints, floats, and vectors stay as they are.
	level_locals_t temp = level;

	// Change the pointers to lengths or indexes.
	for (const field_t* field = levelfields; field->name != NULL; field++)
		ConvertField(field, (byte*)&temp);

	for (int i = 0; i < level.active_buoys; i++)
		for (const field_t* field = bouyfields; field->name != NULL; field++)
			ConvertField(field, (byte*)&temp.buoy_list[i]);

	// Write the block.
	fwrite(&temp, sizeof(temp), 1, f);

	// Now write any allocated data following the edict.
	for (const field_t* field = levelfields; field->name != NULL; field++)
		WriteField(f, field, (byte*)&level);

	for (int i = 0; i < level.active_buoys; i++)
		for (const field_t* field = bouyfields; field->name != NULL; field++)
			WriteField(f, field, (byte*)&level.buoy_list[i]);
}

// All pointer variables (except function pointers) must be handled specially.
static void ReadLevelLocals(FILE* f)
{
	fread(&level, sizeof(level), 1, f);

	// Change the pointers to lengths or indexes.
	for (const field_t* field = levelfields; field->name != NULL; field++)
		ReadField(f, field, (byte*)&level);

	for (int i = 0; i < level.active_buoys; i++)
		for (const field_t* field = bouyfields; field->name != NULL; field++)
			ReadField(f, field, (byte*)&level.buoy_list[i]);

	// Set those console vars we should.
	char temp[20];
	sprintf_s(temp, sizeof(temp), "%f", level.far_clip_dist_f); //mxd. sprintf -> sprintf_s
	gi.cvar_set("r_farclipdist", temp);

	sprintf_s(temp, sizeof(temp), "%f", level.fog); //mxd. sprintf -> sprintf_s
	gi.cvar_set("r_fog", temp);

	sprintf_s(temp, sizeof(temp), "%f", level.fog_density); //mxd. sprintf -> sprintf_s
	gi.cvar_set("r_fog_density", temp);

	// These are pointers and should be reset.
	level.alert_entity = NULL;
	level.last_alert = NULL;

	for (int i = 0; i < MAX_ALERT_ENTS; i++)
	{
		level.alertents[i].inuse = false;
		level.alertents[i].prev_alert = NULL;
		level.alertents[i].next_alert = NULL;
	}
}

#pragma endregion

#pragma region ========================== LEVEL IO ==========================

void WriteLevel(char* filename)
{
	FILE* f;
	if (fopen_s(&f, filename, "wb") != 0) //mxd. fopen -> fopen_s
	{
		gi.error("Couldn't open %s", filename);
		return;
	}

	// Write out edict size for checking.
	const int ed_size = sizeof(edict_t);
	fwrite(&ed_size, sizeof(ed_size), 1, f);

	// Write out a function pointer for checking.
	void* base = (void*)InitGame;
	fwrite(&base, sizeof(base), 1, f);

	// Write out level_locals_t.
	WriteLevelLocals(f);

	// Write out all the entities.
	for (int i = 0; i < globals.num_edicts; i++)
	{
		edict_t* ent = &g_edicts[i];

		// We want to save player entities, even if they are not in use, since when we go from level to a level we've already been to,
		// there may be monsters that are targeting the player, and they have problems if they are targeted at a player
		// that has no data in them, even if the player is not inuse.
		if (ent->inuse || ent->client != NULL)
		{
			fwrite(&i, sizeof(i), 1, f);
			WriteEdict(f, ent);
		}
	}

	const int last = -1; //mxd
	fwrite(&last, sizeof(last), 1, f);

	SaveScripts(f, false);

	// This is a bit bogus - search through the client effects and kill all the FX_PLAYER_EFFECTS before saving,
	// since they will be re-created upon players re-joining the game after a load anyway.
	PerEffectsBuffer_t* fx_buf = gi.Persistant_Effects_Array;
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, fx_buf++)
		if (fx_buf->fx_num == FX_PLAYER_PERSISTANT)
			fx_buf->numEffects = 0;

	// Save all the current persistent effects.
	fwrite(gi.Persistant_Effects_Array, (sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS), 1, f);
	fclose(f);

	// This is a bit bogus - search through the client effects and re-enable all FX_PLAYER_EFFECTS.
	fx_buf = (PerEffectsBuffer_t*)gi.Persistant_Effects_Array;
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, fx_buf++)
		if (fx_buf->fx_num == FX_PLAYER_PERSISTANT)
			fx_buf->numEffects = 1;
}

// SpawnEntities will already have been called on the level the same way it was when the level was saved.
// That is necessary to get the baselines set up identically.
// The server will have cleared all of the world links before calling ReadLevel.
void ReadLevel(char* filename)
{
	FILE* f;
	if (fopen_s(&f, filename, "rb") != 0) //mxd. fopen -> fopen_s
	{
		gi.error("Couldn't open %s", filename);
		return;
	}

	// Free any dynamic memory allocated by loading the level base state.
	gi.FreeTags(TAG_LEVEL);

	// Wipe all the entities.
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));

	globals.num_edicts = MAXCLIENTS + 1;

	// Check edict size.
	int ed_size;
	fread(&ed_size, sizeof(ed_size), 1, f);

	if (ed_size != sizeof(edict_t))
	{
		fclose(f);
		gi.error("ReadLevel: mismatched edict size");

		return;
	}

	void* base;
	fread(&base, sizeof(base), 1, f);

	if (base != (void*)InitGame)
	{
		fclose(f);
		gi.error("ReadLevel: function pointers have moved - file was saved on different version.");

		return;
	}

	// Load the level locals.
	ReadLevelLocals(f);

	// Load all the entities.
	while (true)
	{
		int ent_num;
		if (fread(&ent_num, sizeof(ent_num), 1, f) != 1)
		{
			fclose(f);
			gi.error("ReadLevel: failed to read entnum");

			return;
		}

		if (ent_num == -1)
			break;

		if (ent_num >= globals.num_edicts)
			globals.num_edicts = ent_num + 1;

		edict_t* ent = &g_edicts[ent_num];
		ReadEdict(f, ent);

		// Let the server rebuild world links for this ent.
		ent->last_alert = NULL;
		memset(&ent->area, 0, sizeof(ent->area));

		//NOTE: missiles must be linked in specially. G_LinkMissile links as a SOLID_NOT, even though the entity is SOLID_BBOX.
		if (ent->movetype == MOVETYPE_FLYMISSILE && ent->solid == SOLID_BBOX)
			G_LinkMissile(ent);
		else
			gi.linkentity(ent);

		// Force the monsters just loaded to point at the right anim.
		if (ent->classID > CID_NONE && ent->classID < NUM_CLASSIDS && !classStaticsInitialized[ent->classID]) // Need to call once per level that item is on.
		{
			classStaticsInits[ent->classID]();
			classStaticsInitialized[ent->classID] = true;
		}

		//TODO: ent->curAnimID 0 is NOT 'no animation'! Do we need this check?
		if (ent->classname != NULL && strcmp(ent->classname, "player") != 0 && ent->classID != CID_NONE && classStatics[ent->classID].resInfo != NULL && ent->curAnimID > 0)
			SetAnim(ent, ent->curAnimID);
	}

	LoadScripts(f, false);

	// Load up all the persistent effects and fire them off.
	fread(gi.Persistant_Effects_Array, sizeof(PerEffectsBuffer_t) * MAX_PERSISTANT_EFFECTS, 1, f);
	gi.ClearPersistantEffects();

	fclose(f);

	// Mark all clients as unconnected.
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		edict_t* cl = &g_edicts[i + 1];
		cl->client = &game.clients[i];
		cl->client->playerinfo.pers.connected = false;

		InitPlayerinfo(cl);
		SetupPlayerinfo(cl);
		P_PlayerBasicAnimReset(&cl->client->playerinfo);
	}

	// Do any load-time things at this point.
	for (int i = 0; i < globals.num_edicts; i++)
	{
		edict_t* ent = &g_edicts[i];

		// Fire any cross-level triggers. //TODO: does H2 use cross-level triggers?
		if (ent->inuse && ent->classname != NULL && strcmp(ent->classname, "target_crosslevel_target") == 0)
			ent->nextthink = level.time + ent->delay;
	}
}

#pragma endregion
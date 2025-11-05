//
// g_save.c
//
// Copyright 1998 Raven Software
//

#include "g_save.h" //mxd
#include "g_main.h" //mxd
#include "g_playstats.h"
#include "p_client.h" //mxd
#include "p_dll.h" //mxd
#include "p_view.h" //mxd
#include "q_Physics.h"
#include "sc_Main.h" //mxd
#include "FX.h"
#include "Message.h"
#include "Utilities.h"
#include "g_local.h"

#pragma region ========================== SAVE FIELDS ==========================

const field_t fields[] =
{
	{ "classname",		FOFS(classname),	F_LSTRING,	FFL_NONE },
	{ "origin",			FOFS(s.origin),		F_VECTOR,	FFL_NONE },
	{ "model",			FOFS(model),		F_LSTRING,	FFL_NONE },
	{ "spawnflags",		FOFS(spawnflags),	F_INT,		FFL_NONE },
	{ "speed",			FOFS(speed),		F_FLOAT,	FFL_NONE },
	{ "accel",			FOFS(accel),		F_FLOAT,	FFL_NONE },
	{ "decel",			FOFS(decel),		F_FLOAT,	FFL_NONE },
	{ "target",			FOFS(target),		F_LSTRING,	FFL_NONE },
	{ "targetname",		FOFS(targetname),	F_LSTRING,	FFL_NONE },
	{ "scripttarget",	FOFS(scripttarget),	F_LSTRING,	FFL_NONE },
	{ "pathtarget",		FOFS(pathtarget),	F_LSTRING,	FFL_NONE },
	{ "jumptarget",		FOFS(jumptarget),	F_LSTRING,	FFL_NONE },
	{ "deathtarget",	FOFS(deathtarget),	F_LSTRING,	FFL_NONE },
	{ "killtarget",		FOFS(killtarget),	F_LSTRING,	FFL_NONE },
	{ "combattarget",	FOFS(combattarget),	F_LSTRING,	FFL_NONE },
	{ "message",		FOFS(message),		F_LSTRING,	FFL_NONE },
	{ "text_msg",		FOFS(text_msg),		F_LSTRING,	FFL_NONE },
	{ "team",			FOFS(team),			F_LSTRING,	FFL_NONE },
	{ "wait",			FOFS(wait),			F_FLOAT,	FFL_NONE },
	{ "delay",			FOFS(delay),		F_FLOAT,	FFL_NONE },
	{ "time",			FOFS(time),			F_FLOAT,	FFL_NONE },
	{ "random",			FOFS(random),		F_FLOAT,	FFL_NONE },
	{ "style",			FOFS(style),		F_INT,		FFL_NONE },
	{ "count",			FOFS(count),		F_INT,		FFL_NONE },
	{ "health",			FOFS(health),		F_INT,		FFL_NONE },
	{ "skinnum",		FOFS(s.skinnum),	F_INT,		FFL_NONE },
	{ "sounds",			FOFS(sounds),		F_INT,		FFL_NONE },
	{ "light",			0,					F_IGNORE,	FFL_NONE },
	{ "dmg",			FOFS(dmg),			F_INT,		FFL_NONE },
	{ "angles",			FOFS(s.angles),		F_VECTOR,	FFL_NONE },
	{ "angle",			FOFS(s.angles),		F_ANGLEHACK,FFL_NONE },
	{ "mass",			FOFS(mass),			F_INT,		FFL_NONE },
	{ "volume",			FOFS(volume),		F_FLOAT,	FFL_NONE },
	{ "attenuation",	FOFS(attenuation),	F_FLOAT,	FFL_NONE },
	{ "map",			FOFS(map),			F_LSTRING,	FFL_NONE },
	{ "materialtype",	FOFS(materialtype),	F_INT,		FFL_NONE },
	{ "scale",			FOFS(s.scale),		F_FLOAT,	FFL_NONE },
	{ "color",			FOFS(s.color),		F_RGBA,		FFL_NONE },
	{ "absLight",		FOFS(s.absLight),	F_RGB,		FFL_NONE },
	{ "frame",			FOFS(s.frame),		F_INT,		FFL_NONE },
	{ "mintel",			FOFS(mintel),		F_INT,		FFL_NONE },
	{ "melee_range",	FOFS(melee_range),	F_FLOAT,	FFL_NONE },
	{ "missile_range",			FOFS(missile_range),			F_FLOAT,	FFL_NONE },
	{ "min_missile_range",		FOFS(min_missile_range),		F_FLOAT,	FFL_NONE },
	{ "bypass_missile_chance",	FOFS(bypass_missile_chance),	F_INT,		FFL_NONE },
	{ "jump_chance",			FOFS(jump_chance),				F_INT,		FFL_NONE },
	{ "wakeup_distance",		FOFS(wakeup_distance),			F_FLOAT,	FFL_NONE },
	{ "c_mode",			FOFS(monsterinfo.c_mode),	F_INT,		FFL_NONE }, //BUGFIX, kinda: 'F_INT, F_INT' in original logic.
	{ "homebuoy",		FOFS(homebuoy),				F_LSTRING,	FFL_NONE },
	{ "wakeup_target",	FOFS(wakeup_target),		F_LSTRING,	FFL_NONE },
	{ "pain_target",	FOFS(pain_target),			F_LSTRING,	FFL_NONE },

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
	{ "target2",		FOFS(target2),			F_LSTRING,	FFL_NONE },
	{ "pathtargetname",	FOFS(pathtargetname),	F_LSTRING,	FFL_NONE },
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

	{ NULL, 0, F_INT, FFL_NONE } //BUGFIX: mxd. Added terminator field.
};

// -------- Just for savegames ----------
// All pointer fields should be listed here, or savegames won't work properly (they will crash and burn).
// This wasn't just tacked on to the fields array, because these don't need names, we wouldn't want map fields
// using some of these, and if one were accidentally present twice, it would double swizzle (fuck) the pointer.
static const field_t savefields[] =
{
	{ "", FOFS(classname),			F_LSTRING,	FFL_NONE },
	{ "", FOFS(target),				F_LSTRING,	FFL_NONE },
	{ "", FOFS(target2),			F_LSTRING,	FFL_NONE },
	{ "", FOFS(targetname),			F_LSTRING,	FFL_NONE },
	{ "", FOFS(scripttarget),		F_LSTRING,	FFL_NONE },
	{ "", FOFS(killtarget),			F_LSTRING,	FFL_NONE },
	{ "", FOFS(team),				F_LSTRING,	FFL_NONE },
	{ "", FOFS(pathtarget),			F_LSTRING,	FFL_NONE },
	{ "", FOFS(deathtarget),		F_LSTRING,	FFL_NONE },
	{ "", FOFS(combattarget),		F_LSTRING,	FFL_NONE },
	{ "", FOFS(model),				F_LSTRING,	FFL_NONE },
	{ "", FOFS(map),				F_LSTRING,	FFL_NONE },
	{ "", FOFS(message),			F_LSTRING,	FFL_NONE },
	{ "", FOFS(client),				F_CLIENT,	FFL_NONE },
	{ "", FOFS(item),				F_ITEM,		FFL_NONE },
	{ "", FOFS(goalentity),			F_EDICT,	FFL_NONE },
	{ "", FOFS(movetarget),			F_EDICT,	FFL_NONE },
	{ "", FOFS(enemy),				F_EDICT,	FFL_NONE },
	{ "", FOFS(oldenemy),			F_EDICT,	FFL_NONE },
	{ "", FOFS(activator),			F_EDICT,	FFL_NONE },
	{ "", FOFS(groundentity),		F_EDICT,	FFL_NONE },
	{ "", FOFS(teamchain),			F_EDICT,	FFL_NONE },
	{ "", FOFS(teammaster),			F_EDICT,	FFL_NONE },
	{ "", FOFS(owner),				F_EDICT,	FFL_NONE },
	{ "", FOFS(mynoise),			F_EDICT,	FFL_NONE },
	{ "", FOFS(mynoise2),			F_EDICT,	FFL_NONE },
	{ "", FOFS(target_ent),			F_EDICT,	FFL_NONE },
	{ "", FOFS(chain),				F_EDICT,	FFL_NONE },
	{ "", FOFS(blockingEntity),		F_EDICT,	FFL_NONE },
	{ "", FOFS(last_buoyed_enemy),	F_EDICT,	FFL_NONE },
	{ "", FOFS(placeholder),		F_EDICT,	FFL_NONE },
	{ "", FOFS(fire_damage_enemy),	F_EDICT,	FFL_NONE },

	{ NULL, 0, F_INT, FFL_NONE }
};

static const field_t levelfields[] =
{
	{ "", LLOFS(changemap),		F_LSTRING,	FFL_NONE },
	{ "", LLOFS(sight_client),	F_EDICT,	FFL_NONE },
	{ "", LLOFS(sight_entity),	F_EDICT,	FFL_NONE },

	{ NULL, 0, F_INT, FFL_NONE }
};

static const field_t bouyfields[] =
{
	{ "", BYOFS(pathtarget),	F_LSTRING,	FFL_NONE },
	{ "", BYOFS(target),		F_LSTRING,	FFL_NONE },
	{ "", BYOFS(targetname),	F_LSTRING,	FFL_NONE },
	{ "", BYOFS(jump_target),	F_LSTRING,	FFL_NONE },

	{ NULL, 0, F_INT, FFL_NONE }
};

static const field_t clientfields[] =
{
	{ "", CLOFS(playerinfo.pers.weapon),		F_ITEM,	FFL_NONE },
	{ "", CLOFS(playerinfo.pers.lastweapon),	F_ITEM,	FFL_NONE },
	{ "", CLOFS(playerinfo.pers.defence),		F_ITEM,	FFL_NONE },
	{ "", CLOFS(playerinfo.pers.lastdefence),	F_ITEM,	FFL_NONE },
	{ "", CLOFS(playerinfo.pers.newweapon),		F_ITEM,	FFL_NONE },

	{ NULL, 0, F_INT, FFL_NONE }
};

#pragma endregion

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
	static level_locals_t temp; //mxd. Made local static to avoid excessive stack sizes.

	// Set up some console vars as level save variables.
	const cvar_t* r_farclipdist = gi.cvar("r_farclipdist", FAR_CLIP_DIST, 0);
	level.far_clip_dist_f = r_farclipdist->value;

	const cvar_t* r_fog = Cvar_Get("r_fog", "0", 0);
	level.fog = r_fog->value;

	const cvar_t* r_fog_density = Cvar_Get("r_fog_density", "0", 0);
	level.fog_density = r_fog_density->value;

	// All of the ints, floats, and vectors stay as they are.
	temp = level;

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
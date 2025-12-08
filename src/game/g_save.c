//
// g_save.c
//
// Copyright 1998 Raven Software
//

#include "g_save.h" //mxd
#include "g_save_defs.h" //mxd
#include "g_main.h" //mxd
#include "g_playstats.h"
#include "p_client.h" //mxd
#include "p_dll.h" //mxd
#include "p_view.h" //mxd
#include "sc_Main.h" //mxd
#include "FX.h"
#include "Message.h"
#include "Utilities.h"
#include "g_local.h"

#pragma region ========================== SAVE FIELDS ==========================

#define SAVE_FUNCNAME_MAX_LENGTH	64 //mxd

const field_t fields[] =
{
	{ "classname",		FOFS(classname),	F_LSTRING,	FFL_NONE, NULL },
	{ "origin",			FOFS(s.origin),		F_VECTOR,	FFL_NONE, NULL },
	{ "model",			FOFS(model),		F_LSTRING,	FFL_NONE, NULL },
	{ "spawnflags",		FOFS(spawnflags),	F_INT,		FFL_NONE, NULL },
	{ "speed",			FOFS(speed),		F_FLOAT,	FFL_NONE, NULL },
	{ "accel",			FOFS(accel),		F_FLOAT,	FFL_NONE, NULL },
	{ "decel",			FOFS(decel),		F_FLOAT,	FFL_NONE, NULL },
	{ "target",			FOFS(target),		F_LSTRING,	FFL_NONE, NULL },
	{ "targetname",		FOFS(targetname),	F_LSTRING,	FFL_NONE, NULL },
	{ "scripttarget",	FOFS(scripttarget),	F_LSTRING,	FFL_NONE, NULL },
	{ "pathtarget",		FOFS(pathtarget),	F_LSTRING,	FFL_NONE, NULL },
	{ "jumptarget",		FOFS(jumptarget),	F_LSTRING,	FFL_NONE, NULL },
	{ "deathtarget",	FOFS(deathtarget),	F_LSTRING,	FFL_NONE, NULL },
	{ "killtarget",		FOFS(killtarget),	F_LSTRING,	FFL_NONE, NULL },
	{ "combattarget",	FOFS(combattarget),	F_LSTRING,	FFL_NONE, NULL },
	{ "message",		FOFS(message),		F_LSTRING,	FFL_NONE, NULL },
	{ "text_msg",		FOFS(text_msg),		F_LSTRING,	FFL_NONE, NULL },
	{ "team",			FOFS(team),			F_LSTRING,	FFL_NONE, NULL },
	{ "wait",			FOFS(wait),			F_FLOAT,	FFL_NONE, NULL },
	{ "delay",			FOFS(delay),		F_FLOAT,	FFL_NONE, NULL },
	{ "time",			FOFS(time),			F_FLOAT,	FFL_NONE, NULL },
	{ "random",			FOFS(random),		F_FLOAT,	FFL_NONE, NULL },
	{ "style",			FOFS(style),		F_INT,		FFL_NONE, NULL },
	{ "count",			FOFS(count),		F_INT,		FFL_NONE, NULL },
	{ "health",			FOFS(health),		F_INT,		FFL_NONE, NULL },
	{ "skinnum",		FOFS(s.skinnum),	F_INT,		FFL_NONE, NULL },
	{ "sounds",			FOFS(sounds),		F_INT,		FFL_NONE, NULL },
	{ "light",			0,					F_IGNORE,	FFL_NONE, NULL },
	{ "dmg",			FOFS(dmg),			F_INT,		FFL_NONE, NULL },
	{ "angles",			FOFS(s.angles),		F_VECTOR,	FFL_NONE, NULL },
	{ "angle",			FOFS(s.angles),		F_ANGLEHACK,FFL_NONE, NULL },
	{ "mass",			FOFS(mass),			F_INT,		FFL_NONE, NULL },
	{ "volume",			FOFS(volume),		F_FLOAT,	FFL_NONE, NULL },
	{ "attenuation",	FOFS(attenuation),	F_FLOAT,	FFL_NONE, NULL },
	{ "map",			FOFS(map),			F_LSTRING,	FFL_NONE, NULL },
	{ "materialtype",	FOFS(materialtype),	F_INT,		FFL_NONE, NULL },
	{ "scale",			FOFS(s.scale),		F_FLOAT,	FFL_NONE, NULL },
	{ "color",			FOFS(s.color),		F_RGBA,		FFL_NONE, NULL },
	{ "absLight",		FOFS(s.absLight),	F_RGB,		FFL_NONE, NULL },
	{ "frame",			FOFS(s.frame),		F_INT,		FFL_NONE, NULL },
	{ "mintel",			FOFS(mintel),		F_INT,		FFL_NONE, NULL },
	{ "melee_range",	FOFS(melee_range),	F_FLOAT,	FFL_NONE, NULL },
	{ "missile_range",			FOFS(missile_range),			F_FLOAT,	FFL_NONE, NULL },
	{ "min_missile_range",		FOFS(min_missile_range),		F_FLOAT,	FFL_NONE, NULL },
	{ "bypass_missile_chance",	FOFS(bypass_missile_chance),	F_INT,		FFL_NONE, NULL },
	{ "jump_chance",			FOFS(jump_chance),				F_INT,		FFL_NONE, NULL },
	{ "wakeup_distance",		FOFS(wakeup_distance),			F_FLOAT,	FFL_NONE, NULL },
	{ "c_mode",			FOFS(monsterinfo.c_mode),	F_INT,		FFL_NONE, NULL }, //BUGFIX, kinda: 'F_INT, F_INT' in original logic.
	{ "homebuoy",		FOFS(homebuoy),				F_LSTRING,	FFL_NONE, NULL },
	{ "wakeup_target",	FOFS(wakeup_target),		F_LSTRING,	FFL_NONE, NULL },
	{ "pain_target",	FOFS(pain_target),			F_LSTRING,	FFL_NONE, NULL },

	// Temporary spawn vars - only valid when the spawn function is called.
	{ "lip",			STOFS(lip),				F_INT,		FFL_SPAWNTEMP, NULL },
	{ "distance",		STOFS(distance),		F_INT,		FFL_SPAWNTEMP, NULL },
	{ "height",			STOFS(height),			F_INT,		FFL_SPAWNTEMP, NULL },
	{ "noise",			STOFS(noise),			F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "pausetime",		STOFS(pausetime),		F_FLOAT,	FFL_SPAWNTEMP, NULL },
	{ "item",			STOFS(item),			F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "gravity",		STOFS(gravity),			F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "sky",			STOFS(sky),				F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "skyrotate",		STOFS(skyrotate),		F_FLOAT,	FFL_SPAWNTEMP, NULL },
	{ "skyaxis",		STOFS(skyaxis),			F_VECTOR,	FFL_SPAWNTEMP, NULL },
	{ "minyaw",			STOFS(minyaw),			F_FLOAT,	FFL_SPAWNTEMP, NULL },
	{ "maxyaw",			STOFS(maxyaw),			F_FLOAT,	FFL_SPAWNTEMP, NULL },
	{ "minpitch",		STOFS(minpitch),		F_FLOAT,	FFL_SPAWNTEMP, NULL },
	{ "maxpitch",		STOFS(maxpitch),		F_FLOAT,	FFL_SPAWNTEMP, NULL },
	{ "nextmap",		STOFS(nextmap),			F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "rotate",			STOFS(rotate),			F_INT,		FFL_SPAWNTEMP, NULL },
	{ "target2",		FOFS(target2),			F_LSTRING,	FFL_NONE, NULL },
	{ "pathtargetname",	FOFS(pathtargetname),	F_LSTRING,	FFL_NONE, NULL },
	{ "zangle",			STOFS(zangle),			F_FLOAT,	FFL_SPAWNTEMP, NULL },
	{ "file",			STOFS(file),			F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "radius",			STOFS(radius),			F_INT,		FFL_SPAWNTEMP, NULL },
	{ "offensive",		STOFS(offensive),		F_INT,		FFL_SPAWNTEMP, NULL },
	{ "defensive",		STOFS(defensive),		F_INT,		FFL_SPAWNTEMP, NULL },
	{ "spawnflags2",	STOFS(spawnflags2),		F_INT,		FFL_SPAWNTEMP, NULL },
	{ "cooptimeout",	STOFS(cooptimeout),		F_INT,		FFL_SPAWNTEMP, NULL },

	{ "script",			STOFS(script),			F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm1",			STOFS(parms[0]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm2",			STOFS(parms[1]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm3",			STOFS(parms[2]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm4",			STOFS(parms[3]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm5",			STOFS(parms[4]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm6",			STOFS(parms[5]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm7",			STOFS(parms[6]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm8",			STOFS(parms[7]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm9",			STOFS(parms[8]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },
	{ "parm10",			STOFS(parms[9]),		F_LSTRING,	FFL_SPAWNTEMP, NULL },

	{ NULL, 0, F_INT, FFL_NONE, NULL } //BUGFIX: mxd. Added terminator field.
};

// -------- Just for savegames ----------
// All pointer fields should be listed here, or savegames won't work properly (they will crash and burn).
// This wasn't just tacked on to the fields array, because these don't need names, we wouldn't want map fields
// using some of these, and if one were accidentally present twice, it would double swizzle (fuck) the pointer.
static field_t savefields[] =
{
	{ "", FOFS(classname),					F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(target),						F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(target2),					F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(targetname),					F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(scripttarget),				F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(killtarget),					F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(team),						F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(pathtarget),					F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(deathtarget),				F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(combattarget),				F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(model),						F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(map),						F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(message),					F_LSTRING,	FFL_NONE, NULL },
	{ "", FOFS(monsterinfo.otherenemyname),	F_LSTRING,	FFL_NONE, NULL }, //H2_BUGFIX: mxd. Missing in original logic.
	{ "", FOFS(client),						F_CLIENT,	FFL_NONE, NULL },
	{ "", FOFS(item),						F_ITEM,		FFL_NONE, NULL },
	{ "", FOFS(goalentity),					F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(movetarget),					F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(enemy),						F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(oldenemy),					F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(activator),					F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(groundentity),				F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(teamchain),					F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(teammaster),					F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(owner),						F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(target_ent),					F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(chain),						F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(blockingEntity),				F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(last_buoyed_enemy),			F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(placeholder),				F_EDICT,	FFL_NONE, NULL },
	{ "", FOFS(fire_damage_enemy),			F_EDICT,	FFL_NONE, NULL },

	//mxd. edict function pointers... Names are set for debugging purposes.
	{ "isBlocking",					FOFS(isBlocking),				F_FUNCTION,	FFL_NONE, NULL },
	{ "msgHandler",					FOFS(msgHandler),				F_FUNCTION,	FFL_NONE, NULL },
	{ "think",						FOFS(think),					F_FUNCTION,	FFL_NONE, NULL },
	{ "ai",							FOFS(ai),						F_FUNCTION,	FFL_NONE, NULL },
	{ "bounced",					FOFS(bounced),					F_FUNCTION,	FFL_NONE, NULL },
	{ "isBlocked",					FOFS(isBlocked),				F_FUNCTION,	FFL_NONE, NULL },
	{ "touch",						FOFS(touch),					F_FUNCTION,	FFL_NONE, NULL },
	{ "use",						FOFS(use),						F_FUNCTION,	FFL_NONE, NULL },
	{ "TriggerActivated",			FOFS(TriggerActivated),			F_FUNCTION,	FFL_NONE, NULL },
	{ "blocked",					FOFS(blocked),					F_FUNCTION,	FFL_NONE, NULL },
	{ "pain",						FOFS(pain),						F_FUNCTION,	FFL_NONE, NULL },
	{ "die",						FOFS(die),						F_FUNCTION,	FFL_NONE, NULL },
	{ "oldthink",					FOFS(oldthink),					F_FUNCTION,	FFL_NONE, NULL },
	{ "oldtouch",					FOFS(oldtouch),					F_FUNCTION,	FFL_NONE, NULL },
	{ "cant_attack_think",			FOFS(cant_attack_think),		F_FUNCTION,	FFL_NONE, NULL },
	{ "mood_think",					FOFS(mood_think),				F_FUNCTION,	FFL_NONE, NULL },
	{ "pre_think",					FOFS(pre_think),				F_FUNCTION,	FFL_NONE, NULL },
	{ "post_think",					FOFS(post_think),				F_FUNCTION,	FFL_NONE, NULL },

	//mxd. edict.monsterinfo function pointers... Names are set for debugging purposes.
	{ "monsterinfo.idle",			FOFS(monsterinfo.idle),			F_FUNCTION,	FFL_NONE, NULL },
	{ "monsterinfo.search",			FOFS(monsterinfo.search),		F_FUNCTION,	FFL_NONE, NULL },
	{ "monsterinfo.dodge",			FOFS(monsterinfo.dodge),		F_FUNCTION,	FFL_NONE, NULL },
	{ "monsterinfo.attack",			FOFS(monsterinfo.attack),		F_FUNCTION,	FFL_NONE, NULL },
	{ "monsterinfo.sight",			FOFS(monsterinfo.sight),		F_FUNCTION,	FFL_NONE, NULL },
	{ "monsterinfo.dismember",		FOFS(monsterinfo.dismember),	F_FUNCTION,	FFL_NONE, NULL },
	{ "monsterinfo.alert",			FOFS(monsterinfo.alert),		F_FUNCTION,	FFL_NONE, NULL },
	{ "monsterinfo.checkattack",	FOFS(monsterinfo.checkattack),	F_FUNCTION,	FFL_NONE, NULL },
	{ "monsterinfo.c_callback",		FOFS(monsterinfo.c_callback),	F_FUNCTION,	FFL_NONE, NULL },

	//mxd. edict.moveinfo function pointers...
	{ "moveinfo.endfunc",			FOFS(moveinfo.endfunc),			F_FUNCTION,	FFL_NONE, NULL },

	//mxd. mmove pointer...
	{ "",							FOFS(monsterinfo.currentmove),	F_ANIMMOVE,	FFL_NONE, NULL },

	{ NULL, 0, F_INT, FFL_NONE, NULL }
};

static field_t levelfields[] =
{
	{ "", LLOFS(changemap),		F_LSTRING,	FFL_NONE, NULL },
	{ "", LLOFS(sight_client),	F_EDICT,	FFL_NONE, NULL },
	{ "", LLOFS(sight_entity),	F_EDICT,	FFL_NONE, NULL },

	{ NULL, 0, F_INT, FFL_NONE, NULL }
};

static field_t bouyfields[] =
{
	{ "", BYOFS(pathtarget),	F_LSTRING,	FFL_NONE, NULL },
	{ "", BYOFS(target),		F_LSTRING,	FFL_NONE, NULL },
	{ "", BYOFS(targetname),	F_LSTRING,	FFL_NONE, NULL },
	{ "", BYOFS(jump_target),	F_LSTRING,	FFL_NONE, NULL },

	{ NULL, 0, F_INT, FFL_NONE, NULL }
};

static field_t clientfields[] =
{
	{ "", CLOFS(lastentityhit),					F_EDICT,	FFL_NONE, NULL}, //mxd. Can be -1 (when hit world)!
	{ "", CLOFS(Meteors[0]),					F_EDICT,	FFL_NONE, NULL}, //mxd
	{ "", CLOFS(Meteors[1]),					F_EDICT,	FFL_NONE, NULL}, //mxd
	{ "", CLOFS(Meteors[2]),					F_EDICT,	FFL_NONE, NULL}, //mxd
	{ "", CLOFS(Meteors[3]),					F_EDICT,	FFL_NONE, NULL}, //mxd

	{ "", CLOFS(playerinfo.pers.weapon),		F_ITEM,	FFL_NONE, NULL },
	{ "", CLOFS(playerinfo.pers.lastweapon),	F_ITEM,	FFL_NONE, NULL },
	{ "", CLOFS(playerinfo.pers.defence),		F_ITEM,	FFL_NONE, NULL },
	{ "", CLOFS(playerinfo.pers.lastdefence),	F_ITEM,	FFL_NONE, NULL },
	{ "", CLOFS(playerinfo.pers.newweapon),		F_ITEM,	FFL_NONE, NULL },

	{ NULL, 0, F_INT, FFL_NONE, NULL }
};

#pragma endregion

#pragma region ========================== FIELDS IO ==========================

// Helper function to get the human-readable function definition by an address.
static const func_map_t* GetFunctionByAddress(const byte* address) // YQ2
{
	for (int i = 0; funcs_list[i].name != NULL; i++)
		if (funcs_list[i].address == address)
			return &funcs_list[i];

	return NULL;
}

// Helper function to get the pointer to a function by its human-readable name.
static byte* GetFunctionByName(const char* name) // YQ2
{
	for (int i = 0; funcs_list[i].name != NULL; i++)
		if (strcmp(name, funcs_list[i].name) == 0)
			return funcs_list[i].address;

	return NULL;
}

// Helper function to get the human-readable definition of an animmove_t struct by a pointer.
static const animmove_map_t* GetAnimMoveByAddress(const animmove_t* address) // YQ2
{
	for (int i = 0; mmoves_list[i].name != NULL; i++)
		if (mmoves_list[i].address == address)
			return &mmoves_list[i];

	return NULL;
}

// Helper function to get the pointer to an animmove_t struct by a human-readable definition.
static const animmove_t* GetAnimMoveByName(const char* name) // YQ2
{
	for (int i = 0; mmoves_list[i].name != NULL; i++)
		if (strcmp(name, mmoves_list[i].name) == 0)
			return mmoves_list[i].address;

	return NULL;
}

static void ConvertField(field_t* field, byte* base, const char* obj_name) //mxd. Named 'WriteField1' in original logic. +obj_name arg.
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
			if (*(int*)p == -1) //mxd. Very special "hit world" case (probably used by gclient_t.lastentityhit only)...
				index = -2;
			else if (*(edict_t**)p != NULL)
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

		case F_FUNCTION: // YQ2
			if (*(byte**)p == NULL)
			{
				len = 0;
				field->extra.func_info = NULL;
			}
			else
			{
				field->extra.func_info = GetFunctionByAddress(*(byte**)p);

				if (field->extra.func_info == NULL)
				{
					const char* func_name = ((obj_name != NULL && *obj_name != 0) ? va("%s.%s", obj_name, field->name) : field->name);
					gi.error("ConvertField: %s not in the functions list, can't save game!", func_name);

					return;
				}

				len = (int)strlen(field->extra.func_info->name) + 1;
				assert(len <= SAVE_FUNCNAME_MAX_LENGTH);
			}

			*(int*)p = len;
			break;

		case F_ANIMMOVE: // YQ2
			if (*(byte**)p == NULL)
			{
				len = 0;
				field->extra.amove_info = NULL;
			}
			else
			{
				field->extra.amove_info = GetAnimMoveByAddress(*(animmove_t**)p);

				if (field->extra.amove_info == NULL)
				{
					const char* amove_name = ((obj_name != NULL && *obj_name != 0) ? va("%s.%s", obj_name, field->name) : field->name);
					gi.error("ConvertField: %s not in the animmoves list, can't save game!", amove_name);

					return;
				}

				len = (int)strlen(field->extra.amove_info->name) + 1;
				assert(len <= SAVE_FUNCNAME_MAX_LENGTH);
			}

			*(int*)p = len;
			break;

		default:
			gi.error("ConvertField: unknown field type %i", field->type);
			break;
	}
}

static void WriteField(FILE* f, const field_t* field, byte* base) //mxd. Named 'WriteField2' in original logic.
{
	void* p = &base[field->ofs];

	switch (field->type)
	{
		case F_LSTRING:
		case F_GSTRING:
			if (*(char**)p != NULL)
			{
				const int len = (int)strlen(*(char**)p) + 1;
				fwrite(*(char**)p, len, 1, f);
			}
			break;

		case F_FUNCTION: // YQ2
			if (*(byte**)p != NULL)
			{
				const func_map_t* func = field->extra.func_info;
				assert(func != NULL);

				const int len = (int)strlen(func->name) + 1;
				fwrite(func->name, len, 1, f);
			}
			break;

		case F_ANIMMOVE: // YQ2
			if (*(byte**)p != NULL)
			{
				const animmove_map_t* amove = field->extra.amove_info;
				assert(amove != NULL);

				const int len = (int)strlen(amove->name) + 1;
				fwrite(amove->name, len, 1, f);
			}
			break;

		default:
			break;
	}
}

static void ReadField(FILE* f, const field_t* field, byte* base)
{
	int len;
	int index;
	char func_name[SAVE_FUNCNAME_MAX_LENGTH];

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
			if (index == -2) //mxd. Very special "hit world" case (probably used by gclient_t.lastentityhit only)...
				*(int*)p = -1;
			else if (index == -1)
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

		case F_FUNCTION:
			len = *(int*)p;

			if (len == 0)
			{
				*(byte**)p = NULL;
			}
			else
			{
				if (len > SAVE_FUNCNAME_MAX_LENGTH)
				{
					gi.error("ReadField: %s func name is longer than buffer (%i > %i)!", field->name, len, SAVE_FUNCNAME_MAX_LENGTH);
					return;
				}

				if (fread(func_name, len, 1, f) != 1)
				{
					gi.error("ReadField: can't get %s func name!", field->name);
					return;
				}

				func_name[SAVE_FUNCNAME_MAX_LENGTH - 1] = 0;
				*(byte**)p = GetFunctionByName(func_name);

				if (*(byte**)p == NULL)
					gi.error("ReadField: %s func %s not found in table, can't load game!", field->name, func_name);
			}
			break;

		case F_ANIMMOVE:
			len = *(int*)p;

			if (len == 0)
			{
				*(byte**)p = NULL;
			}
			else
			{
				if (len > SAVE_FUNCNAME_MAX_LENGTH)
				{
					gi.error("ReadField: animmove_t name is longer than buffer (%i > %i)!", len, SAVE_FUNCNAME_MAX_LENGTH);
					return;
				}

				if (fread(func_name, len, 1, f) != 1)
				{
					gi.error("ReadField: can't get animmove_t name!");
					return;
				}

				func_name[SAVE_FUNCNAME_MAX_LENGTH - 1] = 0;
				*(const animmove_t**)p = GetAnimMoveByName(func_name);

				if (*(animmove_t**)p == NULL)
					gi.error("ReadField: animmove_t %s not found in table, can't load game!", func_name);
			}
			break;

		default:
			gi.error("ReadField: unknown field type");
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
	for (field_t* field = &clientfields[0]; field->name != NULL; field++)
		ConvertField(field, (byte*)&temp, "client");

	// Write the block.
	fwrite(&temp, sizeof(temp), 1, f);

	// Now write any allocated data following the edict.
	for (const field_t* field = &clientfields[0]; field->name != NULL; field++)
		WriteField(f, field, (byte*)client);
}

// All pointer variables (except function pointers) must be handled specially.
static void ReadClient(FILE* f, gclient_t* client)
{
	fread(client, sizeof(*client), 1, f);

	for (const field_t* field = &clientfields[0]; field->name != NULL; field++)
		ReadField(f, field, (byte*)client);
}

#pragma endregion

#pragma region ========================== GAME IO ==========================

#define H2R_SAVE_VERSION	"H2RSG1" //mxd

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

	const char save_ver[16] = H2R_SAVE_VERSION; //mxd. Use save version instead of build date.
	fwrite(save_ver, sizeof(save_ver), 1, f);

	//mxd. Write out game size for checking.
	const int game_size = sizeof(game_locals_t);
	fwrite(&game_size, sizeof(game_size), 1, f);

	game.autosaved = autosave;
	fwrite(&game, sizeof(game), 1, f);
	game.autosaved = false;

	//mxd. Write out client size for checking.
	const int client_size = sizeof(gclient_t);
	fwrite(&client_size, sizeof(client_size), 1, f);

	for (int i = 0; i < game.maxclients; i++)
		WriteClient(f, &game.clients[i]);

	SaveScripts(f, true);

	fclose(f);
}

void ReadGame(char* filename)
{
	FILE* f;
	if (fopen_s(&f, filename, "rb") != 0) //mxd. fopen -> fopen_s
	{
		gi.error("Couldn't open %s", filename);
		return;
	}

	char save_ver[16];
	fread(save_ver, sizeof(save_ver), 1, f);
	save_ver[ARRAY_SIZE(save_ver) - 1] = 0;

	if (strcmp(save_ver, H2R_SAVE_VERSION) != 0) //mxd. Use save version instead of build date.
	{
		fclose(f);
		gi.error("Savegame from an older version (expected '%s', got '%s').\n", H2R_SAVE_VERSION, save_ver);

		return;
	}

	//mxd. Check game size.
	int game_size;
	fread(&game_size, sizeof(game_size), 1, f);

	if (game_size != sizeof(game_locals_t))
	{
		fclose(f);
		gi.error("ReadGame: mismatched game locals size (expected %i, got %i).", sizeof(game_locals_t), game_size);

		return;
	}

	gi.FreeTags(TAG_GAME);

	g_edicts = gi.TagMalloc(game.maxentities * (int)sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread(&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc(game.maxclients * (int)sizeof(game.clients[0]), TAG_GAME);

	//mxd. Check client size.
	int client_size;
	fread(&client_size, sizeof(client_size), 1, f);

	if (client_size != sizeof(gclient_t))
	{
		fclose(f);
		gi.error("ReadGame: mismatched gclient size (expected %i, got %i).", sizeof(gclient_t), client_size);

		return;
	}

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
	for (field_t* field = &savefields[0]; field->name != NULL; field++)
		ConvertField(field, (byte*)&temp, ent->classname);

	// Write the block.
	fwrite(&temp, sizeof(temp), 1, f);

	// Now write any allocated data following the edict.
	for (const field_t* field = &savefields[0]; field->name != NULL; field++)
		WriteField(f, field, (byte*)ent);
}

// All pointer variables (except function pointers) must be handled specially.
static void ReadEdict(FILE* f, edict_t* ent)
{
	fread(ent, sizeof(*ent), 1, f);

	//mxd. Clear these (most likely point to invalid data).
	memset(&ent->msgQ.msgs, 0, sizeof(ent->msgQ.msgs));
	ent->s.clientEffects.buf = NULL;
	ent->Script = NULL;
	ent->last_alert = NULL;

	for (const field_t* field = &savefields[0]; field->name != NULL; field++)
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
	for (field_t* field = &levelfields[0]; field->name != NULL; field++)
		ConvertField(field, (byte*)&temp, "level");

	for (int i = 0; i < level.active_buoys; i++)
		for (field_t* field = &bouyfields[0]; field->name != NULL; field++)
			ConvertField(field, (byte*)&temp.buoy_list[i], "buoy");

	//mxd. Write out level locals size for checking.
	const int locals_size = sizeof(level_locals_t);
	fwrite(&locals_size, sizeof(locals_size), 1, f);

	// Write the block.
	fwrite(&temp, sizeof(temp), 1, f);

	// Now write any allocated data following the edict.
	for (const field_t* field = &levelfields[0]; field->name != NULL; field++)
		WriteField(f, field, (byte*)&level);

	for (int i = 0; i < level.active_buoys; i++)
		for (const field_t* field = &bouyfields[0]; field->name != NULL; field++)
			WriteField(f, field, (byte*)&level.buoy_list[i]);
}

// All pointer variables (except function pointers) must be handled specially.
static void ReadLevelLocals(FILE* f)
{
	//mxd. Check level locals size.
	int locals_size;
	fread(&locals_size, sizeof(locals_size), 1, f);

	if (locals_size != sizeof(level_locals_t))
	{
		fclose(f);
		gi.error("ReadLevelLocals: mismatched level locals size (expected %i, got %i).", sizeof(level_locals_t), locals_size);

		return;
	}

	fread(&level, sizeof(level), 1, f);

	// Change the pointers to lengths or indexes.
	for (const field_t* field = &levelfields[0]; field->name != NULL; field++)
		ReadField(f, field, (byte*)&level);

	for (int i = 0; i < level.active_buoys; i++)
		for (const field_t* field = &bouyfields[0]; field->name != NULL; field++)
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

	//mxd. Skip writing InitGame pointer offset. Requires fixed .dll base address to work.

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

	//mxd. Write out fx buffer size for checking.
	const int fx_size = sizeof(PerEffectsBuffer_t);
	fwrite(&fx_size, sizeof(fx_size), 1, f);

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

	//mxd. Skip reading/checking InitGame pointer offset. Requires fixed .dll base address to work.

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

	//mxd. Check fx buffer size.
	int fx_size;
	fread(&fx_size, sizeof(fx_size), 1, f);

	if (fx_size != sizeof(PerEffectsBuffer_t))
	{
		fclose(f);
		gi.error("ReadLevel: mismatched fx buffer size (expected %i, got %i).", sizeof(PerEffectsBuffer_t), fx_size);

		return;
	}

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
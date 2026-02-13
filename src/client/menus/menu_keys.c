//
// menu_keys.c -- logic shared between actionkeys, movekeys, doubletapkeys and shortkeys menus.
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "menu_keys.h"
#include "menu_citymap.h"
#include "menu_help.h"
#include "menu_objectives.h"
#include "menu_worldmap.h"

typedef struct
{
	char* command;
	cvar_t** label;
} BindData_s;

#define NUM_BINDS	59

static BindData_s bindnames[NUM_BINDS] =
{
	// Action keys.
	{ "+attack",			&m_item_attack },
	{ "+defend",			&m_item_defend },
	{ "+action",			&m_item_action },
	{ "weapnext",			&m_item_nextweapon },
	{ "weapprev",			&m_item_prevweapon },
	{ "defnext",			&m_item_nextdef },
	{ "defprev",			&m_item_prevdef },
	{ "+lookaround",		&m_item_lookaround },
	{ "+autoaim",			&m_item_doautoaim }, //mxd
	{ "+lookup",			&m_item_lookup },
	{ "+lookdown",			&m_item_lookdown },
	{ "centerview",			&m_item_centerview },
	{ "+mlook",				&m_item_mouselook },
	{ "+klook",				&m_item_keyboardlook },

	// Move keys.
	{ "+forward",			&m_item_walkforward },
	{ "+back",				&m_item_backpedal },
	{ "+moveleft",			&m_item_stepleft },
	{ "+moveright",			&m_item_stepright },
	{ "+moveup",			&m_item_up },
	{ "+movedown",			&m_item_down },
	{ "+speed",				&m_item_run },
	{ "+creep",				&m_item_creep },
	{ "+left",				&m_item_turnleft },
	{ "+right",				&m_item_turnright },
	{ "+strafe",			&m_item_sidestep },
	{ "+quickturn",			&m_item_quickturn },

	// Shortcut keys.
	{ "menu_help",			&m_item_helpscreen },
	{ "use *powerup",		&m_item_powerup },
	{ "use *ring",			&m_item_bluering },
	{ "use *meteor",		&m_item_meteor },
	{ "use *morph",			&m_item_morph },
	{ "use *tele",			&m_item_teleport },
	{ "use *lshield",		&m_item_shield },
	{ "use *tornado",		&m_item_tornado },
	{ "+inventory",			&m_item_inventory },
	{ "menu_objectives",	&m_banner_objectives },
	{ "menu_world_map",		&m_banner_worldmap },
	{ "menu_city_map",		&m_banner_citymap },
	{ "messagemode",		&m_item_messagemode },
	{ "score",				&m_item_frags },

	// Double-tap keys.
	{ "+flip_forward",		&m_item_flipforward },
	{ "+flip_back",			&m_item_flipback },
	{ "+flip_left",			&m_item_flipleft },
	{ "+flip_right",		&m_item_flipright },
	{ "+roll_forward",		&m_item_rollforward },
	{ "+roll_back",			&m_item_rollback },
	{ "+roll_left",			&m_item_rollleft },
	{ "+roll_right",		&m_item_rollright },
	{ "+quickturn",			&m_item_quickturn },
	{ "+spinattack",		&m_item_spinattack },

	//mxd. System keys.
	{ "save quick",			&m_item_quicksave },
	{ "load quick",			&m_item_quickload },
	{ "menu_savegame",		&m_item_savegame },
	{ "menu_loadgame",		&m_item_loadgame },
	{ "menu_options",		&m_item_options },
	{ "screenshot",			&m_item_screenshot },
	{ "pause",				&m_item_pause },
	{ "quit",				&m_item_quit },
	{ "toggleconsole",		&m_item_toggleconsole },
};

int keys_count;
int keys_category_offset;
qboolean use_doublebind;
qboolean bind_grab;

menuframework_t s_keys_menu;
static menuinputkey_t s_keys_items[14];

// Q2: M_UnbindCommand
static void UnbindCommand(const char* command)
{
	const int len = re.BF_Strlen(command); // H2

	for (int i = 0; i < 256; i++)
	{
		const char* b = (use_doublebind ? keybindings_double[i] : keybindings[i]); // H2

		if (b != NULL && strncmp(b, command, len) == 0)
		{
			if (use_doublebind)
				Key_SetDoubleBinding(i, "");
			else
				Key_SetBinding(i, "");
		}
	}
}

static void KeyBindingFunc(const void* self)
{
	int keys[2];

	const menuinputkey_t* key = self;
	M_FindKeysForCommand(key->generic.localdata[0], keys);

	if (keys[1] != -1)
		UnbindCommand(bindnames[key->generic.localdata[0] + keys_category_offset].command);

	bind_grab = true;
}

void Keys_MenuInit(void)
{
	static char key_labels[NUM_BINDS][MAX_QPATH];

	s_keys_menu.nitems = 0;

	int oy = 0;
	for (int i = 0; i < keys_count; i++, oy += 20)
	{
		const cvar_t* label = *bindnames[keys_category_offset + i].label;
		Com_sprintf(key_labels[i], sizeof(key_labels[i]), "\x02%s", label->string);

		menuinputkey_t* item = &s_keys_items[i];
		item->generic.type = MTYPE_INPUT_KEY;
		item->generic.flags = QMF_SINGLELINE;
		item->generic.x = 0;
		item->generic.y = oy;
		item->generic.localdata[0] = i;
		item->generic.name = key_labels[i];
		item->generic.width = re.BF_Strlen(key_labels[i]);

		Menu_AddItem(&s_keys_menu, item);
	}

	Menu_Center(&s_keys_menu);
}

void Keys_MenuDraw(const char* menu_title) //mxd. Added to reduce code duplication.
{
	char title[MAX_QPATH];

	// Draw menu BG.
	Menu_DrawBG("book/back/b_conback8.bk", cls.m_menuscale);

	if (cls.m_menualpha == 0.0f)
		return;

	// Draw menu title.
	m_menu_side = 0;
	Com_sprintf(title, sizeof(title), "\x03%s", menu_title);
	const int x = M_GetMenuLabelX(re.BF_Strlen(title));
	const int y = M_GetMenuOffsetY(&s_keys_menu);
	re.DrawBigFont(x, y, title, cls.m_menualpha);

	// Draw menu items.
	s_keys_menu.x = M_GetMenuLabelX(0);
	Menu_AdjustCursor(&s_keys_menu, 1);
	Menu_Draw(&s_keys_menu);
}

const char* Keys_MenuKey(const int key)
{
	char cmd[1024];

	const menuinputkey_t* item = (menuinputkey_t*)Menu_ItemAtCursor(&s_keys_menu);
	const int bind_index = item->generic.localdata[0] + keys_category_offset;

	if (bind_grab)
	{
		if (key != K_ESCAPE && key != '`')
		{
			const char* format = (use_doublebind ? "bind_double \"%s\" \"%s\"\n" : "bind \"%s\" \"%s\"\n"); // H2
			Com_sprintf(cmd, sizeof(cmd), format, Key_KeynumToString(key), bindnames[bind_index].command);
			Cbuf_InsertText(cmd);
		}

		bind_grab = false;
		return NULL;
	}

	switch (key)
	{
		case K_ENTER:
		case K_KP_ENTER:
			KeyBindingFunc(item);
			return NULL;

		case K_BACKSPACE:
		case K_DEL:
		case K_KP_DEL:
			UnbindCommand(bindnames[bind_index].command);
			return NULL;

		default:
			return Default_MenuKey(&s_keys_menu, key);
	}
}

void M_FindKeysForCommand(const int command_index, int* twokeys)
{
	twokeys[0] = -1;
	twokeys[1] = -1;

	const char* command = bindnames[keys_category_offset + command_index].command;
	const int len = (int)strlen(command);
	int count = 0;

	for (int i = 0; i < 256; i++)
	{
		const char* b = (use_doublebind ? keybindings_double[i] : keybindings[i]);

		if (b != NULL && strncmp(b, command, len) == 0)
		{
			twokeys[count] = i;
			count++;

			if (count == 2)
				break;
		}
	}
}
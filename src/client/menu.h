//
// menu.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

#define MENU_EMPTY		"<*****>"

#define MAXMENUITEMS	32 // Q2: 64
#define MENU_CENTER_X	(DEF_WIDTH / 2) //mxd
#define MAX_SAVEGAMES	8 // Q2: 15

//mxd. Menu sounds
#define SND_MENU1	"misc/menu1.wav"
#define SND_MENU2	"misc/menu2.wav"
#define SND_MENU3	"misc/menu3.wav"
#define SND_MENU4	"misc/menu4.wav"

extern cvar_t* vid_mode;
extern cvar_t* vid_menu_mode;

extern cvar_t* menus_active;

extern cvar_t* m_item_defaults;
extern cvar_t* m_item_driver;
extern cvar_t* m_item_vidmode;
extern cvar_t* m_item_screensize;
extern cvar_t* m_item_gamma;
extern cvar_t* m_item_brightness;
extern cvar_t* m_item_contrast;
extern cvar_t* m_item_detail;
extern cvar_t* m_item_fullscreen;

// Controls
extern cvar_t* m_item_attack;
extern cvar_t* m_item_defend;
extern cvar_t* m_item_action;
extern cvar_t* m_item_lookup;
extern cvar_t* m_item_lookdown;
extern cvar_t* m_item_centerview;
extern cvar_t* m_item_mouselook;
extern cvar_t* m_item_keyboardlook;
extern cvar_t* m_item_lookaround;
extern cvar_t* m_item_nextweapon;
extern cvar_t* m_item_prevweapon;
extern cvar_t* m_item_nextdef;
extern cvar_t* m_item_prevdef;
extern cvar_t* m_item_walkforward;
extern cvar_t* m_item_backpedal;
extern cvar_t* m_item_turnleft;
extern cvar_t* m_item_turnright;
extern cvar_t* m_item_creep;
extern cvar_t* m_item_run;
extern cvar_t* m_item_stepleft;
extern cvar_t* m_item_stepright;
extern cvar_t* m_item_sidestep;
extern cvar_t* m_item_up;
extern cvar_t* m_item_down;
extern cvar_t* m_item_quickturn;
extern cvar_t* m_item_powerup;
extern cvar_t* m_item_bluering;
extern cvar_t* m_item_meteor;
extern cvar_t* m_item_morph;
extern cvar_t* m_item_teleport;
extern cvar_t* m_item_shield;
extern cvar_t* m_item_tornado;
extern cvar_t* m_item_inventory;
extern cvar_t* m_item_messagemode;
extern cvar_t* m_item_frags;
extern cvar_t* m_item_flipleft;
extern cvar_t* m_item_flipright;
extern cvar_t* m_item_flipforward;
extern cvar_t* m_item_flipback;
extern cvar_t* m_item_rollleft;
extern cvar_t* m_item_rollright;
extern cvar_t* m_item_rollforward;
extern cvar_t* m_item_rollback;
extern cvar_t* m_item_spinattack;

// Generic menu labels
extern cvar_t* m_generic_yes;
extern cvar_t* m_generic_no;
extern cvar_t* m_generic_high;
extern cvar_t* m_generic_low;
extern cvar_t* m_generic_on;
extern cvar_t* m_generic_off;
extern cvar_t* m_generic_violence0;
extern cvar_t* m_generic_violence1;
extern cvar_t* m_generic_violence2;
extern cvar_t* m_generic_violence3;
extern cvar_t* m_generic_crosshair0;
extern cvar_t* m_generic_crosshair1;
extern cvar_t* m_generic_crosshair2;
extern cvar_t* m_generic_crosshair3;
extern cvar_t* m_dmlist;
extern cvar_t* m_cooplist;
extern cvar_t* m_origmode;

// H2. Generic menu item label texts.
extern char m_text_no[MAX_QPATH];
extern char m_text_yes[MAX_QPATH];
extern char m_text_off[MAX_QPATH];
extern char m_text_on[MAX_QPATH];
extern char m_text_low[MAX_QPATH];
extern char m_text_high[MAX_QPATH];

extern char* yes_no_names[];

extern uint m_menu_side;

typedef void (*m_drawfunc_t)(void); //mxd
typedef const char* (*m_keyfunc_t)(int key); //mxd

typedef struct menuframework_t
{
	int x;
	int y;
	int cursor;
	int width;
	int nitems;
	struct menucommon_s* items[MAXMENUITEMS];
	const char* statusbar;
	void (*cursordraw)(struct menuframework_t* m);
} menuframework_s;

enum menuitem_type_e
{
	MTYPE_SLIDER,
	MTYPE_FIELD,
	MTYPE_ACTION,
	MTYPE_SPINCONTROL,
	MTYPE_INPUT_KEY,
	MTYPE_PLAYER_SKIN
};

enum menuitem_flags_e
{
	QMF_LEFT_JUSTIFY	= 1,
	QMF_GRAYED			= 2,
	QMF_NUMBERSONLY		= 4,
	QMF_SINGLELINE		= 8,
	QMF_MULTILINE		= 16,
	QMF_SELECT_SOUND	= 32
};

typedef struct
{
	int type;
	int x;
	int y;
	const char* name;
	int width;
	menuframework_s* parent;
	int cursor_offset; //TODO: unused?
	int localdata[4];
	uint flags;
	void (*callback)(void* self);
} menucommon_s;

typedef struct
{
	menucommon_s generic;
	int curvalue;
	char** itemnames;
} menulist_s;

typedef struct
{
	menucommon_s generic;
} menuaction_s;

typedef struct
{
	menucommon_s generic;
	float minvalue;
	float maxvalue;
	float curvalue;
	float range;
} menuslider_s;

typedef struct
{
	menucommon_s generic;
	char buffer[80];
	int cursor;
	int length;
	int visible_length;
	int visible_offset;
} menufield_s;

typedef struct
{
	menucommon_s generic;
} menuinputkey_s;

void M_PushMenu(m_drawfunc_t draw, m_keyfunc_t key);
void M_PopMenu(void);
char* Default_MenuKey(menuframework_s* menu, int key);
const char* Generic_MenuKey(int key);
qboolean Field_Key(menufield_s* field, int key);
void M_UpdateOrigMode(void); // H2
float M_GetMenuAlpha(void); // H2
int M_GetMenuLabelX(int text_width); // H2
int M_GetMenuOffsetY(const menuframework_s* menu); // H2
void M_DrawTextBox(int x, int y, int width, int lines);
void M_Print(int cx, int cy, int msg_index, paletteRGBA_t color);

void Menu_AddItem(menuframework_s* menu, void* item);
qboolean Menu_SelectItem(const menuframework_s* menu);
void Menu_SlideItem(const menuframework_s* menu, int dir);
void Menu_AdjustCursor(menuframework_s* menu, int dir);
void Menu_Center(menuframework_s* menu);
void Menu_Draw(const menuframework_s* menu);
void Menu_DrawString(int x, int y, const char* name, float alpha, qboolean selected);
void Menu_DrawObjectives(const char* message, int max_line_length); // H2
void Menu_DrawTitle(const cvar_t* title); // H2
menucommon_s* Menu_ItemAtCursor(const menuframework_s* menu);
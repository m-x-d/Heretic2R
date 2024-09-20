//
// menu.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

#define MENU_CENTER_X	(DEF_WIDTH / 2) //mxd

extern cvar_t* vid_mode;
extern cvar_t* vid_menu_mode;

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

typedef void (*m_drawfunc_t)(void); //mxd
typedef const char* (*m_keyfunc_t)(int key); //mxd

typedef struct menuframework_t
{
	int x;
	int y;
	int cursor;
	int width;
	int nitems;
	struct menucommon_s* items[32]; // Q2: 64
	const char* statusbar;
	void (*cursordraw)(struct menuframework_s* m);
} menuframework_s;

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
	const char** itemnames;
} menulist_s;

void M_PushMenu(m_drawfunc_t draw, m_keyfunc_t key);
void M_UpdateOrigMode(void); // H2
int M_GetMenuLabelX(int text_width); // H2
void Menu_DrawString(int x, int y, char* name, float alpha, qboolean selected);
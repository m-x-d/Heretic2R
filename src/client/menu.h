//
// menu.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

extern cvar_t* vid_mode;
extern cvar_t* vid_menu_mode;

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

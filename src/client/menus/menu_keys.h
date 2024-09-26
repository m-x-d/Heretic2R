//
// menu_keys.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "menu.h"

extern int keys_count;
extern int keys_category_offset;
extern qboolean use_doublebind;

extern menuframework_s s_keys_menu;

void Keys_MenuInit(void);
const char* Keys_MenuKey(int key);
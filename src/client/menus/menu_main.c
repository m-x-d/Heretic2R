//
// menu_main.c
//
// Copyright 1998 Raven Software
//

#include "menu_main.h"
#include "client.h"

cvar_t* m_banner_main;

static void M_Main_Draw(void)
{
	NOT_IMPLEMENTED
}

static const char* M_Main_Key(int key)
{
	NOT_IMPLEMENTED
	return NULL;
}

void M_Menu_Main_f(void)
{
	if (cl.frame.playerstate.cinematicfreeze)
		cls.esc_cinematic = 1;
	else
		M_PushMenu(M_Main_Draw, M_Main_Key);
}
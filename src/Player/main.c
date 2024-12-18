//
// main.c
//
// Copyright 1998 Raven Software
//

#include "Player.h"
#include "p_anim_data.h"

PLAYER_API void P_Init(void)
{
	InitItems();
}

PLAYER_API void P_Shutdown(void) { }

PLAYER_API player_export_t GetPlayerAPI(void)
{
	player_export_t pe;

	pe.PlayerSeqData = PlayerSeqData;
	pe.PlayerChickenData = PlayerChickenData;
	pe.p_num_items = p_num_items;
	pe.p_itemlist = p_itemlist;

	return pe;
}
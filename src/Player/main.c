#include "Player.h"
#include "p_anim_data.h"

PLAYER_API void P_Init(void)
{
	InitItems();
}

PLAYER_API void P_Shutdown(void)
{
}

PLAYER_API player_export_t GetPlayerAPI(void)
{
	player_export_t playerExport;

	playerExport.PlayerSeqData=PlayerSeqData;
	playerExport.PlayerChickenData=PlayerChickenData;
	playerExport.p_num_items=p_num_items;
	playerExport.p_itemlist=p_itemlist;

	return(playerExport);
}


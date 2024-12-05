//
// cl_inventory.c -- client inventory screen
//
// Copyright 1998 Raven Software
//

#include "client.h"

void CL_ParseInventory(void) //TODO: unused
{
	for (int i = 0; i < MAX_ITEMS; i++)
		cl.playerinfo.pers.inventory.Items[i] = MSG_ReadShort(&net_message);
}
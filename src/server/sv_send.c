//
// sv_send.c
//
// Copyright 1998 Raven Software
//

#include "server.h"

void SV_BroadcastPrintf(int level, char* fmt, ...)
{
	NOT_IMPLEMENTED
}

void SV_BroadcastCaption(int printlevel, short stringid)
{
	NOT_IMPLEMENTED
}

void SV_BroadcastObituary(int printlevel, short stringid, short client1, short client2)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
void SV_BroadcastCommand(char* fmt, ...)
{
	va_list argptr;
	char string[1024];

	if (sv.state != ss_dead)
	{
		va_start(argptr, fmt);
		vsprintf_s(string, sizeof(string), fmt, argptr); //mxd. vsprintf -> vsprintf_s
		va_end(argptr);

		MSG_WriteByte(&sv.multicast, svc_stufftext);
		MSG_WriteString(&sv.multicast, string);
		SV_Multicast(NULL, MULTICAST_ALL_R);
	}
}

// Sends the contents of sv.multicast to a subset of the clients, then clears sv.multicast.
// MULTICAST_ALL - same as broadcast(origin can be NULL).
// MULTICAST_PVS - send to clients potentially visible from origin.
// MULTICAST_PHS - send to clients potentially hearable from origin.
void SV_Multicast(vec3_t origin, const multicast_t to)
{
	byte* mask;
	
	qboolean reliable = false;
	int area1 = 0;
	int cluster = 0;

	if (to != MULTICAST_ALL_R && to != MULTICAST_ALL)
	{
		const int leafnum = CM_PointLeafnum(origin);
		cluster = CM_LeafCluster(leafnum);
		area1 = CM_LeafArea(leafnum);
	}

	// H2: missing 'svs.demofile' logic

	switch (to)
	{
		case MULTICAST_ALL_R:
			reliable = true;
		// Intentional fallthrough.
		case MULTICAST_ALL:
			mask = NULL;
			break;

		case MULTICAST_PHS_R:
			reliable = true;
		// Intentional fallthrough.
		case MULTICAST_PHS:
			mask = CM_ClusterPHS(cluster);
			break;

		case MULTICAST_PVS_R:
			reliable = true;
		// Intentional fallthrough.
		case MULTICAST_PVS:
			mask = CM_ClusterPVS(cluster);
			break;

		default:
			mask = NULL;
			Com_Error(ERR_FATAL, "SV_Multicast: bad to: %i", to);
			break;
	}

	// Send the data to all relevant clients.
	client_t* client = svs.clients;
	for (int i = 0; i < (int)maxclients->value; i++, client++)
	{
		if (client->state == cs_free || client->state == cs_zombie || (client->state != cs_spawned && !reliable))
			continue;

		if (mask != NULL)
		{
			const int leafnum = CM_PointLeafnum(client->edict->s.origin);
			cluster = CM_LeafCluster(leafnum);
			const int area2 = CM_LeafArea(leafnum);

			if (!CM_AreasConnected(area1, area2))
				continue;

			if (!(mask[cluster >> 3] & (1 << (cluster & 7)))) //mxd. Removed second 'mask != NULL' check
				continue;
		}

		if (reliable)
			SZ_Write(&client->netchan.message, sv.multicast.data, sv.multicast.cursize);
		else
			SZ_Write(&client->datagram, sv.multicast.data, sv.multicast.cursize);
	}

	SZ_Clear(&sv.multicast);
}

void SV_StartSound(vec3_t origin, edict_t* entity, int channel,	int soundindex, float volume, float attenuation, float timeofs)
{
	NOT_IMPLEMENTED
}

void SV_SendClientMessages(qboolean send_client_data)
{
	NOT_IMPLEMENTED
}
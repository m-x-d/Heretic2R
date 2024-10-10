//
// sv_send.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "sv_effects.h"
#include "cl_strings.h"

uint net_transmit_size; // H2

void SV_ClientPrintf(client_t* cl, int level, int message_id) // H2: different definition.
{
	NOT_IMPLEMENTED
}

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
void SV_Multicast(const vec3_t origin, const multicast_t to)
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

#pragma region ========================== FRAME UPDATES ==========================

qboolean SV_SendClientDatagram(client_t* client, qboolean send_client_data)
{
	NOT_IMPLEMENTED
	return false;
}

static void SV_DemoCompleted(void)
{
	NOT_IMPLEMENTED
}

// Returns true if the client is over its current bandwidth estimation and should not be sent another packet.
static qboolean SV_RateDrop(client_t* c)
{
	// Never drop over the loopback.
	if (c->netchan.remote_address.type == NA_LOOPBACK)
		return false;

	int total = 0;
	for (int i = 0; i < RATE_MESSAGES; i++)
		total += c->message_size[i];

	if (total > c->rate)
	{
		// H2: missing c->surpressCount++;
		c->message_size[sv.framenum % RATE_MESSAGES] = 0;
		return true;
	}

	return false;
}

void SV_SendClientMessages(const qboolean send_client_data)
{
	byte msgbuf[MAX_MSGLEN];

	int msglen = 0;

	// Read the next demo message if needed.
	if (sv.state == ss_demo && sv.demofile != NULL && !(int)sv_paused->value)
	{
		// Get the next message.
		uint r = fread(&msglen, 4, 1, sv.demofile);
		if (r != 1 || msglen == -1)
		{
			SV_DemoCompleted();
			return;
		}

		if (msglen > MAX_MSGLEN)
			Com_Error(ERR_DROP, "SV_SendClientMessages: msglen > MAX_MSGLEN");

		r = fread(msgbuf, msglen, 1, sv.demofile);
		if (r != 1)
		{
			SV_DemoCompleted();
			return;
		}
	}

	// Send a message to each connected client.
	int send_mask = 0; // H2

	client_t* c = svs.clients;
	for (int i = 0; i < (int)maxclients->value; i++, c++)
	{
		if (c->state == cs_free)
			continue;

		// If the reliable message overflowed, drop the client.
		const qboolean overflowed = c->datagram.overflowed; // H2
		if (c->netchan.message.overflowed)
		{
			SZ_Clear(&c->netchan.message);
			SZ_Clear(&c->datagram);
			SV_BroadcastObituary(PRINT_HIGH, GM_OVERFLOW, c->edict->s.number, 0); // H2
			Com_Printf("WARNING: reliable overflow for %s\n", c->name); // H2
			SV_DropClient(c);
		}

		send_mask |= EDICT_MASK(c->edict); // H2

		if (sv.state == ss_cinematic || sv.state == ss_demo)
		{
			Netchan_Transmit(&c->netchan, msglen, msgbuf);
		}
		else if (c->state == cs_spawned)
		{
			// Don't overrun bandwidth
			if (SV_RateDrop(c))
				continue;

			SV_SendClientDatagram(c, send_client_data); // H2: new 2-nd arg.
		}
		else if (c->netchan.message.cursize > 0 || curtime - c->netchan.last_sent > 1000)
		{
			// Just update reliable if needed.
			Netchan_Transmit(&c->netchan, 0, NULL);
		}

		if (!overflowed) // H2
		{
			const int mask = EDICT_MASK(c->edict);
			for (int e = 1; e < ge->num_edicts; e++)
			{
				edict_t* ent = EDICT_NUM(e);
				if (ent->just_deleted && (ent->client_sent & mask) != 0)
					ent->client_sent &= ~mask;
			}
		}
	}

	// H2: clear all client effects.
	effects_buffer_index = 0;
	effects_buffer_offset = 0;

	PerEffectsBuffer_t* effect = persistant_effects_array;
	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++, effect++)
	{
		if (effect->fx_num == 0 && effect->numEffects != 0 && effect->send_mask == send_mask)
		{
			memset(effect, 0, sizeof(PerEffectsBuffer_t));
			num_persistant_effects--;
		}
	}
}

#pragma endregion
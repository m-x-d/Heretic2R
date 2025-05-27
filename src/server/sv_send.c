//
// sv_send.c
//
// Copyright 1998 Raven Software
//

#include "server.h"
#include "cmodel.h"
#include "sv_effects.h"
#include "cl_strings.h"
#include "Vector.h"

uint net_transmit_size; // H2

void SV_ClientGameMessage(client_t* cl, const int printlevel, const int message_id) // H2
{
	if (cl->messagelevel <= printlevel)
	{
		MSG_WriteByte(&cl->netchan.message, svc_gamemsg_print);
		MSG_WriteShort(&cl->netchan.message, printlevel << 13 | message_id);
	}
}

// Sends text across to be displayed if the level passes.
void SV_ClientPrintf(client_t* cl, const int printlevel, const char* fmt, ...)
{
	va_list argptr;
	char string[1024];

	if (cl->state != cs_spawned || printlevel < cl->messagelevel) // H2: extra cl->state check.
		return;

	va_start(argptr, fmt);
	vsprintf_s(string, sizeof(string), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	MSG_WriteByte(&cl->netchan.message, svc_print);
	MSG_WriteByte(&cl->netchan.message, printlevel);
	MSG_WriteString(&cl->netchan.message, string);
}

// Sends colored text across to be displayed.
void SV_ClientColorPrintf(client_t* cl, const int printlevel, const byte color, const char* fmt, ...) // H2
{
	va_list argptr;
	char string[1024];

	if (cl->state != cs_spawned)
		return;

	va_start(argptr, fmt);
	vsprintf_s(string, sizeof(string), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	MSG_WriteByte(&cl->netchan.message, svc_nameprint);
	MSG_WriteByte(&cl->netchan.message, printlevel);
	MSG_WriteByte(&cl->netchan.message, color);
	MSG_WriteString(&cl->netchan.message, string);
}

// Sends text to all active clients.
void SV_BroadcastPrintf(const int printlevel, const char* fmt, ...)
{
	va_list argptr;
	char string[2048];

	va_start(argptr, fmt);
	vsprintf_s(string, sizeof(string), fmt, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	// Echo to console
	if ((int)dedicated->value)
		Com_Printf("%s", string); // H2: missing 'mask off high bits' logic.

	client_t* cl = svs.clients;
	for (int i = 0; i < (int)maxclients->value; i++, cl++)
	{
		if (printlevel < cl->messagelevel || cl->state != cs_spawned)
			continue;

		MSG_WriteByte(&cl->netchan.message, svc_print);
		MSG_WriteByte(&cl->netchan.message, printlevel);
		MSG_WriteString(&cl->netchan.message, string);
	}
}

void SV_BroadcastCaption(const int printlevel, const short stringid) // H2
{
	client_t* cl = svs.clients;
	for (int i = 0; i < (int)maxclients->value; i++, cl++)
	{
		if (cl->state != cs_spawned || printlevel < cl->messagelevel)
			continue;

		MSG_WriteByte(&cl->netchan.message, svc_captionprint);
		MSG_WriteShort(&cl->netchan.message, stringid);
	}
}

void SV_BroadcastObituary(const int printlevel, const short stringid, const short client1, const short client2) // H2
{
	if ((int)dedicated->value)
	{
		char* name1 = (client1 > 0 ? svs.clients[client1 - 1].name : "no one");
		char* name2 = (client2 > 0 ? svs.clients[client2 - 1].name : "no one");

		Com_Printf("Client Obituary #%d: %s and %s\n", stringid, name1, name2);
	}

	client_t* cl = svs.clients;
	for (int i = 0; i < (int)maxclients->value; i++, cl++)
	{
		if (cl->state != cs_spawned || printlevel < cl->messagelevel)
			continue;

		MSG_WriteByte(&cl->netchan.message, svc_obituary);
		MSG_WriteShort(&cl->netchan.message, stringid | (printlevel << 13));
		MSG_WriteByte(&cl->netchan.message, client1);
		MSG_WriteByte(&cl->netchan.message, client2);
	}
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
			Com_Error(ERR_FATAL, "SV_Multicast: bad to: %i", to);
			return;
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

static void SV_MulticastSound(const vec3_t origin, const multicast_t to, const int message_start) // H2
{
	byte* mask;

	qboolean reliable = false;
	int area1 = 0;

	if (to != MULTICAST_ALL_R && to != MULTICAST_ALL)
	{
		const int leafnum = CM_PointLeafnum(origin);
		area1 = CM_LeafArea(leafnum);
	}

	switch (to)
	{
		case MULTICAST_ALL_R:
			reliable = true;
		// Intentional fallthrough.
		case MULTICAST_ALL:
			sv.multicast.data[message_start] |= SND_POS;
			mask = NULL;
			break;

		case MULTICAST_PHS_R:
			reliable = true;
		// Intentional fallthrough.
		case MULTICAST_PHS:
		{
			const int leafnum = CM_PointLeafnum(origin);
			const int cluster = CM_LeafCluster(leafnum);
			area1 = CM_LeafArea(leafnum);
			mask = CM_ClusterPHS(cluster);
		} break;

		case MULTICAST_PVS_R:
			reliable = true;
		// Intentional fallthrough.
		case MULTICAST_PVS:
		{
			const int leafnum = CM_PointLeafnum(origin);
			const int cluster = CM_LeafCluster(leafnum);
			area1 = CM_LeafArea(leafnum);
			mask = CM_ClusterPVS(cluster);
		} break;

		default:
			Com_Error(ERR_FATAL, "SV_Multicast: bad to: %i", to);
			return;
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
			const int cluster = CM_LeafCluster(leafnum);
			const int area2 = CM_LeafArea(leafnum);

			if (!CM_AreasConnected(area1, area2))
				continue;

			if (!(mask[cluster >> 3] & (1 << (cluster & 7))))
				continue;
		}

		// Add position flag or skip position info. //TODO: Why is it done here?..
		if (!(sv.multicast.data[message_start] & SND_POS) && (to == MULTICAST_PHS || to == MULTICAST_PHS_R) && !PF_inPVS(client->edict->s.origin, origin))
			sv.multicast.data[message_start] |= SND_POS;

		int send_size;
		if (sv.multicast.data[message_start] & SND_POS)
			send_size = sv.multicast.cursize;
		else
			send_size = sv.multicast.cursize - 6; // Skip position info (last 3 shorts).

		if (reliable)
			SZ_Write(&client->netchan.message, sv.multicast.data, send_size);
		else
			SZ_Write(&client->datagram, sv.multicast.data, send_size);
	}

	SZ_Clear(&sv.multicast);
}

// Each entity can have eight independent sound sources, like voice, weapon, feet, etc.
// If channel & 8, the sound will be sent to everyone, not just things in the PHS.
// FIXME: if entity isn't in PHS, they must be forced to be sent or have the origin explicitly sent.
// Channel 0 is an auto-allocate channel, the others override anything already running on that entity / channel pair.
// An attenuation of 0 will play full volume everywhere in the level.
// Larger attenuations will drop off (max 4 attenuation).
// Timeofs can range from 0.0 to 0.1 to cause sounds to be started later in the frame than they normally would.
// If origin is NULL, the origin is determined from the entity origin or the midpoint of the entity box for bmodels.
//mxd. Parsed by CL_ParseStartSoundPacket().
void SV_StartSound(const vec3_t origin, const edict_t* ent, int channel, const int soundindex, const float volume, const float attenuation, const float timeofs)
{
	vec3_t origin_v;
	qboolean use_phs;

	if (volume < 0.0f || volume > 1.0f)
		Com_Error(ERR_FATAL, "SV_StartSound: volume = %f", (double)volume);

	if (attenuation < 0.0f || attenuation > 4.0f)
		Com_Error(ERR_FATAL, "SV_StartSound: attenuation = %f", (double)attenuation);

	if (timeofs < 0.0f || timeofs > 0.255f)
		Com_Error(ERR_FATAL, "SV_StartSound: timeofs = %f", (double)timeofs);

	if (channel & 8) // No PHS flag.
	{
		use_phs = false;
		channel &= 7;
	}
	else
	{
		use_phs = true;
	}

	int flags = 0;

	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		flags |= SND_VOLUME;

	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		flags |= SND_ATTENUATION;

	// The client doesn't know that bmodels have weird origins. The origin can also be explicitly set.
	if ((ent->svflags & SVF_NOCLIENT) || ent->solid == SOLID_BSP || origin != NULL)
		flags |= SND_POS;

	// Always send the entity number for channel overrides.
	flags |= SND_ENT;

	if (timeofs != 0.0f)
		flags |= SND_OFFSET;

	// Use the entity origin unless it is a bmodel or explicitly specified.
	if (origin == NULL)
	{
		origin = origin_v;

		if (ent->solid == SOLID_BSP)
		{
			for (int i = 0; i < 3; i++)
				origin_v[i] = ent->s.origin[i] + 0.5f * (ent->mins[i] + ent->maxs[i]);
		}
		else
		{
			VectorCopy(ent->s.origin, origin_v);
		}
	}

	MSG_WriteByte(&sv.multicast, svc_sound);

	const int msg_start = sv.multicast.cursize;
	MSG_WriteByte(&sv.multicast, flags);
	MSG_WriteShort(&sv.multicast, soundindex);

	if (flags & SND_VOLUME)
		MSG_WriteByte(&sv.multicast, Q_ftol(volume * 255.0f));

	if (flags & SND_ATTENUATION)
		MSG_WriteByte(&sv.multicast, Q_ftol(attenuation)); // Q2: attenuation * 64

	if (flags & SND_OFFSET)
		MSG_WriteByte(&sv.multicast, Q_ftol(timeofs * 1000.0f));

	if (flags & SND_ENT)
	{
		const int sendchan = (NUM_FOR_EDICT(ent) << 3) | (channel & 7);
		MSG_WriteShort(&sv.multicast, sendchan);
	}

	MSG_WritePos(&sv.multicast, origin); // H2: SND_POS flag is added in SV_MulticastSound().

	if (channel & CHAN_RELIABLE)
	{
		if (use_phs && attenuation != 0.0f)
			SV_MulticastSound(origin, MULTICAST_PHS_R, msg_start);
		else
			SV_MulticastSound(origin, MULTICAST_ALL_R, msg_start);
	}
	else
	{
		if (use_phs && attenuation != 0.0f)
			SV_MulticastSound(origin, MULTICAST_PHS, msg_start);
		else
			SV_MulticastSound(origin, MULTICAST_ALL, msg_start);
	}
}

//mxd. Parsed by CL_ParseStartSoundPacket().
// If channel & 8, the sound will be sent to everyone, not just things in the PHS.
void SV_StartEventSound(const byte event_id, const float leveltime, const vec3_t origin, const edict_t* ent, int channel, const int soundindex, const float volume, const float attenuation, const float timeofs) // H2
{
	vec3_t origin_v;
	qboolean use_phs;

	if (volume < 0.0f || volume > 1.0f)
		Com_Error(ERR_FATAL, "SV_StartSound: volume = %f", (double)volume);

	if (attenuation < 0.0f || attenuation > 4.0f)
		Com_Error(ERR_FATAL, "SV_StartSound: attenuation = %f", (double)attenuation);

	if (timeofs < 0.0f || timeofs > 0.255f)
		Com_Error(ERR_FATAL, "SV_StartSound: timeofs = %f", (double)timeofs);

	if (channel & 8) // No PHS flag.
	{
		use_phs = false;
		channel &= 7;
	}
	else
	{
		use_phs = true;
	}

	int flags = 0;

	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		flags |= SND_VOLUME;

	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		flags |= SND_ATTENUATION;

	// The client doesn't know that bmodels have weird origins. The origin can also be explicitly set.
	if ((ent->svflags & SVF_NOCLIENT) || ent->solid == SOLID_BSP || origin != NULL)
		flags |= SND_POS;

	// Always send the entity number for channel overrides.
	flags |= SND_ENT;

	if (timeofs != 0.0f)
		flags |= SND_OFFSET;

	if (event_id != 0) // H2
		flags |= SND_PRED_INFO;

	// Use the entity origin unless it is a bmodel or explicitly specified.
	if (origin == NULL)
	{
		origin = origin_v;

		if (ent->solid == SOLID_BSP)
		{
			for (int i = 0; i < 3; i++)
				origin_v[i] = ent->s.origin[i] + 0.5f * (ent->mins[i] + ent->maxs[i]);
		}
		else
		{
			VectorCopy(ent->s.origin, origin_v);
		}
	}

	MSG_WriteByte(&sv.multicast, svc_sound);

	const int msg_start = sv.multicast.cursize;
	MSG_WriteByte(&sv.multicast, flags);
	MSG_WriteShort(&sv.multicast, soundindex);

	if (flags & SND_PRED_INFO) // H2
	{
		MSG_WriteByte(&sv.multicast, event_id);
		MSG_WriteFloat(&sv.multicast, leveltime);
	}

	if (flags & SND_VOLUME)
		MSG_WriteByte(&sv.multicast, Q_ftol(volume * 255.0f));

	if (flags & SND_ATTENUATION)
		MSG_WriteByte(&sv.multicast, Q_ftol(attenuation)); // Q2: attenuation * 64

	if (flags & SND_OFFSET)
		MSG_WriteByte(&sv.multicast, Q_ftol(timeofs * 1000.0f));

	if (flags & SND_ENT)
	{
		const int sendchan = (NUM_FOR_EDICT(ent) << 3) | (channel & 7);
		MSG_WriteShort(&sv.multicast, sendchan);
	}

	MSG_WritePos(&sv.multicast, origin); // H2: SND_POS flag is added in SV_MulticastSound().

	if (channel & CHAN_RELIABLE)
	{
		if (use_phs && attenuation != 0.0f)
			SV_MulticastSound(origin, MULTICAST_PHS_R, msg_start);
		else
			SV_MulticastSound(origin, MULTICAST_ALL_R, msg_start);
	}
	else
	{
		if (use_phs && attenuation != 0.0f)
			SV_MulticastSound(origin, MULTICAST_PHS, msg_start);
		else
			SV_MulticastSound(origin, MULTICAST_ALL, msg_start);
	}
}

#pragma region ========================== FRAME UPDATES ==========================

static qboolean SV_SendClientDatagram(client_t* client, const qboolean send_client_data)
{
	byte msg_buf[MAX_MSGLEN];
	sizebuf_t msg;

	SZ_Init(&msg, msg_buf, sizeof(msg_buf));
	msg.allowoverflow = true;

	if (send_client_data) // H2: new check.
	{
		SV_BuildClientFrame(client);
		SV_WriteFrameToClient(client, &msg); // Send over all the relevant entity_state_t and the player_state_t.
		SV_SendClientEffects(client); // H2
	}

	// Copy the accumulated multicast datagram for this client out to the message.
	// It is necessary for this to be after the WriteEntities, so that entity references will be current.
	if (client->datagram.overflowed)
		Com_Printf("WARNING: datagram overflowed for %s\n", client->name);
	else
		SZ_Write(&msg, client->datagram.data, client->datagram.cursize);

	SZ_Clear(&client->datagram);

	if (msg.overflowed)
	{
		// Must have room left for the packet header.
		Com_Printf("WARNING: msg overflowed for %s\n", client->name);
		SZ_Clear(&msg);
	}

	// Send the datagram. // H2: extra return value.
	net_transmit_size = Netchan_Transmit(&client->netchan, msg.cursize, msg.data);

	// Record the size for rate estimation.
	client->message_size[sv.framenum % RATE_MESSAGES] = msg.cursize;

	return true;
}

// Q2 counterpart
static void SV_DemoCompleted(void)
{
	if (sv.demofile != NULL)
	{
		fclose(sv.demofile);
		sv.demofile = NULL;
	}

	SV_Nextserver();
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
	num_effects_buffers = 0;
	clfx_buffer_offset = 0;

	PerEffectsBuffer_t* effect = persistant_effects;
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
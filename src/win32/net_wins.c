//
// net_wins.c -- Exposes the only function needed by Heretic2R.exe.
//
// Copyright 1998 Raven Software
//

#include <winsock.h>
#include "qcommon.h"
#include "Random.h"

#define MAX_LOOPBACK	4

typedef struct
{
	byte data[MAX_MSGLEN];
	int datalen;
} loopmsg_t;

typedef struct // H2
{
	byte data[MAX_MSGLEN];
	int datalen;
	uint timestamp;
	qboolean is_free;
} loopmsg2_t;

typedef struct
{
	loopmsg_t msgs[MAX_LOOPBACK];
	int get;
	int send;
} loopback_t;

#define NUM_SOCKETS			3 //mxd
#define NUM_LOOPMESSAGES	20 //mxd

static loopback_t loopbacks[NUM_SOCKETS];
static loopmsg2_t loopmessages[NUM_SOCKETS][NUM_LOOPMESSAGES]; // H2
static int ip_sockets[NUM_SOCKETS]; // 2 in Q2
static int ipx_sockets[NUM_SOCKETS]; // 2 in Q2

static cvar_t* net_shownet;
static cvar_t* noudp;
static cvar_t* noipx;
static cvar_t* ipxfix; // New in H2

static WSADATA winsockdata;

static void SockadrToNetadr(struct sockaddr* s, netadr_t* a)
{
	NOT_IMPLEMENTED
}

//TODO: use pointer args
qboolean NET_CompareAdr(netadr_t a, netadr_t b)
{
	NOT_IMPLEMENTED
	return false;
}

//TODO: use pointer args
qboolean NET_CompareBaseAdr(netadr_t a, netadr_t b)
{
	NOT_IMPLEMENTED
	return false;
}

//TODO: use pointer arg
char* NET_AdrToString(netadr_t a)
{
	NOT_IMPLEMENTED
	return NULL;
}

qboolean NET_StringToAdr(char* s, netadr_t* a)
{
	NOT_IMPLEMENTED
	return false;
}

static char* NET_ErrorString(void)
{
	NOT_IMPLEMENTED
	return NULL;
}

#pragma region ========================== LOOPBACK BUFFERS FOR LOCAL PLAYER ==========================

static qboolean	NET_GetLoopPacket(const netsrc_t sock, netadr_t* n_from, sizebuf_t* n_message)
{
	// H2: simulate net latency //TODO: dev logic. Remove?
	if (net_latency->value > 0.0f && net_latency->value < 2000.0f)
	{
		const uint time = timeGetTime();
		loopmsg2_t* msg = &loopmessages[sock][0];

		for (int i = 0; i < 20; i++, msg++)
		{
			if (msg->is_free && msg->timestamp < time)
			{
				msg->is_free = false;

				memcpy(n_message->data, msg->data, msg->datalen);
				n_message->cursize = msg->datalen;

				memset(n_from, 0, sizeof(*n_from));
				n_from->type = NA_LOOPBACK;

				return true;
			}
		}

		return false;
	}

	// Q2 logic:
	loopback_t* loop = &loopbacks[sock];

	if (loop->send - loop->get > MAX_LOOPBACK)
		loop->get = loop->send - MAX_LOOPBACK;

	if (loop->get >= loop->send)
		return false;

	const int i = loop->get & (MAX_LOOPBACK - 1);
	loop->get++;

	memcpy(n_message->data, loop->msgs[i].data, loop->msgs[i].datalen);
	n_message->cursize = loop->msgs[i].datalen;
	memset(n_from, 0, sizeof(*n_from));
	n_from->type = NA_LOOPBACK;

	return true;
}

#pragma endregion

qboolean NET_GetPacket(const netsrc_t sock, netadr_t* n_from, sizebuf_t* n_message)
{
	int net_socket;
	struct sockaddr from;

	if (NET_GetLoopPacket(sock, n_from, n_message))
	{
		// H2: simulate packet loss //TODO: dev logic. Remove?
		if (net_receiverate->value > 0.0f && net_receiverate->value < 1.0f && flrand(0.0f, 1.0f) > net_receiverate->value)
		{
			n_message->cursize = 0;
			return false;
		}

		return true;
	}

	for (int protocol = 0; protocol < 2; protocol++)
	{
		if (protocol == 0)
			net_socket = ip_sockets[sock];
		else
			net_socket = ipx_sockets[sock];

		if (net_socket == 0)
			continue;

		int fromlen = sizeof(from);
		const int ret = recvfrom(net_socket, (char*)n_message->data, n_message->maxsize, 0, &from, &fromlen);

		if (ret == -1)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				// Let dedicated servers continue after errors
				if ((int)dedicated->value)
					Com_Printf("NET_GetPacket: %s from %s\n", NET_ErrorString(), NET_AdrToString(*n_from));
				else
					Com_Error(ERR_DROP, "NET_GetPacket: %s from %s", NET_ErrorString(), NET_AdrToString(*n_from));
			}

			continue;
		}

		SockadrToNetadr(&from, n_from);

		if (ret == n_message->maxsize)
		{
			Com_Printf("Oversize packet from %s\n", NET_AdrToString(*n_from));
			continue;
		}

		// H2: simulate packet loss //TODO: dev logic. Remove?
		if (net_receiverate->value > 0.0f && net_receiverate->value < 1.0f && flrand(0.0f, 1.0f) > net_receiverate->value)
		{
			n_message->cursize = 0;
			return false;
		}

		n_message->cursize = ret;
		return true;
	}

	return false;
}

static void NET_OpenIP(void)
{
	NOT_IMPLEMENTED
}

static void NET_OpenIPX(void)
{
	NOT_IMPLEMENTED
}

// A single player game will only use the loopback code
void NET_Config(const qboolean multiplayer)
{
	static qboolean old_config;

	if (old_config == multiplayer)
		return;

	old_config = multiplayer;

	if (multiplayer)
	{
		// Open sockets
		if (!(int)noudp->value)
			NET_OpenIP();

		if (!(int)noipx->value)
			NET_OpenIPX();
	}
	else
	{
		// Shut down any existing sockets (2 in Q2, 3 in H2)
		for (int i = 0; i < 3; i++)
		{
			if (ip_sockets[i] != 0)
			{
				closesocket(ip_sockets[i]);
				ip_sockets[i] = 0;
			}

			if (ipx_sockets[i] != 0)
			{
				closesocket(ipx_sockets[i]);
				ipx_sockets[i] = 0;
			}
		}
	}
}

void NET_Init(void)
{
	if (WSAStartup(MAKEWORD(1, 1), &winsockdata) != 0)
		Com_Error(ERR_FATAL, "Winsock initialization failed.");

	Com_Printf("Winsock Initialized\n");

	noudp = Cvar_Get("noudp", "0", CVAR_NOSET);
	noipx = Cvar_Get("noipx", "0", CVAR_NOSET);
	ipxfix = Cvar_Get("ipxfix", "1", 0); // New in H2

	net_shownet = Cvar_Get("net_shownet", "0", 0);
}

void NET_Shutdown(void)
{
	NOT_IMPLEMENTED
}
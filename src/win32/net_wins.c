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

static void NetadrToSockadr(netadr_t* a, struct sockaddr* s)
{
	NOT_IMPLEMENTED
}

static void SockadrToNetadr(struct sockaddr* s, netadr_t* a)
{
	NOT_IMPLEMENTED
}

// Q2 counterpart
qboolean NET_CompareAdr(const netadr_t* a, const netadr_t* b)
{
	if (a->type != b->type)
		return false;

	switch (a->type)
	{
		case NA_LOOPBACK:
			return true;

		case NA_IP:
			return (a->port == b->port && memcmp(a->ip, b->ip, 4) == 0);

		case NA_IPX:
			return (a->port == b->port && memcmp(a->ipx, b->ipx, 10) == 0);

		default:
			return false;
	}
}

// Q2 counterpart
// Compares without the port
qboolean NET_CompareBaseAdr(const netadr_t* a, const netadr_t* b)
{
	if (a->type != b->type)
		return false;

	switch (a->type)
	{
		case NA_LOOPBACK:
			return true;

		case NA_IP:
			return (memcmp(a->ip, b->ip, 4) == 0);

		case NA_IPX:
			return (memcmp(a->ipx, b->ipx, 10) == 0);

		default:
			return false;
	}
}

static qboolean NET_StringToSockaddr(char* s, struct sockaddr* sadr)
{
	NOT_IMPLEMENTED
	return false;
}

// Q2 counterpart
char* NET_AdrToString(const netadr_t* a)
{
	static char s[64];

	switch (a->type)
	{
		case NA_LOOPBACK:
			Com_sprintf(s, sizeof(s), "loopback");
			break;

		case NA_IP:
			Com_sprintf(s, sizeof(s), "%i.%i.%i.%i:%i", a->ip[0], a->ip[1], a->ip[2], a->ip[3], ntohs(a->port));
			break;

		default:
			Com_sprintf(s, sizeof(s), "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%i", a->ipx[0], a->ipx[1], a->ipx[2], a->ipx[3], a->ipx[4], a->ipx[5], a->ipx[6], a->ipx[7], a->ipx[8], a->ipx[9], ntohs(a->port));
			break;
	}

	return s;
}

// Q2 counterpart
// Input string examples: 'localhost', 'idnewt', 'idnewt:28000', '192.246.40.70', '192.246.40.70:28000'
qboolean NET_StringToAdr(char* s, netadr_t* a)
{
	struct sockaddr sadr;

	if (strcmp(s, "localhost") == 0)
	{
		memset(a, 0, sizeof(netadr_t));
		a->type = NA_LOOPBACK;

		return true;
	}

	if (NET_StringToSockaddr(s, &sadr))
	{
		SockadrToNetadr(&sadr, a);
		return true;
	}

	return false;
}

static char* NET_ErrorString(void)
{
	NOT_IMPLEMENTED
	return NULL;
}

// Q2 counterpart.
qboolean NET_IsLocalAddress(const netadr_t* a)
{
	return a->type == NA_LOOPBACK;
}

#pragma region ========================== LOOPBACK BUFFERS FOR LOCAL PLAYER ==========================

static qboolean NET_GetLoopPacket(const netsrc_t sock, netadr_t* n_from, sizebuf_t* n_message)
{
	// H2: emulate net latency //TODO: dev logic. Remove?
	if (net_latency->value > 0.0f && net_latency->value < 2000.0f)
	{
		const uint time = timeGetTime();
		loopmsg2_t* msg = &loopmessages[sock][0];

		for (int i = 0; i < NUM_LOOPMESSAGES; i++, msg++)
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

//mxd. Removed unused 'to' arg
static void NET_SendLoopPacket(const netsrc_t sock, const int length, const void* data)
{
	// H2: emulate net latency. //TODO: dev logic. Remove?
	if (net_latency->value > 0.0f && net_latency->value < 2000.0f)
	{
		const uint time = timeGetTime();
		loopmsg2_t* msg = &loopmessages[sock ^ 1][0];

		for (int i = 0; i < NUM_LOOPMESSAGES; i++, msg++)
		{
			if (!msg->is_free)
			{
				msg->is_free = true;
				const float nl = net_latency->value;
				msg->timestamp = time + (int)(nl + flrand(-nl * 0.25f, nl * 0.25f));

				memcpy(msg, data, length);
				msg->datalen = length;
			}
		}

		return;
	}

	// Original Q2 logic:
	loopback_t* loop = &loopbacks[sock ^ 1];
	const int index = loop->send & (MAX_LOOPBACK - 1);
	loop->send++;

	memcpy(loop->msgs[index].data, data, length);
	loop->msgs[index].datalen = length;
}

#pragma endregion

qboolean NET_GetPacket(const netsrc_t sock, netadr_t* n_from, sizebuf_t* n_message)
{
	int net_socket;
	struct sockaddr from;

	if (NET_GetLoopPacket(sock, n_from, n_message))
	{
		// H2: emulate packet loss. //TODO: dev logic. Remove?
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
					Com_Printf("NET_GetPacket: %s from %s\n", NET_ErrorString(), NET_AdrToString(n_from));
				else
					Com_Error(ERR_DROP, "NET_GetPacket: %s from %s", NET_ErrorString(), NET_AdrToString(n_from));
			}

			continue;
		}

		SockadrToNetadr(&from, n_from);

		if (ret == n_message->maxsize)
		{
			Com_Printf("Oversize packet from %s\n", NET_AdrToString(n_from));
			continue;
		}

		// H2: emulate packet loss. //TODO: dev logic. Remove?
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

void NET_SendPacket(const netsrc_t sock, const int length, const void* data, netadr_t to)
{
	struct sockaddr addr;
	int net_socket;

	// H2: emulate packet loss. //TODO: dev logic. Remove?
	if (net_sendrate->value > 0.0f && net_sendrate->value < 1.0f && net_sendrate->value < flrand(0.0f, 1.0f))
		return;

	switch (to.type)
	{
		case NA_LOOPBACK:
			NET_SendLoopPacket(sock, length, data);
			return;

		case NA_IP:
		case NA_BROADCAST:
			net_socket = ip_sockets[sock];
			break;

		case NA_IPX:
		case NA_BROADCAST_IPX:
			net_socket = ipx_sockets[sock];
			break;

		default:
			Com_Error(ERR_FATAL, "NET_SendPacket: bad address type");
			return;
	}

	if (net_socket == 0)
		return;

	NetadrToSockadr(&to, &addr);

	if (sendto(net_socket, data, length, 0, &addr, sizeof(addr)) == -1)
	{
		const int err = WSAGetLastError();

		// Wouldblock is silent.
		if (err == WSAEWOULDBLOCK)
			return;

		// Some PPP links don't allow broadcasts.
		if (err == WSAEADDRNOTAVAIL && (to.type == NA_BROADCAST || to.type == NA_BROADCAST_IPX))
			return;

		// Let dedicated servers continue after errors.
		if ((int)dedicated->value)
			Com_Printf("NET_SendPacket ERROR: %s\n", NET_ErrorString());
		else if (err == WSAEADDRNOTAVAIL)
			Com_DPrintf("NET_SendPacket Warning: %s : %s\n", NET_ErrorString(), NET_AdrToString(&to));
		else
			Com_Error(ERR_DROP, "NET_SendPacket ERROR: %s\n", NET_ErrorString());
	}
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
//
// net_wins.c
//
// Copyright 1998 Raven Software
//

#include <winsock.h>
#include <wsipx.h>
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
static cvar_t* ipxfix; // H2

static WSADATA winsockdata;

static void NetadrToSockadr(const netadr_t* a, struct sockaddr* s)
{
	memset(s, 0, sizeof(*s));

	switch (a->type)
	{
		case NA_BROADCAST:
		{
			struct sockaddr_in* s_in = (struct sockaddr_in*)s;
			s_in->sin_family = AF_INET;
			s_in->sin_port = a->port;
			s_in->sin_addr.s_addr = INADDR_BROADCAST;
		} break;

		case NA_IP:
		{
			struct sockaddr_in* s_in = (struct sockaddr_in*)s;
			s_in->sin_family = AF_INET;
			s_in->sin_addr.s_addr = *(int*)&a->ip;
			s_in->sin_port = a->port;
		} break;

		case NA_IPX:
		{
			struct sockaddr_ipx* s_ipx = (struct sockaddr_ipx*)s;
			s_ipx->sa_family = AF_IPX;

			if ((int)ipxfix->value) // H2
				s_ipx->sa_netnum[0] = 0;
			else
				memcpy(s_ipx->sa_netnum, &a->ipx[0], 4);

			memcpy(s_ipx->sa_nodenum, &a->ipx[4], 6);
			s_ipx->sa_socket = a->port;
		} break;

		case NA_BROADCAST_IPX:
		{
			struct sockaddr_ipx* s_ipx = (struct sockaddr_ipx*)s;
			s_ipx->sa_family = AF_IPX;
			memset(s_ipx->sa_netnum, 0, 4);
			memset(s_ipx->sa_nodenum, 0xff, 6);
			s_ipx->sa_socket = a->port;
		} break;

		default:
			break;
	}
}

static void SockadrToNetadr(struct sockaddr* s, netadr_t* a)
{
	switch (s->sa_family)
	{
		case AF_INET:
		{
			const struct sockaddr_in* s_in = (struct sockaddr_in*)s;
			a->type = NA_IP;
			memcpy(a->ip, &s_in->sin_addr.s_addr, sizeof(s_in->sin_addr.s_addr));
			a->port = s_in->sin_port;
		} break;

		case AF_IPX:
		{
			const struct sockaddr_ipx* s_ipx = (struct sockaddr_ipx*)s;
			a->type = NA_IPX;

			if ((int)ipxfix->value) // H2
				memset(&a->ipx[0], 0, 4);
			else
				memcpy(&a->ipx[0], s_ipx->sa_netnum, sizeof(s_ipx->sa_netnum));

			memcpy(&a->ipx[4], s_ipx->sa_nodenum, sizeof(s_ipx->sa_nodenum));
			a->port = s_ipx->sa_socket;
		} break;
		
		default:
			break;
	}
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

// Q2 counterpart
static qboolean NET_StringToSockaddr(const char* s, struct sockaddr* sadr)
{
	memset(sadr, 0, sizeof(*sadr));

	if (strlen(s) >= 23 && s[8] == ':' && s[21] == ':') // Check for an IPX address.
	{
		struct sockaddr_ipx* sa_ipx = (struct sockaddr_ipx*)sadr;

		sa_ipx->sa_family = AF_IPX;

		uint val;
		char temp[3];

		// Network number
		for (int i = 0; i < 4; i++)
		{
			strncpy_s(temp, sizeof(temp), &s[i * 2], 2);
			sscanf_s(temp, "%x", &val);
			sa_ipx->sa_netnum[i] = (char)val;
		}

		// Node number
		for (int i = 0; i < 6; i++)
		{
			strncpy_s(temp, sizeof(temp), &s[i * 2 + 9], 2);
			sscanf_s(temp, "%x", &val);
			sa_ipx->sa_nodenum[i] = (char)val;
		}

		// Socket number
		sscanf_s(&s[22], "%u", &val);
		sa_ipx->sa_socket = htons((ushort)val);
	}
	else
	{
		struct sockaddr_in* sa_in = (struct sockaddr_in*)sadr;

		sa_in->sin_family = AF_INET;
		sa_in->sin_port = 0;

		char copy[128];
		strcpy_s(copy, sizeof(copy), s); //mxd. strcpy -> strcpy_s

		// Strip off a trailing :port if present.
		for (char* colon = copy; *colon != 0; colon++)
		{
			if (*colon == ':')
			{
				*colon = 0;
				sa_in->sin_port = htons((ushort)Q_atoi(colon + 1));
			}
		}

		if (copy[0] >= '0' && copy[0] <= '9')
		{
			sa_in->sin_addr.S_un.S_addr = inet_addr(copy);
		}
		else
		{
			const struct hostent* h = gethostbyname(copy);
			if (h == NULL)
				return false;

			sa_in->sin_addr.S_un.S_addr = *(int*)h->h_addr_list[0];
		}
	}

	return true;

#undef DO
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
qboolean NET_StringToAdr(const char* s, netadr_t* a)
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

// Q2 counterpart
static char* NET_ErrorString(void)
{
	switch (WSAGetLastError())
	{
		case WSAEINTR: return "WSAEINTR";
		case WSAEBADF: return "WSAEBADF";
		case WSAEACCES: return "WSAEACCES";
		case WSAEDISCON: return "WSAEDISCON";
		case WSAEFAULT: return "WSAEFAULT";
		case WSAEINVAL: return "WSAEINVAL";
		case WSAEMFILE: return "WSAEMFILE";
		case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
		case WSAEINPROGRESS: return "WSAEINPROGRESS";
		case WSAEALREADY: return "WSAEALREADY";
		case WSAENOTSOCK: return "WSAENOTSOCK";
		case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
		case WSAEMSGSIZE: return "WSAEMSGSIZE";
		case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
		case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
		case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
		case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
		case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
		case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
		case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
		case WSAEADDRINUSE: return "WSAEADDRINUSE";
		case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
		case WSAENETDOWN: return "WSAENETDOWN";
		case WSAENETUNREACH: return "WSAENETUNREACH";
		case WSAENETRESET: return "WSAENETRESET";
		case WSAECONNABORTED: return "WSWSAECONNABORTEDAEINTR";
		case WSAECONNRESET: return "WSAECONNRESET";
		case WSAENOBUFS: return "WSAENOBUFS";
		case WSAEISCONN: return "WSAEISCONN";
		case WSAENOTCONN: return "WSAENOTCONN";
		case WSAESHUTDOWN: return "WSAESHUTDOWN";
		case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
		case WSAETIMEDOUT: return "WSAETIMEDOUT";
		case WSAECONNREFUSED: return "WSAECONNREFUSED";
		case WSAELOOP: return "WSAELOOP";
		case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
		case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
		case WSASYSNOTREADY: return "WSASYSNOTREADY";
		case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
		case WSANOTINITIALISED: return "WSANOTINITIALISED";
		case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
		case WSATRY_AGAIN: return "WSATRY_AGAIN";
		case WSANO_RECOVERY: return "WSANO_RECOVERY";
		case WSANO_DATA: return "WSANO_DATA";
		default: return "NO ERROR";
	}
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

void NET_SendPacket(const netsrc_t sock, const int length, const void* data, const netadr_t* to)
{
	struct sockaddr addr;
	int net_socket;

	// H2: emulate packet loss. //TODO: dev logic. Remove?
	if (net_sendrate->value > 0.0f && net_sendrate->value < 1.0f && net_sendrate->value < flrand(0.0f, 1.0f))
		return;

	switch (to->type)
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

	NetadrToSockadr(to, &addr);

	if (sendto(net_socket, data, length, 0, &addr, sizeof(addr)) == -1)
	{
		const int err = WSAGetLastError();

		// Wouldblock is silent.
		if (err == WSAEWOULDBLOCK)
			return;

		// Some PPP links don't allow broadcasts.
		if (err == WSAEADDRNOTAVAIL && (to->type == NA_BROADCAST || to->type == NA_BROADCAST_IPX))
			return;

		// Let dedicated servers continue after errors.
		if ((int)dedicated->value)
			Com_Printf("NET_SendPacket ERROR: %s\n", NET_ErrorString());
		else if (err == WSAEADDRNOTAVAIL)
			Com_DPrintf("NET_SendPacket Warning: %s : %s\n", NET_ErrorString(), NET_AdrToString(to));
		else
			Com_Error(ERR_DROP, "NET_SendPacket ERROR: %s\n", NET_ErrorString());
	}
}

// Q2 counterpart
static int NET_IPSocket(const char* net_interface, const int port)
{
	u_long arg = 1;
	int optval = 1;

	const SOCKET newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (newsocket == 0xffffffff)
	{
		if (WSAGetLastError() != WSAEAFNOSUPPORT)
			Com_Printf("WARNING: UDP_OpenSocket: socket: %s\n", NET_ErrorString()); //mxd. No newline in Q2

		return 0;
	}

	// Make it non-blocking.
	if (ioctlsocket(newsocket, FIONBIO, &arg) == -1)
	{
		Com_Printf("WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString());
		return 0;
	}

	// Make it broadcast capable.
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)) == -1)
	{
		Com_Printf("WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
		return 0;
	}

	struct sockaddr_in address;
	if (net_interface == NULL || net_interface[0] == 0 || Q_stricmp(net_interface, "localhost") == 0)
		address.sin_addr.s_addr = INADDR_ANY;
	else
		NET_StringToSockaddr(net_interface, (struct sockaddr*)&address);

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((u_short)port);

	address.sin_family = AF_INET;

	if (bind(newsocket, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		Com_Printf("WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString());
		closesocket(newsocket);

		return 0;
	}

	return (int)newsocket;
}

static void NET_OpenIP(void)
{
	const cvar_t* ip = Cvar_Get("ip", "localhost", CVAR_NOSET);
	const qboolean is_dedicated = ((int)Cvar_VariableValue("dedicated") != 0);

	if (ip_sockets[NS_SERVER] == 0)
	{
		int port = (int)Cvar_Get("ip_hostport", "0", CVAR_NOSET)->value;
		if (port == 0)
		{
			port = (int)Cvar_Get("hostport", "0", CVAR_NOSET)->value;
			if (port == 0)
				port = (int)Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->value;
		}

		ip_sockets[NS_SERVER] = NET_IPSocket(ip->string, port);
		if (is_dedicated && ip_sockets[NS_SERVER] == 0)
			Com_Error(ERR_FATAL, "Couldn't allocate dedicated server IP port");
	}

	//mxd. Skip Gamespy IP port logic.

	// Dedicated servers don't need client ports.
	if (is_dedicated)
		return;

	if (ip_sockets[NS_CLIENT] == 0)
	{
		int port = (int)Cvar_Get("ip_clientport", "0", CVAR_NOSET)->value;
		if (port == 0)
		{
			port = (int)Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->value;
			if (port == 0)
				port = PORT_ANY;
		}

		ip_sockets[NS_CLIENT] = NET_IPSocket(ip->string, port);
		if (ip_sockets[NS_CLIENT] == 0)
			ip_sockets[NS_CLIENT] = NET_IPSocket(ip->string, PORT_ANY);
	}
}

// Q2 counterpart
static int NET_IPXSocket(const int port)
{
	u_long arg = 1;

	const SOCKET newsocket = socket(PF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if (newsocket == 0xffffffff)
	{
		if (WSAGetLastError() != WSAEAFNOSUPPORT)
			Com_Printf("WARNING: IPX_Socket: socket: %s\n", NET_ErrorString());

		return 0;
	}

	// Make it non-blocking.
	if (ioctlsocket(newsocket, FIONBIO, &arg) == -1)
	{
		Com_Printf("WARNING: IPX_Socket: ioctl FIONBIO: %s\n", NET_ErrorString());
		return 0;
	}

	// Make it broadcast capable.
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char*)&arg, sizeof(arg)) == -1)
	{
		Com_Printf("WARNING: IPX_Socket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
		return 0;
	}

	struct sockaddr_ipx	address;
	address.sa_family = AF_IPX;
	memset(address.sa_netnum, 0, 4);
	memset(address.sa_nodenum, 0, 6);

	if (port == PORT_ANY)
		address.sa_socket = 0;
	else
		address.sa_socket = htons((u_short)port);

	if (bind(newsocket, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		Com_Printf("WARNING: IPX_Socket: bind: %s\n", NET_ErrorString());
		closesocket(newsocket);

		return 0;
	}

	return (int)newsocket;
}

static void NET_OpenIPX(void)
{
	const qboolean is_dedicated = ((int)(Cvar_VariableValue("dedicated")) != 0);

	if (ipx_sockets[NS_SERVER] == 0)
	{
		int port = (int)Cvar_Get("ipx_hostport", "0", CVAR_NOSET)->value;
		if (port == 0)
		{
			port = (int)Cvar_Get("hostport", "0", CVAR_NOSET)->value;
			if (port == 0)
				port = (int)Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->value;
		}

		ipx_sockets[NS_SERVER] = NET_IPXSocket(port);
	}

	//mxd. Skip Gamespy IPX port logic.

	// Dedicated servers don't need client ports.
	if (is_dedicated)
		return;

	if (ipx_sockets[NS_CLIENT] == 0)
	{
		int port = (int)Cvar_Get("ipx_clientport", "0", CVAR_NOSET)->value;
		if (port == 0)
		{
			port = (int)Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->value;
			if (port == 0)
				port = PORT_ANY;
		}

		ipx_sockets[NS_CLIENT] = NET_IPXSocket(port);
		if (ipx_sockets[NS_CLIENT] == 0)
			ipx_sockets[NS_CLIENT] = NET_IPXSocket(PORT_ANY);
	}
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
	ipxfix = Cvar_Get("ipxfix", "1", 0); // H2

	net_shownet = Cvar_Get("net_shownet", "0", 0);
}

// Q2 counterpart
void NET_Shutdown(void)
{
	NET_Config(false); // Close sockets.
	WSACleanup();
}
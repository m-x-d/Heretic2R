//
// net_wins.c -- Exposes the only function needed by Heretic2R.exe.
//
// Copyright 1998 Raven Software
//

#include <winsock.h>
#include "qcommon.h"

static int ip_sockets[3]; // 2 in Q2
static int ipx_sockets[3]; // 2 in Q2

static cvar_t* net_shownet;
static cvar_t* noudp;
static cvar_t* noipx;
static cvar_t* ipxfix; // New in H2

static WSADATA winsockdata;

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

qboolean NET_GetPacket(netsrc_t sock, netadr_t* net_from, sizebuf_t* net_message)
{
	NOT_IMPLEMENTED
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
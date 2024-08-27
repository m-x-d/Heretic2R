//
// net_wins.c -- Exposes the only function needed by Heretic2R.exe.
//
// Copyright 1998 Raven Software
//

#include <winsock.h>
#include "qcommon.h"

static cvar_t* net_shownet;
static cvar_t* noudp;
static cvar_t* noipx;
static cvar_t* ipxfix; // New in H2

static WSADATA winsockdata;

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
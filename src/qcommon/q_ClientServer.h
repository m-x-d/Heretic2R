//
// q_ClientServer.h
//
// Copyright 1998 Raven Software
//

#pragma once

// Per-level limits
#define MAX_CLIENTS			32		// Absolute limit
#define MAX_EDICTS			1024	// Must change protocol to increase more
#define MAX_LIGHTSTYLES		256
#define MAX_MODELS			256		// These are sent over the net as bytes
#define MAX_SOUNDS			768 	// so they cannot be blindly increased
#define MAX_IMAGES			256
#define MAX_ITEMS			256

#define	MAX_NETWORKABLE_EDICTS		MAX_EDICTS

#define G_MAX_ENTITIES		"1024"	// String to go into a convar

// Destination class for gi.multicast()
typedef enum
{
	MULTICAST_ALL,
	MULTICAST_PHS,
	MULTICAST_PVS,
	MULTICAST_ALL_R,
	MULTICAST_PHS_R,
	MULTICAST_PVS_R
} multicast_t;
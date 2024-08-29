//
// server.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "qcommon.h"
#include "q_ClientServer.h"

typedef enum
{
	ss_dead,	// No map loaded
	ss_loading,	// Spawning level edicts
	ss_game,	// Actively running
	ss_cinematic,
	ss_demo,
	ss_pic
} server_state_t;

typedef struct
{
	server_state_t state;	// Precache commands are only valid during load

	qboolean attractloop;	// Running cinematics and demos for the local system only
	qboolean loadgame;		// Client begins should reuse existing entity

	uint time; // Always sv.framenum * 100 msec
	int framenum;

	char name[MAX_QPATH]; // Map or cinematic name
	struct cmodel_s* models[MAX_MODELS];

	char configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];
	entity_state_t baselines[MAX_EDICTS];

	// The multicast buffer is used to send a message to a set of clients.
	// It is only used to marshall data until SV_Multicast is called.
	sizebuf_t multicast;
	byte multicast_buf[MAX_MSGLEN];

	// Demo server information
	FILE* demofile;
	qboolean timedemo; // Don't time sync
} server_t;

extern server_t sv; // Local server

// sv_ccmds.c
void SV_InitOperatorCommands(void);
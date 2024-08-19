//
// cl_main.c
//
// Copyright 1998 Raven Software
//

#include "client.h"

cvar_t* cl_paused;

client_static_t cls;
client_state_t cl;

centity_t cl_entities[MAX_NETWORKABLE_EDICTS]; //mxd. MAX_EDICTS in Q2
entity_state_t cl_parse_entities[MAX_PARSE_ENTITIES];

void CL_Init(void)
{
	NOT_IMPLEMENTED
}

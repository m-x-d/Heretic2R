//
// cl_main.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "ResourceManager.h"
#include "sound.h"

cvar_t* cl_paused;

client_static_t cls;
client_state_t cl;

centity_t cl_entities[MAX_NETWORKABLE_EDICTS]; //mxd. MAX_EDICTS in Q2
entity_state_t cl_parse_entities[MAX_PARSE_ENTITIES];

static void CL_InitLocal(void)
{
	NOT_IMPLEMENTED
}

static void CL_InitMessages(void)
{
	NOT_IMPLEMENTED
}

void CL_Init(void)
{
	if ((int)dedicated->value)
		return; // Nothing running on the client

	ResMngr_Con(&cl_FXBufMngr, 1292, 8); // New in H2 (was a separate function there)

	// All archived variables will now be loaded
	Con_Init();
	VID_Init();
	S_Init(); // Sound must be initialized after window is created

	V_Init();

	net_message.data = net_message_buffer;
	net_message.maxsize = sizeof(net_message_buffer);

	M_Init();
	SCR_Init();

	// Missing: cls.disable_screen = true;
	//CDAudio_Init(); //mxd. Skip CDAudio logic.
	CL_InitLocal();
	IN_Init();

	FS_ExecAutoexec();
	Cbuf_Execute();

	// New in H2:
	Cbuf_AddText("exec menus.cfg\n");
	Cbuf_AddText("exec user.cfg\n");
	Cbuf_Execute();

	CL_InitMessages();
}
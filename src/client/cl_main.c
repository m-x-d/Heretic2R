//
// cl_main.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "clfx_dll.h"
#include "cl_smk.h"
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

// Writes key bindings and archived cvars to config.cfg
static void CL_WriteConfiguration(void)
{
	FILE* f;
	char path[MAX_QPATH];

	if (cls.state == ca_uninitialized)
		return;

	Com_sprintf(path, sizeof(path), "%s/config.cfg", FS_Userdir()); // FS_Gamedir in Q2
	FS_CreatePath(path); // H2

	if (fopen_s(&f, path, "w") != 0) //mxd. fopen -> fopen_s
	{
		Com_Printf("Couldn\'t write config.cfg.\n");
		return;
	}

	fprintf(f, "// Generated by Heretic 2, do not modify\n"); // H2: different message
	Key_WriteBindings(f);
	Key_WriteBindings_Double(f); // H2
	fclose(f);

	Cvar_WriteVariables(path);
}

static void InitMessages(void)
{
	NOT_IMPLEMENTED
}

static void ClearGameMessages(void)
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

	InitMessages();
}

// FIXME: this is a callback from Sys_Quit and Com_Error.
// It would be better to run quit through here before the final handoff to the sys code.
void CL_Shutdown(void)
{
	static qboolean isdown = false;

	if (isdown)
	{
		printf("recursive shutdown\n");
		return;
	}

	isdown = true;

	Cvar_SetValue("win_ignore_destroy", 1.0f);

	if (SNDEAX_SetEnvironment != NULL)
	{
		SNDEAX_SetEnvironment(0);
		SNDEAX_SetEnvironment = NULL;
	}

	ResMngr_Des(&cl_FXBufMngr); //mxd. Was a separate function in H2

	CL_WriteConfiguration();

	if (fxapi_initialized)
		SV_UnloadClientEffects();

	P_Freelib();
	SMK_Stop();
	ClearGameMessages();
	//CDAudio_Shutdown(); //mxd. Skip CDAudio logic
	S_Shutdown();
	IN_DeactivateMouse();
	VID_Shutdown();
	SndDll_FreeLibrary();
	NET_Shutdown();
	Z_FreeTags(0);
}
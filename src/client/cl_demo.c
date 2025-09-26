//
// cl_demo.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "qcommon.h"
#include "EffectFlags.h"
#include "Skeletons.h"

// record <demoname>
// Begins recording a demo from the current position.
void CL_Record_f(void)
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: record <demoname>\n");
		return;
	}
 
	if (cls.demorecording)
	{
		Com_Printf("Already recording.\n");
		return;
	}

	if (cls.state != ca_active)
	{
		Com_Printf("You must be in a level to record.\n");
		return;
	}

	// Open the demo file.
	char name[MAX_OSPATH];
	Com_sprintf(name, sizeof(name), "%s/demos/%s.hd2", FS_Gamedir(), Cmd_Argv(1));

	Com_Printf("Recording to '%s'.\n", name);
	FS_CreatePath(name);

	if (fopen_s(&cls.demofile, name, "wb") != 0) //mxd. fopen -> fopen_s
	{
		Com_Printf("ERROR: couldn't open demo file.\n");
		return;
	}

	MSG_WriteByte(&cls.netchan.message, clc_startdemo); // H2. //mxd. Was in a separate function in original version.

	cls.demorecording = true;
	cls.demosavingok = true; // H2
	cls.demowaiting = true; // Don't start saving messages until a non-delta compressed message is received.

	// Write out messages to hold the startup information.
	sizebuf_t buf;
	byte buf_data[MAX_MSGLEN];

	SZ_Init(&buf, buf_data, sizeof(buf_data));

	// Send the serverdata.
	MSG_WriteByte(&buf, svc_serverdata);
	MSG_WriteLong(&buf, PROTOCOL_VERSION);
	MSG_WriteLong(&buf, 0x10000 + cl.servercount);
	MSG_WriteByte(&buf, 1); // Demos are always attract loops.
	MSG_WriteString(&buf, cl.gamedir);
	MSG_WriteByte(&buf, (byte)game_downloadable_type->value); // H2
	MSG_WriteString(&buf, fxe.client_string); // H2
	MSG_WriteShort(&buf, cl.playernum);

	MSG_WriteString(&buf, cl.configstrings[CS_NAME]);

	// Send configstrings.
	for (int i = 0; i < MAX_CONFIGSTRINGS; i++)
	{
		if (cl.configstrings[i][0] == 0)
			continue;

		if (buf.cursize + (int)strlen(cl.configstrings[i]) + 32 > buf.maxsize)
		{
			// Write it out.
			uint num = fwrite(&buf.cursize, 4, 1, cls.demofile);
			cls.demosavingok &= (num > 0); // H2

			num = fwrite(buf.data, buf.cursize, 1, cls.demofile);
			cls.demosavingok &= (num > 0); // H2

			buf.cursize = 0;
		}

		MSG_WriteByte(&buf, svc_configstring);
		MSG_WriteShort(&buf, i);
		MSG_WriteString(&buf, cl.configstrings[i]);
	}

	// Send baselines.
	entity_state_t nullstate = { 0 };

	nullstate.skeletalType = SKEL_NULL; // H2
	nullstate.rootJoint = NULL_ROOT_JOINT; // H2
	nullstate.swapFrame = NO_SWAP_FRAME; // H2

	for (int i = 0; i < MAX_EDICTS; i++)
	{
		entity_state_t* ent = &cl_entities[i].baseline;

		if (ent->modelindex == 0 && (ent->effects & (EF_NODRAW_ALWAYS_SEND | EF_ALWAYS_ADD_EFFECTS)) == 0)
			continue;

		if (buf.cursize + 64 > buf.maxsize)
		{
			// Write it out.
			uint num = fwrite(&buf.cursize, 4, 1, cls.demofile);
			cls.demosavingok &= (num > 0); // H2

			num = fwrite(buf.data, buf.cursize, 1, cls.demofile);
			cls.demosavingok &= (num > 0); // H2

			buf.cursize = 0;
		}

		MSG_WriteByte(&buf, svc_spawnbaseline);
		MSG_WriteDeltaEntity(&nullstate, ent, &buf, true);
	}

	MSG_WriteByte(&buf, svc_stufftext);
	MSG_WriteString(&buf, "precache -1\n"); // Q2: "precache\n"

	// Write it to the demo file.
	uint num = fwrite(&buf.cursize, 4, 1, cls.demofile);
	cls.demosavingok &= (num > 0); // H2

	num = fwrite(buf.data, buf.cursize, 1, cls.demofile);
	cls.demosavingok &= (num > 0); // H2

	// The rest of the demo file will be individual frames.
}

// Stop recording a demo.
void CL_Stop_f(void)
{
	if (!cls.demorecording)
	{
		Com_Printf("Not recording a demo.\n");
		return;
	}

	// Finish up.
	const int len = -1;
	const uint num = fwrite(&len, 4, 1, cls.demofile);
	cls.demosavingok &= (num > 0); // H2

	if (!cls.demosavingok) // H2
		Com_Printf("Error writing demo file.\n");

	fclose(cls.demofile);
	cls.demofile = NULL;
	cls.demorecording = false;

	Com_Printf("Stopped demo.\n");
}
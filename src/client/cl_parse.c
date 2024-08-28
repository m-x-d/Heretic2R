//
// cl_parse.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "sound.h"

char client_string[128];

void CL_RegisterSounds(void)
{
	S_BeginRegistration();

	if (fxe.RegisterSounds != NULL) // H2
		fxe.RegisterSounds();

	for (int i = 1; i < MAX_SOUNDS; i++)
	{
		if (cl.configstrings[CS_SOUNDS + i][0])
		{
			cl.sound_precache[i] = S_RegisterSound(cl.configstrings[CS_SOUNDS + i]);
			Sys_SendKeyEvents(); // Pump message loop
		}
	}

	S_EndRegistration();
}
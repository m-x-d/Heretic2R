//
// cl_parse.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "sound.h"

char client_string[128];

// H2 game messages
char game_message[1024];
int game_message_num_lines;
paletteRGBA_t game_message_color;
qboolean game_message_show_at_top;
float game_message_dispay_time;

int COLOUR(cvar_t* cvar)
{
	NOT_IMPLEMENTED
	return 0;
}

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
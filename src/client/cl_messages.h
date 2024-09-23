//
// cl_messages.h -- H2 game / level messages handling
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_Typedef.h"

#define MAX_MESSAGES	1000

typedef struct GameMessage_s
{
	char* message;
	char* sound_name;
} GameMessage_t;

extern char game_message[1024];
extern int game_message_num_lines;
extern paletteRGBA_t game_message_color;
extern qboolean game_message_show_at_top;
extern float game_message_dispay_time;

char* CL_GetGameString(int i);
char* CL_GetGameWav(int i);
char* CL_GetLevelString(int i);
char* CL_GetLevelWav(int i);
void CL_LoadStrings(void);
void CL_ClearGameMessages(void);

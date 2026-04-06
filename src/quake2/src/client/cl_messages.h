//
// cl_messages.h -- H2 game / level messages handling
//
// Copyright 1998 Raven Software
//

#pragma once

#define MAX_MESSAGES			1000
#define MAX_MESSAGE_LINE_LENGTH	60

typedef struct GameMessage_s
{
	char* message;
	char* sound_name;
} GameMessage_t;

extern char* CL_GetGameString(int i);
extern char* CL_GetGameWav(int i);
extern char* CL_GetLevelString(int i);
extern char* CL_GetLevelWav(int i);
extern void CL_LoadStrings(void);
extern void CL_ClearGameMessages(void);

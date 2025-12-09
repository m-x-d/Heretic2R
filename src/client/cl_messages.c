//
// cl_messages.c
//
// Copyright 1998 Raven Software
//

#include "cl_messages.h"
#include "cl_strings.h"
#include "qcommon.h"

static char* game_messages_text;
static GameMessage_t game_messages[MAX_MESSAGES];

static char* level_messages_text;
static GameMessage_t level_messages[MAX_MESSAGES];

static int LoadMessages(char* filename, char** message_out)
{
	char* buf;

	const int size = FS_LoadFile(filename, (void**)&buf);
	if (size < 1)
		Sys_Error("Unable to load %s", filename);

	*message_out = Z_Malloc(size + 1);
	memcpy(*message_out, buf, size);
	(*message_out)[size] = 0;

	FS_FreeFile(buf);

	return size + 1;
}

//TODO: lines starting with ';' char are supposed to be comments?
static void SplitMessages(char* src, GameMessage_t* dst, const int src_len)
{
	const char* end = src + src_len;

	for (int i = 1; src < end; i++)
	{
		if (i >= MAX_MESSAGES)
		{
			Com_Printf("Too many strings\n");
			return;
		}

		dst[i].message = NULL;
		dst[i].sound_name = NULL;

		char* newline = strchr(src, '\r');
		if (newline == NULL)
			return;

		*newline = '\0';

		// # signifies a .wav file
		char* snd_id = strchr(src, '#');
		if (snd_id != NULL && snd_id < newline)
		{
			dst[i].sound_name = snd_id + 1;
			*snd_id = '\0';
		}

		dst[i].message = src;

		// @ signifies a newline
		while (src != NULL)
		{
			src = strchr(src, '@');
			if (src == NULL)
				break;

			*src = '\n';
			src++;
		}

		src = newline + 2;
	}
}

char* CL_GetGameString(const int i)
{
	const int index = i & MESSAGE_MASK;
	if (index < MAX_MESSAGES) //mxd. '<= MAX_MESSAGES' in original logic.
		return game_messages[index].message;

	return NULL;
}

char* CL_GetGameWav(const int i)
{
	const int index = i & MESSAGE_MASK;
	if (index < MAX_MESSAGES) //mxd. '<= MAX_MESSAGES' in original logic.
		return game_messages[index].sound_name;

	return NULL;
}

char* CL_GetLevelString(const int i)
{
	const int index = i & MESSAGE_MASK;
	if (index < MAX_MESSAGES) //mxd. '<= MAX_MESSAGES' in original logic.
		return level_messages[index].message;

	return NULL;
}

char* CL_GetLevelWav(const int i)
{
	const int index = i & MESSAGE_MASK;
	if (index < MAX_MESSAGES) //mxd. '<= MAX_MESSAGES' in original logic.
		return level_messages[index].sound_name;

	return NULL;
}

void CL_LoadStrings(void)
{
	const cvar_t* cv = Cvar_Get("file_gamemsg", "gamemsg.txt", CVAR_ARCHIVE);
	int len = LoadMessages(cv->string, &game_messages_text);
	SplitMessages(game_messages_text, game_messages, len);

	cv = Cvar_Get("file_levelmsg", "levelmsg.txt", CVAR_ARCHIVE);
	len = LoadMessages(cv->string, &level_messages_text);
	SplitMessages(level_messages_text, level_messages, len);
}

void CL_ClearGameMessages(void)
{
	if (game_messages_text != NULL)
	{
		Z_Free(game_messages_text); //BUGFIX: mxd. Was not zfreed in original logic.
		game_messages_text = NULL;
	}

	if (level_messages_text != NULL)
	{
		Z_Free(level_messages_text); //BUGFIX: mxd. Was not zfreed in original logic.
		level_messages_text = NULL;
	}
}
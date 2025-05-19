//
// g_save.h
//
// Copyright 2025 mxd
//

#pragma once

#include "q_Typedef.h"

// Fields are needed for spawning from the entity string and saving / loading games.
#define FFL_SPAWNTEMP	1

typedef enum
{
	F_INT,
	F_FLOAT,
	F_LSTRING,	// String on disk, pointer in memory, TAG_LEVEL.
	F_GSTRING,	// String on disk, pointer in memory, TAG_GAME.
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,	// Index on disk, pointer in memory.
	F_ITEM,		// Index on disk, pointer in memory.
	F_CLIENT,	// Index on disk, pointer in memory.
	F_RGBA,
	F_RGB,
	F_IGNORE,
} fieldtype_t;

typedef struct
{
	const char* name;
	int ofs;
	fieldtype_t type;
	int flags;
} field_t;

#ifdef __cplusplus
extern "C"
{
#endif
	extern const field_t fields[];
#ifdef __cplusplus
}
#endif

typedef struct
{
	char* string;
	char* wav;
} trig_message_t;

#ifdef __cplusplus
extern "C"
{
#endif
	extern trig_message_t message_text[];
#ifdef __cplusplus
}
#endif

extern void WriteGame(char* filename, qboolean autosave);
extern void ReadGame(char* filename);
extern void WriteLevel(char* filename);
extern void ReadLevel(char* filename);
extern void InitGame(void);
extern void FreeLevelMessagesBuffer(void); //mxd
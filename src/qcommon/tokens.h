//
// tokens.h -- tokens for config strings
//
// Copyright 1998 Raven Software
//

#pragma once

// Sounds
#define TOKEN_S_AMBIENT		254
#define TOKEN_S_CINEMATICS	253
#define TOKEN_S_CORVUS		252
#define TOKEN_S_MISC		251
#define TOKEN_S_MONSTERS	250
#define TOKEN_S_WEAPONS		249
#define TOKEN_S_PLAYER		248
#define TOKEN_S_ITEMS		247

// Models
#define TOKEN_M_MODELS		254
#define TOKEN_M_MONSTERS	253
#define TOKEN_M_OBJECTS		252

//mxd. Data types to simplify tokens handling.
typedef struct
{
	char type;
	char* path;
} PackInfo_t;

extern PackInfo_t sound_pack_infos[];
extern PackInfo_t model_pack_infos[];
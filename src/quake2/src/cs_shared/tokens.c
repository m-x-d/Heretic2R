//
// tokens.c
//
// Copyright 1998 Raven Software
//

#include "tokens.h"

PackInfo_t sound_pack_infos[] =
{
	{ TOKEN_S_AMBIENT,		"ambient" },
	{ TOKEN_S_CINEMATICS,	"cinematics" },
	{ TOKEN_S_CORVUS,		"corvus" },
	{ TOKEN_S_MISC,			"misc" },
	{ TOKEN_S_MONSTERS,		"monsters" },
	{ TOKEN_S_WEAPONS,		"weapons" },
	{ TOKEN_S_PLAYER,		"player" },
	{ TOKEN_S_ITEMS,		"items" },
	{ 0, 0 }
};

PackInfo_t model_pack_infos[] =
{
	{ TOKEN_M_OBJECTS,		"models/objects" },
	{ TOKEN_M_MONSTERS,		"models/monsters" },
	{ TOKEN_M_MODELS,		"models" },
	{ 0, 0 }
};
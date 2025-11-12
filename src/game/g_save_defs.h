//
// g_save_defs.h
//
// Copyright 2025 mxd
//

#pragma once

#include "g_local.h"

// Connects a human-readable function signature with the corresponding pointer.
typedef struct func_map_s
{
	char* name;
	byte* address;
} func_map_t;

// Connects a human-readable animmove_t name with the corresponding pointer.
typedef struct animmove_map_s
{
	char* name;
	const animmove_t* address;
} animmove_map_t;

extern func_map_t funcs_list[];
extern animmove_map_t mmoves_list[];
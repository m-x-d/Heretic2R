//
// snd_loc.h -- private sound functions
//
// Copyright 1998 Raven Software
//

#pragma once

#include "q_shared.h"

typedef struct
{
	int length;
	int loopstart;
	int speed; // Not needed, because converted on load?
	int width;
	int stereo;
	byte data[1]; // Variable sized
} sfxcache_t;

typedef struct sfx_s
{
	char name[MAX_QPATH];
	int registration_sequence;
	sfxcache_t* cache;
	char* truename;
} sfx_t;
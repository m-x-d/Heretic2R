//
// snd_local.h -- Structures local to sound library.
//
// Copyright 2025 mxd
//

#pragma once

#include "client.h"

extern snd_import_t si;

#pragma region ========================== Sound structures ==========================

//mxd. Moved from snd_loc.h. Not used outside of sound library.
typedef struct sfxcache_s
{
	int length;
	int loopstart;
	int speed; // Not needed, because converted on load?
	int width;
	int stereo;
	byte data[1]; // Variable-sized.
} sfxcache_t;

//mxd. Moved from snd_loc.h.
typedef struct sfx_s
{
	char name[MAX_QPATH];
	int registration_sequence;
	sfxcache_t* cache;
} sfx_t;

// Interface to pass data and metadata between the frontend and the backends.
typedef struct
{
	int channels;
	int samples;			// Mono samples in buffer.
	int submission_chunk;	// Don't mix less than this #.
	int samplepos;			// In mono samples.
	int samplebits;
	int speed;
	byte* buffer;
} sound_t; // Q2: dma_t;

#pragma endregion
//
// snd_local.h -- Structures local to sound library.
//
// Copyright 2025 mxd
//

#pragma once

#include "q_shared.h"

#define SNDLIB_DECLSPEC __declspec(dllexport)

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
	//char* truename; //mxd. Missing in H2.
} sfx_t;

typedef struct
{
	int channels;
	int samples;			// Mono samples in buffer.
	int submission_chunk;	// Don't mix less than this #.
	int samplepos;			// In mono samples.
	int samplebits;
	int speed;
	byte* buffer;
} dma_t;
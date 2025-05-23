//
// snd_local.h -- Structures local to sound library.
//
// Copyright 2025 mxd
//

#pragma once

#include "Debug.h" //TODO: remove

#define SNDLIB_DECLSPEC __declspec(dllexport)

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
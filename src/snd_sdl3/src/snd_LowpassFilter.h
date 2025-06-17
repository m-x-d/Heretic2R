//
// snd_LowpassFilter.h
//
// Copyright(C) 2010, 2013 Yamagi Burmeister
// Copyright(C) 2005 Ryan C.Gordon
//

#pragma once

#include "snd_local.h"

#define LPF_REFERENCE_FREQUENCY	5000.0f
#define LPF_DEFAULT_GAIN_HF		0.25f

typedef struct
{
	float a;
	float gain_hf;
	portable_samplepair_t history[2];
	qboolean is_history_initialized;
} LpfContext_t;

extern void LPF_Initialize(LpfContext_t* lpf_context, float gain_hf, int target_frequency);
extern void LPF_UpdateSamples(LpfContext_t* lpf_context, int sample_count, portable_samplepair_t* samples);
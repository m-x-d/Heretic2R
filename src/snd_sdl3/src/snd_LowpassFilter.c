//
// snd_LowpassFilter.c
//
// Copyright(C) 2010, 2013 Yamagi Burmeister
// Copyright(C) 2005 Ryan C.Gordon
//

#include "snd_LowpassFilter.h"

void LPF_Initialize(LpfContext_t* lpf_context, const float gain_hf, const int target_frequency)
{
#define K_TO_PI	6.283185307f

	assert(target_frequency > 0);
	assert(lpf_context != NULL);

	const float g = Clamp(gain_hf, 0.01f, 1.0f);
	const float cw = cosf(K_TO_PI * LPF_REFERENCE_FREQUENCY / (float)target_frequency);
	float a = 0.0f;

	if (g < 0.9999f)
		a = (1.0f - (g * cw) - sqrtf((2.0f * g * (1.0f - cw)) - (g * g * (1.0f - (cw * cw))))) / (1.0f - g);

	lpf_context->a = a;
	lpf_context->gain_hf = gain_hf;
	lpf_context->is_history_initialized = false;
}

void LPF_UpdateSamples(LpfContext_t* lpf_context, int sample_count, portable_samplepair_t* samples)
{
	NOT_IMPLEMENTED
}
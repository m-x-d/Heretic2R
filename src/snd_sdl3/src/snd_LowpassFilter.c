//
// snd_LowpassFilter.c
//
// Copyright(C) 2010, 2013 Yamagi Burmeister
// Copyright(C) 2005 Ryan C.Gordon
//

#include "snd_LowpassFilter.h"

void LPF_Initialize(LpfContext_t* lpf_context, const float gain_hf, const int target_frequency) // YQ2 (lpf_initialize).
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

void LPF_UpdateSamples(LpfContext_t* lpf_context, const int sample_count, portable_samplepair_t* samples) // YQ2 (lpf_update_samples).
{
	assert(lpf_context != NULL);
	assert(sample_count >= 0);
	assert(samples != NULL);

	if (sample_count <= 0)
		return;

	const float a = lpf_context->a;
	portable_samplepair_t* history = lpf_context->history;

	if (!lpf_context->is_history_initialized)
	{
		lpf_context->is_history_initialized = true;

		for (int s = 0; s < 2; s++)
		{
			history[s].left = 0;
			history[s].right = 0;
		}
	}

	for (int s = 0; s < sample_count; s++)
	{
		portable_samplepair_t y;

		// Update left channel.
		y.left = samples[s].left;

		y.left = (int)((float)y.left + a * (float)(history[0].left - y.left));
		history[0].left = y.left;

		y.left = (int)((float)y.left + a * (float)(history[1].left - y.left));
		history[1].left = y.left;


		// Update right channel.
		y.right = samples[s].right;

		y.right = (int)((float)y.right + a * (float)(history[0].right - y.right));
		history[0].right = y.right;

		y.right = (int)((float)y.right + a * (float)(history[1].right - y.right));
		history[1].right = y.right;

		// Update sample.
		samples[s] = y;
	}
}
//
// ce_DLight.h
//
// Copyright 1998 Raven Software
//

#pragma once

#include "client.h"

typedef struct CE_DLight_s
{
	paletteRGBA_t color;
	float intensity;
	float d_intensity;

	//mxd. Color fading settings.
	int fade_start_time;
	int fade_end_time;
	vec3_t fade_color_start;
	vec3_t fade_color_end;
} CE_DLight_t;

extern void InitDLightMngr(void);
extern void ReleaseDLightMngr(void);

extern struct CE_DLight_s* CE_DLight_new(paletteRGBA_t color, float intensity, float d_intensity);
extern void CE_DLight_delete(struct CE_DLight_s* to_delete);
extern void CE_DLight_SetColorFade(struct CE_DLight_s* self, float end_r, float end_g, float end_b, int duration); //mxd
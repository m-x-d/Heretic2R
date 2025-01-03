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
} CE_DLight_t;

extern void InitDLightMngr(void);
extern void ReleaseDLightMngr(void);

extern struct CE_DLight_s* CE_DLight_new(paletteRGBA_t color, float intensity, float d_intensity);
extern void CE_DLight_delete(struct CE_DLight_s* to_delete);
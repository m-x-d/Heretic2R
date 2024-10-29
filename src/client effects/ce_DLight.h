#ifndef CLIENT_H
#define CLIENT_H
#include "client.h"
#endif

typedef struct CE_DLight_s
{
	paletteRGBA_t color;

	float intensity;
	float d_intensity;
} CE_DLight_t;

void InitDLightMngr();
void ReleaseDLightMngr();

struct CE_DLight_s *CE_DLight_new(paletteRGBA_t color, float intensity, float d_intensity);
void CE_DLight_delete(struct CE_DLight_s *toDelete);

#if 0
dlight = CE_DLight_new(, , );
#endif
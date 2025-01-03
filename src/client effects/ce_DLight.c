//
// ce_DLight.c
//
// Copyright 1998 Raven Software
//

#include "ce_Dlight.h"
#include "ResourceManager.h"

static ResourceManager_t dlight_manager;

void InitDLightMngr(void)
{
#define DLIGHT_BLOCK_SIZE 32

	ResMngr_Con(&dlight_manager, sizeof(CE_DLight_t), DLIGHT_BLOCK_SIZE);
}

void ReleaseDLightMngr(void)
{
	ResMngr_Des(&dlight_manager);
}

struct CE_DLight_s* CE_DLight_new(const paletteRGBA_t color, const float intensity, const float d_intensity)
{
	CE_DLight_t* dl = ResMngr_AllocateResource(&dlight_manager, sizeof(*dl));

	dl->color = color;
	dl->intensity = intensity;
	dl->d_intensity = d_intensity;

	return dl;
}

void CE_DLight_delete(struct CE_DLight_s* to_delete)
{
	ResMngr_DeallocateResource(&dlight_manager, to_delete, sizeof(*to_delete));
}
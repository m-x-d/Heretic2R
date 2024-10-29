#include "ce_Dlight.h"
#include "ResourceManager.h"

ResourceManager_t DLightMngr;

void InitDLightMngr()
{
#define DLIGHT_BLOCK_SIZE 32

	ResMngr_Con(&DLightMngr, sizeof(CE_DLight_t), DLIGHT_BLOCK_SIZE);
}

void ReleaseDLightMngr()
{
	ResMngr_Des(&DLightMngr);
}

struct CE_DLight_s *CE_DLight_new(paletteRGBA_t color, float intensity, float d_intensity)
{
	CE_DLight_t *newDLight;

	newDLight = ResMngr_AllocateResource(&DLightMngr, sizeof(*newDLight));

	newDLight->color = color;

	newDLight->intensity = intensity;

	newDLight->d_intensity = d_intensity;

	return newDLight;
}

void CE_DLight_delete(struct CE_DLight_s *toDelete)
{
	ResMngr_DeallocateResource(&DLightMngr, toDelete, sizeof(*toDelete));	
}
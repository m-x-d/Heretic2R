//
// fx_WaterSplash.c -- Named Ambient Effects.c in original logic.
//
// Copyright 1998 Raven Software
//

#include "fx_WaterSplash.h"
#include "Client Effects.h"
#include "g_playstats.h"
#include "Particle.h"
#include "Random.h"

void DoWaterSplash(client_entity_t* effect, paletteRGBA_t color, int count, const qboolean use_lightmap_color)
{
	count = min(500, count);
	const int extra_flags = ((use_lightmap_color && R_DETAIL > DETAIL_HIGH) ? PFL_LM_COLOR : 0); //mxd

	for (int i = 0; i < count; i++)
	{
		color.a = (byte)irand(164, 240); //mxd. Randomize alpha a bit.
		client_particle_t* p = ClientParticle_new(PART_8x8_BUBBLE | extra_flags, color, 1000); //mxd. Use smaller sprite to better match drips size (original logic uses PART_16x16_WATERDROP).

		p->d_alpha = flrand(-128.0f, -96.0f); //mxd. Randomize d_alpha a bit.
		p->scale = flrand(0.7f, 1.2f); //mxd. Randomize scale a bit.
		p->d_scale = flrand(-1.1f, -0.8f); //mxd. Randomize d_scale a bit.
		p->velocity[0] = flrand(-20.0f, 20.0f);
		p->velocity[1] = flrand(-20.0f, 20.0f);
		p->velocity[2] = flrand(20.0f, 30.0f);

		AddParticleToList(effect, p);
	}
}

void FXWaterSplash(centity_t* owner, const int type, const int flags, vec3_t origin) //mxd. Named 'WaterSplash' in original logic.
{
	int count;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPLASH].formatString, &count);

	client_entity_t* effect = ClientEntity_new(type, flags, origin, NULL, 500);
	effect->flags |= (CEF_NO_DRAW | CEF_NOMOVE);

	AddEffect(NULL, effect);

	DoWaterSplash(effect, color_white, count, false); //mxd. Use color_white.
}
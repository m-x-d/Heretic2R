//
// fx_spellchange.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "ce_DLight.h"

#define NUM_SPELL_BITS	12
#define LIGHT_LIFETIME	1000

static qboolean SpellChangeDlightThink(struct client_entity_s* self, centity_t* owner)
{
	if (fx_time - self->startTime <= LIGHT_LIFETIME)
	{
		self->dlight->intensity = 200.0f * (float)(LIGHT_LIFETIME - (fx_time - self->startTime)) / (float)LIGHT_LIFETIME;
		return true;
	}

	return false;
}

void FXSpellChange(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	paletteRGBA_t color;
	int part;

	vec3_t dir;
	int spell_type = 0;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_SPELL_CHANGE].formatString, dir, &spell_type);

	switch (spell_type)
	{
		case 1: // Red / fireball.
			color.c = 0xFF0000FF;
			part = PART_16x16_SPARK_R;
			break;

		case 2: // Indigo / array.
			color.c = 0xFFFF0080;
			part = PART_16x16_SPARK_I;
			break;

		case 3: // Blue / sphere.
			color.c = 0xFFFF0000;
			part = PART_16x16_SPARK_B;
			break;

		case 4: // Green / mace ball.
			color.c = 0xFF00FF00;
			part = PART_16x16_SPARK_G;
			break;

		case 5: // Yellow / firewall.
			color.c = 0xFF0080FF;
			part = PART_16x16_SPARK_Y;
			break;

		case 6: // Big red / red rain bow. //TODO: same as case 1...
			color.c = 0xFF0000FF;
			part = PART_16x16_SPARK_R;
			break;

		case 7: // Big yellow / phoenix.
			color.c = 0xFF00FFFF;
			part = PART_32x32_FIRE1;
			break;

		case 0: // Default color--white.
		default:
			color.c = 0x80FFFFFF;
			part = PART_16x16_LIGHTNING;
			break;
	}

	VectorScale(dir, -32.0f, dir);

	// Create the new effect.
	client_entity_t* spell_puff = ClientEntity_new(type, (int)(flags | CEF_OWNERS_ORIGIN | CEF_NO_DRAW | CEF_ADDITIVE_PARTS), origin, NULL, 100);

	spell_puff->radius = 32.0f;
	spell_puff->dlight = CE_DLight_new(color, 150.0f, 0.0f);
	spell_puff->startTime = fx_time;
	spell_puff->Update = SpellChangeDlightThink;

	// Attach some particles to it.
	for (int i = 0; i < NUM_SPELL_BITS; i++)
	{
		client_particle_t* spell_bit = ClientParticle_new(part, color_white, 500);

		VectorSet(spell_bit->velocity, flrand(-32.0f, 32.0f), flrand(-32.0f, 32.0f), flrand(16.0f, 64.0f));
		VectorAdd(dir, spell_bit->velocity, spell_bit->velocity);
		spell_bit->d_scale = -2.0f;
		spell_bit->scale = 6.0f;

		AddParticleToList(spell_puff, spell_bit);
	}

	AddEffect(owner, spell_puff);
}
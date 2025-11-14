//
// fx_tome.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "ce_dlight.h"

#define TOME_RADIUS				6.3f //mxd. 5.0 in original logic. Updated to match model dimension.
#define TOME_VRADIUS			7.9f //mxd
#define TOME_SCALE				0.55f //mxd
#define TOME_ORBIT_DIST			20.0f
#define TOME_ORBIT_SCALE		0.0025f
#define TOME_SPIN_FACTOR		0.004f

#define TOME_SPARK_SCALE		10.0f //mxd. Named TOME_SCALE in original logic.
#define TOME_SPARK_ACCELERATION	(-64.0f) //mxd. Named TOME_SPARK_ACCEL in original logic.
#define TOME_SPARKS_SPAWN_RATE	100 //mxd
#define TOME_NUM_SPARKS			4 //mxd

#define TOME_FADEIN_ANIM_LENGTH		500.0f //mxd
#define TOME_FADEOUT_ANIM_LENGTH	1000.0f //mxd

static struct model_s* tome_model;
static struct sfx_s* tome_expired_sound; //mxd

void PreCacheTome(void)
{
	tome_model = fxi.RegisterModel("models/Spells/book/tris.fm");
}

void PreCacheTomeSFX(void) //mxd
{
	tome_expired_sound = fxi.S_RegisterSound("monsters/mork/shieldgone.wav"); // Unused sound in original logic.
}

// Update the position of the Tome of Power relative to its owner.
static void TomeOfPowerAnimate(client_entity_t* tome, const centity_t* owner)
{
	const float time = (float)fx_time; //mxd
	const float step = time - (float)tome->nextThinkTime;

	vec3_t pos =
	{
		cosf(time * TOME_ORBIT_SCALE) * TOME_ORBIT_DIST,
		sinf(time * TOME_ORBIT_SCALE) * TOME_ORBIT_DIST,
		15.0f + sinf(time * 0.0015f) * 10.0f
	};

	//mxd. Update dynamic light.
	tome->dlight->intensity = 150.0f + cosf(time * 0.01f) * 20.0f;

	//mxd. Book appear animation.
	if (fx_time < tome->tome_fadein_end_time)
	{
		const float lerp = (float)(tome->tome_fadein_end_time - fx_time) / TOME_FADEIN_ANIM_LENGTH; // [1.0 .. 0.0]
		const float scaler = 1.0f + (1.0f - cosf(lerp * ANGLE_90)) * 2.0f;
		pos[0] *= scaler;
		pos[1] *= scaler;

		// Rotate the book.
		tome->r.angles[YAW] += step * max(0.01f * lerp, TOME_SPIN_FACTOR);

		// Scale-in the book.
		if (tome->r.scale < TOME_SCALE)
			tome->r.scale += step * 0.002f;

		tome->dlight->intensity *= 1.0f - lerp;
	}
	else if (fx_time < tome->tome_fadeout_end_time) //mxd. Book disappear animation.
	{
		const float lerp = 1.0f - (float)(tome->tome_fadeout_end_time - fx_time) / TOME_FADEOUT_ANIM_LENGTH; // [0.0 .. 1.0]
		const float oz = 25.0f + (1.0f - cosf(lerp * ANGLE_90)) * 32.0f;
		pos[2] = LerpFloat(pos[2], oz, lerp);

		// Rotate the book.
		tome->r.angles[YAW] += step * max(lerp * 0.02f, TOME_SPIN_FACTOR);

		// Scale-out the book.
		if (lerp > 0.75f)
			tome->r.scale = max(0.001f, tome->r.scale - step * 0.002f);

		tome->dlight->intensity *= 1.0f - lerp;
	}
	else
	{
		// Rotate the book.
		tome->r.angles[YAW] += step * TOME_SPIN_FACTOR;
	}

	VectorAdd(owner->origin, pos, tome->r.origin);
}

// Spawn Tome of Power sparks. Disabled in original logic --mxd.
static void TomeOfPowerSpawnSparks(client_entity_t* tome)
{
	const float radius = TOME_RADIUS * tome->r.scale;
	const float vradius = TOME_VRADIUS * tome->r.scale;
	const vec3_t right = { sinf(tome->r.angles[YAW]), -cosf(tome->r.angles[YAW]), 0.0f };

	vec3_t p1;
	VectorScale(right, radius, p1);

	vec3_t p2;
	VectorScale(right, -radius, p2);

	for (int i = 0; i < TOME_NUM_SPARKS; i++)
	{
		const paletteRGBA_t color = //mxd. Randomize color a bit.
		{
			.r = (byte)(tome->color.r + irand(-16, 16)),
			.g = (byte)(tome->color.g + irand(-16, 16)),
			.b = (byte)(tome->color.b + irand(-32, 0)),
			.a = (byte)(tome->color.a + irand(-16, 16)),
		};

		client_particle_t* spark = ClientParticle_new(PART_16x16_STAR, color, 1000);

		//mxd. Spawn below tome model, distribute evenly, align with book frame.
		vec3_t offset;
		const float lerp = (float)i / (TOME_NUM_SPARKS - 1);
		VectorLerp(p1, lerp, p2, offset);

		spark->origin[0] = tome->r.origin[0] + offset[0];
		spark->origin[1] = tome->r.origin[1] + offset[1];
		spark->origin[2] = tome->r.origin[2] - vradius - 1.0f;

		spark->scale = TOME_SPARK_SCALE * flrand(0.75f, 1.25f) * tome->r.scale; //mxd. Randomize scale a bit.
		VectorSet(spark->velocity, flrand(-20.0f, 20.0f), flrand(-20.0f, 20.0f), flrand(-10.0f, 10.0f));
		spark->acceleration[2] = TOME_SPARK_ACCELERATION + flrand(-8.0f, 8.0f); //mxd. Randomize acceleration a bit.
		spark->d_scale = flrand(-20.0f, -15.0f);
		spark->d_alpha = flrand(-500.0f, -400.0f);

		AddParticleToList(tome, spark);
	}
}

// Update the Tome of power, so that more sparkles zip out of it, and the light casts pulses.
static qboolean TomeOfPowerThink(client_entity_t* tome, centity_t* owner)
{
	//mxd. Fade-out effect ended, remove entity.
	if (tome->tome_fadeout_end_time > 0 && tome->tome_fadeout_end_time <= fx_time)
		return false;

	//mxd. Start fade-out effect?
	if (!(owner->current.effects & EF_POWERUP_ENABLED) && tome->tome_fadeout_end_time == 0)
	{
		fxi.S_StartSound(tome->r.origin, -1, CHAN_ITEM, tome_expired_sound, 1.0f, ATTN_NORM, 0.0f); //mxd. Add expire sound.
		tome->tome_fadeout_end_time = fx_time + (int)TOME_FADEOUT_ANIM_LENGTH;
	}

	TomeOfPowerAnimate(tome, owner);

	if (tome->lastThinkTime < fx_time)
	{
		TomeOfPowerSpawnSparks(tome);
		tome->lastThinkTime = fx_time + TOME_SPARKS_SPAWN_RATE;
	}

	return true;
}

// Original version of the tome of power. Casts a blue light etc.
void FXTomeOfPower(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* tome = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. next_think_time:100 in original logic.

	tome->radius = 128.0f;
	tome->r.model = &tome_model;
	tome->flags |= (CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS);
	tome->r.scale = 0.001f;
	COLOUR_SETA(tome->color, 32, 32, 255, 229); //mxd. Use macro.

	tome->lastThinkTime = fx_time;
	tome->tome_fadein_end_time = fx_time + (int)TOME_FADEIN_ANIM_LENGTH; //mxd

	tome->dlight = CE_DLight_new(tome->color, 150.0f, 0.0f);
	tome->Update = TomeOfPowerThink;

	AddEffect(owner, tome);
}
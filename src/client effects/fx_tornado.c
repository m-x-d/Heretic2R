//
// fx_tornado.c
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Utilities.h"
#include "Vector.h"
#include "ce_DLight.h"
#include "g_playstats.h"

#define TORNADO_RADIUS					80.0f

#define BALL_RADIUS						40
#define BALL_PARTICLE_SCALE				10.0f
#define BALL_PARTICLES_COUNT			4
#define BALL_EXPLOSION_PARTICLES_COUNT	20

static struct model_s* tornado_models[2];

void PreCacheTornado(void)
{
	tornado_models[0] = fxi.RegisterModel("sprites/fx/haloblue.sp2");
	tornado_models[1] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

// Make the tornado ball spark.
static qboolean TornadoBallThink(struct client_entity_s* self, centity_t* owner)
{
	if (owner->current.effects & EF_SPEED_ACTIVE)
	{
		if (self->alpha == 0.0f)
			return false;

		self->alpha -= 1.0f;
		return true;
	}

	const int count = GetScaledCount(BALL_PARTICLES_COUNT, 0.4f);

	for (int i = 0; i < count; i++)
	{
		const int part = (irand(0, 1) ? PART_16x16_LIGHTNING : PART_16x16_SPARK_B);
		client_particle_t* ce = ClientParticle_new(part | PFL_ADDITIVE, color_white, 1000);

		ce->color.a = 245;
		ce->scale = BALL_PARTICLE_SCALE;
		ce->d_scale = -0.5f * BALL_PARTICLE_SCALE;

		const vec3_t angles = { flrand(180.0f, 360.0f), flrand(0.0f, 360.0f), 0.0f };

		vec3_t fwd;
		AngleVectors(angles, fwd, NULL, NULL);
		VectorScale(fwd, BALL_RADIUS, ce->velocity);

		VectorScale(ce->velocity, -1.0f, ce->acceleration);

		AddParticleToList(self, ce);
	}

	self->updateTime = 150;

	return true;
}

// Create the ball that gets tossed out of Crovus when he casts the tornado spell.
void FXTornadoBall(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* glow = ClientEntity_new(type, flags | CEF_VIEWSTATUSCHANGED, origin, NULL, 60);

	glow->radius = 50.0f;
	glow->r.model = &tornado_models[0]; // Blue halo model.
	glow->r.flags = (RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA);
	glow->r.scale = 0.4f;
	glow->LifeTime = fx_time + (int)(TORNADO_DURATION * 1000.0f) + 200;
	glow->Update = TornadoBallThink;

	AddEffect(owner, glow);
}

static qboolean TornadoThink(struct client_entity_s* self, centity_t* owner)
{
	// If the effect is dead, just return.
	if (!(owner->current.effects & EF_SPEED_ACTIVE))
	{
		self->alpha -= 1.0f;
		return true;
	}

	const paletteRGBA_t color = { .c = 0xc0ffffff };
	const int count = GetScaledCount(5, 0.8f);

	for (int i = 0; i < count; i++)
	{
		int part;
		float scale;

		if (irand(0, 1))
		{
			part = PART_16x16_SPARK_B;
			scale = flrand(6.0f, 8.0f);
		}
		else
		{
			part = PART_16x16_LIGHTNING;
			scale = flrand(4.0f, 5.0f);
		}

		client_particle_t* ce = ClientParticle_new((int)(part | PFL_NEARCULL | PFL_MOVE_CYL_Z), color, 1750);

		ce->scale = scale;
		ce->d_scale = (r_detail->value + 1.0f) * 2.0f;

		if (R_DETAIL == DETAIL_LOW)
			ce->d_alpha = -220.0f;
		else if (R_DETAIL == DETAIL_NORMAL)
			ce->d_alpha = -200.0f;
		else
			ce->d_alpha = -180.0f; //TODO: separate case for DETAIL_UBERHIGH.

		if (irand(0, 1))
		{
			ce->type |= PFL_PULSE_ALPHA;
			ce->d_alpha *= -2.0f;
			ce->color.a = 1;
		}

		ce->origin[CYL_RADIUS] = 0.1f * TORNADO_RADIUS;
		ce->origin[CYL_YAW] = flrand(0.0f, ANGLE_360);
		ce->origin[CYL_Z] = 0.0f;

		ce->acceleration[CYL_RADIUS] = TORNADO_RADIUS;
		ce->acceleration[CYL_YAW] = ANGLE_360 * 2.5f;
		ce->acceleration[CYL_Z] = flrand(TORNADO_RADIUS * 1.5f, TORNADO_RADIUS * 1.75f);

		AddParticleToList(self, ce);
	}

	TornadoBallThink(self, owner);

	return true;
}

void FXTornado(centity_t* owner, const int type, int flags, vec3_t origin)
{
	int duration;
	if (R_DETAIL == DETAIL_LOW)
		duration = 150;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 100;
	else
		duration = 60; //TODO: separate case for DETAIL_UBERHIGH.

	flags &= ~CEF_OWNERS_ORIGIN;
	flags |= (CEF_ADDITIVE_PARTS | CEF_CHECK_OWNER | CEF_DONT_LINK | CEF_NO_DRAW);
	client_entity_t* base = ClientEntity_new(type, flags, origin, NULL, duration);

	base->radius = 50.0f;
	base->Update = TornadoThink;

	if (R_DETAIL >= DETAIL_HIGH)
	{
		const paletteRGBA_t color = { .c = 0xffff4444 };
		base->dlight = CE_DLight_new(color, 170.0f, 0.0f);
	}

	AddEffect(owner, base);
}

// Explode the ball in the middle of the shrine.
void FXTornadoBallExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	// Create the dummy entity, so particles can be attached.
	client_entity_t* base = ClientEntity_new(type, (int)(flags | CEF_NO_DRAW) & ~CEF_NOMOVE, origin, NULL, 1400);

	base->radius = 100.0f;
	AddEffect(NULL, base);

	int count = GetScaledCount(BALL_EXPLOSION_PARTICLES_COUNT, 0.4f);

	// Create a bunch of exploding particles.
	for (int i = 0; i < count; i++)
	{
		const int part = (irand(0, 1) ? PART_16x16_LIGHTNING : PART_16x16_SPARK_B);
		client_particle_t* ce = ClientParticle_new(part, color_white, 1150);

		ce->color.a = 245;
		ce->scale = BALL_PARTICLE_SCALE;
		ce->d_scale = -0.5f * BALL_PARTICLE_SCALE;

		const vec3_t angles = { flrand(0.0f, 360.0f), flrand(0.0f, 360.0f), 0.0f };

		vec3_t fwd;
		AngleVectors(angles, fwd, NULL, NULL);
		VectorScale(fwd, BALL_RADIUS, ce->velocity);
		VectorScale(ce->velocity, -0.7f, ce->acceleration);

		AddParticleToList(base, ce);
	}

	// Create a bunch of exploding particles.
	count *= 3;

	for (int i = 0; i < count; i++)
	{
		const byte c = (byte)irand(0, 255);
		const paletteRGBA_t color = { .r = 255, .g = c, .b = c, .a = 255 };

		client_particle_t* ce = ClientParticle_new(PART_4x4_WHITE | PFL_SOFT_MASK, color, 400);

		VectorSet(ce->origin, 1.0f, 1.0f, 1.0f);

		const vec3_t angles = { flrand(0.0f, ANGLE_360), flrand(0.0f, ANGLE_360), flrand(0.0f, ANGLE_360) };
		DirFromAngles(angles, ce->origin);

		Vec3ScaleAssign(flrand(40.0f, 50.0f), ce->origin);
		VectorScale(ce->origin, -15.1f, ce->acceleration);

		AddParticleToList(base, ce);
	}

	// Add an additional flash as well.
	client_entity_t* flash = ClientEntity_new(-1, flags, origin, NULL, 250);

	flash->radius = 64.0f;
	flash->r.model = &tornado_models[1]; // Halo sprite.
	flash->r.flags |= RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	flash->r.frame = 1;
	flash->d_alpha = -4.0f;
	flash->d_scale = -4.0f;

	AddEffect(NULL, flash);
}
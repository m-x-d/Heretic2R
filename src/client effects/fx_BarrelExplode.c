//
// fx_BarrelExplode.c -- Named 'fx_objects.c' in original logic --mxd.
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_DLight.h"
#include "g_playstats.h"

#define BARREL_EXPLODE_SPEED					140.0f // H2: 80
#define BARREL_EXPLODE_GRAVITY					(-320.0f)
#define BARREL_EXPLODE_SCALE					14.0f
#define BARREL_EXPLODE_BALL_COLUMNS				5
#define BARREL_EXPLODE_BALLS_PER_COLUMN			3
#define BARREL_EXPLODE_NUM_BITS					24 // H2: 16
#define BARREL_EXPLODE_NUM_SMOKE_BITS			8 //mxd
#define BARREL_EXPLODE_SMOKE_LIFETIME			1500 //mxd
#define BARREL_EXPLODE_INITIAL_DLIGHT_RADIUS	86.0f //mxd

static struct model_s* explosion_models[2];
static struct sfx_s* explosion_sound; //mxd

void PreCacheBarrelExplode(void) //mxd. Named 'PreCacheObjects' in original logic.
{
	explosion_models[0] = fxi.RegisterModel("models/fx/explosion/outer/tris.fm");
	explosion_models[1] = fxi.RegisterModel("models/fx/explosion/inner/tris.fm"); //mxd
}

void PreCacheBarrelExplodeSFX(void) //mxd
{
	explosion_sound = fxi.S_RegisterSound("weapons/PhoenixHit.wav");
}

static qboolean SmallBarrelExplosionUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXPhoenixExplosionSmallBallThink' in original logic.
{
	if (self->alpha < 0.01f)
		return false;

	const float vel_factor = (float)(fx_time - self->lastThinkTime) / 2000.0f;
	self->lastThinkTime = fx_time;

	// Spin the ball of fire while it expands and fades.
	self->r.angles[YAW] += self->velocity2[YAW] * vel_factor;
	self->r.angles[PITCH] += self->velocity2[PITCH] * vel_factor;

	return true;
}

static qboolean BarrelExplosionUpdate(client_entity_t* self, centity_t* owner) //mxd. Original logic used FXPhoenixExplosionBallThink().
{
	if (!SmallBarrelExplosionUpdate(self, owner))
	{
		//mxd. Fade-out dlight first...
		if (self->dlight->color.r > 16)
		{
			self->dlight->color.r = (byte)((float)self->dlight->color.r * 0.95f);
			self->dlight->color.g = (byte)((float)self->dlight->color.g * 0.85f);
			self->dlight->intensity *= 0.995f;

			return true;
		}

		//mxd. Allow lingering smoke to expire...
		self->Update = RemoveSelfAI;
		self->updateTime = BARREL_EXPLODE_SMOKE_LIFETIME;
		self->dlight->intensity = 0.0f;
		self->flags |= CEF_NO_DRAW;

		return true;
	}

	const float dlight_lerp = (self->r.scale - 1.0f) / 1.26f; // r.scale: [1.0 .. 2.26].
	self->dlight->intensity = BARREL_EXPLODE_INITIAL_DLIGHT_RADIUS + BARREL_EXPLODE_RADIUS * dlight_lerp;
	self->dlight->intensity = min(BARREL_EXPLODE_RADIUS * 1.15f, self->dlight->intensity);

	return true;
}

// Create FX_BARREL_EXPLODE effect.
void FXBarrelExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	static const paletteRGBA_t explosion_color = { .c = 0xff00ffff };

	// Create smaller explosion spheres.
	int count = irand(BARREL_EXPLODE_BALL_COLUMNS - 2, BARREL_EXPLODE_BALL_COLUMNS);
	float angle = flrand(0.0f, ANGLE_360);
	float angle_increment = ANGLE_360 / (float)count;

	for (int i = 0; i < count; i++)
	{
		vec3_t angles;
		angles[PITCH] = flrand(-ANGLE_85, -ANGLE_35);
		angles[YAW] = angle + flrand(-angle_increment * 0.3f, angle_increment * 0.3f);
		angles[ROLL] = 0.0f;

		angle += angle_increment;

		vec3_t balls_dir;
		RealAngleVectors(angles, balls_dir, NULL, NULL);

		vec3_t balls_offset;
		VectorScale(balls_dir, flrand(78.0f, 96.0f), balls_offset);
		Vec3AddAssign(origin, balls_offset);

		for (int c = 0; c < BARREL_EXPLODE_BALLS_PER_COLUMN; c++)
		{
			const float pos_scaler = 0.25f + (float)c / (float)BARREL_EXPLODE_BALLS_PER_COLUMN * 0.75f;
			const float size_scaler = 1.0f - pos_scaler;

			vec3_t ball_pos;
			VectorLerp(origin, pos_scaler, balls_offset, ball_pos);

			client_entity_t* sub_explosion = ClientEntity_new(FX_BARREL_EXPLODE, CEF_BROADCAST, ball_pos, NULL, 0); //mxd. Update each frame.

			sub_explosion->radius = 128.0f;
			sub_explosion->r.model = &explosion_models[1]; // Inner explosion model.
			sub_explosion->r.frame = irand(0, 1); //mxd
			sub_explosion->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT | RF_FULLBRIGHT); //mxd. +RF_FULLBRIGHT flag.
			sub_explosion->r.angles[YAW] = flrand(-ANGLE_180, ANGLE_180); //mxd
			sub_explosion->r.angles[PITCH] = flrand(-ANGLE_180, ANGLE_180); //mxd
			sub_explosion->velocity2[YAW] = flrand(-ANGLE_180, ANGLE_180);
			sub_explosion->velocity2[PITCH] = flrand(-ANGLE_180, ANGLE_180);

			VectorScale(balls_dir, flrand(240.0f, 290.0f) * pos_scaler, sub_explosion->velocity);

			sub_explosion->r.scale = flrand(1.8f, 2.2f) * size_scaler;
			sub_explosion->d_scale = flrand(-2.5f, -2.0f) * size_scaler;
			sub_explosion->alpha = flrand(0.5f, 0.75f);
			sub_explosion->d_alpha = flrand(-2.5f, -1.5f) * max(0.45f, pos_scaler);

			sub_explosion->lastThinkTime = fx_time;
			sub_explosion->Update = SmallBarrelExplosionUpdate;

			AddEffect(NULL, sub_explosion);
		}
	}

	// Create the main big explosion sphere.
	client_entity_t* explosion = ClientEntity_new(type, flags | CEF_ADDITIVE_PARTS, origin, NULL, 0); //mxd. Update each frame.

	explosion->radius = 128.0f;
	explosion->r.model = &explosion_models[0]; // Explosion model.
	explosion->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT | RF_FULLBRIGHT); //mxd. +RF_FULLBRIGHT flag.
	explosion->d_alpha = -3.0f; // H2: 3.0
	explosion->d_scale = 3.5f;
	explosion->r.angles[YAW] = flrand(-ANGLE_180, ANGLE_180); //mxd
	explosion->r.angles[PITCH] = flrand(-ANGLE_180, ANGLE_180); //mxd
	explosion->velocity2[YAW] = flrand(-ANGLE_180, ANGLE_180);
	explosion->velocity2[PITCH] = flrand(-ANGLE_180, ANGLE_180);
	explosion->dlight = CE_DLight_new(explosion_color, BARREL_EXPLODE_INITIAL_DLIGHT_RADIUS, 0.0f);

	explosion->lastThinkTime = fx_time;
	explosion->Update = BarrelExplosionUpdate;

	AddEffect(NULL, explosion);

	// Add some glowing blast particles.
	const vec3_t vel = VEC3_SET(0.0f, 0.0f, flrand(BARREL_EXPLODE_SPEED * 0.8f, BARREL_EXPLODE_SPEED)); //mxd. Randomize velocity.
	count = GetScaledCount(BARREL_EXPLODE_NUM_BITS, 0.3f); //mxd. Use GetScaledCount().

	for (int i = 0; i < count; i++)
	{
		client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), explosion_color, 1000);

		VectorRandomSet(spark->velocity, BARREL_EXPLODE_SPEED);
		Vec3AddAssign(vel, spark->velocity);
		spark->acceleration[2] = flrand(BARREL_EXPLODE_GRAVITY * 0.8f, BARREL_EXPLODE_GRAVITY * 1.2f); //mxd. Randomize acceleration.

		spark->scale = flrand(BARREL_EXPLODE_SCALE, BARREL_EXPLODE_SCALE * 1.2f); //mxd. Randomize scale.
		spark->d_scale = flrand(-30.0f, -20.0f);
		spark->d_alpha = flrand(-400.0f, -320.0f);

		AddParticleToList(explosion, spark);
	}

	//mxd. Add lingering smoke.
	count = GetScaledCount(BARREL_EXPLODE_NUM_SMOKE_BITS, 0.3f);
	angle = flrand(0.0f, ANGLE_360);
	angle_increment = ANGLE_360 / (float)count;

	for (int i = 0; i < count; i++)
	{
		const float part_angle = angle + flrand(-angle_increment * 0.3f, angle_increment * 0.3f);
		angle += angle_increment;

		client_particle_t* smoke = ClientParticle_new(PART_32x32_STEAM, color_white, BARREL_EXPLODE_SMOKE_LIFETIME);

		const float scaler = flrand(0.1f, 1.0f);
		const vec3_t offset = VEC3_SET(cosf(part_angle), sinf(part_angle), 0.0f);

		VectorRandomSet(smoke->origin, 16.0f);
		VectorMA(smoke->origin, BARREL_EXPLODE_RADIUS * (1.0f - scaler) * 0.6f, offset, smoke->origin);
		smoke->origin[2] += flrand(8.0f, 16.0f);

		VectorRandomSet(smoke->acceleration, 8.0f);
		smoke->acceleration[2] += flrand(8.0f, 16.0f);

		smoke->color.a = (byte)(flrand(32.0f, 48.0f) * (0.5f + scaler * 0.5f));
		smoke->scale = flrand(24.0f, 32.0f) * (0.5f + scaler * 0.5f);
		smoke->d_scale = smoke->scale * flrand(0.2f, 0.3f);
		smoke->d_alpha = flrand(-40.0f, -30.0f);

		AddParticleToList(explosion, smoke);
	}

	fxi.S_StartSound(origin, -1, CHAN_AUTO, explosion_sound, 1.0f, ATTN_NORM, 0.0f);
}
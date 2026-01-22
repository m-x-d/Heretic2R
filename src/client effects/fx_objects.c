//
// fx_objects.c -- //TODO: Rename to fx_BarrelExplode?
//
// Copyright 1998 Raven Software
//

#include "Client Effects.h"
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "ce_DLight.h"

#define BARREL_EXPLODE_SPEED	80.0f
#define BARREL_EXPLODE_BALLS	3
#define BARREL_EXPLODE_BITS		16
#define BARREL_EXPLODE_GRAVITY	(-320.0f)
#define BARREL_EXPLODE_SCALE	14.0f
#define BARREL_EXPLODE_TIME_MAX	750 //mxd

static struct model_s* explosion_models[3];
static struct sfx_s* explosion_sound; //mxd

void PreCacheBarrelExplode(void) //mxd. Named 'PreCacheObjects' in original logic.
{
	explosion_models[0] = fxi.RegisterModel("models/fx/explosion/outer/tris.fm");
	explosion_models[1] = fxi.RegisterModel("models/fx/explosion/inner/tris.fm"); //mxd
	explosion_models[2] = fxi.RegisterModel("sprites/fx/halo.sp2");
}

void PreCacheBarrelExplodeSFX(void) //mxd
{
	explosion_sound = fxi.S_RegisterSound("weapons/PhoenixHit.wav");
}

static qboolean SmallBarrelExplosionUpdate(client_entity_t* self, centity_t* owner) //mxd. Named 'FXPhoenixExplosionSmallBallThink' in original logic.
{
	if (fx_time - self->startTime > BARREL_EXPLODE_TIME_MAX)
		return false;

	const float vel_factor = (float)(fx_time - self->lastThinkTime) / 1000.0f;
	self->lastThinkTime = fx_time;

	// Spin the ball of fire while it expands and fades.
	self->r.angles[YAW] += self->velocity2[YAW] * vel_factor;
	self->r.angles[PITCH] += self->velocity2[PITCH] * vel_factor;

	return true;
}

static qboolean BarrelExplosionUpdate(client_entity_t* self, centity_t* owner) //mxd. Original logic used FXPhoenixExplosionBallThink().
{
	if (!SmallBarrelExplosionUpdate(self, owner))
		return false;

	if (self->dlight->intensity > 0.0f)
		self->dlight->intensity -= 5.0f;

	return true;
}

static client_entity_t* CreateSmallBarrelExplosion(const vec3_t ball_origin)
{
	client_entity_t* sub_explosion = ClientEntity_new(FX_WEAPON_PHOENIXEXPLODE, CEF_BROADCAST, ball_origin, NULL, 0); //mxd. Update each frame.

	sub_explosion->radius = 128.0f;
	sub_explosion->r.model = &explosion_models[1]; // Inner explosion model.
	sub_explosion->r.frame = irand(0, 1); //mxd
	sub_explosion->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT | RF_FULLBRIGHT); //mxd. +RF_FULLBRIGHT flag.
	sub_explosion->lastThinkTime = fx_time;
	sub_explosion->r.angles[YAW] = flrand(-ANGLE_180, ANGLE_180); //mxd
	sub_explosion->r.angles[PITCH] = flrand(-ANGLE_180, ANGLE_180); //mxd
	sub_explosion->velocity2[YAW] = flrand(-ANGLE_180, ANGLE_180);
	sub_explosion->velocity2[PITCH] = flrand(-ANGLE_180, ANGLE_180);
	sub_explosion->Update = SmallBarrelExplosionUpdate;

	return sub_explosion;
}

// Create Effect FX_BARREL_EXPLODE
void FXBarrelExplode(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	static const paletteRGBA_t explosion_color = { .c = 0xff00ffff };

	// Create three smaller explosion spheres.
	for (int i = 0; i < BARREL_EXPLODE_BALLS; i++)
	{
		const float ball_num = (float)i;
		client_entity_t* sub_explosion = CreateSmallBarrelExplosion(origin);

		VectorRandomSet(sub_explosion->velocity, BARREL_EXPLODE_SPEED);
		sub_explosion->r.scale = 0.1f;
		sub_explosion->d_scale = 3.0f + ball_num;
		sub_explosion->d_alpha = -1.5f - ball_num * 0.5f;

		AddEffect(NULL, sub_explosion);
	}

	// Create the main big explosion sphere.
	client_entity_t* explosion = ClientEntity_new(type, flags, origin, NULL, 0); //mxd. Update each frame.

	explosion->radius = 128.0f;
	explosion->r.model = &explosion_models[0]; // Explosion model.
	explosion->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
	explosion->flags |= (CEF_ADDITIVE_PARTS | CEF_PULSE_ALPHA);
	explosion->alpha = 0.1f;
	explosion->r.scale = 0.1f;
	explosion->d_alpha = 3.0f;
	explosion->d_scale = 5.0f;
	explosion->startTime = fx_time;
	explosion->lastThinkTime = fx_time;
	explosion->velocity2[YAW] = flrand(-ANGLE_180, ANGLE_180);
	explosion->velocity2[PITCH] = flrand(-ANGLE_180, ANGLE_180);

	explosion->dlight = CE_DLight_new(explosion_color, 150.0f, 0.0f);
	explosion->Update = BarrelExplosionUpdate;
	AddEffect(NULL, explosion);

	// Add some glowing blast particles.
	vec3_t dir;
	VectorScale(vec3_up, BARREL_EXPLODE_SPEED, dir);

	for (int i = 0; i < BARREL_EXPLODE_BITS; i++)
	{
		client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), explosion_color, 2000);

		VectorRandomSet(spark->velocity, BARREL_EXPLODE_SPEED);
		Vec3AddAssign(dir, spark->velocity);
		spark->acceleration[2] = BARREL_EXPLODE_GRAVITY;

		spark->scale = BARREL_EXPLODE_SCALE;
		spark->d_scale = flrand(-20.0f, -10.0f);
		spark->d_alpha = flrand(-400.0f, -320.0f);
		spark->duration = (int)(255.0f * 2000.0f / -spark->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(explosion, spark);
	}

	// ...and a big-ass flash.
	client_entity_t* halo = ClientEntity_new(-1, flags, origin, NULL, 250);

	halo->radius = 128.0f;
	halo->r.model = &explosion_models[2]; // Halo sprite.
	halo->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
	halo->r.frame = 1;
	halo->r.scale = 2.0f;
	halo->d_alpha = -4.0f;
	halo->d_scale = -4.0f;

	AddEffect(NULL, halo);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, explosion_sound, 1.0f, ATTN_NORM, 0.0f);
}
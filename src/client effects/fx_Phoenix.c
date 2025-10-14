//
// fx_Phoenix.c
//
// Copyright 1998 Raven Software
//

#include "fx_Phoenix.h" //mxd
#include "fx_PhoenixLocal.h" //mxd
#include "Particle.h"
#include "Random.h"
#include "Vector.h"
#include "Utilities.h"
#include "ce_Dlight.h"
#include "g_playstats.h"

#define EXPLODE_SPEED			160.0f
#define EXPLODE_GRAVITY			(-320.0f)
#define EXPLODE_SCALE			14.0f
#define EXPLODE_NUM_BITS		32
#define EXPLODE_BALL_SPEED		64.0f
#define EXPLODE_NUM_SMALLBALLS	3
#define EXPLODE_LIFETIME		50
#define EXPLODE_TIME_MAX		750

#define FIRETRAIL_PARTS			4
#define FIRETRAIL_RADIUS		6.0f
#define FIRETRAIL_SPEED			16.0f
#define FIRETRAIL_SCALE			12.0f
#define FIRETRAIL_ACCEL			32.0f

#define SMOKETRAIL_RADIUS		2.0f
#define SMOKETRAIL_SCALE		0.25f
#define SMOKETRAIL_ALPHA		0.5f

static struct model_s* phoenix_models[6];
static struct sfx_s* phoenix_explode_sounds[2]; //mxd

void PreCachePhoenix(void)
{
	phoenix_models[0] = fxi.RegisterModel("sprites/fx/steam_add.sp2");
	phoenix_models[1] = fxi.RegisterModel("models/spells/phoenixarrow/tris.fm");
	phoenix_models[2] = fxi.RegisterModel("sprites/fx/halo.sp2");
	phoenix_models[3] = fxi.RegisterModel("sprites/spells/phoenix.sp2");
	phoenix_models[4] = fxi.RegisterModel("models/fx/explosion/inner/tris.fm");
	phoenix_models[5] = fxi.RegisterModel("models/fx/explosion/outer/tris.fm");
}

void PreCachePhoenixExplodeSFX(void) //mxd
{
	phoenix_explode_sounds[0] = fxi.S_RegisterSound("weapons/PhoenixHit.wav");
	phoenix_explode_sounds[1] = fxi.S_RegisterSound("weapons/PhoenixPowerHit.wav");
}

#pragma region ========================== PHOENIX EXPLOSION ==========================

static qboolean PhoenixMissileThink(client_entity_t* missile, centity_t* owner)
{
	int duration;

	if (R_DETAIL == DETAIL_LOW)
		duration = 1400;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 1700;
	else
		duration = 2000; //TODO: even longer duration for DETAIL_UBERHIGH?

	// Here we want to shoot out flame to either side.
	vec3_t angles;
	VectorScale(missile->r.angles, RAD_TO_ANGLE, angles);

	vec3_t fwd;
	vec3_t right;
	AngleVectors(angles, fwd, right, NULL);
	VectorScale(fwd, -4.0f * FIRETRAIL_SPEED, fwd);
	VectorScale(right, FIRETRAIL_SPEED, right);

	// Throw smoke to each side, alternating.
	const float side = ((missile->LifeTime-- & 1) ? 1.0f : -1.0f); //mxd. 1.0 - right, -1.0 - left.

	vec3_t smoke_origin;
	VectorSet(smoke_origin,
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS / 2.0f, SMOKETRAIL_RADIUS / 2.0f));

	VectorAdd(smoke_origin, missile->origin, smoke_origin);

	client_entity_t* smoke = ClientEntity_new(-1, CEF_DONT_LINK, smoke_origin, NULL, duration);

	smoke->radius = 64.0f; //BUGFIX: 128.0 for the left side in original version. Why?..
	smoke->r.model = &phoenix_models[0]; // steam_add sprite.
	smoke->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	smoke->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
	smoke->alpha = SMOKETRAIL_ALPHA;
	smoke->r.scale = SMOKETRAIL_SCALE;
	VectorScale(right, 2.0f * side, smoke->velocity);
	smoke->d_scale = 2.0f; // Rate of change in scale.
	smoke->d_alpha = -1.0f;

	AddEffect(NULL, smoke);	// Add the smoke as independent world smoke.

	// Add fire to the tail. Attach it to the smoke because it doesn't get out of the fx radius so quickly.
	for (int i = 0; i < FIRETRAIL_PARTS; i++)
	{
		const paletteRGBA_t light_color = { .r = 0xff, .g = 0x7f, .b = 0x00, .a = 0xe5 };
		client_particle_t* flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), light_color, duration);

		VectorSet(flame->origin,
			flrand(-FIRETRAIL_RADIUS, FIRETRAIL_RADIUS),
			flrand(-FIRETRAIL_RADIUS, FIRETRAIL_RADIUS),
			flrand(-FIRETRAIL_RADIUS / 3.0f, FIRETRAIL_RADIUS / 3.0f));

		VectorAdd(missile->origin, flame->origin, flame->origin);
		flame->scale = FIRETRAIL_SCALE;

		VectorSet(flame->velocity,
			flrand(-FIRETRAIL_SPEED, FIRETRAIL_SPEED),
			flrand(-FIRETRAIL_SPEED, FIRETRAIL_SPEED),
			flrand(-1.0f, 1.0f));

		// Make the fire shoot out the back and to the side
		VectorAdd(flame->velocity, fwd, flame->velocity);

		// Alternate left and right side of phoenix.
		if (i & 1)
			VectorAdd(flame->velocity, right, flame->velocity);
		else
			VectorSubtract(flame->velocity, right, flame->velocity);

		flame->acceleration[2] = FIRETRAIL_ACCEL;
		flame->d_scale = flrand(-15.0f, -10.0f);
		flame->d_alpha = flrand(-200.0f, -160.0f);
		flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(smoke, flame);
	}

	// Update animation frame.

	// Check if the time is up.
	if (fxi.cl->time >= missile->lastThinkTime)
	{
		// Set up animations to go the other direction.
		if (missile->NoOfAnimFrames == 7)
		{
			// Set to go backwards to 3.
			missile->NoOfAnimFrames = 3;
			missile->r.frame = 7;
		}
		else
		{
			// Set to go forward to 7
			missile->NoOfAnimFrames = 7;
			missile->r.frame = 3;
		}

		missile->Scale = -1;
		missile->lastThinkTime = fxi.cl->time + (4 * 50);
	}
	else
	{
		missile->r.frame = missile->NoOfAnimFrames - (int)(missile->Scale * ((float)(missile->lastThinkTime - fxi.cl->time) / 50.0f)) - 1;
	}

	// Remember for even spread of particles.
	VectorCopy(missile->r.origin, missile->origin);

	return true;
}

static qboolean PhoenixExplosionSmallBallThink(client_entity_t* ball, centity_t* owner) //mxd. Moved above FXPhoenixExplosionBallThink.
{
	if (fxi.cl->time - ball->startTime > EXPLODE_TIME_MAX)
		return false;

	const float vel_factor = (float)(fxi.cl->time - ball->lastThinkTime) / 1000.0f;
	ball->lastThinkTime = fxi.cl->time;

	// Spin the ball of fire while it expands and fades.
	ball->r.angles[YAW] += ball->velocity2[YAW] * vel_factor;
	ball->r.angles[PITCH] += ball->velocity2[PITCH] * vel_factor;

	return true;
}

// This is also exported for use in FXBarrelExplode.
qboolean PhoenixExplosionBallThink(client_entity_t* ball, centity_t* owner)
{
	if (!PhoenixExplosionSmallBallThink(ball, owner))
		return false;

	if (ball->dlight->intensity > 0.0f)
		ball->dlight->intensity -= 5.0f;

	return true;
}

static qboolean PhoenixExplosionBirdThink(client_entity_t* bird, centity_t* owner)
{
	bird->LifeTime--;

	if (bird->LifeTime <= 0)
		return false;

	int duration;
	if (R_DETAIL == DETAIL_LOW)
		duration = 175;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 210;
	else
		duration = 250; //TODO: even longer duration for DETAIL_UBERHIGH?

	// Spawn another trail bird.
	vec3_t pos;
	VectorRandomSet(pos, 8.0f);
	VectorAdd(pos, bird->r.origin, pos);

	client_entity_t* new_bird = ClientEntity_new(-1, bird->r.flags, pos, NULL, duration);

	new_bird->radius = 128.0f;
	new_bird->r.model = &phoenix_models[3]; // Phoenix sprite.
	new_bird->r.frame = 1;
	new_bird->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
	new_bird->r.scale = bird->r.scale;
	new_bird->alpha = bird->alpha;
	new_bird->d_alpha = -4.0f * bird->alpha;
	new_bird->d_scale = 2.0f;
	VectorCopy(bird->velocity, new_bird->velocity);

	AddEffect(NULL, new_bird);

	return true;
}

// This is also exported for use in FXBarrelExplode.
client_entity_t* CreatePhoenixSmallExplosion(const vec3_t ball_origin) //TODO: rename to FXPhoenixCreateSmallExplosion?
{
	client_entity_t* sub_explosion = ClientEntity_new(FX_WEAPON_PHOENIXEXPLODE, CEF_BROADCAST, ball_origin, NULL, 17);

	sub_explosion->radius = 128.0f;
	sub_explosion->r.model = &phoenix_models[4]; // Inner explosion model.
	sub_explosion->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	sub_explosion->startTime = fxi.cl->time;
	sub_explosion->lastThinkTime = fxi.cl->time;
	sub_explosion->velocity2[YAW] = flrand(-ANGLE_180, ANGLE_180);
	sub_explosion->velocity2[PITCH] = flrand(-ANGLE_180, ANGLE_180);
	sub_explosion->Update = PhoenixExplosionSmallBallThink;

	return sub_explosion;
}

#pragma endregion

#pragma region ========================== POWERED PHOENIX EXPLOSION ==========================

static qboolean PhoenixExplosionBirdPowerThink(client_entity_t* bird, centity_t* owner)
{
	bird->LifeTime--;
	return (bird->LifeTime > 0);
}

static void PhoenixExplodePower(const int type, int flags, const vec3_t origin, const vec3_t dir)
{
#define PHOENIXPOWER_NUMTRAILS			11
#define PHOENIXPOWER_PARTS_PER_TRAIL	8
#define PHOENIXPOWER_RADIUS				72.0f
#define PHOENIXPOWER_TRAIL_SCALER		(18.0f / 8.0f) //mxd

	flags |= CEF_OWNERS_ORIGIN;

	// Create the main big explosion sphere.
	client_entity_t* explosion = ClientEntity_new(type, flags, origin, NULL, 17);

	explosion->radius = 128.0f;
	explosion->r.model = &phoenix_models[5]; // Outer explosion model.
	explosion->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	explosion->flags |= CEF_ADDITIVE_PARTS;
	explosion->r.scale = 0.1f;
	explosion->d_alpha = -2.0f / 1.5f;
	explosion->LifeTime = EXPLODE_LIFETIME;

	const paletteRGBA_t light_color = { .c = 0xff00ffff };
	explosion->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	explosion->Update = PhoenixExplosionBallThink;

	AddEffect(NULL, explosion);

	const int num_trails = GetScaledCount(PHOENIXPOWER_NUMTRAILS - 4, 0.8f) + 4; // 4 is the minimum.
	const int num_particles = GetScaledCount(PHOENIXPOWER_PARTS_PER_TRAIL - 4, 0.8f) + 4; // Ditto.

	for (int i = 0; i < num_trails; i++)
	{
		const float angle = (float)i / (float)num_trails * ANGLE_360; //mxd
		const float cos_val = cosf(angle);
		const float sin_val = sinf(angle);

		vec3_t phoenix_origin;
		VectorCopy(origin, phoenix_origin);
		phoenix_origin[0] += cos_val * PHOENIXPOWER_RADIUS;
		phoenix_origin[1] += sin_val * PHOENIXPOWER_RADIUS;

		vec3_t end_pos;
		VectorCopy(phoenix_origin, end_pos);
		end_pos[2] -= 64.0f;

		trace_t trace;
		fxi.Trace(phoenix_origin, vec3_origin, vec3_origin, end_pos, CONTENTS_SOLID, CEF_CLIP_TO_WORLD, &trace);

		const qboolean hit_ground = (trace.fraction < 1.0f); //mxd
		client_entity_t* halo = ClientEntity_new(-1, flags, (hit_ground ? trace.endpos : phoenix_origin), NULL, 1000);

		halo->radius = 128.0f;
		halo->r.model = &phoenix_models[2]; // Halo sprite.
		halo->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
		halo->r.frame = 1;
		halo->r.scale = 1.5f;
		halo->d_alpha = -1.0f;

		AddEffect(NULL, halo);

		for (int c = 0; c < num_particles; c++)
		{
			client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), color_white, 1000);

			spark->origin[0] = cos_val * 72.0f;
			spark->origin[1] = sin_val * 72.0f;

			const float particle_height = (float)(c * PHOENIXPOWER_PARTS_PER_TRAIL) / (float)num_particles;

			// When burst in the air, alternate up and down.
			const float side = (!hit_ground && !(c & 1) ? -1.0f : 1.0f); //mxd

			spark->velocity[2] = particle_height * side * 15.0f * PHOENIXPOWER_TRAIL_SCALER + flrand(0.0f, 15.0f);

			spark->scale = 32.0f - particle_height * PHOENIXPOWER_TRAIL_SCALER / 3.0f;
			spark->d_scale = -particle_height * PHOENIXPOWER_TRAIL_SCALER;
			spark->d_alpha = flrand(-400.0f, -320.0f) / 1.3f;
			spark->duration = (int)(255.0f * 2000.0f / -spark->d_alpha); // Time taken to reach zero alpha.

			AddParticleToList(explosion, spark);
		}
	}

	const float detail_scale = ((R_DETAIL == DETAIL_LOW) ? 1.5f : 2.0f);

	// ...and draw the phoenix rising from the explosion.
	client_entity_t* phoenix_outer = ClientEntity_new(type, flags, origin, NULL, 100);

	phoenix_outer->radius = 128.0f;
	phoenix_outer->r.model = &phoenix_models[3]; // Phoenix sprite.
	phoenix_outer->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	VectorScale(dir, 192.0f, phoenix_outer->velocity);
	phoenix_outer->acceleration[2] = 256.0f;
	phoenix_outer->d_alpha = -1.5f;
	phoenix_outer->d_scale = detail_scale;
	phoenix_outer->LifeTime = 6;
	phoenix_outer->Update = PhoenixExplosionBirdPowerThink;

	AddEffect(NULL, phoenix_outer);

	// Inner phoenix.
	client_entity_t* phoenix_inner = ClientEntity_new(type, flags, origin, NULL, 100);

	phoenix_inner->radius = 128.0f;
	phoenix_inner->r.model = &phoenix_models[3]; // Phoenix sprite.
	phoenix_inner->r.flags = RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT;
	VectorScale(dir, 192.0f, phoenix_inner->velocity);
	phoenix_inner->acceleration[2] = 256.0f;
	phoenix_inner->d_scale = -1.0f / 0.6f;
	phoenix_inner->LifeTime = 6;
	phoenix_inner->Update = PhoenixExplosionBirdPowerThink;

	AddEffect(NULL, phoenix_inner);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, phoenix_explode_sounds[1], 1.0f, ATTN_NORM, 0.0f);
}

static qboolean PhoenixMissilePowerThink(client_entity_t* missile, centity_t* owner)
{
	int duration;

	if (R_DETAIL == DETAIL_LOW)
		duration = 1400;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 1700;
	else
		duration = 2000; //TODO: DETAIL_UBERHIGH.

	// Here we want to shoot out flame to either side.
	vec3_t angles;
	VectorScale(missile->r.angles, RAD_TO_ANGLE, angles);

	vec3_t fwd;
	vec3_t right;
	AngleVectors(angles, fwd, right, NULL);
	VectorScale(fwd, -4.0f * FIRETRAIL_SPEED, fwd);
	VectorScale(right, FIRETRAIL_SPEED, right);

	// Throw smoke to each side, alternating.
	const float side = ((missile->LifeTime-- & 1) ? 1.0f : -1.0f); //mxd. 1.0 - right, -1.0 - left.

	vec3_t smoke_origin;
	VectorSet(smoke_origin,
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS, SMOKETRAIL_RADIUS),
		flrand(-SMOKETRAIL_RADIUS / 2.0f, SMOKETRAIL_RADIUS / 2.0f));

	VectorAdd(smoke_origin, missile->origin, smoke_origin);

	client_entity_t* smoke = ClientEntity_new(-1, CEF_DONT_LINK, smoke_origin, NULL, duration);

	smoke->radius = 64.0f;
	smoke->r.model = &phoenix_models[0]; // steam_add sprite.
	smoke->r.flags = RF_TRANSLUCENT | RF_TRANS_ADD | RF_TRANS_ADD_ALPHA;
	smoke->flags |= CEF_ADDITIVE_PARTS | CEF_ABSOLUTE_PARTS;
	smoke->alpha = SMOKETRAIL_ALPHA;
	smoke->r.scale = SMOKETRAIL_SCALE * 2.5f;
	VectorScale(right, 2.0f * side, smoke->velocity);
	smoke->d_scale = 2.0f; // Rate of change in scale.
	smoke->d_alpha = -1.0f;

	AddEffect(NULL, smoke);	// Add the smoke as independent world smoke.

	// Add fire to the tail. Attach it to the smoke because it doesn't get out of the fx radius so quickly.
	const float trail_offset = FIRETRAIL_RADIUS / 3.0f; //mxd
	const float trail_speed = FIRETRAIL_SPEED / 3.0f; //mxd

	for (int i = 0; i < FIRETRAIL_PARTS; i++)
	{
		const paletteRGBA_t light_color = { .r = 0xff, .g = 0x7f, .b = 0x00, .a = 0xe5 };
		client_particle_t* flame = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), light_color, duration);

		VectorRandomSet(flame->origin, trail_offset);
		VectorAdd(missile->origin, flame->origin, flame->origin);
		flame->scale = FIRETRAIL_SCALE;

		VectorSet(flame->velocity, flrand(-trail_speed, trail_speed), flrand(-trail_speed, trail_speed), flrand(-1.0f, 1.0f));

		// Make the fire shoot out the back and to the side.
		VectorAdd(flame->velocity, fwd, flame->velocity);

		// Alternate left and right side of phoenix
		if (i & 1)
			VectorAdd(flame->velocity, right, flame->velocity);
		else
			VectorSubtract(flame->velocity, right, flame->velocity);

		flame->acceleration[2] = FIRETRAIL_ACCEL;
		flame->d_scale = flrand(-15.0f, -10.0f);
		flame->d_alpha = flrand(-200.0f, -160.0f);
		flame->duration = (int)(255.0f * 1000.0f / -flame->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(smoke, flame);
	}

	// Remember for even spread of particles.
	VectorCopy(missile->r.origin, missile->origin);

	return true;
}

#pragma endregion

void FXPhoenixMissile(centity_t* owner, const int type, const int flags, vec3_t origin)
{
	client_entity_t* missile = ClientEntity_new(type, flags | CEF_DONT_LINK, origin, NULL, 25);
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_PHOENIXMISSILE].formatString, missile->velocity);

	const float velocity = PHOENIX_ARROW_SPEED * ((flags & CEF_FLAG8) ? 0.5f : 1.0f);
	Vec3ScaleAssign(velocity, missile->velocity);

	vec3_t dir;
	VectorCopy(missile->velocity, dir);
	VectorNormalize(dir);
	AnglesFromDir(dir, missile->r.angles);

	missile->radius = 256.0f;
	missile->r.model = &phoenix_models[1]; // Phoenix arrow model.
	missile->flags |= CEF_ADDITIVE_PARTS;
	missile->lastThinkTime = fxi.cl->time + (50 * 7); // Time to play last frame.
	missile->NoOfAnimFrames = 7; // End on frame number 7.
	missile->Scale = 1.0f; // Positive frame count.
	missile->r.scale = 0.8f;
	missile->color.c = 0xff00ffff;
	missile->LifeTime = 1000;

	if (R_DETAIL > DETAIL_LOW)
		missile->dlight = CE_DLight_new(missile->color, 150.0f, 0.0f);

	if (flags & CEF_FLAG6)
		missile->Update = PhoenixMissilePowerThink;
	else
		missile->Update = PhoenixMissileThink;

	AddEffect(owner, missile);
}

void FXPhoenixExplode(centity_t* owner, const int type, int flags, vec3_t origin)
{
	vec3_t dir;
	vec3_t scorch_dir;
	fxi.GetEffect(owner, flags, clientEffectSpawners[FX_WEAPON_PHOENIXEXPLODE].formatString, dir, scorch_dir);

	// Make the scorch mark if we should.
	if (flags & CEF_FLAG8)
		FXClientScorchmark(origin, scorch_dir);

	if (flags & CEF_FLAG6)
	{
		// Powered-up version.
		PhoenixExplodePower(type, flags, origin, dir);
		return;
	}

	flags |= CEF_OWNERS_ORIGIN;

	if (R_DETAIL > DETAIL_LOW)
	{
		const int count = ((R_DETAIL > DETAIL_NORMAL) ? EXPLODE_NUM_SMALLBALLS : EXPLODE_NUM_SMALLBALLS - 1);

		// Create three smaller explosion spheres.
		for (int i = 0; i < count; i++)
		{
			const float ball_num = (float)i;
			client_entity_t* sub_explosion = CreatePhoenixSmallExplosion(origin);

			for (int c = 0; c < 3; c++)
				sub_explosion->velocity[c] = flrand(-EXPLODE_BALL_SPEED, EXPLODE_BALL_SPEED) + (dir[c] * EXPLODE_BALL_SPEED);

			sub_explosion->r.scale = 0.1f;
			sub_explosion->d_scale = 3.0f + ball_num;
			sub_explosion->d_alpha = -1.5f - ball_num * 0.5f;

			AddEffect(NULL, sub_explosion);
		}
	}

	// Create the main big explosion sphere.
	client_entity_t* explosion = ClientEntity_new(type, flags, origin, NULL, 17);

	explosion->radius = 128.0f;
	explosion->r.model = &phoenix_models[5]; // Outer explosion model.
	explosion->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
	explosion->flags |= (CEF_ADDITIVE_PARTS | CEF_PULSE_ALPHA);
	explosion->alpha = 0.1f;
	explosion->r.scale = 0.1f;
	explosion->d_alpha = 3.0f;
	explosion->d_scale = 5.0f;
	explosion->startTime = fxi.cl->time;
	explosion->lastThinkTime = fxi.cl->time;
	explosion->velocity2[YAW] = flrand(-ANGLE_180, ANGLE_180);
	explosion->velocity2[PITCH] = flrand(-ANGLE_180, ANGLE_180);

	const paletteRGBA_t light_color = { .c = 0xff00ffff };
	explosion->dlight = CE_DLight_new(light_color, 150.0f, 0.0f);
	explosion->Update = PhoenixExplosionBallThink;

	AddEffect(NULL, explosion);

	// Add some glowing blast particles.
	vec3_t vel;
	VectorScale(dir, EXPLODE_SPEED, vel);
	const int count = GetScaledCount(EXPLODE_NUM_BITS, 0.3f);

	for (int i = 0; i < count; i++)
	{
		client_particle_t* spark = ClientParticle_new(irand(PART_32x32_FIRE0, PART_32x32_FIRE2), light_color, 2000);

		VectorRandomSet(spark->velocity, EXPLODE_SPEED);
		Vec3AddAssign(vel, spark->velocity);
		spark->acceleration[2] = EXPLODE_GRAVITY;
		spark->scale = EXPLODE_SCALE;
		spark->d_scale = flrand(-20.0f, -10.0f);
		spark->d_alpha = flrand(-400.0f, -320.0f);
		spark->duration = (int)(255.0f * 2000.0f / -spark->d_alpha); // Time taken to reach zero alpha.

		AddParticleToList(explosion, spark);
	}

	// ...and a big-ass flash.
	client_entity_t* flash = ClientEntity_new(-1, flags, origin, NULL, 250);

	flash->radius = 128.0f;
	flash->r.model = &phoenix_models[2]; // Halo sprite.
	flash->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
	flash->r.frame = 1;
	flash->r.scale = 1.5f;
	flash->d_alpha = -4.0f;
	flash->d_scale = -4.0f;

	AddEffect(NULL, flash);

	int duration;
	if (R_DETAIL == DETAIL_LOW)
		duration = 150;
	else if (R_DETAIL == DETAIL_NORMAL)
		duration = 125;
	else
		duration = 100; //TODO: even shorter update interval for DETAIL_UBERHIGH?

	// ...and draw the phoenix rising from the explosion.
	client_entity_t* phoenix = ClientEntity_new(type, flags, origin, NULL, duration);

	phoenix->radius = 128.0f;
	phoenix->r.model = &phoenix_models[3]; // Phoenix sprite.
	phoenix->r.flags = (RF_TRANS_ADD | RF_TRANS_ADD_ALPHA | RF_TRANSLUCENT);
	phoenix->r.scale = 0.1f;
	VectorScale(vel, 0.25f, phoenix->velocity);
	phoenix->acceleration[2] = 64.0f;
	phoenix->d_alpha = -1.0f;
	phoenix->d_scale = 1.25f;
	phoenix->LifeTime = 10;
	phoenix->Update = PhoenixExplosionBirdThink;

	AddEffect(NULL, phoenix);

	fxi.S_StartSound(origin, -1, CHAN_AUTO, phoenix_explode_sounds[0], 1.0f, ATTN_NORM, 0.0f);
}